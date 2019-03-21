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

#include "PwmIn.h"

PwmIn::PwmIn(PinName pwmSense, int numSamplesToAverage) : _pwmSense(pwmSense), _numSamplesToAverage(numSamplesToAverage) {
    _pwmSense.rise(callback(this, &PwmIn::rise));
    _pwmSense.fall(callback(this, &PwmIn::fall));

    _period = 0.0;
    _pulseWidth = 0.0;
    _periodSampleSum = 0.0;
    _pulseWidthSampleSum = 0.0;
    _sampleCount = 0;

    _periodSamples = new float[_numSamplesToAverage]();
    _pulseWidthSamples = new float[_numSamplesToAverage]();
    
    _timer.start();
}

PwmIn::~PwmIn() {
    delete [] _pulseWidthSamples;
    delete [] _periodSamples;
}

float PwmIn::period() {
    return _period;
}

float PwmIn::avgPeriod() {
    return _avgPeriod;
}

float PwmIn::pulseWidth() {
    return _pulseWidth;
}

float PwmIn::avgPulseWidth() {
    return _avgPulseWidth;
}

float PwmIn::dutyCycle() {
    return _pulseWidth / _period;
}

float PwmIn::avgDutyCycle() {
    return _avgPulseWidth / _avgPeriod;
}

void PwmIn::rise() {
    _period = _timer.read();
    _timer.reset();

    _avgPeriod = PwmIn::movingAvg(_periodSamples, &_periodSampleSum, _period, _sampleCount);
}

void PwmIn::fall() {
    _pulseWidth = _timer.read();

    _avgPulseWidth = PwmIn::movingAvg(_pulseWidthSamples, &_pulseWidthSampleSum, _pulseWidth, _sampleCount);

    _sampleCount++;

    if (_sampleCount >= _numSamplesToAverage) {
        _sampleCount = 0;
    }
}

float PwmIn::movingAvg(float * p_samples, float * p_sampleSum, float newSample, int newIndex) {
    *p_sampleSum -= p_samples[newIndex];
    p_samples[newIndex] = newSample;
    *p_sampleSum += p_samples[newIndex];

    return *p_sampleSum / (float) _numSamplesToAverage;
}
