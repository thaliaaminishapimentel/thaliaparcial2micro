/* SPDX-License-Identifier: MIT */
/*
 * FRDM-MCXA156 integrated challenge: GPIO + CTIMER1 + ADC1 + LPUART0.
 * Copy this file over NXP's imported lpadc_polling.c, see README.
 * Hardware mappings: A0=J4-2=P1_10/ADC1_A8; SW2=P1_7 (active low),
 * D2=P3_1=activity LED, D4=P3_31=alarm LED, MCU-Link VCOM=LPUART0.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "app.h"
#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_ctimer.h"
#include "fsl_gpio.h"
#include "fsl_lpadc.h"
#include "fsl_lpuart.h"
#include "fsl_reset.h"
#include "alarm_core.h"

#define TICK_TIMER CTIMER1
#define TICK_CLOCK_HZ CLOCK_GetCTimerClkFreq(1U)
#define UART_DEVICE LPUART0
#define UART_IRQn LPUART0_IRQn
#define ACT_LED_PIN 1U           /* GPIO3 / P3_1 -> Arduino D2 (J1-6) */
#define ALARM_LED_PIN 31U        /* GPIO3 / P3_31 -> Arduino D4 (J1-10) */
#define BUTTON_PIN 7U           /* GPIO1 / SW2 */
#define RX_RING_SIZE 256U
#define TX_RING_SIZE 1024U

static volatile uint32_t g_millis;
static volatile uint16_t rx_head, rx_tail, tx_head, tx_tail;
static volatile bool rx_buffer_overflow;
static uint8_t rx_ring[RX_RING_SIZE];
static uint8_t tx_ring[TX_RING_SIZE];
static alarm_core_t alarm;
static alarm_debounce_t debounce;
static bool activity_led;
static bool conversion_pending;

/* CTIMER driver dispatches its MATCH0 IRQ using this callback. */
static void tick_callback(uint32_t flags)
{
    (void)flags;
    ++g_millis;
}
static ctimer_callback_t timer_callbacks[] =
    { tick_callback, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static void init_timer(void)
{
    ctimer_config_t cfg;
    ctimer_match_config_t match;
    uint32_t timer_hz;

    CLOCK_AttachClk(kFRO_HF_to_CTIMER1);
    CLOCK_SetClockDiv(kCLOCK_DivCTIMER1, 1U);
    RESET_ReleasePeripheralReset(kCTIMER1_RST_SHIFT_RSTn);

    timer_hz = TICK_CLOCK_HZ;
    CTIMER_GetDefaultConfig(&cfg);
    cfg.prescale = 0U; /* / (0+1) = /1; e.g. 96MHz clock. */
    CTIMER_Init(TICK_TIMER, &cfg);
    match.enableCounterReset = true;
    match.enableCounterStop = false;
    match.matchValue = timer_hz / 1000U; /* 96,000 counts -> 1ms at 96MHz. */
    match.outControl = kCTIMER_Output_NoAction;
    match.outPinInitState = false;
    match.enableInterrupt = true;
    CTIMER_RegisterCallBack(TICK_TIMER, timer_callbacks, kCTIMER_MultipleCallback);
    CTIMER_SetupMatch(TICK_TIMER, kCTIMER_Match_0, &match);
    CTIMER_StartTimer(TICK_TIMER);
}

static void init_adc(void)
{
    lpadc_config_t cfg;
    lpadc_conv_command_config_t cmd;
    lpadc_conv_trigger_config_t trigger;

    /* A0: ADC1_A8, externally connected 0..3.3V; N=16, VREF=VDDA. */
    LPADC_GetDefaultConfig(&cfg);
#if defined(FSL_FEATURE_LPADC_HAS_CFG_PWRSEL) && (FSL_FEATURE_LPADC_HAS_CFG_PWRSEL == 1U)
    cfg.powerLevelMode = kLPADC_PowerLevelAlt4;
#endif
    cfg.enableAnalogPreliminary = true;
#if !(defined(FSL_FEATURE_LPADC_HAS_CFG_REFSEL) && (FSL_FEATURE_LPADC_HAS_CFG_REFSEL == 0U))
    cfg.referenceVoltageSource = kLPADC_ReferenceVoltageAlt3;
#endif
    LPADC_Init(ADC1, &cfg);
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CALOFS) && FSL_FEATURE_LPADC_HAS_CTRL_CALOFS
    LPADC_DoOffsetCalibration(ADC1);
#endif
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_REQ) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_REQ
    SDK_DelayAtLeastUs(1U, SystemCoreClock); /* one-time init only */
    LPADC_DoAutoCalibration(ADC1);
#elif defined(FSL_FEATURE_LPADC_HAS_CFG_CALOFS) && FSL_FEATURE_LPADC_HAS_CFG_CALOFS
    SDK_DelayAtLeastUs(1U, SystemCoreClock); /* one-time init only */
    LPADC_DoAutoCalibration(ADC1);
#endif

    LPADC_GetDefaultConvCommandConfig(&cmd);
    cmd.channelNumber = 8U;
    cmd.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
    LPADC_SetConvCommandConfig(ADC1, 1U, &cmd);
    LPADC_GetDefaultConvTriggerConfig(&trigger);
    trigger.targetCommandId = 1U;
    trigger.enableHardwareTrigger = false;
    LPADC_SetConvTriggerConfig(ADC1, 0U, &trigger);
}

static void init_uart(void)
{
    lpuart_config_t cfg;
    LPUART_GetDefaultConfig(&cfg);
    cfg.baudRate_Bps = 115200U;
    cfg.parityMode = kLPUART_ParityDisabled;
    cfg.stopBitCount = kLPUART_OneStopBit;
    cfg.enableTx = true;
    cfg.enableRx = true;
    /* This clock was initialized by BOARD_InitDebugConsole() in NXP's base. */
    LPUART_Init(UART_DEVICE, &cfg, BOARD_DEBUG_UART_CLK_FREQ);
    LPUART_EnableInterrupts(UART_DEVICE, kLPUART_RxDataRegFullInterruptEnable);
    EnableIRQ(UART_IRQn);
}

/* Only the UART ISR touches tx_tail/rx_head. No parsing in interrupts. */
void LPUART0_IRQHandler(void)
{
    uint32_t flags = LPUART_GetStatusFlags(UART_DEVICE);
    if ((flags & kLPUART_RxDataRegFullFlag) != 0U)
    {
        uint8_t c = LPUART_ReadByte(UART_DEVICE);
        uint16_t next = (uint16_t)((rx_head + 1U) % RX_RING_SIZE);
        if (next != rx_tail)
        {
            rx_ring[rx_head] = c;
            rx_head = next;
        }
        else
        {
            rx_buffer_overflow = true;
        }
    }
    if ((flags & kLPUART_TxDataRegEmptyFlag) != 0U)
    {
        if (tx_tail != tx_head)
        {
            LPUART_WriteByte(UART_DEVICE, tx_ring[tx_tail]);
            tx_tail = (uint16_t)((tx_tail + 1U) % TX_RING_SIZE);
        }
        else
        {
            LPUART_DisableInterrupts(UART_DEVICE, kLPUART_TxDataRegEmptyInterruptEnable);
        }
    }
    SDK_ISR_EXIT_BARRIER;
}

/* Atomically queue WHOLE lines. TX is sent by the interrupt, never blocking. */
static bool uart_queue(const char *line)
{
    size_t i, len = strlen(line);
    uint16_t free_bytes;
    if (len == 0U || len >= TX_RING_SIZE) return false;
    DisableIRQ(UART_IRQn);
    free_bytes = (uint16_t)((tx_tail + TX_RING_SIZE - tx_head - 1U) % TX_RING_SIZE);
    if (len > free_bytes)
    {
        EnableIRQ(UART_IRQn);
        return false; /* Never send a partial/corrupted line. */
    }
    for (i = 0; i < len; ++i)
    {
        tx_ring[tx_head] = (uint8_t)line[i];
        tx_head = (uint16_t)((tx_head + 1U) % TX_RING_SIZE);
    }
    LPUART_EnableInterrupts(UART_DEVICE, kLPUART_TxDataRegEmptyInterruptEnable);
    EnableIRQ(UART_IRQn);
    return true;
}

static bool uart_pop(uint8_t *c)
{
    if (rx_head == rx_tail) return false;
    *c = rx_ring[rx_tail];
    rx_tail = (uint16_t)((rx_tail + 1U) % RX_RING_SIZE);
    return true;
}

static void write_alarm_led(void)
{
    GPIO_PinWrite(GPIO3, ALARM_LED_PIN, alarm.alarm ? 1U : 0U);
}

static void report_status(void)
{
    char line[96];
    (void)snprintf(line, sizeof(line),
                   "t=%lu;ADC=%u;PCT=%u;THR=%u;MODE=%s;ALARM=%u\r\n",
                   (unsigned long)g_millis, (unsigned)alarm.adc,
                   (unsigned)alarm.percent, (unsigned)alarm.threshold,
                   alarm.running ? "RUN" : "PAUSE", alarm.alarm ? 1U : 0U);
    (void)uart_queue(line);
}

static void handle_command(const char *line)
{
    uint8_t thr = 0;
    char reply[20];
    alarm_cmd_t cmd = alarm_parse(line, &thr);
    switch (cmd)
    {
        case ALARM_CMD_IGNORE: break;
        case ALARM_CMD_STATUS: report_status(); break;
        case ALARM_CMD_START:
            alarm_start(&alarm);
            write_alarm_led();
            (void)uart_queue("OK RUN\r\n");
            break;
        case ALARM_CMD_STOP:
            alarm_stop(&alarm);
            write_alarm_led();
            (void)uart_queue("OK PAUSE\r\n");
            break;
        case ALARM_CMD_THR:
            alarm_set_thr(&alarm, thr);
            write_alarm_led();
            (void)snprintf(reply, sizeof(reply), "OK THR=%u\r\n", (unsigned)thr);
            (void)uart_queue(reply);
            break;
        default: (void)uart_queue("ERR\r\n"); break;
    }
}

static void process_rx(void)
{
    static char line[34]; /* 32 chars + optional CR + NUL */
    static unsigned length;
    static bool dropping;
    uint8_t ch;

    /* When interrupt RX ring has overflowed, resync to the NEXT LF. */
    if (rx_buffer_overflow)
    {
        rx_buffer_overflow = false;
        dropping = true;
    }
    while (uart_pop(&ch))
    {
        if (ch == '\n')
        {
            if (dropping)
            {
                (void)uart_queue("ERR\r\n");
            }
            else
            {
                if ((length > 0U) && (line[length - 1U] == '\r')) --length;
                if (length > 32U) (void)uart_queue("ERR\r\n");
                else
                {
                    line[length] = '\0';
                    handle_command(line);
                }
            }
            length = 0U;
            dropping = false;
        }
        else if (!dropping)
        {
            if (length < 33U) line[length++] = (char)ch;
            else dropping = true;
        }
    }
}

static void receive_adc_if_ready(void)
{
    lpadc_conv_result_t result;
    bool available;
    if (!conversion_pending) return;
#if defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U)
    available = LPADC_GetConvResult(ADC1, &result, 0U);
#else
    available = LPADC_GetConvResult(ADC1, &result);
#endif
    if (available)
    {
        conversion_pending = false;
        alarm_sample(&alarm, (uint16_t)result.convValue);
        write_alarm_led();
    }
}

int main(void)
{
    uint32_t adc_at = 0U, led_at = 0U, tel_at = 0U;
    BOARD_InitHardware(); /* From NXP example: pins, clock, ADC clock, VCOM. */
    alarm_init(&alarm);
    alarm_debounce_init(&debounce, GPIO_PinRead(GPIO1, BUTTON_PIN) == 0U);
    GPIO_PinWrite(GPIO3, ACT_LED_PIN, 0U);
    GPIO_PinWrite(GPIO3, ALARM_LED_PIN, 0U);
    init_adc();
    init_uart();
    init_timer();

    for (;;)
    {
        uint32_t now = g_millis;
        process_rx(); /* Commands take priority over periodic reports. */
        receive_adc_if_ready();
        if (alarm_debounce_pressed(&debounce,
                                  GPIO_PinRead(GPIO1, BUTTON_PIN) == 0U, now))
        {
            alarm_toggle(&alarm);
            write_alarm_led();
        }
        if ((uint32_t)(now - adc_at) >= 100U)
        {
            adc_at += 100U;
            if (!conversion_pending)
            {
                conversion_pending = true;
                LPADC_DoSoftwareTrigger(ADC1, 1U);
            }
        }
        if ((uint32_t)(now - led_at) >= 500U)
        {
            led_at += 500U;
            activity_led = !activity_led;
            GPIO_PinWrite(GPIO3, ACT_LED_PIN, activity_led ? 1U : 0U);
        }
        if ((uint32_t)(now - tel_at) >= 1000U)
        {
            tel_at += 1000U;
            report_status();
        }
    }
}
