idf_component_register(
    SRCS "config.cpp" "config_json.cpp" "config_protocol.cpp"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash as_pipeline json
)
