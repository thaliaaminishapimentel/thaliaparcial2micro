
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de senal analogica
 * Placa: FRDM-MCXA156
 *
 * Programacion de tareas periodicas
 * mediante una base de tiempo de 1 ms.
 */

#ifndef PERIODIC_TASKS_H_
#define PERIODIC_TASKS_H_

#include <stdint.h>

/* Intervalos de ejecucion en milisegundos */
#define PERIODO_ADC_MS    100U
#define PERIODO_LED_MS    500U
#define PERIODO_UART_MS  1000U

/* Indicadores de tareas pendientes */
#define TAREA_ADC   (1U << 0)
#define TAREA_LED   (1U << 1)
#define TAREA_UART  (1U << 2)

/* Estructura para controlar los tiempos */
typedef struct
{
    uint32_t ultimo_adc;
    uint32_t ultimo_led;
    uint32_t ultimo_uart;

} tareas_periodicas_t;

/* Inicializar los contadores */
static inline void Tareas_Iniciar(
    tareas_periodicas_t *tareas,
    uint32_t tiempo_actual)
{
    tareas->ultimo_adc = tiempo_actual;
    tareas->ultimo_led = tiempo_actual;
    tareas->ultimo_uart = tiempo_actual;
}

/* Comprobar que tareas deben ejecutarse */
static inline uint8_t Tareas_Actualizar(
    tareas_periodicas_t *tareas,
    uint32_t tiempo_actual)
{
    uint8_t pendientes = 0U;

    /* Lectura del ADC cada 100 ms */
    if ((uint32_t)(tiempo_actual -
        tareas->ultimo_adc) >= PERIODO_ADC_MS)
    {
        tareas->ultimo_adc += PERIODO_ADC_MS;
        pendientes |= TAREA_ADC;
    }

    /* Alternar el LED cada 500 ms */
    if ((uint32_t)(tiempo_actual -
        tareas->ultimo_led) >= PERIODO_LED_MS)
    {
        tareas->ultimo_led += PERIODO_LED_MS;
        pendientes |= TAREA_LED;
    }

    /* Transmision UART cada 1000 ms */
    if ((uint32_t)(tiempo_actual -
        tareas->ultimo_uart) >= PERIODO_UART_MS)
    {
        tareas->ultimo_uart += PERIODO_UART_MS;
        pendientes |= TAREA_UART;
    }

    return pendientes;
}

#endif /* PERIODIC_TASKS_H_ */

