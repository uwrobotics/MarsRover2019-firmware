/* mbed simple H-bridge motor controller
 * Copyright (c) 2007-2010, sford, http://mbed.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#ifndef MBED_MOTOR_H
#define MBED_MOTOR_H

#ifndef MOTOR_DEFAULT_FREQUENCY_HZ
#define MOTOR_DEFAULT_FREQUENCY_HZ 1000 // 1 kHz
#endif
 
#include "mbed.h"
 
/** Interface to control a standard DC motor 
 *
 * with an H-bridge using a PwmOut and 2 DigitalOuts
 */
class Motor {

public:

    typedef struct {
        PinName pwmPin;
        PinName dirPin;
        bool inverted;

    } t_motorConfig;
 
    /** Create a motor control interface    
     *
     * @param pwm       A PwmOut pin, driving the H-bridge enable line to control the speed
     * @param dir       A DigitalOut, set high when the motor should go forward, low when backwards
     * @param freqInHz  Output PWM frequency, default 1kHz
     * @param inverted  If true, then forward speed will set dir to 0 instead of 1, otherwise inverse
     * @param limit     Maximum speed magnitude
     */
    Motor(PinName pwm, PinName dir, bool inverted = false, int freqInHz = MOTOR_DEFAULT_FREQUENCY_HZ, float limit = 1.0);

    Motor(t_motorConfig motorConfig);

    /** Set the speed of the motor
     * 
     * @param speed The speed of the motor as a normalised value between -1.0 and 1.0
     */
    void setSpeed(float speed);
    Motor& operator=(int speed);

    /** Read the current speed of the motor
     * 
     * @return Current speed of motor
     */
    float getSpeed();
 
protected:
    PwmOut _pwm;
    DigitalOut _dir;
    bool _inverted;
    float _limit;
};
 
#endif
