cmake_minimum_required(VERSION 3.19)

project(rtweekend-wasm C)

add_executable(${PROJECT_NAME} wasm/src/lib.c)
target_include_directories(${PROJECT_NAME} PUBLIC wasm/src)
set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)

target_compile_options(${PROJECT_NAME}
        PRIVATE -Wall
        PRIVATE --target=wasm32
        PRIVATE -Oz
        PRIVATE -nostdlib
        PRIVATE -fvisibility=hidden
        PRIVATE -std=c11
        PRIVATE -ffunction-sections
        PRIVATE -fdata-sections
        PRIVATE -fno-exceptions
        PRIVATE -fno-rtti
        PRIVATE -ffast-math
        PRIVATE -flto
        )

target_link_options(${PROJECT_NAME}
        PRIVATE --export=__heap_base
        PRIVATE --no-entry
        PRIVATE --strip-all
        PRIVATE --export-dynamic
        PRIVATE --allow-undefined
#        PRIVATE --initial-memory=${TOTAL_MEMORY}
#        PRIVATE -z stack-size=${STACK_SIZE}
        PRIVATE --error-limit=0
        PRIVATE --lto-O3
        PRIVATE -O3
        PRIVATE --gc-sections
        )

