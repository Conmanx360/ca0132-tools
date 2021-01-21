# Quartet DSP (X-Fi DSP)
The ca0132 codec is essentially the same hardware as the Sound Blaster X-Fi,
with the difference being that it sits behind an 8051 microcontroller which
handles the HDA interface.


Unless firmware is loaded, the DSP is disabled. If the firmware is uploaded,
the DSP is taken out of it's halt state and set to run. Once the DSP is running,
all audio streams are routed through it and it's DMA controllers.

## Quartet DSP overview:
First, a disclaimer: There is very little information online on this DSP, and
everything I know about it I've learned through reverse engineering or reading
archives of the sound blaster website. I'm also not an expert on digital signal
processing, so there's a good chance I've missed out on things obvious to
an expert. So, that being said, don't take anything here as being 100% correct,
expect a few mistakes, and verify everything yourself. :)


The Quartet DSP is a quad-core 32-bit "TIMD" (thread interleaved multiple data),
as described in this [Anandtech article](https://www.anandtech.com/show/1776/4)
from 2005. Each of the four individual DSP's has two 'data paths', which
essentially allow each core to do two math operations within one instruction.
A total of four register moves can be performed in one instruction, with an
assembly example of:

```
MOV_P @A_R7_X_INC, R00 :
      @A_R7_Y_INC, R01 /
MOV R02, R03 :
    R06, R07;
```

Allowing for moving R00/R01 into the respective address register 7 Xram/Yram data
locations and moving R03/R07 into R02/R06 in one instruction.


Registers are 32-bits in width, with registers 4/5 and 12/13 having an extra
40 bits to act as accumulators.


The DSP supports 32-bit floating point as well as 32-bit integer math. Most
documentation mentions fixed point math as well, however I haven't happened
upon any of these instructions so far.


Even though all registers are 32-bits, all addresses are only 16-bits, with
each address representing a 32-bit word. For ram, there are two sets of
addresses, X/Y, which represent X/Y ram. Addresses 0x0000-0xdfff are mapped
to xram/yram, 0xe000-0xefff are mapped to axram/ayram, and 0xf000-0xffff
seems to be mapped to some sort of configuration registers.


Program memory is read as 32-bit words, but is actually only 25-bits in width.
The maximum instruction length is 4-words, with length being tied to the first
2 bits. Instructions starting with 0x000 are length 1, instructions starting
with 0x010 are length 2, and instructions starting with 0x018 are length 4.
The reset vector for each DSP core starts at program memory 0xfff8 for DSP 0,
with an offset of two for the rest of the DSP's. Meaning, 0xfffa for DSP1,
0xfffc for DSP2, and 0xfffe for DSP3.

## Registers:
Here, I will attempt to give a brief overview of all registers.

The register address space is a maximum of 11-bits, with the upper
3 allowing for selection between XGPRAM/YGPRAM and regular register
addresses.


Instructions with register operand values encoded as less than 11-bits
default to the regular register mapping. Meaning, XGPRAM/YGPRAM registers
can only be used with instructions that have the full 11-bit address encoding.

| Reg Addr | Description |
| -------- | ----------- |
