#ifndef MOTOR_H
#define MOTOR_H

enum Motor {
    RIGHT_FRONT,
    LEFT_FRONT,
    RIGHT_BACK,
    LEFT_BACK
};

void initMotorsDefault();
void initMotors(int, int, int, int);

void setPWMMotor(enum Motor, int);


#endif
