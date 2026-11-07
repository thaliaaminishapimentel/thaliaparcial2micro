
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de senal analogica
 * Placa: FRDM-MCXA156
 *
 * Control de LED de actividad y LED de alarma.
 */

#ifndef LED_CONTROL_H_
#define LED_CONTROL_H_

#include <stdbool.h>
#include "fsl_gpio.h"

/* Pines utilizados para los LED */
#define LED_ACTIVIDAD_GPIO GPIO3
#define LED_ACTIVIDAD_PIN  1U

#define LED_ALARMA_GPIO    GPIO3
#define LED_ALARMA_PIN     31U

/* Encender o apagar el LED rojo de actividad */
static inline void LED_Actividad_Escribir(bool encendido)
{
    GPIO_PinWrite(LED_ACTIVIDAD_GPIO,
                  LED_ACTIVIDAD_PIN,
                  encendido ? 1U : 0U);
}

/* Encender o apagar el LED blanco de alarma */
static inline void LED_Alarma_Escribir(bool encendido)
{
    GPIO_PinWrite(LED_ALARMA_GPIO,
                  LED_ALARMA_PIN,
                  encendido ? 1U : 0U);
}

#endif /* LED_CONTROL_H_ */
