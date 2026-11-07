
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de senal analogica
 * Placa: FRDM-MCXA156
 *
 * Configuracion de pines para GPIO, ADC y UART.
 */

#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_reset.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "pin_mux.h"

/* Inicializacion de los pines de arranque */
void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

/* Configuracion principal de pines */
void BOARD_InitPins(void)
{
    /* Habilitar relojes GPIO */
    CLOCK_EnableClock(kCLOCK_GateGPIO1);
    CLOCK_EnableClock(kCLOCK_GateGPIO3);

    /* Habilitar relojes PORT */
    CLOCK_EnableClock(kCLOCK_GatePORT0);
    CLOCK_EnableClock(kCLOCK_GatePORT1);
    CLOCK_EnableClock(kCLOCK_GatePORT3);

    /* Liberar perifericos del estado de reset */
    RESET_ReleasePeripheralReset(kGPIO1_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kGPIO3_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kLPUART0_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kADC1_RST_SHIFT_RSTn);

    RESET_ReleasePeripheralReset(kPORT0_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kPORT1_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kPORT3_RST_SHIFT_RSTn);

    /* Configuracion de pines UART */
    const port_pin_config_t uart_pin = {
        kPORT_PullUp,
        kPORT_LowPullResistor,
        kPORT_FastSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_NormalDriveStrength,
        kPORT_MuxAlt2,
        kPORT_InputBufferEnable,
        kPORT_InputNormal,
        kPORT_UnlockRegister
    };

    /* Configuracion de entrada analogica */
    const port_pin_config_t analog_pin = {
        kPORT_PullDisable,
        kPORT_LowPullResistor,
        kPORT_FastSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_NormalDriveStrength,
        kPORT_MuxAlt0,
        kPORT_InputBufferDisable,
        kPORT_InputNormal,
        kPORT_UnlockRegister
    };

    /* Configuracion de salida digital */
    const port_pin_config_t gpio_out = {
        kPORT_PullDisable,
        kPORT_LowPullResistor,
        kPORT_FastSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_NormalDriveStrength,
        kPORT_MuxAlt0,
        kPORT_InputBufferEnable,
        kPORT_InputNormal,
        kPORT_UnlockRegister
    };

    /* Configuracion de entrada digital */
    const port_pin_config_t gpio_in = {
        kPORT_PullUp,
        kPORT_LowPullResistor,
        kPORT_FastSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_NormalDriveStrength,
        kPORT_MuxAlt0,
        kPORT_InputBufferEnable,
        kPORT_InputNormal,
        kPORT_UnlockRegister
    };

    /* Direccion de los GPIO */
    const gpio_pin_config_t led = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U
    };

    const gpio_pin_config_t sw = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };

    /* UART: comunicacion con MCU-Link */
    PORT_SetPinConfig(PORT0, 2U, &uart_pin);
    PORT_SetPinConfig(PORT0, 3U, &uart_pin);

    /* ADC: potenciometro conectado a A0 */
    PORT_SetPinConfig(PORT1, 10U, &analog_pin);

    /* SW2: pulsador integrado */
    PORT_SetPinConfig(PORT1, 7U, &gpio_in);

    /* LED rojo: indicador de actividad */
    PORT_SetPinConfig(PORT3, 1U, &gpio_out);

    /* LED blanco: indicador de alarma */
    PORT_SetPinConfig(PORT3, 31U, &gpio_out);

    /* Inicializar GPIO */
    GPIO_PinInit(GPIO1, 7U, &sw);
    GPIO_PinInit(GPIO3, 1U, &led);
    GPIO_PinInit(GPIO3, 31U, &led);
}
