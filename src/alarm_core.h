
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de senal analogica
 * Placa: FRDM-MCXA156
 *
 * Conversion ADC y procesamiento de comandos UART.
 */

#ifndef ALARM_CORE_H
#define ALARM_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Conversion de ADC de 16 bits a porcentaje */
static inline uint8_t alarm_pct16(uint16_t adc)
{
    return (uint8_t)(
        ((uint32_t)adc * 100U + 32767U) / 65535U
    );
}

/* Tipos de comandos admitidos */
typedef enum
{
    ALARM_CMD_IGNORE,
    ALARM_CMD_STATUS,
    ALARM_CMD_THR,
    ALARM_CMD_START,
    ALARM_CMD_STOP,
    ALARM_CMD_ERR

} alarm_cmd_t;

/* Analizar los comandos recibidos por UART */
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

    /* Solicitar estado del sistema */
    if (strcmp(line, "STATUS") == 0)
    {
        return ALARM_CMD_STATUS;
    }

    /* Iniciar funcionamiento */
    if (strcmp(line, "START") == 0)
    {
        return ALARM_CMD_START;
    }

    /* Pausar funcionamiento */
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

    /* Convertir los caracteres numericos */
    for (i = 4; line[i] != '\0'; ++i)
    {
        /* Rechazar caracteres no numericos */
        if (line[i] < '0' || line[i] > '9')
        {
            return ALARM_CMD_ERR;
        }

        value = value * 10U +
                (unsigned)(line[i] - '0');

        /* Umbral maximo permitido */
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

#endif /* ALARM_CORE_H */
