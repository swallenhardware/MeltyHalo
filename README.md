# MeltyHalo
Arduino code to control my MeltyBrain battlebot

This Arduino project is the control code for Halo, the beetleweight Battlebot designed by Spencer Allen and Pierce Jensen.

The full writeup for this project can be found at https://www.swallenhardware.io/battlebots/

Current functionality:
-Can communicate with the controller one-way over serial
-Can drive the brushless ESCs
-Can operate as a ram bot with standard arcade controls
-Can receive and debounce IR beacon pulses
-Can translate in spin mode using beacon-based sensing
-Can drive the flicker display for both spin and standard drive modes
-Incorporates safety features such as dead-man and loss of comms shutdowns, and a watchdog timer.

TODO:
-Fix the non-blocking I2C communication to the accelerometer
-Add the second-order accelerometer algorithm
-Add accelerometer "sanity checking" to the beacon sense to mitigate false triggers
-Rewrite the flicker display code to use a linked-list of structs
-Add the font table for spin display
-Add return communications back to the controller to report battery voltage and status
