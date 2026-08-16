cmake_minimum_required(VERSION 3.22)

# Set CMAKE_PROJECT_NAME via project()
project(rtos_test)
enable_language(C CXX ASM)

# Set the following variable to `ON` to print all cmake variables
set(CMAKE_DUMP_VARIABLES            OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)

set(CMAKE_C_STANDARD                11)
set(CMAKE_C_STANDARD_REQUIRED       ON)
set(CMAKE_C_EXTENSIONS              ON)
set(CMAKE_CXX_STANDARD              14)
set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_CXX_EXTENSIONS            ON)

set(DOXYGEN_EXECUTABLE  C:/Users/Kolya/Desktop/progs_engineer/doxygen-1.18.0.windows.x64.bin/doxygen.exe)
set(TARGET_BIN_OUTPUT ${CMAKE_PROJECT_NAME}.bin)
set(TARGET_HEX_OUTPUT ${CMAKE_PROJECT_NAME}.hex)

# Define the default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
message(NOTICE "Build type: ${CMAKE_BUILD_TYPE}")

# Get the number of processors available
include(ProcessorCount)
ProcessorCount(NPROC)

# Include the microcontroller-specific parameters
include(cmake/microcontrollers/common.cmake)
include(cmake/microcontrollers/stm32f103-gcc.cmake)

# Add library files
add_subdirectory(lib)

# Set defines
set(DEFINES
    STM32F10X_MD
    $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
)

# Set include paths
set(INCLUDES
    ${PROJECT_SOURCE_DIR}/mcal/st-stm32f103/include
    ${PROJECT_SOURCE_DIR}/include
)

# Set source files
set(SOURCES_ASM
    ${PROJECT_SOURCE_DIR}/mcal/st-stm32f103/gcc-arm/startup_stm32f10x_md.s
)
set(SOURCES_C
    ${PROJECT_SOURCE_DIR}/mcal/st-stm32f103/source/usart_tx.c
    ${PROJECT_SOURCE_DIR}/source/at24c02.c
    ${PROJECT_SOURCE_DIR}/source/bmp180.c
    ${PROJECT_SOURCE_DIR}/source/rtos.c
    ${PROJECT_SOURCE_DIR}/source/ssd1306.c
    ${PROJECT_SOURCE_DIR}/source/system_stm32f10x.c
)

# Specify target executable
add_executable(${CMAKE_PROJECT_NAME})

# Specify target compile definitions
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    ${DEFINES}
)

# Specify target include directories
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${INCLUDES}
)

# Specify target source files
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${SOURCES_ASM}
    ${SOURCES_C}
)

# Specify libraries to link
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    lib
)

# Generate size information
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
)

# Generate binary output
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMAND ${CMAKE_COMMAND} -E echo "Create bin output: ${TARGET_BIN_OUTPUT}"
)

# Generate hex output
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_COMMAND} -E echo "Create hex output: ${TARGET_HEX_OUTPUT}"
)

# Add custom target to generate doxygen documentation
add_custom_target(doxygen
    COMMAND ${DOXYGEN_EXECUTABLE} docs/doxygen/Doxyfile
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generate doxygen documentation..."
)

# Specify additional clean files
set_property(TARGET ${CMAKE_PROJECT_NAME} APPEND PROPERTY ADDITIONAL_CLEAN_FILES
    ${CMAKE_PROJECT_NAME}.map
    ${CMAKE_PROJECT_NAME}.bin
    ${CMAKE_PROJECT_NAME}.hex
)

# Dump all cmake variables
if(CMAKE_DUMP_VARIABLES)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endif()
