file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/razor/src/*.c ${PROJECT_SOURCE_DIR}/razor/src/*.cpp)

if (SOURCES STREQUAL "")
    message(FATAL_ERROR "No source files found for ${RAZOR_NAME}")
endif()

add_library(${RAZOR_NAME} SHARED ${SOURCES})

target_include_directories(${RAZOR_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/razor/include)
add_compile_definitions(${RAZOR_NAME} RE_BUILD_DLL=1 RE_ASSERT_ENABLED=1 RE_LOGGER_ENABLED=1)

#############################
# VULKAN                    #
#############################

find_package(Vulkan QUIET)

if (NOT Vulkan_FOUND)
    if (DEFINED ENV{VULKAN_SDK})
        set(Vulkan_INCLUDE_DIR $ENV{VULKAN_SDK}/Include)
        set(Vulkan_LIBRARIES $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)
        set(Vulkan_FOUND TRUE)
    elseif (EXISTS "/usr/include/vulkan/vulkan.h")
        set(Vulkan_INCLUDE_DIR "/usr/include")
        set(Vulkan_LIBRARIES vulkan)
        set(Vulkan_FOUND TRUE)
    endif()
endif()

if (Vulkan_FOUND)
    target_link_libraries(${RAZOR_NAME} PRIVATE ${Vulkan_LIBRARIES})
    target_include_directories(${RAZOR_NAME} PRIVATE ${Vulkan_INCLUDE_DIR})
else() # Currently, Vulkan is required
    message(FATAL_ERROR "Could not find Vulkan package for ${RAZOR_NAME}")
endif()