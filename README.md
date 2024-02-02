# nrfx_pwm

Driving GPIO with PWM peripheral, using nrfx.
LEDs chosen for visual feedback.

in pwm_set_duty_cycle, there are three sequences labelled screenshot A, screenshot B, and screenshot C for some sample outputs.
when using waveform mode you only get 3 output channels instead of 4.

## screenshot A
one instance, 4 outputs, varying duty cycles
![image](https://github.com/droidecahedron/nrfx_pwm/assets/63935881/fae9f85b-38fb-4c3f-90cf-1ec1746f04f2)

## screenshot B
one instance, 3 outputs, using waveform mode.

## screenshot C
once instance, 3 outputs, same short pulse but delayed by a certain amount. (depends on count top, period, clk)
![image](https://github.com/droidecahedron/nrfx_pwm/assets/63935881/12fb30f4-b159-4d97-a498-86d6ad50e4b8)


### branch: multiple-instance
multiple instances for distinct frequencies on each i/o. this branch is kind of unfinished because to generate the above screenshots only needed one instance.
