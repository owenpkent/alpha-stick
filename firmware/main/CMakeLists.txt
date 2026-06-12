idf_component_register(
    SRCS "main.cpp"
    REQUIRES as_sensing as_pipeline as_modes as_hid_usb as_hid_ble as_aslink as_config as_web nvs_flash
)
