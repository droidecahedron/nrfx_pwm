/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/irq.h>
#include <nrfx_pwm.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   50

#define PWM_COUNTERTOP  100

#define NUM_PWM_CHANS 4

static nrfx_pwm_t pwm0 = NRFX_PWM_INSTANCE(0);
static nrfx_pwm_t pwm1 = NRFX_PWM_INSTANCE(1);
static nrfx_pwm_t pwm2 = NRFX_PWM_INSTANCE(2);
static nrfx_pwm_t pwm3 = NRFX_PWM_INSTANCE(3);

static int pwm_init(void)
{
	static nrfx_pwm_config_t const config0 =
    {
        .output_pins =
        {
            28,						// channel 0
            29,						// channel 1
            30,                 	// channel 2
            31	                    // channel 3
        },
        .irq_priority = 5,
        .base_clock   = NRF_PWM_CLK_1MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = PWM_COUNTERTOP,
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    return (nrfx_pwm_init(&pwm0, &config0, NULL, NULL) == NRFX_SUCCESS) ? 0 : -1;

	// If PWM callbacks are to be used, remember to configure the interrupts correctly
	//IRQ_DIRECT_CONNECT(PWM1_IRQn, 0, nrfx_pwm_1_irq_handler, 0);
	//irq_enable(PWM1_IRQn);
}

static void pwm_set_duty_cycle(uint32_t* duty_cycles)
{
	static bool pwm_running = false;

    // This array cannot be allocated on stack (hence "static") and it must be in RAM 
    static nrf_pwm_values_individual_t seq_values;
    uint16_t *pwm_chan_seq = (uint16_t *)&seq_values; // point to the struct for loop increment instead

	// Update the respective channels
    for(int pwm_chan=0; pwm_chan < NUM_PWM_CHANS; pwm_chan++)
	    pwm_chan_seq[pwm_chan] = (duty_cycles[pwm_chan] <= PWM_COUNTERTOP) ? duty_cycles[pwm_chan] : PWM_COUNTERTOP;

    nrf_pwm_sequence_t const seq =
    {
        .values.p_individual = &seq_values,
        .length              = NRF_PWM_VALUES_LENGTH(seq_values),
        .repeats             = 0,
        .end_delay           = 0
    };

	if(!pwm_running){
		pwm_running = true;
		(void)nrfx_pwm_simple_playback(&pwm0, &seq, 1000, NRFX_PWM_FLAG_LOOP);
	}
}


int main(void)
{
	if(pwm_init() != 0){
		printk("Error initializing PWM\n");
		return -1;
	}

	printk("NRFX PMW example started\n");

    // each ind channel gets its own duty cycle
    uint32_t duty_cycles[NUM_PWM_CHANS] = {PWM_COUNTERTOP >> 3, PWM_COUNTERTOP >> 2, PWM_COUNTERTOP >> 1, PWM_COUNTERTOP - 1};

    pwm_set_duty_cycle(duty_cycles);

	while (1) {
		//static int counter = 0;
		// pwm_set_duty_cycle(counter, PWM_COUNTERTOP - counter);
		// counter = (counter + 1) % PWM_COUNTERTOP;
		k_msleep(SLEEP_TIME_MS);
	}

    return 0;
}