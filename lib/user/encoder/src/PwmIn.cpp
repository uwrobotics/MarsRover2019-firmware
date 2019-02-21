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

PwmIn::PwmIn(PinName pwmSense) : _pwmSense(pwmSense) {
    _pwmSense.rise(callback(this, &PwmIn::rise));
    _pwmSense.fall(callback(this, &PwmIn::fall));

    _period = 0.0;
    _pulseWidth = 0.0;
    _pulsesToAverage = PWM_IN_AVERAGE_COUNT_DEFAULT;

    _timer.start();
}

PwmIn::PwmIn(PinName pwmSense, int pulsesToAverage) : _pwmSense(pwmSense), _pulsesToAverage(pulsesToAverage) {
    _pwmSense.rise(callback(this, &PwmIn::rise));
    _pwmSense.fall(callback(this, &PwmIn::fall));

    _period = 0.0;
    _pulseWidth = 0.0;
    
    _timer.start();
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
    _sumPeriod += _period; 
    _timer.reset();
}

void PwmIn::fall() {
    _pulseWidth = _timer.read();
    _sumPulseWidth += _pulseWidth;

    // If the number of pulses to average has been reached, calculate the averages
    if (_pulseCount >= _pulsesToAverage) {
        _avgPeriod = _sumPeriod / (float) _pulsesToAverage;
        _avgPulseWidth = _sumPulseWidth / (float) _pulsesToAverage;

        _sumPeriod = 0.0;
        _sumPulseWidth = 0.0;
        _pulseCount = 0;
    }

    _pulseCount++;
}
