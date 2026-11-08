
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de senal analogica
 * Placa: FRDM-MCXA156
 *
 * Conversion ADC, comandos UART,
 * estados RUN/PAUSE, histeresis
 * y antirrebote del pulsador SW2.
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

    if (value < 10U)
    {
        return ALARM_CMD_ERR;
    }

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
    c->threshold = 70U;
    c->percent = 0U;
    c->adc = 0U;

    c->has_sample = false;
    c->running = true;
    c->alarm = false;
    c->reset_decision = true;
}

/* ========================================
 * REEVALUACION DE LA ALARMA
 * ======================================== */

static inline void alarm_restart_decision(
    alarm_core_t *c)
{
    if (!c->running)
    {
        c->alarm = false;
        return;
    }

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
        alarm_restart_decision(c);
    }
}

/* Activar modo PAUSE */
static inline void alarm_stop(alarm_core_t *c)
{
    c->running = false;
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
 * CAMBIO DEL UMBRAL
 * ======================================== */

static inline void alarm_set_thr(
    alarm_core_t *c,
    uint8_t thr)
{
    c->threshold = thr;
    alarm_restart_decision(c);
}

/* ========================================
 * ACTUALIZACION ADC E HISTERESIS
 * ======================================== */

/*
 * Encender: PCT >= THR
 * Apagar:   PCT < (THR - 5)
 * Retener:  dentro de la banda
 */

static inline void alarm_sample(
    alarm_core_t *c,
    uint16_t adc)
{
    c->adc = adc;
    c->percent = alarm_pct16(adc);
    c->has_sample = true;

    if (!c->running)
    {
        c->alarm = false;
    }
    else if (c->reset_decision)
    {
        alarm_restart_decision(c);
    }
    else if (c->percent >= c->threshold)
    {
        c->alarm = true;
    }
    else if (
        c->percent <
        (uint8_t)(c->threshold - 5U))
    {
        c->alarm = false;
    }

    /* En la banda conserva el estado */
}

/* ========================================
 * ANTIRREBOTE DEL PULSADOR SW2
 * ======================================== */

/*
 * El debounce permite detectar una
 * pulsacion estable durante 30 ms.
 *
 * Cada pulsacion genera una unica accion.
 * Mantener presionado el boton no genera
 * nuevas pulsaciones.
 */

#define ALARM_DEBOUNCE_MS 30U

/* Estructura del pulsador */
typedef struct
{
    bool raw;
    bool stable;
    uint32_t changed_at;

} alarm_debounce_t;

/* Inicializar el sistema de antirrebote */
static inline void alarm_debounce_init(
    alarm_debounce_t *d,
    bool initial_raw)
{
    d->raw = initial_raw;
    d->stable = initial_raw;
    d->changed_at = 0U;
}

/*
 * Procesar el estado del pulsador.
 *
 * pressed:
 * true  = boton presionado
 * false = boton liberado
 *
 * now:
 * tiempo actual en milisegundos.
 *
 * Devuelve true una sola vez cuando
 * se confirma una nueva pulsacion.
 */

static inline bool alarm_debounce_pressed(
    alarm_debounce_t *d,
    bool pressed,
    uint32_t now)
{
    /* Detectar cambio de estado */
    if (pressed != d->raw)
    {
        d->raw = pressed;
        d->changed_at = now;
    }

    /* Verificar estabilidad de 30 ms */
    if (
        (d->raw != d->stable) &&
        ((uint32_t)(now - d->changed_at)
            >= ALARM_DEBOUNCE_MS))
    {
        d->stable = d->raw;

        /* Solo generar evento al presionar */
        return d->stable;
    }

    return false;
}

#endif /* ALARM_CORE_H */

