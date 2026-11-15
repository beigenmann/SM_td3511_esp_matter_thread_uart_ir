idf_component_register(
    SRCS "ir_reader.cpp" "obis_parser.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_timer esp_driver_uart 
)
