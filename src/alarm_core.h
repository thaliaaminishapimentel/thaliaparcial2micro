
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de senal analogica
 * Placa: FRDM-MCXA156
 *
 * Conversion de lecturas ADC a porcentaje.
 */

#ifndef ALARM_CORE_H
#define ALARM_CORE_H

#include <stdint.h>

/*
 * El ADC trabaja con una resolucion de 16 bits.
 *
 * Valor minimo: 0
 * Valor maximo: 65535
 *
 * Se convierte la lectura a un porcentaje
 * comprendido entre 0 y 100.
 */

static inline uint8_t alarm_pct16(uint16_t adc)
{
    /* Conversion con redondeo entero */
    return (uint8_t)(
        ((uint32_t)adc * 100U + 32767U) / 65535U
    );
}

#endif /* ALARM_CORE_H */
