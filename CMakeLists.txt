# Минимальная версия CMake
cmake_minimum_required(VERSION 3.16)

# Включаем ESP-IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Название проекта
project(esp32_rtos_demo)