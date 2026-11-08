
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de senal analogica
 * Placa: FRDM-MCXA156
 *
 * Conversion ADC, comandos UART,
 * estados RUN y PAUSE e histeresis.
 */

#ifndef ALARM_CORE_H
#define ALARM_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* ========================================
 * CONVERSION ADC A PORCENTAJE
 * ======================================== */

static inline uint8_t alarm_pct16(uint16_t adc)
{
    return (uint8_t)(
        ((uint32_t)adc * 100U + 32767U) / 65535U
    );
}

/* ========================================
 * COMANDOS UART
 * ======================================== */

typedef enum
{
    ALARM_CMD_IGNORE,
    ALARM_CMD_STATUS,
    ALARM_CMD_THR,
    ALARM_CMD_START,
    ALARM_CMD_STOP,
    ALARM_CMD_ERR

} alarm_cmd_t;

/* Analizar comandos recibidos por UART */
static inline alarm_cmd_t alarm_parse(
    const char *line,
    uint8_t *new_thr)
{
    unsigned value = 0U;
    size_t i;

    /* Ignorar lineas vacias */
    if (line[0] == '\0')
    {
        return ALARM_CMD_IGNORE;
    }

    /* Comando STATUS */
    if (strcmp(line, "STATUS") == 0)
    {
        return ALARM_CMD_STATUS;
    }

    /* Comando START */
    if (strcmp(line, "START") == 0)
    {
        return ALARM_CMD_START;
    }

    /* Comando STOP */
    if (strcmp(line, "STOP") == 0)
    {
        return ALARM_CMD_STOP;
    }

    /* Verificar formato del comando THR */
    if (strncmp(line, "THR ", 4) != 0)
    {
        return ALARM_CMD_ERR;
    }

    /* Verificar que exista un valor */
    if (line[4] == '\0')
    {
        return ALARM_CMD_ERR;
    }

    /* Convertir caracteres numericos */
    for (i = 4; line[i] != '\0'; ++i)
    {
        if (line[i] < '0' || line[i] > '9')
        {
            return ALARM_CMD_ERR;
        }

        value = value * 10U +
                (unsigned)(line[i] - '0');

        if (value > 95U)
        {
            return ALARM_CMD_ERR;
        }
    }

    /* Umbral minimo permitido */
    if (value < 10U)
    {
        return ALARM_CMD_ERR;
    }

    /* Guardar el nuevo umbral */
    *new_thr = (uint8_t)value;

    return ALARM_CMD_THR;
}

/* ========================================
 * ESTRUCTURA PRINCIPAL DEL SISTEMA
 * ======================================== */

typedef struct
{
    uint8_t threshold;
    uint8_t percent;
    uint16_t adc;

    bool has_sample;
    bool running;
    bool alarm;
    bool reset_decision;

} alarm_core_t;

/* ========================================
 * INICIALIZACION DEL SISTEMA
 * ======================================== */

static inline void alarm_init(alarm_core_t *c)
{
    /* Umbral inicial de 70 por ciento */
    c->threshold = 70U;

    /* Valores iniciales del ADC */
    c->percent = 0U;
    c->adc = 0U;

    /* Todavia no existe una muestra ADC */
    c->has_sample = false;

    /* Iniciar en modo RUN */
    c->running = true;

    /* Alarma inicialmente apagada */
    c->alarm = false;

    /* Primera evaluacion pendiente */
    c->reset_decision = true;
}

/* ========================================
 * REEVALUACION DE LA ALARMA
 * ======================================== */

static inline void alarm_restart_decision(
    alarm_core_t *c)
{
    /* La alarma debe apagarse en PAUSE */
    if (!c->running)
    {
        c->alarm = false;
        return;
    }

    /* Verificar si existe una muestra ADC */
    if (c->has_sample)
    {
        c->alarm =
            (c->percent >= c->threshold);

        c->reset_decision = false;
    }
    else
    {
        c->alarm = false;
        c->reset_decision = true;
    }
}

/* ========================================
 * CONTROL DE ESTADOS RUN Y PAUSE
 * ======================================== */

/* Activar modo RUN */
static inline void alarm_start(alarm_core_t *c)
{
    if (!c->running)
    {
        c->running = true;

        /* Reevaluar estado de la alarma */
        alarm_restart_decision(c);
    }
}

/* Activar modo PAUSE */
static inline void alarm_stop(alarm_core_t *c)
{
    c->running = false;

    /* Apagar alarma */
    c->alarm = false;
}

/* Alternar entre RUN y PAUSE */
static inline void alarm_toggle(alarm_core_t *c)
{
    if (c->running)
    {
        alarm_stop(c);
    }
    else
    {
        alarm_start(c);
    }
}

/* ========================================
 * CAMBIO DEL UMBRAL DE ALARMA
 * ======================================== */

/* Actualizar umbral y reevaluar alarma */
static inline void alarm_set_thr(
    alarm_core_t *c,
    uint8_t thr)
{
    /* El analizador UART valida 10 a 95 */
    c->threshold = thr;

    /* Evaluar inmediatamente la alarma */
    alarm_restart_decision(c);
}

/* ========================================
 * ACTUALIZACION ADC E HISTERESIS
 * ======================================== */

/*
 * Reglas de la histeresis:
 *
 * 1. Enciende cuando PCT >= THR.
 *
 * 2. Apaga cuando PCT < (THR - 5).
 *
 * 3. Entre esos limites conserva
 *    el estado anterior de la alarma.
 *
 * 4. En modo PAUSE la alarma
 *    permanece apagada.
 */

static inline void alarm_sample(
    alarm_core_t *c,
    uint16_t adc)
{
    /* Guardar nueva lectura ADC */
    c->adc = adc;

    /* Convertir lectura a porcentaje */
    c->percent = alarm_pct16(adc);

    /* Confirmar existencia de muestra */
    c->has_sample = true;

    /* En PAUSE, apagar la alarma */
    if (!c->running)
    {
        c->alarm = false;
    }

    /* Primera muestra o reinicio */
    else if (c->reset_decision)
    {
        alarm_restart_decision(c);
    }

    /* Encender si alcanza el umbral */
    else if (c->percent >= c->threshold)
    {
        c->alarm = true;
    }

    /* Apagar debajo de la histeresis */
    else if (
        c->percent <
        (uint8_t)(c->threshold - 5U))
    {
        c->alarm = false;
    }

    /*
     * Si el porcentaje esta dentro de
     * la banda de histeresis, no se
     * modifica el estado de la alarma.
     */
}

#endif /* ALARM_CORE_H */
