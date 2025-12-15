file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/example/src/*.c ${PROJECT_SOURCE_DIR}/example/src/*.cpp)

if (SOURCES STREQUAL "")
    message(FATAL_ERROR "No source files found for ${EXAMPLE_NAME}!")
endif()

add_executable(${EXAMPLE_NAME} ${SOURCES})

target_include_directories(${EXAMPLE_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/razor/include
)

target_link_libraries(${EXAMPLE_NAME}
    PRIVATE ${RAZOR_NAME}
)