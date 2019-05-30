/******************************************************************//**
* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
**********************************************************************/

// ow_usdelay configuration
//
// Make PROC_CLOCK_MHZ and OVERHEAD_TUNING divisible by PROC_CYCLES_PER_LOOP
// for best results
//
// Processor clock in MHz
#define PROC_CLOCK_MHZ (__SYSTEM_HFX / 1000000)
//
// Fraction where OverheadTime(us) = OVERHEAD_TUNING / PROC_CLOCK_MHZ
#define OVERHEAD_TUNING 18
//
// ARM specifies 1-3 cycles for pipeline refill following a branch
#define PIPELINE_REFILL_PROC_CYCLES 1

// ow_usdelay constants
#define PROC_CYCLES_PER_LOOP (2 + PIPELINE_REFILL_PROC_CYCLES)
#define LOOPS_PER_US (PROC_CLOCK_MHZ / PROC_CYCLES_PER_LOOP)
#define LOOPS_REMOVED_TUNING (OVERHEAD_TUNING / PROC_CYCLES_PER_LOOP)

// OneWireTiming offsets
#define tRSTL_OFFSET    0
#define tMSP_OFFSET     2
#define tW0L_OFFSET     4
#define tW1L_OFFSET     6
#define tMSR_OFFSET     8
#define tSLOT_OFFSET   10

// Define a code section
  .syntax unified
  .section .text
// void ow_usdelay(unsigned int time_us)
  .thumb_func
  .global ow_usdelay
ow_usdelay:
  cmp R0, #0 // Return if time_us equals zero
  beq ow_usdelay_return
  mov R2, #LOOPS_PER_US
  mul R0, R0, R2
  sub R0, R0, #LOOPS_REMOVED_TUNING
loop:
  subs R0, R0, #1
  bne loop
ow_usdelay_return:
  bx R14
  
// void write_ow_gpio_low(volatile uint32_t * portReg, unsigned int pinMask)
.macro write_ow_gpio_low
  ldr R2, [R0]
  bic R2, R2, R1
  str R2, [R0]
  .endm
  
// void write_ow_gpio_high(volatile uint32_t * portReg, unsigned int pinMask)
.macro write_ow_gpio_high
  ldr R2, [R0]
  orr R2, R2, R1
  str R2, [R0]
  .endm
  
// void ow_bit(uint8_t * sendRecvBit, const volatile uint32_t * inReg,
//             volatile uint32_t * outReg, unsigned int pinMask,
//             const OneWireTiming * timing)
  .thumb_func
  .global ow_bit
ow_bit:
  push {R4-R8, R14}
  // Retrive extra parameters from stack
  add R6, SP, #24 // Find beginning of stack: 6 scratch registers * 4 bytes each
  ldr R6, [R6] // Load timing struct
  ldrh R4, [R6, #tSLOT_OFFSET]
  ldrh R5, [R6, #tMSR_OFFSET]
  // R0: sendRecvBit
  // R1: inReg
  // R2: outReg
  // R3: pinMask
  // R4: tSLOT
  // R5: tMSR
  // R6: timing
  // R7: Scratch
  // R8: Scratch
  // R14: Scratch
  
  // Reorganize registers for upcoming function calls
  mov R8, R1 // inReg to R8
  mov R7, R2 // outReg to R7
  mov R1, R3 // pinMask to R1
  mov R3, R0 // sendRecvBit to R3
  // R0: Scratch
  // R1: pinMask
  // R2: Scratch
  // R3: sendRecvBit
  // R4: tSLOT
  // R5: tMSR
  // R6: timing
  // R7: outReg
  // R8: inReg
  // R14: Scratch
  
  // if (*sendRecvBit & 1)
  ldrb R14, [R3]
  tst R14, #1
  beq write_zero
  ldrh R6, [R6, #tW1L_OFFSET] // tW1L
  sub R4, R4, R5 // tREC = tSLOT - tMSR
  sub R5, R5, R6 // delay2 = tMSR - tLW1L
  // R0: Scratch
  // R1: pinMask
  // R2: Scratch
  // R3: sendRecvBit
  // R4: tREC
  // R5: delay2
  // R6: tW1L
  // R7: outReg
  // R8: inReg
  // R14: Scratch
  mov R0, R7 // outReg
  write_ow_gpio_low // Pull low
  mov R0, R6 // tLOW
  bl ow_usdelay // Delay for tLOW
  mov R0, R7 // outReg
  write_ow_gpio_high // Release pin
  mov R0, R5 // delay2
  bl ow_usdelay // Delay for sample time
  ldr R5, [R8] // Read *inReg
  b recovery_delay
  // else
write_zero:
  ldrh R6, [R6, #tW0L_OFFSET] // tW0L
  sub R4, R4, R6 // tREC = tSLOT - tLW0L
  sub R6, R6, R5 // delay2 = tW0L - tMSR
  // R0: Scratch
  // R1: pinMask
  // R2: Scratch
  // R3: sendRecvBit
  // R4: tREC
  // R5: tMSR
  // R6: delay2
  // R7: outReg
  // R8: inReg
  // R14: Scratch
  mov R0, R7 // outReg
  write_ow_gpio_low // Pull low
  mov R0, R5 // tMSR
  bl ow_usdelay // Delay for tMSR
  ldr R5, [R8] // Read *inReg
  mov R0, R6 // delay2
  bl ow_usdelay // Delay for release
  mov R0, R7 // outReg
  write_ow_gpio_high // Release pin
  // endif (*sendRecvBit & 1)
  // R0: Scratch
  // R1: pinMask
  // R2: Scratch
  // R3: sendRecvBit
  // R4: tREC
  // R5: *inReg
  // R6: Scratch
  // R7: outReg
  // R8: inReg
  // R14: Scratch
  
recovery_delay:
  mov R0, R4
  bl ow_usdelay // Delay for tREC
  
  // Parse received bit
  // *sendRecvBit = ((*inReg & pinMask) == pinMask)
  and R5, R5, R1
  cmp R5, R1
  ite eq
  moveq R5, #1
  movne R5, #0
  strb R5, [R3]
  
  pop {R4-R8, R14}
  bx R14
  .end
  