
/* SPDX-License-Identifier: MIT */

/*
 * Proyecto: Monitor de senal analogica
 * Placa: FRDM-MCXA156
 *
 * Configuracion del ADC para el potenciometro.
 */

#ifndef ADC_CONTROL_H_
#define ADC_CONTROL_H_

#include <stdint.h>
#include <stdbool.h>

#include "fsl_common.h"
#include "fsl_lpadc.h"

/* Configuracion del ADC */
#define ADC_CONTROL_BASE       ADC1
#define ADC_CONTROL_CANAL      8U
#define ADC_CONTROL_COMANDO    1U

/* Inicializar ADC1 */
static inline void ADC_Control_Iniciar(void)
{
    lpadc_config_t config;
    lpadc_conv_command_config_t comando;
    lpadc_conv_trigger_config_t trigger;

    /* Obtener configuracion predeterminada */
    LPADC_GetDefaultConfig(&config);

#if defined(FSL_FEATURE_LPADC_HAS_CFG_PWRSEL) && \
    (FSL_FEATURE_LPADC_HAS_CFG_PWRSEL == 1U)
    config.powerLevelMode = kLPADC_PowerLevelAlt4;
#endif

    /* Habilitar circuito analogico */
    config.enableAnalogPreliminary = true;

#if !(defined(FSL_FEATURE_LPADC_HAS_CFG_REFSEL) && \
      (FSL_FEATURE_LPADC_HAS_CFG_REFSEL == 0U))
    config.referenceVoltageSource =
        kLPADC_ReferenceVoltageAlt3;
#endif

    /* Inicializar modulo ADC */
    LPADC_Init(ADC_CONTROL_BASE, &config);

    /* Calibracion del ADC */
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CALOFS) && \
    FSL_FEATURE_LPADC_HAS_CTRL_CALOFS
    LPADC_DoOffsetCalibration(ADC_CONTROL_BASE);
#endif

#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_REQ) && \
    FSL_FEATURE_LPADC_HAS_CTRL_CAL_REQ
    SDK_DelayAtLeastUs(1U, SystemCoreClock);
    LPADC_DoAutoCalibration(ADC_CONTROL_BASE);
#elif defined(FSL_FEATURE_LPADC_HAS_CFG_CALOFS) && \
      FSL_FEATURE_LPADC_HAS_CFG_CALOFS
    SDK_DelayAtLeastUs(1U, SystemCoreClock);
    LPADC_DoAutoCalibration(ADC_CONTROL_BASE);
#endif

    /* Configuracion de conversion */
    LPADC_GetDefaultConvCommandConfig(&comando);

    comando.channelNumber = ADC_CONTROL_CANAL;

    /* Resolucion alta de 16 bits */
    comando.conversionResolutionMode =
        kLPADC_ConversionResolutionHigh;

    LPADC_SetConvCommandConfig(
        ADC_CONTROL_BASE,
        ADC_CONTROL_COMANDO,
        &comando
    );

    /* Configurar disparo por software */
    LPADC_GetDefaultConvTriggerConfig(&trigger);

    trigger.targetCommandId = ADC_CONTROL_COMANDO;
    trigger.enableHardwareTrigger = false;

    LPADC_SetConvTriggerConfig(
        ADC_CONTROL_BASE,
        0U,
        &trigger
    );
}

/* Solicitar una conversion analogica */
static inline void ADC_Control_SolicitarLectura(void)
{
    LPADC_DoSoftwareTrigger(ADC_CONTROL_BASE, 1U);
}

/* Obtener lectura cuando este disponible */
static inline bool ADC_Control_Leer(uint16_t *valor)
{
    lpadc_conv_result_t resultado;
    bool disponible;

#if defined(FSL_FEATURE_LPADC_FIFO_COUNT) && \
    (FSL_FEATURE_LPADC_FIFO_COUNT == 2U)
    disponible = LPADC_GetConvResult(
        ADC_CONTROL_BASE, &resultado, 0U);
#else
    disponible = LPADC_GetConvResult(
        ADC_CONTROL_BASE, &resultado);
#endif

    if (disponible && (valor != NULL))
    {
        *valor = (uint16_t)resultado.convValue;
    }

    return disponible;
}

#endif /* ADC_CONTROL_H_ */
