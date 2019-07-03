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

#ifndef PWM_IN_DEFAULT_NUM_SAMPLES_TO_AVERAGE
#define PWM_IN_DEFAULT_NUM_SAMPLES_TO_AVERAGE 12
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

    typedef struct {
        PinName pwmPin;
        float zeroAngleDutyCycle;
        float minAngleDegrees;
        float maxAngleDegrees;
        bool inverted;
    } t_absoluteEncoderConfig;

    /** Create a PwmIn with a specified number of pulses to average
     *
     * @param pwmSense           The pwm input pin (must support InterruptIn)
     * @param numSamplesToAverage The number of PWM measurements to sum before averaging
     */ 
    PwmIn(PinName pwmSense, int numSamplesToAverage = PWM_IN_DEFAULT_NUM_SAMPLES_TO_AVERAGE);

    ~PwmIn();
    
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

    /** Read the average duty cycle velocity
     *
     * @returns the average duty cycle velocity as a 0.0-1.0 percentage/second
     */
    float avgDutyCycleVelocity();

protected:
    
    InterruptIn _pwmSense;
    Timer _timer;

    float _pulseWidth, _period;
    float _avgPulseWidth, _avgPeriod, _avgDutyCycle, _prevAvgDutyCycle, _avgDutyCycleVelocity;
    
    int _sampleCount;
    int _numSamplesToAverage; 

    float * _p_pulseWidthSamples;
    float * _p_periodSamples;

    float _pulseWidthSampleSum;
    float _periodSampleSum;

    void rise();
    void fall();
    float movingAvg(float * p_samples, float * p_sampleSum, float newSample, int newIndex);

};

#endif
