idf_component_register(
    SRCS "app_main.cpp" "matter_meter.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_matter nvs_flash driver ir_reader
)
