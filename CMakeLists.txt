
# Proyecto de Microcontroladores
# Monitor de señal analógica con alarma configurable
# Placa: NXP FRDM-MCXA156

cmake_minimum_required(VERSION 3.22.0)

# Configuración de la placa
set(board frdmmcxa156)
set(board_root examples/_boards)

# Sistema de compilación MCUXpresso SDK
include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

# Definición del proyecto
project(analog_alarm LANGUAGES C CXX ASM
    PROJECT_BOARD_PORT_PATH
    ${board_root}/${board}/driver_examples/lpadc/polling
)

include(${SdkRootDirPath}/CMakeLists.txt)

# Archivos que se incorporarán en las siguientes etapas
mcux_add_source(
    SOURCES
        src/lpadc_polling.c
        src/hardware_init.c
        src/pin_mux.c
        src/app.h
        src/pin_mux.h
        src/alarm_core.h
)

mcux_add_include(INCLUDES src)

# Generación del archivo binario
mcux_convert_binary(
    BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin
)
