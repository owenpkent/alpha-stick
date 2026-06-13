#include "as_hid_usb/usb_hid.h"

#include <string.h>

#include "esp_log.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"

static const char *TAG = "usb_hid";

enum {
    REPORT_ID_GAMEPAD = 1,
    REPORT_ID_MOUSE = 2,
    REPORT_ID_KEYBOARD = 3,
};

// Custom gamepad: X/Y as int16 (docs/DESIGN_V2.md R8 wants 16-bit reports),
// Z press as uint8, 16 buttons. Mouse and keyboard use stock TinyUSB shapes.
#define AS_REPORT_DESC_GAMEPAD(...)                          \
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                  \
    HID_USAGE(HID_USAGE_DESKTOP_GAMEPAD),                    \
    HID_COLLECTION(HID_COLLECTION_APPLICATION),              \
        __VA_ARGS__                                          \
        HID_USAGE(HID_USAGE_DESKTOP_X),                      \
        HID_USAGE(HID_USAGE_DESKTOP_Y),                      \
        HID_LOGICAL_MIN_N(-32767, 2),                        \
        HID_LOGICAL_MAX_N(32767, 2),                         \
        HID_REPORT_SIZE(16),                                 \
        HID_REPORT_COUNT(2),                                 \
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),   \
        HID_USAGE(HID_USAGE_DESKTOP_Z),                      \
        HID_LOGICAL_MIN(0),                                  \
        HID_LOGICAL_MAX_N(255, 2),                           \
        HID_REPORT_SIZE(8),                                  \
        HID_REPORT_COUNT(1),                                 \
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),   \
        HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),               \
        HID_USAGE_MIN(1),                                    \
        HID_USAGE_MAX(16),                                   \
        HID_LOGICAL_MIN(0),                                  \
        HID_LOGICAL_MAX(1),                                  \
        HID_REPORT_SIZE(1),                                  \
        HID_REPORT_COUNT(16),                                \
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),   \
    HID_COLLECTION_END

static const uint8_t s_hid_report_descriptor[] = {
    AS_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_ID_GAMEPAD)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE)),
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
};

enum {
    ITF_NUM_HID = 0,
    ITF_NUM_CDC,
    ITF_NUM_CDC_DATA,
    ITF_NUM_TOTAL,
};

#define EPNUM_HID        0x81
#define EPNUM_CDC_NOTIF  0x82
#define EPNUM_CDC_OUT    0x03
#define EPNUM_CDC_IN     0x83

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_CDC_DESC_LEN)

static const uint8_t s_configuration_descriptor[] = {
    // 500 mA: covers 80-120 mA typ plus WiFi config-mode peaks (HARDWARE.md).
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0, 500),
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 4, HID_ITF_PROTOCOL_NONE,
                       sizeof(s_hid_report_descriptor), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 1),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 5, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT,
                       EPNUM_CDC_IN, 64),
};

// VID 0x303A is Espressif's; PID is a placeholder. Request a pid.codes PID
// before any public release.
static const tusb_desc_device_t s_device_descriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,           // IAD composite (CDC present)
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A,
    .idProduct = 0x4001,
    .bcdDevice = 0x0200,  // V2
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

static const char *s_string_descriptor[] = {
    (const char[]){0x09, 0x04},  // 0: language, English
    "Alpha Stick Project",       // 1: manufacturer
    "Alpha Stick",               // 2: product
    "AS2-0001",                  // 3: serial (placeholder)
    "Alpha Stick HID",           // 4
    "Alpha Stick Config",        // 5
};

// CDC config channel placeholder: logs received bytes until the JSON protocol
// (docs/FIRMWARE.md, WebSerial) lands.
static void cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    (void)event;
    uint8_t buf[64];
    size_t n = 0;
    // Drain the FIFO fully: a JSON config line can exceed one buf, and the
    // callback may not fire again for bytes already buffered.
    while (tinyusb_cdcacm_read(itf, buf, sizeof(buf), &n) == ESP_OK && n > 0) {
        ESP_LOGI(TAG, "cdc rx %u bytes (config protocol TODO)", (unsigned)n);
    }
}

void usb_hid_init(void)
{
    // VERIFY (Phase 0): field names match esp_tinyusb ^1 at scaffold time;
    // adjust if the component API drifted.
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = &s_device_descriptor,
        .string_descriptor = s_string_descriptor,
        .string_descriptor_count =
            sizeof(s_string_descriptor) / sizeof(s_string_descriptor[0]),
        .external_phy = false,
        .configuration_descriptor = s_configuration_descriptor,
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    const tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 256,
        .callback_rx = &cdc_rx_callback,
    };
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));

    ESP_LOGI(TAG, "composite HID (gamepad/mouse/keyboard) + CDC installed");
}

bool usb_hid_ready(void)
{
    return tud_mounted() && tud_hid_n_ready(0);
}

// Threading note (VERIFY in Phase 0): these submit reports from the router
// task, which runs concurrently with esp_tinyusb's tud_task. TinyUSB device
// calls are not guaranteed reentrant against tud_task. On the S3 DCD the
// interrupt-disabled critical sections make this work in practice, but confirm
// under sustained 1 kHz load (DUAL mode especially); if endpoints wedge,
// funnel submissions through a queue drained in the USB task context.
bool usb_hid_send_gamepad(int16_t x, int16_t y, uint8_t z, uint16_t buttons)
{
    struct __attribute__((packed)) {
        int16_t x;
        int16_t y;
        uint8_t z;
        uint16_t buttons;
    } report = {x, y, z, buttons};
    return tud_hid_n_report(0, REPORT_ID_GAMEPAD, &report, sizeof(report));
}

bool usb_hid_send_mouse(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel)
{
    return tud_hid_n_mouse_report(0, REPORT_ID_MOUSE, buttons, dx, dy, wheel, 0);
}

// TinyUSB HID callbacks

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return s_hid_report_descriptor;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen)
{
    (void)instance;
    // Some KVM/BLE bridges probe GET_REPORT. Answer input requests with a
    // zeroed report of the right length instead of stalling. The report ID is
    // carried in the setup packet, so the payload here omits it.
    if (report_type != HID_REPORT_TYPE_INPUT) {
        return 0;
    }
    uint16_t len = 0;
    switch (report_id) {
        case REPORT_ID_GAMEPAD:  len = 7; break;  // int16 x,y + u8 z + u16 buttons
        case REPORT_ID_MOUSE:    len = sizeof(hid_mouse_report_t); break;
        case REPORT_ID_KEYBOARD: len = sizeof(hid_keyboard_report_t); break;
        default: return 0;
    }
    if (len > reqlen) {
        len = reqlen;
    }
    memset(buffer, 0, len);
    return len;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize)
{
    // Keyboard LED output reports land here; nothing to do yet.
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}
