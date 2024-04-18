#include <iostream>
#include <wiringPi.h>
#include <unistd.h>
#include <atomic>
#include <thread>
#include <string>

class StepperMotor {
private:
    int pins[4];
    int stepDelay;
    std::atomic<bool> stopRequested;
    std::atomic<bool> direction; // false for clockwise, true for counterclockwise

    void stepMotor() {
        int stepSequence[][4] = {
            {1, 0, 0, 0},
            {1, 1, 0, 0},
            {0, 1, 0, 0},
            {0, 1, 1, 0},
            {0, 0, 1, 0},
            {0, 0, 1, 1},
            {0, 0, 0, 1},
            {1, 0, 0, 1}
        };
        int stepCount = sizeof(stepSequence) / sizeof(stepSequence[0]);
        while (!stopRequested) {
            for (int i = 0; i < stepCount && !stopRequested; ++i) {
                int index = direction ? i : (stepCount - 1 - i);
                for (int pin = 0; pin < 4; ++pin) {
                    digitalWrite(pins[pin], stepSequence[index][pin]);
                }
                usleep(stepDelay);
            }
        }
        for (int pin : pins) {  // Ensure all pins are low after stopping
            digitalWrite(pin, LOW);
        }
    }

public:
    StepperMotor(int pin1, int pin2, int pin3, int pin4) : stopRequested(false), direction(false) {
        pins[0] = pin1;
        pins[1] = pin2;
        pins[2] = pin3;
        pins[3] = pin4;
        wiringPiSetupGpio();  // Use Broadcom GPIO pin numbers
        for (int i = 0; i < 4; ++i) {
            pinMode(pins[i], OUTPUT);
            digitalWrite(pins[i], LOW);
        }
        stepDelay = 1000;  // Set a default delay between steps
    }

    void setSpeed(int RPM) {
        stepDelay = 60000000 / (4096 * RPM);
    }

    void start() {
        stopRequested = false;
        std::thread motorThread(&StepperMotor::stepMotor, this);
        motorThread.detach();
    }

    void stop() {
        stopRequested = true;
    }

    void changeDirection(bool newDirection) {
        direction = newDirection;
    }

    ~StepperMotor() {
        stop();
        for (int pin : pins) {
            digitalWrite(pin, LOW);
        }
    }
};

int main() {
    StepperMotor motor1(5, 6, 13, 19); // Motor 1 using GPIO BCM pin numbers
    StepperMotor motor2(14, 15, 18, 23); // Motor 2 using GPIO BCM pin numbers

    motor1.setSpeed(15); // Set speed of motor 1 to 15 RPM
    motor2.setSpeed(15); // Set speed of motor 2 to 15 RPM

    motor1.start();
    motor2.start();

    std::string command;
    std::cout << "Motor Control (commands: '1s', '1t', '1cw', '1ccw', '2s', '2t', '2cw', '2ccw', 'x'):" << std::endl;
    
    while (true) {
        std::cin >> command;
        if (command == "1t") {
            motor1.stop();
        } else if (command == "1s") {
            motor1.start();
        } else if (command == "1cw") {
            motor1.changeDirection(false);
        } else if (command == "1ccw") {
            motor1.changeDirection(true);
        } else if (command == "2t") {
            motor2.stop();
        } else if (command == "2s") {
            motor2.start();
        } else if (command == "2cw") {
            motor2.changeDirection(false);
        } else if (command == "2ccw") {
            motor2.changeDirection(true);
        } else if (command == "x") {
            break;
        }
    }

    return 0;
}
