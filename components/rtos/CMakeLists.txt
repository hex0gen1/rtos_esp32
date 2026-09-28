

idf_component_register(
    SRCS 
        "src/rtos.c"
        "port/port_xtensa.S"
    INCLUDE_DIRS 
        "include"
    REQUIRES 
        esp_timer log esp_system
)
