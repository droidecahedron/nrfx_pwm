/*
 * Copyright (c) 2024 Nordic Semiconductor
 *
 * Author: Johnny Nguyen (@droidecahedron)
 *
 * Simple pwm sequence that changes every SLEEP_TIME_MS.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <nrfx_pwm.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define SLEEP_TIME_MS 2000
#define PWM_COUNTERTOP 10000
#define NUM_PWM_CHANS 4
#define NUM_SEQUENCES 3

// Some arbitrary number of clock cycles to match required resolution, more clock cycles gets wider
#define CYCLE_A (1 * 1000)
#define CYCLE_B (2 * 1000)
#define CYCLE_C (3 * 1000)
#define CYCLE_D (4 * 1000)

static nrfx_pwm_t my_pwm = NRFX_PWM_INSTANCE(1);
static uint8_t seq_idx = 0;

static int pwm_init(void)
{
    static nrfx_pwm_config_t const config0 = {
        .output_pins =
            {
                28, // channel 0
                29, // channel 1
                30, // channel 2
                31, // channel 3, NRF_PWM_PIN_NOT_CONNECTED if you want to use waveform to adjust COUNTERTOP
            },
        .irq_priority = 5,
        .base_clock = NRF_PWM_CLK_1MHz,
        .count_mode = NRF_PWM_MODE_UP,
        .top_value = PWM_COUNTERTOP,
        .load_mode = NRF_PWM_LOAD_INDIVIDUAL, // change to NRF_PWM_LOAD_WAVE_FORM for adjustable COUNTERTOP
        .step_mode = NRF_PWM_STEP_AUTO};
    return (nrfx_pwm_init(&my_pwm, &config0, NULL, NULL) == NRFX_SUCCESS) ? 0 : -1;

    // If PWM callbacks are to be used, remember to configure the interrupts
    // correctly IRQ_DIRECT_CONNECT(PWM1_IRQn, 0, nrfx_pwm_1_irq_handler, 0);
    // irq_enable(PWM1_IRQn);
}

static void pwm_set_duty_cycle(uint8_t sequence_id)
{
    bool pwm_running = false;

    // This array cannot be allocated on stack (hence "static") and it must be in
    // RAM note: if using waveform mode for adjustable countertop, the sequence
    // data type is nrf_pwm_values_wave_form_t
    static nrf_pwm_values_individual_t sequence_A;          // sequence A is a pretty basic PWM duty cycle.
    sequence_A.channel_0 = PWM_COUNTERTOP / 10 | (1 << 15); // flips really early, inverted polarity
    sequence_A.channel_1 = PWM_COUNTERTOP / 4;
    sequence_A.channel_2 = PWM_COUNTERTOP / 2; // should flip half way through
    sequence_A.channel_3 = PWM_COUNTERTOP / 10;

    // column 1 for channel 1, it's on then off for the rest of the sequence.
    // column 2 for channel 2, , etc.

    // a little more interesting, dilating cycles
    static nrf_pwm_values_individual_t sequence_B[] = {
        {0x8000 | (CYCLE_A / 2), 0x8000 | (CYCLE_B / 2), 0x8000 | (CYCLE_C / 2), CYCLE_A},
        {(CYCLE_A / 2), (CYCLE_B / 2), (CYCLE_C / 2), CYCLE_B},
        {0, 0, 0, CYCLE_C},
        {CYCLE_A, CYCLE_B, CYCLE_C, CYCLE_D},
    };

    // short pulses, then OFF.
    const uint16_t short_pulse = PWM_COUNTERTOP / 10 | (1 << 15);
    static nrf_pwm_values_individual_t sequence_C[] = {
        {short_pulse, PWM_COUNTERTOP, PWM_COUNTERTOP, PWM_COUNTERTOP},
        {PWM_COUNTERTOP, short_pulse, PWM_COUNTERTOP, PWM_COUNTERTOP},
        {PWM_COUNTERTOP, PWM_COUNTERTOP, short_pulse, PWM_COUNTERTOP},
        {PWM_COUNTERTOP, PWM_COUNTERTOP, PWM_COUNTERTOP, short_pulse},
    };

    const uint16_t long_pulse = PWM_COUNTERTOP / 2 | (1 << 15);
    static nrf_pwm_values_individual_t sequence_D[] = {
        {short_pulse, PWM_COUNTERTOP, long_pulse, PWM_COUNTERTOP},
        {PWM_COUNTERTOP, long_pulse, PWM_COUNTERTOP, short_pulse},
        {long_pulse, PWM_COUNTERTOP, short_pulse, PWM_COUNTERTOP},
        {PWM_COUNTERTOP, short_pulse, PWM_COUNTERTOP, long_pulse},
    };

    // sequence args for simple playback
    // use member element .p_wave_form for waveform mode.
    nrf_pwm_sequence_t const seqA = {
        .values.p_individual = &sequence_A, .length = NRF_PWM_VALUES_LENGTH(sequence_A), .repeats = 0, .end_delay = 0};
    nrf_pwm_sequence_t const seqB = {
        .values.p_individual = &sequence_B, .length = NRF_PWM_VALUES_LENGTH(sequence_B), .repeats = 0, .end_delay = 0};
    nrf_pwm_sequence_t const seqC = {
        .values.p_individual = &sequence_C, .length = NRF_PWM_VALUES_LENGTH(sequence_C), .repeats = 0, .end_delay = 0};
    nrf_pwm_sequence_t const seqD = {
        .values.p_individual = &sequence_D, .length = NRF_PWM_VALUES_LENGTH(sequence_D), .repeats = 0, .end_delay = 0};

    if (!pwm_running)
    {
        printk("Sequence num: %d\n", sequence_id);
        pwm_running = true;
        switch (sequence_id)
        {
        case 0:
            (void)nrfx_pwm_simple_playback(&my_pwm, &seqA, 1000, NRFX_PWM_FLAG_LOOP);
            break;
        case 1:
            (void)nrfx_pwm_simple_playback(&my_pwm, &seqB, 1000, NRFX_PWM_FLAG_LOOP);
            break;
        case 2:
            (void)nrfx_pwm_simple_playback(&my_pwm, &seqC, 1000, NRFX_PWM_FLAG_LOOP);
            break;
        case 3:
            (void)nrfx_pwm_simple_playback(&my_pwm, &seqD, 1000, NRFX_PWM_FLAG_LOOP);
            break;
        default:
            break;
        }
    }
}

int main(void)
{
    if (pwm_init() != 0)
    {
        printk("Error initializing PWM\n");
        return -1;
    }

    printk("NRFX PMW example started\n");

    while (1)
    {
        pwm_set_duty_cycle((seq_idx++ & NUM_SEQUENCES)); // mask the overflow for circular buf.
                                                         // only works for powers of 2 - 1.
        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}