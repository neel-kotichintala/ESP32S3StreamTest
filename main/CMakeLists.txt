idf_component_register(SRCS "qrcode_demo_main.c"
                     INCLUDE_DIRS "."
                     PRIV_REQUIRES esp32-camera quirc classifier esp_wifi esp_event esp_netif nvs_flash esp_timer vfs fatfs esp_http_client lwip json)

target_compile_options(${COMPONENT_LIB} PRIVATE -O3)

idf_component_get_property(quirc_lib_name espressif__quirc COMPONENT_LIB)
target_compile_options(${quirc_lib_name} PRIVATE -O3)
