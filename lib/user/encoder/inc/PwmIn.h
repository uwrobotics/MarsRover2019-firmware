/* mbed PwmIn Library
 * Copyright (c) 2008-2010, sford
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

#ifndef MBED_PWMIN_H
#define MBED_PWMIN_H

#ifndef PWM_IN_AVERAGE_COUNT_DEFAULT
#define PWM_IN_AVERAGE_COUNT_DEFAULT 4
#endif

#include "mbed.h"

/** PwmIn class to read PWM inputs
 * 
 * Uses InterruptIn to measure the changes on the input
 * and record the time they occur
 *
 * @note uses InterruptIn, so not available on p19/p20
 */
class PwmIn {
public:
    /** Create a PwmIn with the default number of pulses for averaging
     *
     * @param pwmSense The pwm input pin (must support InterruptIn)
     */ 
    PwmIn(PinName pwmSense);

    /** Create a PwmIn with a specified number of pulses to average
     *
     * @param pwmSense           The pwm input pin (must support InterruptIn)
     * @param pulsesToAverage The number of PWM measurements to sum before averaging
     */ 
    PwmIn(PinName pwmSense, int pulsesToAverage);
    
    /** Read the current period
     *
     * @returns the period in seconds
     */
    float period();

    /** Read the average period
     *
     * @returns the average period in seconds
     */
    float avgPeriod();
    
    /** Read the current pulse width
     *
     * @returns the pulsewidth in seconds
     */
    float pulseWidth();

    /** Read the average pulse width
     *
     * @returns the average pulsewidth in seconds
     */
    float avgPulseWidth();
    
    /** Read the current duty cycle
     *
     * @returns the duty cycle as a percentage, represented between 0.0-1.0
     */
    float dutyCycle();

    /** Read the average duty cycle
     *
     * @returns the average duty cycle as a percentage, represented between 0.0-1.0
     */
    float avgDutyCycle();

protected:        
    void rise();
    void fall();
    
    InterruptIn _pwmSense;
    Timer _timer;

    float _pulseWidth, _period;
    float _avgPulseWidth, _avgPeriod;
    double _sumPulseWidth, _sumPeriod;
    
    int _pulseCount;
    int _pulsesToAverage; 
};

#endif
