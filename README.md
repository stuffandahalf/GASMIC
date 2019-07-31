# GASMIC
Greg's retargettable assembler written in pure C.
[![Build Status](https://travis-ci.org/stuffandahalf/GASMIC.svg?branch=master)](https://travis-ci.org/stuffandahalf/GASMIC)

## Requirements
* A C compiler
* CMake

## Build
Build by running `mkdir build && cd build && cmake .. && make`

## Retargetting
This assembler can be retargetted by adding a new folder within src/targets/ the name of your architecture.
You will need to create two files within this folder:
* arch.h - Contains basic information about your architecture(s).
* arch.c - Contains the instructions available in your architecture and the registers and addressing modes they support.
Additionaly, you wil need to update src/targets.h to initialize your architecture for use

Refer to the default 6x09 target folder for more direction.
