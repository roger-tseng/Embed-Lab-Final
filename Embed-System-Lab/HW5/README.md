# Lab 5: PWM and Logic Analyzer

In this lab, we experiment with the capabilities of the PwmOut API in Mbed for pulse-width modulation (PWM).

First, we naively modify the frequency and duty cycle of our generated PWM signal, and verify the correctness of our signal. 

Second, we attempt to implement sinusoidal pulse-width modulation, a technique often used to generate three-phase sine waves in motor drives.

# Usage
1. Install required software for ZeroPlus Logic Cube Analyzer by following instructions given [here](https://www.youtube.com/watch?v=gULjmHH_nKE)
2. Also follow the tutorial above to connect the logic analyzer to the STM32 board with two wires.
3. Import a new program from [here](https://github.com/ARMmbed/mbed-os-snippet-PwmOut_ex_3). Update Mbed OS version to 6.15.0, and import the [required library for the B-L475E-IOT01 board](https://os.mbed.com/teams/ST/code/BSP_B-L475E-IOT01/).
4. Change the floating point value in ```led.write()``` to generated PWM signals with desired frequency and duty cycle values.
5. Replace ```main.cpp``` with the file of the same name in this repository.
6. Change the library used for ```printf``` in ```targets.json``` under the ```mbed-os/targets``` folder by changing ```printf-lib``` from ```minimal-printf``` to ```std```.
7. Build and run ```main.cpp```, and observe the generated waveform in the logic analyzer.