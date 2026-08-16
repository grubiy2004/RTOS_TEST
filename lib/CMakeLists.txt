cmake_minimum_required(VERSION 3.22)

project(lib)
enable_language(C CXX ASM)

# Set include paths
SET(INCLUDES_LIB_CMSIS
    CMSIS/Device/ST/STM32F1xx/Include
    CMSIS/Include
)

SET(INCLUDES_LIB_cmsis_config
    cmsis-config
)

SET(INCLUDES_LIB_Event_Recorder
    EventRecorder/include
)

SET(INCLUDES_LIB_FreeRTOS
    FreeRTOS/include
    FreeRTOS/portable
)

SET(SOURCES_LIB_Event_Recorder
    EventRecorder/source/EventRecorder.c
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

add_library(lib INTERFACE)

target_include_directories(lib INTERFACE
    ${INCLUDES_LIB_CMSIS}
    ${INCLUDES_LIB_cmsis_config}
    ${INCLUDES_LIB_Event_Recorder}
    ${INCLUDES_LIB_FreeRTOS}
)

target_sources(lib INTERFACE
    ${SOURCES_LIB_Event_Recorder}
    ${SOURCES_LIB_FreeRTOS}
)
