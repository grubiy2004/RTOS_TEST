cmake_minimum_required(VERSION 3.22)

project(lib)
enable_language(C CXX ASM)

# Set include paths
SET(INCLUDES_LIB_CMSIS
    CMSIS/Device/ST/STM32F1xx/Include
    CMSIS/Include
)

SET(INCLUDES_LIB_CMSIS_DSP
    CMSIS/CMSIS-DSP/Include
    CMSIS/CMSIS-DSP/PrivateInclude
)

SET(INCLUDES_LIB_FreeRTOS
    FreeRTOS/include
    FreeRTOS/portable
)

SET(SOURCES_LIB_FreeRTOS
    FreeRTOS/source/croutine.c
    FreeRTOS/source/event_groups.c
    FreeRTOS/source/heap_4.c
    FreeRTOS/source/list.c
    FreeRTOS/source/queue.c
    FreeRTOS/source/stream_buffer.c
    FreeRTOS/source/tasks.c
    FreeRTOS/source/timers.c
    FreeRTOS/portable/port.c
)

set(SOURCES_LIB_CMSIS_DSP
    # TransformFunctions
    CMSIS/CMSIS-DSP/Source/TransformFunctions/arm_rfft_q15.c
    CMSIS/CMSIS-DSP/Source/TransformFunctions/arm_rfft_init_q15.c
    CMSIS/CMSIS-DSP/Source/TransformFunctions/arm_cfft_q15.c
    CMSIS/CMSIS-DSP/Source/TransformFunctions/arm_cfft_init_q15.c
    CMSIS/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix4_q15.c
    CMSIS/CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix4_init_q15.c
    CMSIS/CMSIS-DSP/Source/TransformFunctions/arm_bitreversal.c
    CMSIS/CMSIS-DSP/Source/TransformFunctions/arm_bitreversal2.c

    # Complex Math Functions
    CMSIS/CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mag_squared_q15.c

    # Statistics Functions
    CMSIS/CMSIS-DSP/Source/StatisticsFunctions/arm_max_q15.c

    # CommonTables
    CMSIS/CMSIS-DSP/Source/CommonTables/arm_common_tables.c
    CMSIS/CMSIS-DSP/Source/CommonTables/arm_const_structs.c

    # BasicMathFunctions
    CMSIS/CMSIS-DSP/Source/BasicMathFunctions/arm_shift_q15.c
)

add_library(lib INTERFACE)

target_include_directories(lib INTERFACE
    ${INCLUDES_LIB_CMSIS}
    ${INCLUDES_LIB_CMSIS_DSP}
    ${INCLUDES_LIB_FreeRTOS}
)

target_sources(lib INTERFACE
    ${SOURCES_LIB_FreeRTOS}
    ${SOURCES_LIB_CMSIS_DSP}
)
