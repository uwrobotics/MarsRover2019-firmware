# Mars Rover 2019 Firmware

## Platform: [STM32F091](https://www.st.com/resource/en/datasheet/stm32f091rc.pdf) / [NUCLEO-F091RC](https://os.mbed.com/platforms/ST-Nucleo-F091RC/)

Firmware for the 2019 University of Waterloo Mars Rover. Contains:
- Arm MBED SDK source ([lib/mbed](https://github.com/uwrobotics/MarsRover2019-firmware/tree/master/lib/mbed))
- additional external libaries ([lib/user](https://github.com/uwrobotics/MarsRover2019-firmware/tree/master/lib/user))
- additional libraries written by the team ([lib/user](https://github.com/uwrobotics/MarsRover2019-firmware/tree/master/lib/user))
- test applications for testing code components ([app/test_xxxx](https://github.com/uwrobotics/MarsRover2019-firmware/tree/master/app))
- applications for running on each board ([app](https://github.com/uwrobotics/MarsRover2019-firmware/tree/master/app))

## Best Contribution Practices and Tips

- Create a branch in the format `yourName/featureName` for every feature you are working on
- Rebase onto master and test on hardware before merging into master
- Create a pull request to merge any branch into master and select everyone else working on the feature as reviewers
  - Name the pull request `issueNumber: FeatureTitle`
- When merging a pull request that fixes an issue title the commit `Fixes #issueNumber: FeatureTitle`
  - Adding `Fixes #1: ...` will automatically closes the associated issue, in this example issue #1
- Clean binaries between making changes to the makefile
- There seems to be an annoying mix of CamelCase and snake_case in MBED but just try to be consistent with whatever code is nearby
- Squash when merging pull requests

## UWRT Firmware Development Instructions

1. Download source code 
    
    `git clone https://github.com/uwrobotics/MarsRover2019-firmware.git`

2. Download toolchain (gcc and make)
   
   For Ubuntu 16.04
    - `sudo apt-get install gcc-arm-none-eabi`
		
	For Windows
    - Download [make for windows](http://gnuwin32.sourceforge.net/packages/make.htm) (choose Complete package, except sources)
    - Download [gcc-arm-none-eabi for windows](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) (windows 32 bit)
    - Add gcc .exe files to path (usually `C:\Program Files (x86)\GNU Tools ARM Embedded\<version>\bin`)
    - Add make.exe to path (usually `C:\Program Files (x86)\GnuWin32\bin`)
	
	For Mac
    - Open Command Line
    - Install Homebrew if not installed 
    	`/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
    - Download auto-run script, which will auto install <arm-none-eabi-gcc> with latest version
    	`brew tap ARMmbed/homebrew-formulae`
    - Install <arm-none-eabi-gcc> via HomeBrew
    	`brew install arm-none-eabi-gcc`

After deploying, the Nucleo will begin to flash red and green. Once the LED stays green, power-cycle the board by unplugging and replugging the 5V connector on the Nucleo.
	
3. Change directory into root directory

    `cd MarsRover2019-firmware`

4. Run make with the target pin mapping and application

    Ex. Compile the science application for the science board:  
    `make APP=science PINMAP=science`

    Ex. Compile the CAN test application for the nucleo development board:  
    `make APP=test_i2c PINMAP=nucleo`

    Ex. Compile the I2C test application for the safety board:  
    `make APP=test_can PINMAP=safety`

5. Deploy onto board

    Find the application .bin file, located in the build/app directory.

	For Ubuntu 16.04
		
    - Install libusb `sudo apt install libusb-1.0-0-dev`
    - Drag and Drop .bin file into NODE_F091RC folder
	
	For Windows
    
    - Download [st-link utility](http://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link004.html). Scroll down to Get Software
    - connect USB to nucleo board and open st-link utility
    - load code by going to Target->Program and browse for .bin file
	
	For Mac
    - Drag and Drop .bin file into NODE_F091RC disk (Will show up like other usb devices after connecting) or run `cp build/blinky_out.bin /Volumes/NODE_F091RC/`
    
## Using the Nucleo Dev Board to Program the Rover Boards

In order to use the Nucleo development board as a programmer, the two jumpers labelled NUCLEO - ST-LINK will need to be removed. This will sever the ST-LINK debugger portion of the Nucleo from the MCU side, allowing it to be used as a general debugger.

The ST-LINK debugger can then be connected via header CN4 (pins 1-5 with 1 nearest to the SWD label) to a rover board debug header (pins should be labelled) to program it according to the following table:

```
+-----------------------+-----------------------------------+
| Nucleo CN4 Pin Number | Rover Board Debug Header Pin Name |
+-----------------------+-----------------------------------+
| 1 (VREF)              | VCC                               |
| 2 (SWCLK)             | CLK                               |
| 3 (GND)               | GND                               |
| 4 (SWDIO)             | IO                                |
| 5 (NRST)              | RST                               |
| 6 (SWO)               | Not Connected                     |
+-----------------------+-----------------------------------+
```

After deploying the binary to the board, the Nucleo's `LD1` LED will flash red and green. Programming is complete when the LED stays green, so don't powercycle the board before this.
