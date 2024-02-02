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

#define PWM_COUNTERTOP  10000

#define NUM_PWM_CHANS 4

static nrfx_pwm_t my_pwm = NRFX_PWM_INSTANCE(1);

static int pwm_init(void)
{
	static nrfx_pwm_config_t const config0 =
    {
        .output_pins =
        {
            28,						// channel 0
            29,						// channel 1
            30,                 	// channel 2
            NRF_PWM_PIN_NOT_CONNECTED	                    // channel 3 but waveform only does 3 at a time
        },
        .irq_priority = 5,
        .base_clock   = NRF_PWM_CLK_1MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = PWM_COUNTERTOP,
        .load_mode    = NRF_PWM_LOAD_WAVE_FORM,
        .step_mode    = NRF_PWM_STEP_AUTO
    };
    return (nrfx_pwm_init(&my_pwm, &config0, NULL, NULL) == NRFX_SUCCESS) ? 0 : -1;

	// If PWM callbacks are to be used, remember to configure the interrupts correctly
	//IRQ_DIRECT_CONNECT(PWM1_IRQn, 0, nrfx_pwm_1_irq_handler, 0);
	//irq_enable(PWM1_IRQn);
}

static void pwm_set_duty_cycle(uint32_t* duty_cycles)
{
	static bool pwm_running = false;

    // This array cannot be allocated on stack (hence "static") and it must be in RAM
    //pass this to the sequence for screenshot A of repo 
    static nrf_pwm_values_wave_form_t seq_values;
    seq_values.channel_0 = PWM_COUNTERTOP/10 | (1 << 15); //flips really early, inverted polarity
    seq_values.channel_1 = PWM_COUNTERTOP/4;
    seq_values.channel_2 = PWM_COUNTERTOP/2; // should flip half way through
    seq_values.counter_top = PWM_COUNTERTOP;

    //screenshot B of repo
    #define CYCLE_A  (2*1000)   // Some arbitrary number of clock cycles to match required resolution
    #define CYCLE_B  (2*2000)	// More clock cycles gets wider
    #define CYCLE_C  (2*3000)
    #define CYCLE_D  (2*4000)
    nrf_pwm_values_wave_form_t peculiarSequenceB[] = {
        //   Index     Normal pin          Inverted     (Spare)  Top Value
        //   =====     =================== ==========   =======  =========
        { /*   0  */   0x8000|(CYCLE_A/2), (CYCLE_A/2), 0,       CYCLE_A  },
        { /*   1  */   0x8000|(CYCLE_B/2), (CYCLE_B/2), 0,       CYCLE_B  },
        { /*   2  */   0x8000|(CYCLE_C/2), (CYCLE_C/2), 0,       CYCLE_C  },
        { /*   3  */   0x8000|(CYCLE_D/2), (CYCLE_D/2), 0,       CYCLE_D  },
    };

    //screenshot C of repo
    const uint16_t short_pulse = PWM_COUNTERTOP/10 | (1 << 15);
    static nrf_pwm_values_wave_form_t peculiarSequenceC[] = {
        { /*   0  */   short_pulse,     PWM_COUNTERTOP, PWM_COUNTERTOP, PWM_COUNTERTOP},
        { /*   1  */   PWM_COUNTERTOP,  short_pulse,    PWM_COUNTERTOP, PWM_COUNTERTOP},
        { /*   2  */   PWM_COUNTERTOP,  PWM_COUNTERTOP, short_pulse,    PWM_COUNTERTOP  },
        { /*   3  */   PWM_COUNTERTOP,  PWM_COUNTERTOP, PWM_COUNTERTOP, PWM_COUNTERTOP  },
    };
    //column 1 for channel 1, it's on then off for the rest of the sequence.
    //column 2 for channel 2, , etc.etc.

    nrf_pwm_sequence_t const seq =
    {
        .values.p_individual = &peculiarSequenceC,
        .length              = NRF_PWM_VALUES_LENGTH(peculiarSequenceC),
        .repeats             = 0,
        .end_delay           = 0
    };

	if(!pwm_running){
		pwm_running = true;
		(void)nrfx_pwm_simple_playback(&my_pwm, &seq, 1000, NRFX_PWM_FLAG_LOOP);
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
		k_msleep(SLEEP_TIME_MS);
	}

    return 0;
}