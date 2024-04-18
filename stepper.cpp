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

    void stepMotor(int totalSteps, bool ccw, int stepDelayMicros, const std::string& stepMode) {
        int halfStepSequence[][4] = {
            {1, 0, 0, 0},
            {1, 1, 0, 0},
            {0, 1, 0, 0},
            {0, 1, 1, 0},
            {0, 0, 1, 0},
            {0, 0, 1, 1},
            {0, 0, 0, 1},
            {1, 0, 0, 1}
        };
        int fullStepSequence[][4] = {
            {1, 1, 0, 0},
            {0, 1, 1, 0},
            {0, 0, 1, 1},
            {1, 0, 0, 1}
        };
        int waveStepSequence[][4] = {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
        };

        int(*stepSequence)[4];
        int stepCount;

        if (stepMode == "full") {
            stepSequence = fullStepSequence;
            stepCount = sizeof(fullStepSequence) / sizeof(fullStepSequence[0]);
        } else if (stepMode == "wave") {
            stepSequence = waveStepSequence;
            stepCount = sizeof(waveStepSequence) / sizeof(waveStepSequence[0]);
        } else { // Default to half-step if anything else
            stepSequence = halfStepSequence;
            stepCount = sizeof(halfStepSequence) / sizeof(halfStepSequence[0]);
        }

        for (int i = 0; i < totalSteps && !stopRequested; ++i) {
            int index = ccw ? i % stepCount : (stepCount - 1 - i % stepCount);
            for (int pin = 0; pin < 4; ++pin) {
                digitalWrite(pins[pin], stepSequence[index][pin]);
            }
            usleep(stepDelayMicros);
        }
        for (int pin : pins) {  // Ensure all pins are low after stopping
            digitalWrite(pin, LOW);
        }
    }

public:
    StepperMotor(int pin1, int pin2, int pin3, int pin4) : stopRequested(false) {
        pins[0] = pin1;
        pins[1] = pin2;
        pins[2] = pin3;
        pins[3] = pin4;
        wiringPiSetupGpio();  // Use Broadcom GPIO pin numbers
        for (int i = 0; i < 4; ++i) {
            pinMode(pins[i], OUTPUT);
            digitalWrite(pins[i], LOW);
        }
    }

    void start(int steps, bool ccw, int RPM, const std::string& stepMode) {
        stopRequested = false;
        int delayMicros = 60000000 / (4096 * RPM); // Convert RPM to delay in microseconds
        std::thread motorThread(&StepperMotor::stepMotor, this, steps, ccw, delayMicros, stepMode);
        motorThread.detach();
    }

    void stop() {
        stopRequested = true;
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

    std::string command;
    int steps, speed;
    bool ccw;
    std::string stepMode;
    std::cout << "Motor Control (commands: '1 start [steps] [cw/ccw] [speed] [full/half/wave]', '1 stop', '2 start [steps] [cw/ccw] [speed] [full/half/wave]', '2 stop', 'exit'):" << std::endl;

    while (true) {
        std::cin >> command;
        if (command == "1") {
            std::cin >> command;
            if (command == "start") {
                std::cin >> steps >> command >> speed >> stepMode;
                ccw = (command == "ccw");
                motor1.start(steps, ccw, speed, stepMode);
            } else if (command == "stop") {
                motor1.stop();
            }
        } else if (command == "2") {
            std::cin >> command;
            if (command == "start") {
                std::cin >> steps >> command >> speed >> stepMode;
                ccw = (command == "ccw");
                motor2.start(steps, ccw, speed, stepMode);
            } else if (command == "stop") {
                motor2.stop();
            }
        } else if (command == "exit") {
            break;
        }
    }

    return 0;
}
