# rpi-robotics-ws

This is a collection of experiments for Raspberry Pi 5 robotics application.

Project designed to run on a Raspberry Pi 5 in a Docker container. Uses Arduino Nano for motor control and PWM.

controller module contains a method for taking input from a gamepad controller, sending this to an Arduino via serial communication, which then can be used to drive a DC motor using a PWM signal.

lglip-tests contains a series of software timed PWM tests and serial communcation for a gps module (Ardusimple RTK2B).
Primarily using lgpio library: http://abyz.me.uk/lg/index.html
