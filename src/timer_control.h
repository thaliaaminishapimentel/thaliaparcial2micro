
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de senal analogica
 * Placa: FRDM-MCXA156
 *
 * Temporizador CTIMER1 con interrupcion
 * para generar una base de tiempo de 1 ms.
 */

#ifndef TIMER_CONTROL_H_
#define TIMER_CONTROL_H_

#include <stdint.h>
#include <stddef.h>

#include "fsl_clock.h"
#include "fsl_reset.h"
#include "fsl_ctimer.h"

/* Temporizador utilizado */
#define TIMER_CONTROL_BASE CTIMER1

/* Contador de milisegundos */
static volatile uint32_t timer_milisegundos = 0U;

/* Funcion ejecutada en cada interrupcion */
static void Timer_Control_Callback(uint32_t flags)
{
    (void)flags;

    timer_milisegundos++;
}

/* Tabla de funciones callback */
static ctimer_callback_t timer_callbacks[] = {
    Timer_Control_Callback,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* Inicializar el temporizador CTIMER1 */
static inline void Timer_Control_Iniciar(void)
{
    ctimer_config_t config;
    ctimer_match_config_t match;

    uint32_t frecuencia_timer;

    /* Seleccionar reloj FRO_HF */
    CLOCK_AttachClk(kFRO_HF_to_CTIMER1);

    /* Configurar divisor del reloj */
    CLOCK_SetClockDiv(kCLOCK_DivCTIMER1, 1U);

    /* Liberar CTIMER1 del reset */
    RESET_ReleasePeripheralReset(
        kCTIMER1_RST_SHIFT_RSTn
    );

    /* Consultar frecuencia del temporizador */
    frecuencia_timer = CLOCK_GetCTimerClkFreq(1U);

    /* Configuracion predeterminada */
    CTIMER_GetDefaultConfig(&config);

    /* Prescaler igual a cero */
    config.prescale = 0U;

    /* Inicializar CTIMER1 */
    CTIMER_Init(TIMER_CONTROL_BASE, &config);

    /* Configurar evento cada 1 ms */
    match.enableCounterReset = true;
    match.enableCounterStop = false;

    match.matchValue = frecuencia_timer / 1000U;

    match.outControl = kCTIMER_Output_NoAction;
    match.outPinInitState = false;

    /* Habilitar interrupciones */
    match.enableInterrupt = true;

    /* Registrar callback */
    CTIMER_RegisterCallBack(
        TIMER_CONTROL_BASE,
        timer_callbacks,
        kCTIMER_MultipleCallback
    );

    /* Configurar MATCH0 */
    CTIMER_SetupMatch(
        TIMER_CONTROL_BASE,
        kCTIMER_Match_0,
        &match
    );

    /* Iniciar el temporizador */
    CTIMER_StartTimer(TIMER_CONTROL_BASE);
}

/* Obtener el tiempo transcurrido en milisegundos */
static inline uint32_t Timer_Control_Millis(void)
{
    return timer_milisegundos;
}

#endif /* TIMER_CONTROL_H_ */

