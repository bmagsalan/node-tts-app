// Import the necessary modules
// const Gpio = require('pigpio').Gpio;
import { Gpio } from 'pigpio'; // Importing Gpio directly from pigpio


// Define the BYJMotor class
export class BYJMotor{
    constructor(pins = [4, 17, 27, 22], name = "BYJMotorX", motorType = "28BYJ") {
        this.name = name;
        this.motorType = motorType;
        this.stopMotor = false;
        this.pins = pins; // Use passed GPIO pin numbers
        this.initDelay = 1; // Initial delay after GPIO pins initialized but before motor is moved
    }

    motorStop() {
        this.stopMotor = true;
    }

    async motorRun(wait = 0.001, steps = 512, ccwise = false, verbose = false, stepType = "half", initDelay = this.initDelay) {
        if (steps < 0) {
            console.log("Error: Step number must be greater than 0");
            return;
        }

        try {
            this.stopMotor = false;
            for (let pin of this.pins) {
                const gpio = new Gpio(pin, { mode: Gpio.OUTPUT });
                gpio.digitalWrite(0); // Initialize pin state to LOW
            }gpiozero
            await sleep(initDelay * 1000); // Convert seconds to milliseconds

            const stepSequences = {
                "full": [[1, 0, 0, 1], [1, 0, 0, 0], [1, 1, 0, 0], [0, 1, 0, 0], [0, 1, 1, 0], [0, 0, 1, 0], [0, 0, 1, 1], [0, 0, 0, 1]],
                "half": [[1, 0, 0, 0], [1, 1, 0, 0], [0, 1, 0, 0], [0, 1, 1, 0], [0, 0, 1, 0], [0, 0, 1, 1], [0, 0, 0, 1], [1, 0, 0, 1]],
                "wave": [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]]
            };

            const stepSequence = stepSequences[stepType.toLowerCase()];

            if (ccwise) {
                stepSequence.reverse();
            }

            const performSteps = async () => {
                for (let i = 0; i < steps; i++) {
                    if (this.stopMotor) {
                        throw new Error("Motor stopped");
                    }
                    for (let j = 0; j < stepSequence.length; j++) {
                        for (let k = 0; k < this.pins.length; k++) {
                            const gpio = new Gpio(this.pins[k]);
                            gpio.digitalWrite(stepSequence[j][k]);
                        }
                        await sleep(wait * 1000); // Convert seconds to milliseconds
                    }
                }
            };

            await performSteps();

        } catch (error) {
            console.error("Error: Unexpected error during motor run:", error);
        } finally {
            for (let pin of this.pins) {
                const gpio = new Gpio(pin);
                gpio.digitalWrite(0); // Set pin state to LOW
                gpio.mode(Gpio.INPUT); // Reset pin mode to INPUT
            }
        }
    }
}

// Function to sleep for a given number of milliseconds
function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

// // Export the BYJMotor class
// module.exports = BYJMotor;
