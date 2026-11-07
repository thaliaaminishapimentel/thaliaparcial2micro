
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de señal analogica
 * Placa: FRDM-MCXA156
 *
 * Inicializacion del hardware principal.
 */

#include "pin_mux.h"
#include "board.h"
#include "clock_config.h"
#include "fsl_clock.h"
#include "fsl_reset.h"

/* Inicializacion de los perifericos */

void BOARD_InitHardware(void)
{
    /* Liberar el modulo ADC1 del reset */
    RESET_ReleasePeripheralReset(kADC1_RST_SHIFT_RSTn);

    /* Configurar reloj del ADC1 a 12 MHz */
    CLOCK_AttachClk(kFRO12M_to_ADC1);

    /* Establecer divisor de reloj */
    CLOCK_SetClockDiv(kCLOCK_DivADC1, 1U);

    /* Inicializar los pines */
    BOARD_InitPins();

    /* Inicializar los relojes principales */
    BOARD_InitBootClocks();

    /* Inicializar la consola de depuracion */
    BOARD_InitDebugConsole();
}
