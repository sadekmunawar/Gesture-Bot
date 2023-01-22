# Gesture-Bot
This project uses two ATmega328P microcontrollers. One controls the actuation of the robot, and the other detects gesttures and transmits commands.

## Hardware
* ATmega328P (2x)
* Accelerometer (ADXL335)
* RF Module (NRF24L01)


## Software
transmit.c runs on the ATmega328P that sends control-data to the robot.  

receive_control.c runs on the ATmega328P that is on the robot. It controls the wheels and the arm.  receive_control.c also detects objects with an ultrasonic sensor.

Both files rely on the "nrf24.h" library for wireless communication. Here is link to the source of the library: https://github.com/swharden/AVR-projects/tree/master/ATMega328%202018-02-25%20nrf24L01

![r1](https://user-images.githubusercontent.com/76756708/213895349-ca5b2ab2-6976-4b78-af26-893cebf7fab2.jpg)

### Devpost link: https://devpost.com/software/collision-proof-vehicle
### Presentation link: https://www.youtube.com/watch?v=7XA9kC1BxIw
