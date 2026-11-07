
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de senal analogica
 * Placa: FRDM-MCXA156
 *
 * Conversion ADC, comandos UART
 * y control de estados RUN y PAUSE.
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

    /* Verificar formato THR */
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

    /* Guardar nuevo umbral */
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

    /* Valor inicial del ADC */
    c->percent = 0U;
    c->adc = 0U;

    /* Todavia no existe una muestra */
    c->has_sample = false;

    /* Iniciar en modo RUN */
    c->running = true;

    /* Alarma inicialmente apagada */
    c->alarm = false;

    /* Pendiente primera evaluacion */
    c->reset_decision = true;
}

/* ========================================
 * REEVALUACION DE LA ALARMA
 * ======================================== */

static inline void alarm_restart_decision(
    alarm_core_t *c)
{
    /* En PAUSE la alarma debe apagarse */
    if (!c->running)
    {
        c->alarm = false;
        return;
    }

    /* Evaluar si existe una lectura ADC */
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
    /* Detener funcionamiento de alarma */
    c->running = false;

    /* Apagar la alarma */
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

#endif /* ALARM_CORE_H */
