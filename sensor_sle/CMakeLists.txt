#===============================================================================
# @brief    cmake file
# Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
#===============================================================================
if(DEFINED CONFIG_SAMPLE_SUPPORT_SENSOR_SLE_SERVER)
set(SOURCES_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/sensor_sle_server/sle_uart_server_adv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sensor_sle_server/sle_uart_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sensor_sle.c
    ${CMAKE_CURRENT_SOURCE_DIR}/dht11/dht11.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sensor_sta/sensor_sta.c
)
set(HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/sensor_sle_server)

elseif(DEFINED CONFIG_SAMPLE_SUPPORT_SENSOR_SLE_CLIENT)
set(SOURCES_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/sensor_sle_client/sle_uart_client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/max30102/max30102.c
    ${CMAKE_CURRENT_SOURCE_DIR}/adxl345/adxl345.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sensor_sle.c
)
set(HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/sensor_sle_client)
endif()

set(SOURCES "${SOURCES}" ${SOURCES_LIST} PARENT_SCOPE)
set(PUBLIC_HEADER "${PUBLIC_HEADER}" ${HEADER_LIST} PARENT_SCOPE)

add_definitions(-DCONFIG_SLE_UART_BUS=0)