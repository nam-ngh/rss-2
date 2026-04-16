# Robotic IR Communication Experiments

Contains Arduino code for experimenting Infra-Red (IR) communications between 2 Pololu 3Pi+ robots. 

We aim for the transmitting robot (Tx) to repeatedly send sequences of integers from 0 to 20 as bytes, by physically switching its IR emitters & off. The receiving robot (Rx) meanwhile tries to detect IR signals from its line sensors reading and decode the binary.

Each experiment consists of 10 trial runs, each involves sending and receiving the same message sequence (0-20) and the accuracy of the message decoded / number of bits dropped across runs are observed.

## To run

- Via the Arduino IDE, Upload Tx.ino + all .h files to one Pololu 3Pi+ robot. Upload Rx.ino + .h files to the other.
- Place 2 robots face to face at a distance of choice (usually up to 20cm apart)
- Open Serial Monitor, turn on the Rx robot first and wait for calibration finish
- Turn on the Tx robot to start sending messages

By the end of all trial runs, accuracy/error rates can be observed from Serial to assess the reliability of IR communication.
