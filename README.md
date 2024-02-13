# nrfx_pwm

## hardware / documentation
- nRF5340DK / [nRF5340 doc](https://infocenter.nordicsemi.com/topic/struct_nrf53/struct/nrf5340.html), [nRF5340DK doc](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fstruct_nrf53%2Fstruct%2Fnrf5340.html)
<img src="https://github.com/droidecahedron/nrf-blueberry/assets/63935881/12612a0e-9f81-4431-8b22-f69704248f89" width=25% height=25%>

Driving GPIO with PWM peripheral, using nrfx. (https://infocenter.nordicsemi.com/topic/ps_nrf5340/pwm.html?cp=4_0_0_6_22)
LEDs chosen for visual feedback.

in `pwm_set_duty_cycle`, there are three sequences labelled screenshot A, screenshot B, and screenshot C for some sample outputs.
when using waveform mode you only get 3 output channels instead of 4.

You just change some of what you pass in that function (all commented) and you can get the below outputs.

> You can address four pwm channels if you don't need to adjust countertop

## screenshot A
one instance, 4 outputs, varying duty cycles
![image](https://github.com/droidecahedron/nrfx_pwm/assets/63935881/fae9f85b-38fb-4c3f-90cf-1ec1746f04f2)
You'll need to modify `config0` such that `NRF_PWM_PIN_NOT_CONNECTED` is 31 instead for LED4, the `load_mode` to be `NRF_PWM_LOAD_INDIVIDUAL`, and `p_wave_form` can be `p_individual` where you pass sequence.

## screenshot B
one instance, 3 outputs, using waveform mode.
![image](https://github.com/droidecahedron/nrfx_pwm/assets/63935881/7df6ae4f-cbe1-45a2-a41e-7b9e5412b2cf)


## screenshot C
once instance, 3 outputs, same short pulse but delayed by a certain amount. (depends on count top, period, clk)
![image](https://github.com/droidecahedron/nrfx_pwm/assets/63935881/12fb30f4-b159-4d97-a498-86d6ad50e4b8)


### branch: multiple-instance
multiple instances for distinct frequencies on each i/o. this branch is kind of unfinished because to generate the above screenshots only needed one instance. (thanks @inductivekickback)
