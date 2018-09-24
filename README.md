# MeltyHalo
Arduino code to control my MeltyBrain battlebot

This Arduino project is the control code for Halo, the beetleweight Battlebot designed by Spencer Allen and Pierce Jensen.

The full writeup for this project can be found at https://www.swallenhardware.io/battlebots/


Current functionality:

-Can communicate with the controller two-way over serial

-Can drive the brushless ESCs

-Can operate as a ram bot with standard arcade controls

-Can receive and debounce IR beacon pulses

-Can measure rotational speed using the accelerometer

-Can translate in spin mode using beacon, accelerometer, and hybrid sensing

-Uses the accelerometer to account for failures in the beacon, and vice-versa.

-Automatically falls-back to beacon-only motion if the accelerometer fails, and can be set to accelerometer-only if the beacon becomes unreliable.

-Can drive the flicker display for both spin and standard drive modes

-Incorporates safety features such as dead-man and loss of comms shutdowns, and a watchdog timer.


TODO:

-Fix lingering edge cases in translation code math

-Further refine translation code to keep the ESC's happy. They sometimes get mad and restart at present.
