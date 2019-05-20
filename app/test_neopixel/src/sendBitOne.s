    AREA sendBitOne, CODE, READONLY
; Export sendBitOne function location so that C compiler can find it and link
    EXPORT sendBitOne
sendBitOne
;
; LED1 gets value (passed from C compiler in R0)
; On Nucleo, LED1 is on PA_5 (mask: 0x20, 1<<5)
; On Science, LED1 is on PC_0 (mask: 0x00)
; from stm32f091:   #define PERIPH_BASE           ((uint32_t)0x40000000U)     
;					#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x08000000)
;					#define GPIOA_BASE            (AHB2PERIPH_BASE + 0x00000000)
;					#define GPIOC_BASE            (AHB2PERIPH_BASE + 0x00000800)
;					...
;                   __IO uint32_t BSRR;         /*!< GPIO port bit set/reset register,      Address offset: 0x1A */
;                   __IO uint32_t BRR;          /*!< GPIO bit reset register,               Address offset: 0x28 */
; therefore, Port A starts at 0x48000000 and Port C starts at 0x48000800
; Port A, BSRR: 0x4800001A, BRR: 0x48000028; Port C: BSRR: 0x4800081A, BRR: 0x48000828
;  
; Load GPIO Port 1 base address in register R1 
    LDR     R1, =0x48000000 
; Move bit mask in register R2 for bit 18 only
    MOV.W   R2, #0x20   

;set led 1 port bit using GPIO BRR register and mask
	STR   R2, [R1,#0x1A]  ; clear LED1 bit
;clear 1 port bit using GPIO BSRR register and mask
	;STR   R2, [R1,#0x28]  ;  set LED1 bit

; Return to C using link register (Branch indirect using LR - a return)
    BX      LR
    END
 