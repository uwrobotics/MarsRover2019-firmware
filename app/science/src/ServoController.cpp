// Controller for the auger drill

#include "ServoController.h"
#include "mbed.h"
#include "Servo.h"
#include "PinNames.h"

ServoController::ServoController(ServoController::t_servoConfig servoConfig) :
    m_servoConfig(servoConfig), m_funnelServo(servoConfig.funnelServoPin) {

    m_isFunnelOpen = false;
}

void ServoController::setFunnelRest() {
    m_funnelServo = m_servoConfig.funnelRestPos;
    m_ticker.detach();
}

void ServoController::setFunnelUp() {
    m_funnelServo = m_servoConfig.funnelUpPos;
    m_ticker.attach(callback(this, &ServoController::setFunnelRest), 1.0);
    m_isFunnelOpen = false;
}

void ServoController::setFunnelDown() {
    m_funnelServo = m_servoConfig.funnelDownPos;
    m_ticker.attach(callback(this, &ServoController::setFunnelRest), 1.0);
    m_isFunnelOpen = true;
}

bool ServoController::isFunnelOpen(void) {
    return m_isFunnelOpen;
}
