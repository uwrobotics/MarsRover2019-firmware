#ifndef MOIST_H
#define MOIST_H

#include "mbed.h"

void moist_init(void);
float moist_read(void);
unsigned short moist_read_u16(void);

#endif // MOIST_H