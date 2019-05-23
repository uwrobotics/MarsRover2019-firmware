# Makefile for Mars Rover 2019 custom board firmware apps
###############################################################################
# Can be used to cross-compile compatible apps on different STM32-F091RC boards
# Just run `make` command in same directory to see options.
#
# Example: $ make APP=test_blinky BOARD=nucleo
#          $ make APP=test_blinky BOARD=science
#          $ make APP=arm_lower   BOARD=arm
#
###############################################################################

BUILD_PATH    := build
APPS_PATH	  := app
APP_OUT_PATH  := ../$(BUILD_PATH)/$(APP)
APP_PATH      := ../$(APPS_PATH)/$(APP)
LIB_PATH      := ../lib
USER_LIB_PATH := $(LIB_PATH)/user
CONFIG_PATH   := ../config

COMPILE_FLAGS_TO_TRIGGER_TOUCH := $(BOARD)
TOUCH_ON_COMPILE_FLAGS_CHANGE  := $(CONFIG_PATH)/PinNames.h

###############################################################################

# Cross-platform directory manipulation
ifeq ($(shell echo $$OS),$$OS)
    MAKE_DIR = if not exist "$(1)" mkdir "$(1)"
    RM_DIR = rmdir /S /Q "$(1)"
    RM_FILE_TYPE = del /S /Q "$(1)" "$(2)"

else
    MAKE_DIR = '$(SHELL)' -c "mkdir -p \"$(1)\""
    RM_DIR = '$(SHELL)' -c "rm -rfv \"$(1)\""
    RM_FILE_TYPE = '$(SHELL)' -c "find $(1) -name \"$(2)\" -delete -print"
endif

null :=
space := ${null} ${null}
${space} := ${space}

define \n


endef

# Move to the build directory
ifeq (,$(filter $(BUILD_PATH),$(notdir $(CURDIR))))
.SUFFIXES:
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKETARGET = '$(MAKE)' --no-print-directory -C $(BUILD_PATH) -f '$(mkfile_path)' \
		'SRCDIR=$(CURDIR)' $(MAKECMDGOALS)

.PHONY: $(BUILD_PATH) clean
all:

ifeq ($(filter $(APP), $(patsubst app/%/,%,$(sort $(dir $(wildcard app/*/))))),)
	$(error APP is not set or is not supported. ${\n}Select an app to build with APP=app_name:${\n}${\n}$(subst ${ },${\n},$(patsubst $(APPS_PATH)/%/,%,$(sort $(dir $(wildcard app/*/)))))${\n}${\n})
endif

ifeq ($(filter $(BOARD)," nucleo arm science safety "),)
	$(error BOARD is not set or is not supported. Set BOARD=board_name:${\n}${\n}safety${\n}arm${\n}science${\n}nucleo${\n}${\n}))
endif

	+@$(call MAKE_DIR,$(BUILD_PATH)/$(APP))
	+@$(MAKETARGET)

$(BUILD_PATH): all

Makefile : ;
% :: $(BUILD_PATH) ; :

makefile : ;
% :: $(BUILD_PATH) ; :

clean :
	$(call RM_DIR,$(BUILD_PATH))
	$(call RM_FILE_TYPE,app,*.o)
	$(call RM_FILE_TYPE,app,*.d)
	$(call RM_FILE_TYPE,lib/user,*.d)
	$(call RM_FILE_TYPE,lib/user,*.o)

clean-mbed: 
	$(call RM_FILE_TYPE,lib/mbed,*.d)
	$(call RM_FILE_TYPE,lib/mbed,*.o)

else

# Trick rules into thinking we are in the root, when we are in the bulid dir
VPATH = ..

# Boiler-plate
###############################################################################
# Project settings

PROJECT := $(APP)_$(BOARD)

# Project settings
###############################################################################
# Objects and Paths

APP_SRC_C += $(wildcard $(APP_PATH)/src/*.c)
LIB_SRC_C += $(wildcard $(USER_LIB_PATH)/*/src/*.c)

APP_SRC_CPP += $(wildcard $(APP_PATH)/src/*.cpp)
LIB_SRC_CPP += $(wildcard $(USER_LIB_PATH)/*/src/*.cpp)

APP_INC += -I$(APP_PATH)/inc
LIB_INC += $(addprefix -I,$(wildcard $(USER_LIB_PATH)/*/inc))

SRC_FILES_C   = $(APP_SRC_C)   $(LIB_SRC_C)
SRC_FILES_CPP = $(APP_SRC_CPP) $(LIB_SRC_CPP)

OBJECTS += $(SRC_FILES_C:.c=.o) $(SRC_FILES_CPP:.cpp=.o)

MBED_OBJECTS += $(LIB_PATH)/mbed/cmsis/TARGET_CORTEX_M/mbed_tz_context.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/AnalogIn.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/BusIn.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/BusInOut.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/BusOut.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/CAN.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/Ethernet.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/FlashIAP.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/I2C.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/I2CSlave.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/InterruptIn.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/InterruptManager.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/MbedCRC.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/RawSerial.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/SPI.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/SPISlave.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/Serial.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/SerialBase.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/TableCRC.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/Ticker.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/Timeout.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/Timer.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/TimerEvent.o
MBED_OBJECTS += $(LIB_PATH)/mbed/drivers/UARTSerial.o
MBED_OBJECTS += $(LIB_PATH)/mbed/hal/mbed_critical_section_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/hal/mbed_flash_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/hal/mbed_gpio.o
MBED_OBJECTS += $(LIB_PATH)/mbed/hal/mbed_itm_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/hal/mbed_lp_ticker_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/hal/mbed_lp_ticker_wrapper.o
MBED_OBJECTS += $(LIB_PATH)/mbed/hal/mbed_pinmap_common.o
MBED_OBJECTS += $(LIB_PATH)/mbed/hal/mbed_sleep_manager.o
MBED_OBJECTS += $(LIB_PATH)/mbed/hal/mbed_ticker_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/hal/mbed_us_ticker_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/ATCmdParser.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/CallChain.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/FileBase.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/FileHandle.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/FilePath.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/FileSystemHandle.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/LocalFileSystem.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/Stream.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_alloc_wrappers.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_application.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_assert.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_board.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_critical.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_error.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_error_hist.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_interface.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_mem_trace.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_mktime.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_poll.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_retarget.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_rtc_time.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_sdk_boot.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_semihost_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_stats.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_wait_api_no_rtos.o
MBED_OBJECTS += $(LIB_PATH)/mbed/platform/mbed_wait_api_rtos.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/PeripheralPins.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/device/TOOLCHAIN_GCC_ARM/startup_stm32f091xc.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/device/cmsis_nvic.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/device/system_clock.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/analogin_device.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/analogout_device.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_adc.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_adc_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_can.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_cec.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_comp.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_cortex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_crc.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_crc_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_dac.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_dac_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_dma.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_flash.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_flash_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_gpio.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_i2c.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_i2c_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_i2s.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_irda.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_iwdg.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_pcd.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_pcd_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_pwr.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_pwr_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_rcc.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_rcc_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_rtc.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_rtc_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_smartcard.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_smartcard_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_smbus.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_spi.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_spi_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_tim.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_tim_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_tsc.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_uart.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_uart_ex.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_usart.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_hal_wwdg.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_adc.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_comp.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_crc.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_crs.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_dac.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_dma.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_exti.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_gpio.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_i2c.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_pwr.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_rcc.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_rtc.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_spi.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_tim.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_usart.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/stm32f0xx_ll_utils.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device/system_stm32f0xx.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/flash_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/gpio_irq_device.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/pwmout_device.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/serial_device.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/spi_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/analogin_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/analogout_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/can_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/gpio_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/gpio_irq_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/hal_tick_overrides.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/i2c_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/lp_ticker.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/mbed_crc_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/mbed_overrides.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/pinmap.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/port_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/pwmout_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/rtc_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/serial_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/sleep.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/stm_spi_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/trng_api.o
MBED_OBJECTS += $(LIB_PATH)/mbed/targets/TARGET_STM/us_ticker.o

MBED_INC += -I$(LIB_PATH)/mbed
MBED_INC += -I$(LIB_PATH)/mbed/cmsis
MBED_INC += -I$(LIB_PATH)/mbed/cmsis/TARGET_CORTEX_M
MBED_INC += -I$(LIB_PATH)/mbed/drivers
MBED_INC += -I$(LIB_PATH)/mbed/hal
MBED_INC += -I$(LIB_PATH)/mbed/hal/storage_abstraction
MBED_INC += -I$(LIB_PATH)/mbed/platform
MBED_INC += -I$(LIB_PATH)/mbed/targets/TARGET_STM
MBED_INC += -I$(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0
MBED_INC += -I$(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC
MBED_INC += -I$(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/device
MBED_INC += -I$(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/device

INCLUDE_PATHS += -I$(CONFIG_PATH)
INCLUDE_PATHS += -I$(LIB_PATH)
INCLUDE_PATHS += $(APP_INC) $(LIB_INC) $(MBED_INC)

LIBRARY_PATHS += -L$(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/device/TOOLCHAIN_GCC_ARM
LIBRARIES += 
LINKER_SCRIPT ?= $(LIB_PATH)/mbed/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/device/TOOLCHAIN_GCC_ARM/STM32F091XC.ld

# Objects and Paths
###############################################################################
# Tools and Flags

AS      := arm-none-eabi-gcc
CC      := arm-none-eabi-gcc
CPP     := arm-none-eabi-g++
LD      := arm-none-eabi-gcc
ELF2BIN := arm-none-eabi-objcopy

PREPROC := arm-none-eabi-cpp -E -P -Wl,--gc-sections -Wl,--wrap,main -Wl,--wrap,_malloc_r \
		   -Wl,--wrap,_free_r -Wl,--wrap,_realloc_r -Wl,--wrap,_memalign_r -Wl,--wrap,_calloc_r \
		   -Wl,--wrap,exit -Wl,--wrap,atexit -Wl,-n -mcpu=cortex-m0 -mthumb

ifeq ($(BOARD),nucleo)
	COMMON_FLAGS += -DNUCLEO_PINMAP
else ifeq ($(BOARD),arm)
	COMMON_FLAGS += -DROVERBOARD_ARM_PINMAP
else ifeq ($(BOARD),science)
	COMMON_FLAGS += -DROVERBOARD_SCIENCE_PINMAP
else ifeq ($(BOARD),safety)
	COMMON_FLAGS += -DROVERBOARD_SAFETY_PINMAP
endif

COMMON_FLAGS += -include
COMMON_FLAGS += ../$(CONFIG_PATH)/mbed_config.h
COMMON_FLAGS += -c
COMMON_FLAGS += -Wall
COMMON_FLAGS += -Wextra
COMMON_FLAGS += -Wno-unused-parameter
COMMON_FLAGS += -Wno-missing-field-initializers
COMMON_FLAGS += -fmessage-length=0
COMMON_FLAGS += -fno-exceptions
COMMON_FLAGS += -fno-builtin
COMMON_FLAGS += -ffunction-sections
COMMON_FLAGS += -fdata-sections
COMMON_FLAGS += -funsigned-char
COMMON_FLAGS += -MMD
COMMON_FLAGS += -fno-delete-null-pointer-checks
COMMON_FLAGS += -fomit-frame-pointer
COMMON_FLAGS += -Os
COMMON_FLAGS += -g1
COMMON_FLAGS += -mcpu=cortex-m0
COMMON_FLAGS += -mthumb

C_FLAGS += -std=gnu99
C_FLAGS += $(COMMON_FLAGS)

CXX_FLAGS += -std=gnu++98
CXX_FLAGS += -fno-rtti
CXX_FLAGS += -Wvla
CXX_FLAGS += $(COMMON_FLAGS)

ASM_FLAGS += -x
ASM_FLAGS += assembler-with-cpp
ASM_FLAGS += $(INCLUDE_PATHS)
ASM_FLAGS += $(COMMON_FLAGS)


LD_FLAGS := -Wl,--gc-sections -Wl,--wrap,main -Wl,--wrap,_malloc_r -Wl,--wrap,_free_r \
            -Wl,--wrap,_realloc_r -Wl,--wrap,_memalign_r -Wl,--wrap,_calloc_r -Wl,--wrap,exit \
            -Wl,--wrap,atexit -Wl,-n -mcpu=cortex-m0 -mthumb 

LD_SYS_LIBS := -Wl,--start-group -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys  -Wl,--end-group

# Tools and Flags
###############################################################################
# Rules

.PHONY: all

all: compile_flags $(APP_OUT_PATH)/$(PROJECT).bin # $(APP_OUT_PATH)/$(PROJECT).hex

.PHONY: force

ifneq($(OS),Windows_NT)
compile_flags: force
	+@echo $(COMPILE_FLAGS_TO_TRIGGER_TOUCH) | cmp -s - $@ && echo "Compile flags unmodified: No flag dependent files to recompile." \
		|| (echo $(COMPILE_FLAGS_TO_TRIGGER_TOUCH) > $@ && touch $(TOUCH_ON_COMPILE_FLAGS_CHANGE))
endif

.s.o:
	+@$(call MAKE_DIR,$(dir $@))
	+@echo "Assemble: $(notdir $<)"
	@$(AS) -c $(ASM_FLAGS) -o $@ $<

.S.o:
	+@$(call MAKE_DIR,$(dir $@))
	+@echo "Assemble: $(notdir $<)"
	@$(AS) -c $(ASM_FLAGS) -o $@ $<

.c.o:
	+@$(call MAKE_DIR,$(dir $@))
	+@echo "Compile: $(notdir $<)"
	@$(CC) $(C_FLAGS) $(INCLUDE_PATHS) -o $@ $<

.cpp.o:
	+@$(call MAKE_DIR,$(dir $@))
	+@echo "Compile: $(notdir $<)"
	@$(CPP) $(CXX_FLAGS) $(INCLUDE_PATHS) -o $@ $<

$(APP_OUT_PATH)/$(PROJECT).link_script.ld: $(LINKER_SCRIPT)
	@$(PREPROC) $< -o $@

$(APP_OUT_PATH)/$(PROJECT).elf: $(OBJECTS) $(MBED_OBJECTS) $(APP_OUT_PATH)/$(PROJECT).link_script.ld 
	+@echo "link: $(notdir $@) \n"
	@$(LD) $(LD_FLAGS) -T $(filter-out %.o, $^) $(LIBRARY_PATHS) --output $@ $(filter %.o, $^) $(LIBRARIES) $(LD_SYS_LIBS)

$(APP_OUT_PATH)/$(PROJECT).bin: $(APP_OUT_PATH)/$(PROJECT).elf
	$(ELF2BIN) -O binary $< $@
	+@echo "===== bin file ready to flash: $@ ====="

# $(APP_OUT_PATH)/$(PROJECT).hex: $(APP_OUT_PATH)/$(PROJECT).elf
# 	$(ELF2BIN) -O ihex $< $@


# Rules
###############################################################################
# Dependencies

DEPS = $(OBJECTS:.o=.d) $(MBED_OBJECTS:.o=.d)
-include $(DEPS)

endif
