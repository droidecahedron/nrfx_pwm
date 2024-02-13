# nrfx_pwm

## Overview
Driving GPIO with PWM peripheral, using nrfx. (https://infocenter.nordicsemi.com/topic/ps_nrf5340/pwm.html?cp=4_0_0_6_22)

LEDs chosen for visual feedback. (GPIO 28-31 for PWM chans)

This shows how to use nrfx to interface with the Nordic PWM peripheral, which is quite a handy and unique peripheral since it can also generate arbitrary waveforms. This application can run without CPU intervention, but the main task only switches which sequence is played by the peripheral every `SLEEP_TIME_MS`. Given the low CPU intervention, it would be quite arbitrary to add whatever else you need, such as BLE or other peripherals (like ADC)


## hardware / documentation
- nRF5340DK / [nRF5340 doc](https://infocenter.nordicsemi.com/topic/struct_nrf53/struct/nrf5340.html), [nRF5340DK doc](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fstruct_nrf53%2Fstruct%2Fnrf5340.html)
<img src="https://github.com/droidecahedron/nrf-blueberry/assets/63935881/12612a0e-9f81-4431-8b22-f69704248f89" width=25% height=25%>
<br>
<img src="https://github.com/droidecahedron/nrfx_pwm/assets/63935881/86076c67-dce1-4ebb-aa1a-80e0461b0765" width=30% height=30%>
<img src="https://github.com/droidecahedron/nrfx_pwm/assets/63935881/e9ffa475-7a6f-4d53-829b-03adf76bd0a4" width=40% height=40%>
<br>
Boxed in red is header location if you want to measure the waveforms for yourself.



## Scope Shots
![image](https://github.com/droidecahedron/nrfx_pwm/assets/63935881/783bedd7-c829-4593-9dd4-bd32e9351353)

> Note: Readme C code for sequences is formatted slightly differently to make visualizing the columns for each sequence easier.

### Sequence A
Your run of the mill set-duty-cycle and forget
```c
 static nrf_pwm_values_individual_t sequence_A;          // sequence A is a pretty basic PWM duty cycle.
    sequence_A.channel_0 = PWM_COUNTERTOP / 10 | (1 << 15); // flips really early, inverted polarity
    sequence_A.channel_1 = PWM_COUNTERTOP / 4;
    sequence_A.channel_2 = PWM_COUNTERTOP / 2; // should flip half way through
    sequence_A.channel_3 = PWM_COUNTERTOP / 10;
```
![image](https://github.com/droidecahedron/nrfx_pwm/assets/63935881/4da0ab1c-fee3-4455-a556-9f0a6c835236)

### Sequence B
Stretching resolution windows, with inversion and toggling-off
```c
static nrf_pwm_values_individual_t sequence_B[] = {
        {0x8000 | (CYCLE_A / 2), 0x8000 | (CYCLE_B / 2), 0x8000 | (CYCLE_C / 2), CYCLE_A},
        {(CYCLE_A / 2),          (CYCLE_B / 2),          (CYCLE_C / 2),          CYCLE_B},
        {0,                      0,                      0,                      CYCLE_C},
        {CYCLE_A,                CYCLE_B,                CYCLE_C,                CYCLE_D},
    };
```
![image](https://github.com/droidecahedron/nrfx_pwm/assets/63935881/e2cf8786-9b89-46d3-a0c2-55cae73e3780)

### Sequence C
Time delayed pulses that are off majority of the time (turn off by setting the value to `COUNTERTOP` in the seq)
```c
    const uint16_t short_pulse = PWM_COUNTERTOP / 10 | (1 << 15);
    static nrf_pwm_values_individual_t sequence_C[] = {
        {short_pulse,    PWM_COUNTERTOP, PWM_COUNTERTOP, PWM_COUNTERTOP},
        {PWM_COUNTERTOP, short_pulse,    PWM_COUNTERTOP, PWM_COUNTERTOP},
        {PWM_COUNTERTOP, PWM_COUNTERTOP, short_pulse,    PWM_COUNTERTOP},
        {PWM_COUNTERTOP, PWM_COUNTERTOP, PWM_COUNTERTOP, short_pulse},
    };
```
![image](https://github.com/droidecahedron/nrfx_pwm/assets/63935881/7f51a016-47a0-49bc-8e2e-5cfbf823aefa)

### Sequence D
Time delayed pulses with alternating short and long pulses
```c
    const uint16_t long_pulse = PWM_COUNTERTOP / 2 | (1 << 15);
    static nrf_pwm_values_individual_t sequence_D[] = {
        {short_pulse,    PWM_COUNTERTOP, long_pulse,     PWM_COUNTERTOP},
        {PWM_COUNTERTOP, long_pulse,     PWM_COUNTERTOP, short_pulse},
        {long_pulse,     PWM_COUNTERTOP, short_pulse,    PWM_COUNTERTOP},
        {PWM_COUNTERTOP, short_pulse,    PWM_COUNTERTOP, long_pulse},
    };
```
![image](https://github.com/droidecahedron/nrfx_pwm/assets/63935881/5bfb26a4-c83c-44dd-a5f3-4ded6e655d5a)


# Extra Notes

in `pwm_set_duty_cycle`, there are three sequences for sample outputs that it rotates through.
when using waveform mode you only get 3 output channels instead of 4. If you need an adjustable countertop, use waveform mode.

You'll need to modify `config0` such that `NRF_PWM_PIN_NOT_CONNECTED` is 31 instead for LED4, the `load_mode` to be `NRF_PWM_LOAD_INDIVIDUAL`, and `p_wave_form` can be `p_individual` where you pass sequence.

You can also have multiple PWM instances.


```c
//if you had a single channel, this is an interesting one.
// nrf_pwm_values_wave_form_t peculiarSequenceSingleChannel[] = {
//     //   Index     Normal pin          Inverted     (Spare)  Top Value
//     //   =====     =================== ==========   =======  =========
//     { /*   0  */   0x8000|(CYCLE_A/2), (CYCLE_A/2), 0,       CYCLE_A  },
//     { /*   1  */   0x8000|(CYCLE_B/2), (CYCLE_B/2), 0,       CYCLE_B  },
//     { /*   2  */   0x8000|(CYCLE_C/2), (CYCLE_C/2), 0,       CYCLE_C  },
//     { /*   3  */   0x8000|(CYCLE_D/2), (CYCLE_D/2), 0,       CYCLE_D  },
// };
```
> And transposed if you are using multiple channels. i.e. each column is each channel.

### branches:
`multiple-instance` for distinct frequencies on each i/o. this branch is kind of unfinished because to generate the above screenshots only needed one instance. (thanks @inductivekickback)

`snapshot_version` old version that has you manually change the structs you feed the playback fxn.
