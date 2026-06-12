idf_component_register(
    SRCS "router.cpp"
    INCLUDE_DIRS "include"
    REQUIRES as_sensing as_pipeline as_config as_hid_usb esp_timer
)
