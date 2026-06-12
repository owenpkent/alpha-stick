idf_component_register(
    SRCS "sensing.cpp" "tmag5273.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_timer as_pipeline as_config
)
