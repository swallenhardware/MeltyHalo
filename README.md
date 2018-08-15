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

-Can drive the flicker display for both spin and standard drive modes

-Incorporates safety features such as dead-man and loss of comms shutdowns, and a watchdog timer.


TODO:

-Add accelerometer "sanity checking" to the beacon sense to mitigate false triggers

-Add support for spin direction switching

-Add auto orientation detection

-Add the font table for spin display
