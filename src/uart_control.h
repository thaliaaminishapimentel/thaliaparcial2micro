
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de senal analogica
 * Placa: FRDM-MCXA156
 *
 * Configuracion de comunicacion UART.
 */

#ifndef UART_CONTROL_H_
#define UART_CONTROL_H_

#include <stdint.h>

#include "board.h"
#include "fsl_common.h"
#include "fsl_lpuart.h"

/* Periferico UART utilizado */
#define UART_CONTROL_BASE LPUART0

/* Configuracion de comunicacion */
#define UART_CONTROL_BAUDRATE 115200U

/* Inicializar el modulo UART */
static inline void UART_Control_Iniciar(void)
{
    lpuart_config_t config;

    /* Obtener configuracion predeterminada */
    LPUART_GetDefaultConfig(&config);

    /* Velocidad de comunicacion */
    config.baudRate_Bps = UART_CONTROL_BAUDRATE;

    /* Comunicacion sin paridad */
    config.parityMode = kLPUART_ParityDisabled;

    /* Un bit de parada */
    config.stopBitCount = kLPUART_OneStopBit;

    /* Habilitar transmision y recepcion */
    config.enableTx = true;
    config.enableRx = true;

    /* Inicializar LPUART0 */
    LPUART_Init(
        UART_CONTROL_BASE,
        &config,
        BOARD_DEBUG_UART_CLK_FREQ
    );
}

/*
 * Habilitar interrupcion de recepcion.
 *
 * Esta funcion se utilizara cuando agreguemos
 * el controlador de interrupciones UART.
 */
static inline void UART_Control_HabilitarRX(void)
{
    LPUART_EnableInterrupts(
        UART_CONTROL_BASE,
        kLPUART_RxDataRegFullInterruptEnable
    );

    EnableIRQ(LPUART0_IRQn);
}

#endif /* UART_CONTROL_H_ */
