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

-Automatically falls back to beacon-only motion if the accelerometer fails, and can be set to accelerometer-only if the beacon becomes unreliable.

-Operates a POV display while in spin mode

-Incorporates safety features such as dead-man switch, loss of comms shutdowns, and a watchdog timer.


TODO:

-Finish support for manual trim adjust
