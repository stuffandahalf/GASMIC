# GASMIC
Greg's retargettable assembler written in pure C.

## Requirements
* A C compiler
* GMake

## Build
Build by running `make TARGET=${your_target}`.
If you do not specify the desired target, it defaults to building for MC6809/HD6309.

## Retargetting
This assembler can be retargetted by adding a new folder to the src/targets/ folder with the name of your architecture.
You will need to create tw ofiles within this folder:
* arch.h -- Contains basic information about your architecture(s).
* arch.c -- Contains the instructions available in your architecture and the registers and addressing modes they support.

Refer to the default 6x09 target folder for more direction.
