ArduinoBMP085
=============

A class to use the BMP085 barrometric pressure sensor for Arduino-style microcontrollers.


This class is in use on a Teensy3 using nox771's Teensy3-specific adaption of TwoWire.
If this is to be run on an Arduino, it should be sufficient to change the name of the file in the #include directives.

TODO:
Make this automatically check for the platform-specific i2c implementations.
Add support for the interrupt pin.


