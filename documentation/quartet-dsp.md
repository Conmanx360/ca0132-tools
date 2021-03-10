# Quartet DSP (X-Fi DSP) Documentation:

The ca0132 codec is essentially the same hardware as the Sound Blaster X-Fi,
with the difference being that it sits behind an 8051 microcontroller which
handles the HDA interface.


Unless firmware is loaded, the DSP is disabled. If the firmware is uploaded,
the DSP is taken out of it's halt state and set to run. Once the DSP is running,
all audio streams are routed through it and it's DMA controllers.

## Sections:
- [Overview](#quartet-dsp-overview)
- [Registers](#registers)
  - [Local Hardware Registers](#local-hardware-registers)
  - [Address/Address Modifier Registers](#addressaddress-modifier-registers)
  - [Constant Registers](#constant-registers)
  - [Timer Registers](#timer-registers)
  - [Indirect Address Registers](#indirect-address-registers)
  - [Status Registers](#status-registers)
  - [Address Register Base/Length Registers](#address-register-baselength-registers)
  - [Bitmask Registers](#bitmask-registers)
  - [DMA Configuration Registers](#dma-configuration-registers)
  - [XGPRAM/YGPRAM](#xgpramygpram)
- [Opcodes/Assembly](#opcodesassembly)
  - [Opcodes](#opcodes)
  - [Assembly Syntax](#assembly-syntax)
  - [Move Instructions](#move-instructions)
    - [Register-Register Move](#register-register-move)
    - [Register-Literal Move](#register-literal-move)
    - [Register-RAM Move](#register-ram-move)
    - [Register-Stack Move](#register-stack-move)
  - [Bit Manipulation Instructions](#bit-manipulation-instructions)
    - [Bitwise Logic Operators](#bitwise-logic-operators)
    - [Bit Shifts](#bitshifts)
    - [Bit Counting](#bit-counting)
    - [Single Bit Modify](#single-bit-modify)
    - [Multi Bit Modify](#multi-bit-modify)
    - [Multi Bit Extract](#multi-bit-extract)
    - [Interrupt Bit Clear](#interrupt-bit-clear)
  - [Program Flow Instructions](#program-flow-instructions)
    - [Conditional Codes](#conditional-codes)
    - [Address Set](#address-set)
    - [Returns](#returns)
    - [Interrupts](#interrupts)
    - [Loops](#loops)
  - [Integer Math Instructions](#integer-math-instructions)
    - [Add/Subtract](#integer-addsubtract)
    - [Multiply](#integer-multiply)
    - [Fused Multiply Add/Subtract](#integer-fused-multiply-add-and-subtract)
    - [Interpolate](#integer-interpolate)
    - [Triple Add](#integer-triple-add)
  - [Floating Point Math Instructions](#floating-point-math-instructions)
    - [Add/Subtract/Multiply](#floating-point-addsubtractmultiply)
    - [Fused Multiply Add/Subtract](#floating-point-fused-multiply-add-and-subtract)
    - [Sine/Cosine/Arctangent](#sinecosinearctangent)
    - [Reciprocal](#reciprocal)
    - [Reciprocal Division](#reciprocal-division)
    - [DFT Calculation](#dft-calculation)
  - [Value Manipulation Instructions](#value-manipulation-instructions)
    - [Absolute Value](#absolute-value)
    - [Negate Value](#negate-value)
    - [Type Conversion](#type-conversion)
    - [Value Comparison](#value-comparison)
    - [Floating Point Value Extract](#floating-point-value-extract)


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

|   Address   |                        Description                               |
| ----------- | ---------------------------------------------------------------  |
| 0x300-0x30f | Local hardware registers.                                   |
| 0x310-0x320 | Address registers/Address modifier registers. |
| 0x320-0x34b | Constant registers. |
| 0x34c-0x357 | Unknown registers. |
| 0x358-0x35f | Timer registers. |
| 0x360-0x37f | Indirect address registers. |
| 0x380-0x38b | Status registers. |
| 0x38c-0x3a7 | Unknown registers. |
| 0x3a8-0x3b7 | Address Base/Length registers.
| 0x3b8-0x3bf | Bitmask registers. |
| 0x3c0-0x3ff | DMA Configuration registers. |
| 0x400-0x40f | XGPRAM. |
| 0x600-0x60f | YGPRAM. |

### Local Hardware Registers:
These are the main general purpose registers, labeled local hardware registers
in the X-Fi register definition header.


Registers R04/R05 and R12/R13 are unique in that they have an extra 40-bits, accessed
through the MOV\_T1/MOV\_T2 instructions. I'm assuming they're meant to be used as
accumulators, with the extra bits being guard bits. MOV\_T1 accesses bits 32-63, and
MOV\_T2 accesses bits 64-69.


| Addr        |  Reg    |
| ------------| ------- |
| 0x300-0x30f | R00-R15 |


### Address/Address Modifier Registers:
These registers are used for storing addresses to read/write. The associated
modifiers are used to increment the addresses by the modifier value, or as
an offset depending on the instruction.


Modifiers are 32-bit signed integers, allowing for negative/positive
incrementing/decrementing.


|     Addr    |          Reg            |
| ------------| ----------------------- |
| 0x310-0x317 | A\_R0-A\_R7             |
| 0x318-0x31f | A\_R0\_MDFR-A\_R7\_MDFR |

### Constant Registers:
These registers store constant values, and cannot be modified. The first 26 are
identical to the constant registers on the Sound Blaster Audigy as10k1. The new ones
seem to be mainly floating point constants.


|  Addr |     Reg        |
| ------| -------------- |
| 0x320 | CR\_0x00000000 |
| 0x321 | CR\_0x00000001 |
| 0x322 | CR\_0x00000002 |
| 0x323 | CR\_0x00000003 |
| 0x324 | CR\_0x00000004 |
| 0x325 | CR\_0x00000008 |
| 0x326 | CR\_0x00000010 |
| 0x327 | CR\_0x00000020 |
| 0x328 | CR\_0x00000100 |
| 0x329 | CR\_0x00010000 |
| 0x32a | CR\_0x00000800 |
| 0x32b | CR\_0x10000000 |
| 0x32c | CR\_0x20000000 |
| 0x32d | CR\_F\_2 (Floating point 2.0f.) |
| 0x32e | CR\_0x80000000 |
| 0x32f | CR\_0x7fffffff |
| 0x330 | CR\_0xffffffff |
| 0x331 | CR\_0xfffffffe |
| 0x332 | CR\_0xc0000000 |
| 0x333 | CR\_0x4f1bbcdc |
| 0x334 | CR\_0x5a7ef9db |
| 0x335 | CR\_0x00100000 |
| 0x336 | CR\_0x2d413ccd (Start of new X-Fi constants.) |
| 0x337 | CR\_0x80000001 |
| 0x338 | CR\_0x08000000 |
| 0x339 | CR\_0x02000000 |
| 0x33a | CR\_0x00800000 |
| 0x33b | CR\_0x00200000 |
| 0x33c | CR\_0x00080000 |
| 0x33d | CR\_0x0000001f |
| 0x33e | CR\_0x0000000f |
| 0x33f | CR\_0x00000007 |
| 0x340 | CR\_0x00000006 |
| 0x341 | CR\_0x00000005 |
| 0x342 | CR\_F\_PI (Floating point pi.) |
| 0x343 | CR\_F\_PI\_DIV\_2 |
| 0x344 | CR\_F\_PI\_DIV\_4 |
| 0x345 | CR\_F\_1\_DIV\_PI |
| 0x346 | CR\_F\_1\_DIV\_2PI |
| 0x347 | CR\_F\_0.5 |
| 0x348 | CR\_F\_1 |
| 0x349 | CR\_F\_NEG\_1 |
| 0x34a | CR\_F\_3 |
| 0x34b | CR\_F\_SQRT\_0.5 |


### Timer Registers:
Timer related registers, seemingly with one for each DSP. I haven't done
much testing with them, but it seems that the \_PER\_ENB register is related
to which interrupt is signaled when the timer overflows and what value to 
count to before overflowing.


The COUNTER register seems to be split into two 16-bit portions, with the lower
16-bits being the current counter value, and the upper 16-bits being the value
to count to before signaling an interrupt and resetting back to 0.

| Addr  |     Reg         |
| ----- | --------------- |
| 0x358 | TIME0\_PER\_ENB |
| 0x359 | TIME0\_COUNTER  |
| 0x35a | TIME1\_PER\_ENB |
| 0x35b | TIME1\_COUNTER  |
| 0x35c | TIME2\_PER\_ENB |
| 0x35d | TIME2\_COUNTER  |
| 0x35e | TIME3\_PER\_ENB |
| 0x35f | TIME3\_COUNTER  |

### Indirect Address Registers:
These registers allow for indirectly reading/writing the data stored at the address
in the respective address register, as well as reading/writing and incrementing the 
address afterwards by the respective address modifier register.

| Addr  |     Reg        |
| ----- | -------------- |
| 0x360 | @A\_R0\_X\_REG |
| 0x361 | @A\_R0\_Y\_REG |
| 0x362 | @A\_R1\_X\_REG |
| 0x363 | @A\_R1\_Y\_REG |
| 0x364 | @A\_R2\_X\_REG |
| 0x365 | @A\_R2\_Y\_REG |
| 0x366 | @A\_R3\_X\_REG |
| 0x367 | @A\_R3\_Y\_REG |
| 0x368 | @A\_R4\_X\_REG |
| 0x369 | @A\_R4\_Y\_REG |
| 0x36a | @A\_R5\_X\_REG |
| 0x36b | @A\_R5\_Y\_REG |
| 0x36c | @A\_R6\_X\_REG |
| 0x36d | @A\_R6\_Y\_REG |
| 0x36e | @A\_R7\_X\_REG |
| 0x36f | @A\_R7\_Y\_REG |
| 0x370 | @A\_R0\_X\_INC\_REG |
| 0x371 | @A\_R0\_Y\_INC\_REG |
| 0x372 | @A\_R1\_X\_INC\_REG |
| 0x373 | @A\_R1\_Y\_INC\_REG |
| 0x374 | @A\_R2\_X\_INC\_REG |
| 0x375 | @A\_R2\_Y\_INC\_REG |
| 0x376 | @A\_R3\_X\_INC\_REG |
| 0x377 | @A\_R3\_Y\_INC\_REG |
| 0x378 | @A\_R4\_X\_INC\_REG |
| 0x379 | @A\_R4\_Y\_INC\_REG |
| 0x37a | @A\_R5\_X\_INC\_REG |
| 0x37b | @A\_R5\_Y\_INC\_REG |
| 0x37c | @A\_R6\_X\_INC\_REG |
| 0x37d | @A\_R6\_Y\_INC\_REG |
| 0x37e | @A\_R7\_X\_INC\_REG |
| 0x37f | @A\_R7\_Y\_INC\_REG |

### Status Registers.
These are status related registers.


`COND_REG` is the status register for arithmetic operations, and contains seven bits
for each data path. Bits 0-7 are data path 1, and bits 8-14 are data path 2. From
my own personal testing, I believe the bits are laid out as:
| Bit |        Definition         |
| --- | ------------------------- |
|  0  |  (C)arry/Borrow.          |
|  1  |  (M)inus, or negative.    |
|  2  |  (Z)ero.                  |
|  3  |  (N)ormalized.            |
|  4  |  (S)aturate, or overflow. |
|  5  |  (X), unknown.            |
|  6  |  (Y), unknown.            |

Most of the bits are pretty self explanatory. The (M)inus flag means the result
was negative, the (N)ormalized flag behaves the same as the emu10k1's normalize
flag, and the (X)/(Y) flags are currently unknown.


The STA\_S\_STACK\_DATA/STA\_S\STACK\_PTR registers are used to store the COND\_REG
when an interrupt is triggered. It gets pushed onto the stack at the beginning of
the interrupt, and popped when RETI is called. STACK\_DATA is the current value.


The rest are self explanatory, the only one I'm unsure of is the STACK\_FLAG\_REG.

| Addr  |     Reg         |
| ----- | --------------- |
| 0x380 | COND\_REG        |
| 0x381 | STACK\_FLAG\_REG |
| 0x382 | PC\_STK\_PTR |
| 0x383 | SR\_B0\_04\_UNK |
| 0x384 | CUR\_LOOP\_ADR\_REG |
| 0x385 | CUR\_LOOP\_CNT\_REG |
| 0x386 | TOP\_LOOP\_CNT\_ST\_REG |
| 0x387 | TOP\_LOOP\_COUNT\_ADR\_REG |
| 0x388 | LOOP\_STACK\_PTR |
| 0x389 | STA\_S\_STACK\_DATA |
| 0x38a | STA\_S\_STACK\_PTR |
| 0x38b | PROG\_CNT\_REG     |


### Address Register Base/Length Registers:
These registers are used for looping through a series of addresses. When the
respective address register get's 'LENG' offset away from the base, the address
loops back around.


This is used by default on A\_R7 for the main stack. Setting the BASE register
automatically sets the respective address register, so be careful when trying
to zero it out that you do that first before setting the address register.


| Addr  |     Reg     |
| ----- | ----------- |
| 0x3a8 | A\_R0\_BASE |
| 0x3a9 | A\_R1\_BASE |
| 0x3aa | A\_R2\_BASE |
| 0x3ab | A\_R3\_BASE |
| 0x3ac | A\_R4\_BASE |
| 0x3ad | A\_R5\_BASE |
| 0x3ae | A\_R6\_BASE |
| 0x3af | A\_R7\_BASE |
| 0x3b0 | A\_R0\_LENG |
| 0x3b1 | A\_R1\_LENG |
| 0x3b2 | A\_R2\_LENG |
| 0x3b3 | A\_R3\_LENG |
| 0x3b4 | A\_R4\_LENG |
| 0x3b5 | A\_R5\_LENG |
| 0x3b6 | A\_R6\_LENG |
| 0x3b7 | A\_R7\_LENG |




### Bitmask Registers:
These registers seem to be intended for bitwise use. I don't know too much about
them, other than that the INT\_CONT\_PEND\_REG is set by the onboard 8051 when an
SCP command is sent. I'm assuming that INT\_CONT\_SERV\_REG is used to signal that
the interrupt has been serviced, and I'm not sure what the MASK\_REG represents.


SEMAPHORE\_G seems to be a global semaphore register to signal that a resource
is in use by one of the DSP's.


I'm not sure what the GPIO\_REG pins are connected to, if anything.


| Addr  |     Reg     |
| ----- | ----------- |
| 0x3b8 | SEMAPHORE\_G\_REG |
| 0x3b9 | INT\_CONT\_MASK\_REG |
| 0x3ba | INT\_CONT\_PEND\_REG |
| 0x3bb | INT\_CONT\_SERV\_REG |
| 0x3bc | SR\_B1\_28\_UNK |
| 0x3bd | SR\_B1\_29\_UNK |
| 0x3be | SR\_B1\_30\_UNK |
| 0x3bf | GPIO\_REG |



### DMA Configuration Registers:
These registers are used to configure the DSP's DMA controllers.


When the DSP is enabled, these are used to route audio to the DSP to be processed.
They are also manually configured by the driver to upload the firmware, by sending
the data as an audio stream.


On the ca0132, these are mapped to HCI addresses 0x110000-0x110fc0.


| Addr  |     Reg     |
| ----- | ----------- |
| 0x3c0 | DMACFG\_0\_REG |
| 0x3c1 | DMA\_ADR\_OFS\_0\_REG |
| 0x3c2 | DMA\_XFR\_CNT\_0\_REG |
| 0x3c3 | DMA\_IRQ\_CNT\_0\_REG |
| 0x3c4 | DMACFG\_1\_REG |
| 0x3c5 | DMA\_ADR\_OFS\_1\_REG |
| 0x3c6 | DMA\_XFR\_CNT\_1\_REG |
| 0x3c7 | DMA\_IRQ\_CNT\_1\_REG |
| 0x3c8 | DMACFG\_2\_REG |
| 0x3c9 | DMA\_ADR\_OFS\_2\_REG |
| 0x3ca | DMA\_XFR\_CNT\_2\_REG |
| 0x3cb | DMA\_IRQ\_CNT\_2\_REG |
| 0x3cc | DMACFG\_3\_REG |
| 0x3cd | DMA\_ADR\_OFS\_3\_REG |
| 0x3ce | DMA\_XFR\_CNT\_3\_REG |
| 0x3cf | DMA\_IRQ\_CNT\_3\_REG |
| 0x3d0 | DMACFG\_4\_REG |
| 0x3d1 | DMA\_ADR\_OFS\_4\_REG |
| 0x3d2 | DMA\_XFR\_CNT\_4\_REG |
| 0x3d3 | DMA\_IRQ\_CNT\_4\_REG |
| 0x3d4 | DMACFG\_5\_REG |
| 0x3d5 | DMA\_ADR\_OFS\_5\_REG |
| 0x3d6 | DMA\_XFR\_CNT\_5\_REG |
| 0x3d7 | DMA\_IRQ\_CNT\_5\_REG |
| 0x3d8 | DMACFG\_6\_REG |
| 0x3d9 | DMA\_ADR\_OFS\_6\_REG |
| 0x3da | DMA\_XFR\_CNT\_6\_REG |
| 0x3db | DMA\_IRQ\_CNT\_6\_REG |
| 0x3dc | DMACFG\_7\_REG |
| 0x3dd | DMA\_ADR\_OFS\_7\_REG |
| 0x3de | DMA\_XFR\_CNT\_7\_REG |
| 0x3df | DMA\_IRQ\_CNT\_7\_REG |
| 0x3e0 | DMACFG\_8\_REG |
| 0x3e1 | DMA\_ADR\_OFS\_8\_REG |
| 0x3e2 | DMA\_XFR\_CNT\_8\_REG |
| 0x3e3 | DMA\_IRQ\_CNT\_8\_REG |
| 0x3e4 | DMACFG\_9\_REG |
| 0x3e5 | DMA\_ADR\_OFS\_9\_REG |
| 0x3e6 | DMA\_XFR\_CNT\_9\_REG |
| 0x3e7 | DMA\_IRQ\_CNT\_9\_REG |
| 0x3e8 | DMACFG\_A\_REG |
| 0x3e9 | DMA\_ADR\_OFS\_A\_REG |
| 0x3ea | DMA\_XFR\_CNT\_A\_REG |
| 0x3eb | DMA\_IRQ\_CNT\_A\_REG |
| 0x3ec | DMACFG\_B\_REG |
| 0x3ed | DMA\_ADR\_OFS\_B\_REG |
| 0x3ee | DMA\_XFR\_CNT\_B\_REG |
| 0x3ef | DMA\_IRQ\_CNT\_B\_REG |
| 0x3f0 | DMACFG\_CH\_SEL\_0\_REG |
| 0x3f1 | DMACFG\_CH\_SEL\_1\_REG |
| 0x3f2 | DMACFG\_CH\_SEL\_2\_REG |
| 0x3f3 | DMACFG\_CH\_SEL\_3\_REG |
| 0x3f4 | DMACFG\_CH\_SEL\_4\_REG |
| 0x3f5 | DMACFG\_CH\_SEL\_5\_REG |
| 0x3f6 | DMACFG\_CH\_SEL\_6\_REG |
| 0x3f7 | DMACFG\_CH\_SEL\_7\_REG |
| 0x3f8 | DMACFG\_CH\_SEL\_8\_REG |
| 0x3f9 | DMACFG\_CH\_SEL\_9\_REG |
| 0x3fa | DMACFG\_CH\_SEL\_A\_REG |
| 0x3fb | DMACFG\_CH\_SEL\_B\_REG |
| 0x3fc | DMACFG\_CH\_START\_REG |
| 0x3fd | DMACFG\_CH\_STAT\_REG |
| 0x3fe | DMACFG\_CH\_PROP\_REG |
| 0x3ff | DMACFG\_ACTIVE\_REG |

### XGPRAM/YGPRAM:
These allow for an extra 16 data values to be stored. In the X-Fi's register
definition header, XGPRAM/YGPRAM have different sizes per DSP core, with core 0 having
a range of 0x1ff. However, on the ca0132, there seems to be only 16 words of data on
XGPRAM/YGPRAM each. Setting the upper address bits does nothing, with only the lower
4 being used.


XGPRAM\_000 seems to store the original call stack address.


|     Addr    |          Reg            |
| ------------| ----------------------- |
| 0x400-0x40f | XGPRAM\_000-XGPRAM\_015 |
| 0x600-0x60f | YGPRAM\_000-YGPRAM\_015 |


# Opcodes/Assembly:
Below are explanations as to how opcodes are encoded, the custom assembly syntax, and
descriptions of instruction behavior.


## Opcodes:
According to Creative, the Quartet DSP has 235 opcodes. From testing, opcodes seem
to be 8-bits, and come in four different lengths.


For length one instructions, the opcode starts at bit 24. Example:


`0x00c80000`


Has opcode 0xc8, which is MOV. To get length 2 MOV, we shift the opcode to the right
by 1, and set the most significant bit (which in the Quartet DSP's case, is bit 25). Example:


`0x01640000`


To get the length 4 instruction, we set bits 25 and 24, giving us:


`0x01e40000`


All instructions have 3 different lengths, each with a different operand layout. For instance:



```
0x00c80221 is MOV:1 R01, CR_0x00000001;


0x01647fff 0x01f01643 is MOV:2 R01, CR_0x00000001;


0x01e47fff 0x01ffe000 0x00180990 0x01301321 is MOV:4 R01, CR_0x00000001 : R01, CR_0x00000001;
(The part after the colon is for data path 2, length four moves always need to use both data
paths.)
```


Some length 2 and 4 instructions can also include a parallel instruction, to execute at the
same time as the main instruction. An example:

```
MOV_P @A_R7_X_INC, R02 : 
      @A_R7_Y_INC, R03 / 
MOV R02, CR_0x00000002;
```


Gives opcode `0x0164447f 0x01f02645`, which moves R02 into the address stored at A\_R7 in xram,
moves R03 into the address stored at A\_R7 in YRAM, and increments the address register by it's
modifier value. It also moves the constant register with the value 0x02 into R02.


## Assembly Syntax:
The basic syntax is:


dst\_reg = src\_reg:
`[INSTRUCTION] [DST_REG], [SRC_REG];`

r\_reg = x\_reg +/-/\* y\_reg:
`[INSTRUCTION] [R_REG], [X_REG], [Y_REG];`


An instruction at the start, with operands separated by commas and instructions terminated
with a semicolon.


Instructions that can use the second data path have their operands split by a colon.
dst0\_reg = src0\_reg : dst1\_reg = src1\_reg:
`[INSTRUCTION] [DST0_REG], [SRC0_REG] : [DST1_REG], [SRC1_REG];`


Instructions can optionally have a `:1`, `:2`, or `:4` as a suffix to signify the
desired instruction length. If one is not chosen, the assembler will always attempt
to use the shortest instruction possible. Example:

`MOV:4 R00, R02 : R01, R03;`

This gives us a length four move instruction.


Parallel instructions are always added at the start, and are terminated with a forward slash
symbol. Example:

```
MOV_P @A_R0_X, R02 /
MOV R00, R01;
```

Parallel instructions can also use both data paths, and have the same syntax as main instructions. Example:

```
MOV_P @A_R0_X, R02 : @A_R0_Y, R03 /
MOV R00, R01;
```

Parallel ops cannot have a length suffix.


It is also valid to print the instruction twice, which is what the disassembler does. This sometimes
helps when looking at many lines of assembly at once. Example:

```
MOV_P @A_R0_X, R02 :
MOV_P @A_R0_Y, R03 /
MOV R00, R01 :
MOV R06, R07;
```
## Move Instructions:
Instructions for moving between a source and destination in memory.


### Register-Register Move:
The main register-register move instructions come in three types.

- `MOV` is a regular move, and moves between all register normally.
- `MOV_T1` Moves the upper 32-bits of R04/R05/R12/R13 (accumulators.)
- `MOV_T2` moves the upper 8 bits of R04/R05/R12/R13.

The main MOV instruction can also be used with source modifiers, which are:

- Increment, example: `MOV R00, R01++;`.
- Decrement, example: `MOV R00, R01--;`.
- Rotate right by 1, example: `MOV R00, R01 >> 1;`.
- Rotate left  by 1, example: `MOV R00, R01 << 1;`.

These modifiers can only be used with regular MOV instructions, so MOV\_T1 and MOV\_T2 are incompatible.
[Operand register ranges here.](#mov-src-dst-register-ranges)


### Register-Literal Move:
MOV literals consistent of:

- `MOV`, which is a normal literal MOV. Also has MOV\_T1 and MOV\_T2 variants.
- `MOV_L`, which moves only to the lower 16-bits. This should be used in 16-bit value sets. Has \_T1 and \_T2 variants.
- `MOV_U`, which moves only to the upper 16-bits. This should be used in 16-bit value sets. Only has \_T1 variant, as \_T2 is only 8-bits.

Examples:
```
/* Full moves require 4 operands always. */
MOV R00, #0xfef00000 : /* R00 = 0xfef00000. */
MOV CR_0x00000000, #0x00000000; /* Use constant register 0 as destination if data path 2 is unused. */

MOV_L R00, #0xfef0; /* R00 = 0x0000fef0. */

MOV_U R00, #0xfef0; /* R00 = 0xfef00000. */
```

[Operand register ranges here.](#mov-src-dst-literal-ranges)


### Register-RAM Move:
MOVX instructions take a register argument and an indirect address from an address register
with an offset. Offsets can either be a literal value or an address modifier register. MOVX
has \_T1 and regular MOV variants, but no \_T2 variant. For single data path reads, either
X or Y RAM can be selected. For dual data path reads, data path 1 must be XRAM, and data path
2 must be YRAM. No MOVX instruction supports parallel ops.

- Literal address offset example: `MOVX @A_R0_X - 17, R02;`.
- Address register modifier offset example: `MOVX R00, @A_R1_Y + A_MD2;`
- Dual data path literal offset example: `MOVX R00, @A_R2_X + 5 : R01, @A_R2_Y + 3;`
- Dual data path address register modifier offset example: `MOVX @A_R3_X + A_MD2, R00 : @A_R2_Y + A_MD3, R01;`

[Operand register ranges here.](#movx-a_reg-offset-register-ranges)


## Bit Manipulation Instructions:

### Bitwise Logic Operators:
The DSP has bitwise instructions for AND, OR, XOR, and CMPL. Examples below:

- `AND R00, R02, R03;` r00 = r02 & r03.
- `OR R00, R02, R03;`, r00 = r02 | r03.
- `XOR R00, R02, R03;`, r00 = r02 ^ r03.
- `CMPL R00, R01;`, r00 = ~r01.

Register ranges for binary operations [here,](#r--x-y-register-ranges), register ranges for
unary operations (CMPL) [here.]((#mov-src-dst-register-ranges)

### Bitshifts:
The DSP has three different kinds of shifts:

- Rotate, `RO`, which wraps bits around upon sliding off the end.
- Shift, `SH`, which loses bits when they slide off the end.
- Arithmetic Shift, `A_SH`, same as regular shift, except sign bits are preserved.

There is also an instruction that performs a right shift without setting a destination register,
only setting the status registers, which is useful for checking certain conditions. Examples:


```
r01 = 0x80000003.

RO_R R00, R01, #0x01; /* r00 = 0xc0000001, first bit wraps around. */
RO_L R00, R01, #0x01; /* r00 = 0x00000007, final bit wraps around. */

SH_R R00, R01, #0x01; /* r00 = 0x40000001, first bit cut off. */
SH_L R00, R01, #0x01; /* r00 = 0x00000006, final bit cut off. */

A_SH_R R00, R01, #0x01; /* r00 = 0xc0000001, first bit cut off, sign extended. */
A_SH_L R00, R01, #0x01; /* r00 = 0x00000006, final bit cut off, sign into carry flag. */

SH_R_CHK R00, R01, #0x01; /* r00 unmodified, but flags are set. */
```

Bit shifts use the R = X Y layout, so a register can also be used as the shift value.
Register ranges are [here.](#r--x-y-register-ranges)

### Bit Counting:
These instructions count from the most significant bit until the specified condition is
met. There are two possible conditions:

- Find First Set, `FFS`, stops counting at the first set bit.
- Find First Unset, `FFU`, stops counting at the first unset bit.

There is also a variation of Find First Set for signed integers, which counts the
bits of the absolute value. This instruction is `I_ABS_FFS`. Examples:

```
r01 = 0x80000000.
r02 = 0x00000001. 
r03 = 0xffffffff.

FFS R00, R01; /* r00 = 0x00000000, since the MSB is set. */
FFU R00, R01; /* r00 = 0x00000001, since the bit after the MSB is unset. */
I_ABS_FFS R00, R01 /* r00 = 0x00000000, as this is -0x7ffffff. */

FFS R00, R02; /* r00 = 0x0000001f, 31, since the only bit set is the LSB. */
FFU R00, R02; /* r00 = 0x00000000, since the MSB is unset. */
I_ABS_FFS R00, R02; /* r00 = 0x0000001e, 30, due to this being treated as
                     * positive signed integer, which has a shorter range. */

FFS R00, R03; /* r00 = 0x00000000, since the MSB is set. */
FFU R00, R03; /* r00 = 0x00000020, 32, since no bit is unset. */
I_ABS_FFS R00, R03; /* r00 = 0x0000001f, since this is -1, and the absolute value is 1. 
                     * Due to negative signed integers having a higher value range than
                     * positive signed integer values, the count starts one bit higher. */

```

[Operand register ranges here.](#mov-src-dst-register-ranges)


### Single Bit Modify:
These instructions modify single bits in a given register. The possible bit modifications are:

- Set, `SET_BIT`, which sets a given bit.
- Clear, `CLR_BIT`, which clears a given bit.
- Toggle, `TGL_BIT`, which inverts a bit, i.e if the bit was set, it's cleared, if it was unset, it's set.

There are also variations for setting and clearing bits in the `SEMAPHORE_G_REG` register. Examples:
```
r01 = 0x00000001.

SET_BIT R00, R01, #0x01; /* r00 = 0x00000003, bit 1 is now set. */
CLR_BIT R00, R01, #0x00; /* r00 = 0x00000000, bit 0 is now unset. */
TGL_BIT R00, R01, #0x00; /* r00 = 0x00000000, bit 0 toggled to 0. */
TGL_BIT R00, R01, #0x02; /* r00 = 0x00000005, bit 2 is toggled to 1. */

SET_SEM_G_BIT R04, R01, #0x01; /* Sets SEMAPHORE_G_REG to 0x00000003.
                                * If the destination register isn't R04, it 
                                * the specified register has it's value cleared. */
CLR_SEM_G_BIT R04, R01, #0x00; /* Sets SEMAPHORE_G_REG to 0x00000000. */

```

Register ranges are [here.](#r--x-y-register-ranges)


### Multi Bit Modify:
These instructions allow for modifying a set of bits within a register value. It takes a source register,
a starting bit, a bit count, and a value to set. Each data path must always be used for these instructions,
as operands are spread out between them. The destination of data path two is with the value of the starting
bit added to the bit count. The normal behavior is to clear the bits in the source before setting them to
the new value supplied. The `_T1` behavior doesn't clear them, which behaves more like a logical OR. The
`_T2` behavior clears all bits above the starting bit of the value to be set. Examples:

```
r01 = 0x10;
r02 = 0x01234567;
r03 = 0x06;
r06 = 0x08;

SET_BITS R00, R01, R02, R03 : CR_0x00000000, CR_0x00000000, CR_0x00000000, R06; /* r00 = 0x01064567; */
Equivalent C expression: r00 = ((r03 & ((1 << r06) - 1)) << r01) | (r02 & ~(((1 << r06) - 1)) << r01);

SET_BITS_T1 R00, R01, R02, R03 : CR_0x00000000, CR_0x00000000, CR_0x00000000, R06;
Results in r00 = 0x01274567;
Equivalent C expression: r00 = ((r03 & ((1 << r06) - 1)) << r01) | r02;

SET_BITS_T2 R00, R01, R02, R03 : CR_0x00000000, CR_0x00000000, CR_0x00000000, R06;
Results in r00 = 0x00064567;
Equivalent C expression: r00 = ((r03 & ((1 << r06) - 1)) << r01) | (r02 & ~((1 << (32 - r01))));

Data path 2's X and Y registers go unused.
```

Register ranges are [here.](#r--x-y-a-register-ranges)


### Multi Bit Extract
This instruction extracts bits from a given register. It takes a starting bit, a bit count, and a source
register. Both data paths must always be used for this instruction, as operands are spread out between
the two. It also stores the value of the starting bit and bit count added together in the destination
register for the second data path, which would be the highest bit of the extracted value. Example:


```
r01 = 0x00fef000.
r02 = 0x0000000c.
r03 = 0x00000008.

GET_BITS R00, R01, R02 : R06, R03, CR_0x00000000; /* r00 = 0x000000ef. r06 = 0x00000014. */

Equivalent C expression: r00 = (r01 >> r02) & ((1 << r03) - 1);
Operand Y on data path 2 goes unused.
```
Register ranges are [here.](#r--x-y-register-ranges)

### Interrupt Bit Clear
Clears the interrupt pin provided by the literal, `INT_CLR #0x00;` through
`INT_CLR #0x0f;`. Not much testing has been done, but it seems each DSP has 4
of these specific interrupts, with the `INT_CONT_PEND_REG` containing a bitmask
for which interrupts need handled and which have been handled. Example:

```
int_cont_pend_reg = 0x00100405;

INT_CLR #0x03; /* After this, int_cont_pend_reg = 0x00300405. */

The upper bits signify that a particular interrupt has been handled,
and the lower bits signify which ones still need handled.
```

## Program Flow Instructions

### Conditional Codes
Conditional codes are an 8-bit literal value which is split into 3 parts, bits 0-3, bits 4-6,
and bit 7. I will attempt to explain each sections behavior below.

The lowest four bits seem to match the behavior in the following table:
| Code |             Behavior               |
| ---- | ---------------------------------- |
| 0x00 | `if (C)`                           |
| 0x01 | `if (Z)`                           |
| 0x02 | `if (!M + (M * S))`                |
| 0x03 | `if (!Z * (!M + (M * S)))`         |
| 0x04 | `if (!Z * C)`                      |
| 0x05 | `if (!Z * !C)`                     |
| 0x06 | `if (X)`                           |
| 0x07 | `if (Y)`                           |
| 0x08 | `if (X + Y)`                       |
| 0x09 | `if (M)`                           |
| 0x0a | Unknown, seems to be always true.  |
| 0x0b | Unknown, seems to be always true.  |
| 0x0c | Unknown, seems to be always true.  |
| 0x0d | Unknown, seems to be always true.  |
| 0x0e | `if (0)`                           |
| 0x0f | `if (1)`                           |


The second set of bits, 4-6, seem to add an extra modifier on top
of the conditional supplied from the first four bits, with the
behavior described in the table below:
| Code |       Behavior                     |
| ---- | ---------------------------------- |
| 0x00 | `if (cond0)`                       |
| 0x10 | `if (N + (cond0))`                 |
| 0x20 | `if (S + (cond0))`                 |
| 0x30 | `if (N + S + (cond0))`             |
| 0x40 | `if (cond0)`, extra info below.    |
| 0x50 | `if (N * (cond0))`                 |
| 0x60 | `if (S * (cond0))`                 |
| 0x70 | `if (N * S * (cond0))`             |

`cond0` represents the conditional from the first four bits, 0-3. The code `0x40`
behaves the same as `0x00`, except that unlike `0x0e` always evaluating to false, `0x4e`
always evaluates to true.


The final bit, bit 7, just inverts the conditional from the above two parts. It is
essentially `if (!(cond))`.


Conditional flag letter definitions can be found [here.](#status-registers) Common
conditional values obtained from the compare instructions can be found [here.](#value-comparison)


### Address Set
There are currently four different instructions for setting the program counter,
`CALL`, which is a function call,  `JMP`, which is a normal jump, and `JMPC/JMPC_T1` which
are not yet understood. Each instruction also has an `S_` prefix variant, which takes a
signed integer offset from the current PC value. Each instruction contains an 8-bit 
conditional value, which is explained in the previous section. Examples:

- Call literal address example: `CALL #0x0f, #0xdf00;`.
- Call address register example: `CALL #0x0f, A_R0;`.
- Jump literal address example: `JMP #0x0f, #0x0e20;`.
- Jump address register example: `JMP #0x0f, A_R0;`.
- Call literal PC offset example: `S_CALL #0x0f, #-23;`.
- Jump literal PC offset example: `JMP #0x0f, #5;`.


### Returns 
There are currently two different known return variants: 

- `RET`, which is a regular function call return.
- `RETI`, which is an interrupt return.

Each get their return address from the call stack and decrement the `PC_STK_PTR` register.
`RETI` also restores the `COND_CODE` register and decrements the `STA_S_STACK_PTR` register.


### Interrupts
There are currently 3 known interrupt instructions that effect control flow:

- `HALT`, which halts the DSP until an interrupt is signaled.
- `INT_DISABLE`, which prevents the DSP from being interrupted.
- `INT_ENABLE`, which enables the DSP to be interrupted.


### Loops


## Integer Math Instructions
Integers have many different variations of the basic add, subtract and multiply
operations due to the accumulator being 70-bits combined. I will attempt to
describe each below:



### Integer Add/Subtract:
Add and subtract instructions come in two different destination variations,
upper and lower. Instructions that store their result in the upper portion
of the accumulator are defined by the `U_` prefix. They also have two separate
result bounding variations, saturation and overflow, defined by the `_S` and
`_O` suffixes, respectively.


Upper instructions store their results into the `_T1` portion of the accumulator
register, which means we have a 40-bit result. Non upper instructions use the
entire combined 70 bits of the accumulator.


Saturation instructions bound the result to 63-bits for full range instructions,
and 31 for upper. Saturation instructions will not overflow once they reach the
highest possible value for the given sign, instead they 'stick' to the highest value.
For unsigned integers, this is `0x7fffffffffffffff`, and for signed integers, it's
`0x8000000000000000`.


Overflow instructions will use the entire 70 bits of the accumulator, and wrap around
upon overflow.


`ADDC` is add with carry, which adds an extra 1 if the carry flag was set, and `SUBB` is
subtract with borrow, which subtracts an extra 1 if the borrow flag was set.


Below are all the possible assembly ops:
|      Opcode      |                     Behavior                     |
| ---------------- | ------------------------------------------------ |
| U\_ADD\_S        | Upper accumulator add, saturate.                 |
| U\_SUB\_S        | Upper accumulator subtract, saturate.            |
| U\_ADD\_O        | Upper accumulator add, overflow.                 |
| U\_ADDC\_O       | Upper accumulator add carry, overflow.           |
| U\_SUB\_O        | Upper accumulator sub, overflow.                 |
| U\_SUBB\_O       | Upper accumulator sub with borrow, overflow.     |
| U\_ADDSUB\_O     | Upper acc add on dp0, sub on dp1. Overflow.      |
| U\_SUBADD\_O     | Upper acc sub on dp0, add on dp1. Overflow.      |
| ADD\_S           | Full accumulator add, saturate.                  |
| ADDC\_S          | Full accumulator add carry, saturate.            |
| SUB\_S           | Full accumulator subtract, saturate.             |
| SUBB\_S          | Full accumulator sub with borrow, saturate.      |
| ADD\_O           | Full accumulator add, overflow.                  |
| ADDC\_O          | Full accumulator add carry, overflow.            |
| SUB\_O           | Full accumulator subtract, overflow.             |
| SUBB\_O          | Full accumulator sub with borrow, overflow.      |



### Integer Multiply:
Like add and subtract, multiply instructions come in two different destination
variations. However, multiply behaves differently. For 'lower' multiply (defined by the
`L` prefix), the result starts in the upper 32-bits, and is shifted to the right by 31 bits.
This matches the behavior of the emu10k1 DSP, which used these as 'fractional' multiplies.


Multiply has unsigned variations, which don't sign extend the operand. These are defined
by the `_US` suffix. Normally, when a value such as `0x80000000` is multiplied, since the
MSB is set, the rest of the upper 40-bits are assumed to be set. In unsigned instructions,
his is not the case. The regular instruction treats the result as signed, and extends
the sign if bit 63 is set. The `_T1` instruction treats the result as unsigned. These might
be considered saturation and overflow, although they don't exactly fit that behavior fully.


Like before, there are saturate and overflow variations, which behave the same as the addition and
subtraction instructions.


Below are all the possible assembly ops (reminder, `L` instructions start in
the upper 32-bits of the accumulator):
|      Opcode      |                     Behavior                                           |
| ---------------- | ---------------------------------------------------------------------- |
| LMUL\_S          | r =  (x * y >> 31). Saturate.                                          |
| LMUL\_O          | r =  (x * y >> 31). Overflow.                                          |
| LNMUL\_O         | r = -(x * y >> 31). Result is negated. Overflow.                       |
| LMUL\_US\_O      | r =  (x * y >> 31). Overflow. Unsigned operands                        |
| LMUL\_US\_O\_T1  | r =  (x * y >> 31). Overflow. Unsigned operands + result.              |
| MUL\_O           | r =  (x * y). Overflow.                                                |
| NMUL\_O          | r = -(x * y). Result is negated. Overflow.                             |
| MUL\_US\_O       | r =  (x * y). Overflow. Unsigned operands.                             |
| MUL\_US\_O\_T1   | r =  (x * y). Overflow. Unsigned operands + result.                    |



### Integer Fused Multiply Add and Subtract:
Like the basic `r = x y` math, there are many variations of `r = x y a` integer instructions.
I will describe them below.

There are basically add, add accumulate, add accumulate move, negate mul, subtract,
upper/lower,

The main difference with these instructions are the `_AC` and `_MV` suffixes.

- `_AC` means accumulate, so the format is `r += (x y) a` rather than `r = (x y) a`.
- `_MV` means move, in these instructions take the format of `r += (x * y); a = r;`.

The plain accumulate instructions only take two operands, X and Y, so they're technically not
`r = x y a` instructions, although they fit in more here than in the original catagory.

Accumulate move instructions will move the upper part of the accumulator on `L` instructions, and
the lower portion on regular. On non `L` saturation instructions, if the
result was larger than the lower 32-bits, the move will always set the
register to the saturation value. This means that even if we didn't saturate
the full 63 bits and the accumulator is fine, the move will return
`0x7fffffff` for positive, and `0x80000000` for negative.


Without either suffix, the behavior is `r = (x * y) +/- a;`.


For lower instructions, the added ACC register is added in the upper 32-bits
of the accumulator. Example:
```
R00 = 0x00000002;
R01 = 0x00000004;
R02 = 0x00000020;

LMA_S R04, R00, R01, R02;

Would result in: 0x00 00000020 00000010.

R02 is added in the upper 32-bits, and R00 * R01 is shifted to the right by
31-bits.
```

Lower instructions:
|      Opcode      | Args |                     Behavior                                 |
| ---------------- | ---- | ------------------------------------------------------------ |
| LMA\_AC\_S       |  2   | r +=  (x * y >> 31). Saturate.                               |
| LMA\_S           |  3   | r =   (x * y >> 31) + a. Saturate.                           |
| LMA\_AC\_MV\_S   |  3   | r +=  (x * y >> 31), a = upper r. Saturate.                  |
| LNMA\_LMA\_S     |  3   | r0 = -(x * y >> 31) + a, r1 = (x * y >> 31) + a. Saturate.   |
| LNMA\_S          |  3   | r =  -(x * y) + a. Saturate.                                 |
| LMS\_AC\_MV\_S   |  3   | r += -(x * y), a = upper r. Saturate.                        |
| LMS\_S           |  3   | r =   (x * y) - a. Saturate.                                 |
| LMA\_AC\_O       |  2   | r +=  (x * y >> 31). Overflow.                               |
| LMA\_O           |  3   | r =   (x * y >> 31) + a. Overflow.                           |
| LMA\_AC\_MV\_O   |  3   | r +=  (x * y >> 31), a = upper r. Overflow.                  |
| LNMA\_LMA\_O     |  3   | r0 = -(x * y >> 31) + a, r1 = (x * y >> 31) + a. Overflow.   |
| LNMA\_O          |  3   | r =  -(x * y) + a. Overflow.                                 |
| LMS\_O           |  3   | r =   (x * y) - a. Overflow.                                 |


Full range instructions:
|      Opcode      | Args |                     Behavior                       |
| ---------------- | ---- | -------------------------------------------------- |
| MA\_AC\_S        |  2   | r += (x * y). Saturate.                            |
| MA\_S            |  3   | r = (x * y) + a. Saturate.                         |
| MA\_C\_S         |  3   | r = (x * y) + a + carry-flag. Saturate.            |
| MA\_AC\_MV\_S    |  3   | r += (x * y), a = r. Saturate.                     |
| NMA\_MA\_S       |  3   | r0 = -(x0 * y0) + a, r1 = (x1 * y1) + a. Saturate. |
| NMA\_S           |  3   | r = -(x * y) + a. Saturate.                        |
| MS\_S            |  3   | r = (x * y) - a. Saturate.                         |
| MA\_AC\_O        |  2   | r += (x * y). Overflow.                            |
| MA\_O            |  3   | r = (x * y) + a. Overflow.                         |
| MA\_C\_O         |  3   | r = (x * y) + a + carry-flag. Overflow.            |
| MA\_AC\_MV\_O    |  3   | r += (x * y), a = r. Overflow.                     |
| NMA\_MA\_O       |  3   | r0 = -(x0 * y0) + a, r1 = (x1 * y1) + a. Overflow. |
| NMA\_O           |  3   | r = -(x * y) + a. Overflow.                        |
| MS\_O            |  3   | r = (x * y) - a. Overflow.                         |


### Integer Interpolate:
These are the same as the emu10k1's `INTERP` instruction. Copied below from the as10k1 manual:

```
Used for linear interpolating between two points. "X" should be positive and represents a
fractional value between 0 and 1. "x" is the fraction of the interval between A and Y where
the desired value is located.

The INTERP instruction is not only useful for linear interpolation, it can also be used for
rescaling values. In such a case, the input must be bounded by [0,1], the output will be
bounded by [A,Y]. Thus the intruction can be though of as:

interp    R,MIN,X,MAX

where MIN and MAX are the bounds of your output.
```


|      Opcode      | Args |                     Behavior                                                   |
| ---------------- | ---- | ------------------------------------------------------------------------------ |
| INTERP           |  3   | r = (x * (y - a) >> 31) + a. Saturate, I believe.                              |
| INTERP\_T1       |  3   | r = (x * (y - a) >> 31) + a. I think this may be overflow, but cannot confirm. |

### Integer Triple Add:
This is the same as the emu10k1's `ACC3` instruction. It adds all three operands together,
and stores the result in the upper 32-bits of the accumulator.


|      Opcode      | Args |                     Behavior                     |
| ---------------- | ---- | ------------------------------------------------ |
| U\_ADD3\_S       |  3   | r = a + x + y. Saturate.                         |


## Floating Point Math Instructions
Floating point math instructions are much simpler than their integer counterparts.
Since only 32-bit floats are used, instructions only use the upper 32-bits of the
accumulator, which means we don't need separate instructions for different bit
lengths.

### Floating Point Add/Subtract/Multiply:
Floating point add, subtract and multiply instructions are pretty basic. All results
are stored in the upper 32-bits of the accumulator.

Below are the floating point add, subtract and multiply ops:
|      Opcode      |                      Behavior                    |
| ---------------- | ------------------------------------------------ |
| F\_ADD           | r  =  x + y.                                     |
| F\_SUB           | r  =  x - y.                                     |
| F\_SUBADD        | r0 =  x - y, r1 = x + y.                         |
| F\_ADDSUB        | r0 =  x + y, r1 = x - y.                         |
| F\_MUL           | r  =  x * y.                                     |
| F\_NMUL          | r  = -(x * y).                                   |

### Floating Point Fused Multiply Add and Subtract:
Floating point fused multiply add and subtract instructions are once again similar
to their integer counterparts, except much much basic. All results are stored in the
upper 32-bits of the accumulator.


Below are all the floating point fused multiply add and subtract ops:
|      Opcode       | Args |                      Behavior                        |
| ----------------- | ---- | ---------------------------------------------------- |
| F\_MA\_AC         |  2   | r +=   (x * y).                                      |
| F\_MA             |  3   | r  =   (x * y) + a.                                  |
| F\_MA\_AC\_MV     |  3   | r +=   (x * y), a = r.                               |
| F\_MA\_NMA        |  3   | r0 =   (x * y) + a, r1 = -(x * y) + a.               |
| F\_NMA\_MA        |  3   | r0 =  -(x * y) + a, r1 =  (x * y) + a.               |
| F\_NMA            |  3   | r  =  -(x * y) + a.                                  |
| F\_MA\_MS\_AC\_MV |  3   | r0 +=  (x0 * y0), a0 = r0. r1 -= (x1 * y1), a1 = r1. |
| F\_MS             |  3   | r  =   (x * y) - a.                                  |



### Sine/Cosine/Arctangent:
The DSP has instructions for calculating sine, cosine, and arctangent. These instructions
take their angle values in radians.


For sine and cosine, the calculations are combined into one `F_SINCOS` instruction that
stores the result of sine in the destination of data path 1, and the absolute value of
the angle in the destination of data path 2. Data path 2 has it's source ignored. R12
is always used to store the result of cosine, regardless of which operands are set.


For arctangent, the `F_ARCTAN` instruction only computes the arctangent of the angle.
If data path 2 is in use, the destination is just cleared, making it useless.

These instructions use the move instruction layout, example:
```
F_SINCOS R04, CR_F_NEG_1 : R13, CR_0x00000000;
F_ARCTAN R04, CR_F_NEG_1;
```

Below are the instructions:
|      Opcode      |                     Behavior                         |
| ---------------- | ---------------------------------------------------- |
| F\_SINCOS        | dst0 = sin(src0), dst1 = abs(src0), r12 = cos(src0). |
| F\_ARCTAN        | dst0 = atan(src0), dst1 = 0.                         |



### Reciprocal:
The DSP has two floating point reciprocal instructions, one just calculates the
reciprocal of the given value, and the other calculates the reciprocal of the
square root of the given value. Both can only operate on a single data path.


Below are the instructions:
|      Opcode      |                     Behavior                         |
| ---------------- | ---------------------------------------------------- |
| F\_RCP           | dst0 = 1 / src0.                                     |
| F\_RCP\_SQRT     | dst0 = 1 / sqrt(src0).                               |


### Reciprocal Division:
As far as I can tell, the DSP has no dedicated division instructions. However, this
instruction seems to be a way of using a floating point reciprocal to do division.


This is accomplished by taking three values, the numerator, the denominator,
and the reciprocal of the denominator. This instruction basically does this:

```
r00 = 2.0f.
r01 = 1.0f / 7.0f, the reciprocal of the denominator.
r12 = 7.0f, the denominator.

So, essentially we're calculating 2.0f / 7.0f.

F_RCP_DIV R00, R00, R01 : R01, R12, R01;

After this instruction:
r00 = r00 * r01.
r01 = r12 * r01.

Now, here's the odd part:
r12 is always set in this instruction, regardless of what destination register is set.
To get the value r12 is set with, we take the result of data path 2, finding the first
set bit starting from the least significant bit, unsetting that bit, then setting all bits
above it. We also lower the floating point exponent by 1, making it 2 ^ -1 instead of 2 ^ 0.

With these values, the first result is would be:

r01 = 0x3f804000; float 1.001953.

The first set bit is bit 14, which if we unset it, and set all the bits above it, we get
0x3fff8000, and if we subtract one from the exponent, we get 0x3f7f8000.

Now, if we repeat the same instruction:

F_RCP_DIV R00, R00, R01 : R01, R12, R01;

We end up with r01 = 0x3f800020. The first set bit is bit 5, which if we unset it and set
all bits above it, we get 0x3fffffc0. Subtracting one from the exponent, we get 0x3f7fffc0.

After about three iterations of this, we end up with 0x3f800000 (float 1.0f) and 0x3f7fffff (0.99999999).

Somebody with more understanding of floating point may be able to give a better explanation of why this
is done, but it seems to be slowly rounding towards a more correct answer. Each iteration gives a much
cleaner result which is stored in data path 0.


```

|      Opcode      | Args |                     Behavior                            |
| ---------------- | ---- | ------------------------------------------------------- |
| F\_RCP\_DIV      |  2   | r0 = x0 * y0, r1 = x1 * y1, r12 = behavior shown above. |


### DFT Calculation:
These instructions implement common math used in discrete fourier transform operations. Each
instruction requires both data paths to be in use. I am not an expert in this, so my explanation
may leave a bit to be desired, but I'll try my best:

`F_DFT_TWDL` implements the 'twiddle' calculation of a DFT, where we take four values:
- `in_r`, or input real. The real number portion of the input.
- `in_i`, or input imaginary, the imaginary portion of the input.
- `tw_r`, or twiddle real. The real portion of the twiddle factor.
- `tw_i`, or twiddle imaginary. The imaginary portion of the twiddle factor.

The instruction behavior is shown below:
```
F_DFT_TWDL R04, R00, R01 : R12, R02, R03;

would have:
R04 = (R00 * R02) - (R01 * R03), which is the equivalent of (in_r * tw_r) - (in_i * tw_i);
R12 = (R00 * R03) + (R01 * R02), which is the equivalent of (in_r * tw_i) + (in_i * tw_r);

Where R04 contains the real result, and R12 contains the imaginary result.
```

`F_DFT_BFLY` implements the butterfly calculation of a DFT, where we take 4 inputs:
- `in_r` from the previous twiddle calculation.
- `in_i` from the previous twiddle calculation.
- `res_r` which is the real result from the previous twiddle calculation.
- `res_i` which is the imaginary result from the previous twiddle calculation.


We also have four outputs, two pairs of real and imaginary results. The instruction
behavior is shown below:
```
F_DFT_BFLY R04, R00, R01, R05 :
           R12, R02, R03, R13;

would have:
R04 = R00 + R02, which is the equivalent of in_r + res_r.
R12 = R01 + R03, which is the equivalent of in_i + res_i.
R05 = R00 - R02, which is the equivalent of in_r - res_r.
R13 = R01 - R03, which is the equivalent of in_i - res_i.
```

|      Opcode      | Args |                     Behavior                     |
| ---------------- | ---- | ------------------------------------------------ |
| F\_DFT\_TWDL     |  2   | Does DFT Twiddle, described above.               |
| F\_DFT\_BFLY     |  3   | Does DFT Butterfly, described above.             |



## Value Manipulation Instructions


### Absolute Value
Basic absolute value instruction. Has float and integer version.

- `I_ABS R00, R01;`, r00 = abs(r01).
- `F_ABS R00, R01;`, r00 = abs(r01).


### Negate Value
#### INV:
Basic sign inversion instruction. Has float and integer version.

- `I_INV R00, R01;`, r00 = -r01.
- `F_INV R00, R01;`, r00 = -r01.


### Type Conversion
These instructions convert values between integer and floating point representations.
The formatting is a little bit odd though, as it uses `r = x y` representation. Register ranges
are [here.](#r--x-y-register-ranges)

#### I\_TO\_F:
Takes two integer operands in x and y, and creates a floating point value in r. Example:

`I_TO_F R00, R01, R02;`, r00 = (float)(r01 * (pow(2, r02))).


If the x value is zero, the floating point exponent is still set.


#### F\_TO\_I:
Takes a float value in x, an integer value in y, and creates an integer value in r. Example:

`F_TO_I R00, R01, R02;`, r00 = (int)(r01 * (pow(2, r02))).


So, if the y value is 0, you get a straight float to int conversion. Otherwise, you get the floating
point value in x multiplied by 2 to the power of y.

### Value Comparison
The value comparison instructions for both integer and floating point are essentially a
subtract instruction, except the result is discarded and only the `COND_REG` register is
set. These can then be used for conditional checks.

#### Integer Comparison
Integer value comparisons have a few different possible outcomes. Since conditional
checks only apply to the conditional bits of data path 1, those are the only ones I'll be
listing below:

| Condition | `COND_REG` Result |
| --------- | ----------------- |
|  X ==  Y  | 0x68d (000 1101)  |
|  X  >  Y  | 0x689 (000 1001)  |
|  X  > -Y  | 0x688 (000 1000)  |
|  X  <  Y  | 0x68a (000 1010)  |
| -X  <  Y  | 0x68b (000 1011)  |

The reason that some results of the same conditional have the first bit (the carry bit)
set is because the ALU treats integer subtraction as addition, it essentially does `X + -(Y)`.
This means that in certain circumstances, even though two conditions are the same, i.e
`X < Y` and `-X < Y`, the results in the `COND_REG` can be different.


#### Float Comparison
Floating point value comparisons are much more basic. I will list them below:

| Condition | `COND_REG` Result |
| --------- | ----------------- |
|  X ==  Y  | 0x204 (000 0100)  |
|  X  >  Y  | 0x200 (000 0000)  |
|  X  <  Y  | 0x202 (000 0010)  |

Due to these being handled by the FPU and not having any twos complement trickery, they're pretty
straight forward.


## Floating Point Value Extract
### Significand Extract

### Exponent Extract

### Exponent Add


## Parallel Instructions:

### MOV\_P/MOV\_T1\_P Instructions:

### EXEC\_COND\_P Instructions:

## Register Ranges:
Register ranges for each instruction type and length.


### MOV SRC, DST Register Ranges:
#### Single Length Register Range:
##### Single Data Path:
|   Operand Type    |            Range             |
| ----------------- | ---------------------------- |
| Source            | 7-bits, R00-@A\_R7\_INC\_REG.|
| Destination       | 7-bits, R00-@A\_R7\_INC\_REG.|


##### Dual Data Path:
|   Operand Type    |            Range             |
| ----------------- | ---------------------------- |
| Source0           | 4-bits, R00-R15.             |
| Destination0      | 3-bits, R00-R07.             |
| Source1           | 4-bits, R00-R15.             |
| Destination1      | 3-bits, R08-R15. Base 8.     |

#### Double Length Register Range:
##### Single Data Path:
|   Operand Type    |            Range                    |
| ----------------- | ----------------------------------- |
| Source            | 10-bits, R00-DMA\_CFG\_ACTIVE\_REG. |
| Destination       | 10-bits, R00-DMA\_CFG\_ACTIVE\_REG. |


##### Dual Data Path:
|   Operand Type    |            Range             |
| ----------------- | ---------------------------- |
| Source0           | 4-bits, R00-R15.             |
| Destination0      | 4-bits, R00-R15.             |
| Source1           | 4-bits, R00-R15.             |
| Destination1      | 4-bits, R00-R15.             |


##### Literal Address:
Literal operands take the form of `@#0x0000_X` or `@#0x0000_Y` for X and Y RAM, respectively.
|   Operand Type     |            Range                   |
| -----------------  | ---------------------------------- |
| Source/Dest Reg    | 4-bits, R00-R15.                  |
| Source/Dest Literal| 16-bits, 0x0000-0xffff XRAM/YRAM. |


##### Literal Value:
Length two literal value moves are limited to 16-bit values, and must be the same
literal value for both destination registers. Literal value operands take the form of
`#0xffff`.
|   Operand Type     |            Range                   |
| -----------------  | ---------------------------------- |
| Destination0       | 11-bits, R00-YGPRAM\_015. | 
| Source0 Literal    | 16-bits, #0x0000-#0xffff. |
| Destination1       | 11-bits, R00-YGPRAM\_015. | 
| Source1 Literal    | 16-bits, #0x0000-#0xffff. |


#### Quad Length Register Range:
##### Dual Data Path (default):
|   Operand Type    |            Range              |
| ----------------- | ----------------------------- |
| Source0           | 11-bits, R00-YGPRAM\_015.     |
| Destination0      | 11-bits, R00-YGPRAM\_015.     |
| Source1           | 11-bits, R00-YGPRAM\_015.     |
| Destination1      | 11-bits, R00-YGPRAM\_015.     |

##### Literal Value:
Length four literal value moves are 32-bits each, and can be different for each
data path.
|   Operand Type     |            Range                   |
| -----------------  | ---------------------------------- |
| Destination0       | 11-bits, R00-YGPRAM\_015.          | 
| Source0 Literal    | 32-bits, #0x00000000-#0xffffffff   |
| Destination1       | 11-bits, R00-YGPRAM\_015.          | 
| Source1 Literal    | 32-bits, #0x00000000-#0xffffffff   |


### MOV SRC, DST Literal Ranges:
#### Single Length Register Range:
Single length literal move instructions seem to be broken, so I haven't included it.
It seems to overlap the literal value with the register operand.


#### Double Length Register Range:
Length two literal value moves are limited to 16-bit values, and must be the same
literal value for both destination registers. Literal value operands take the form of
`#0xffff`.
|   Operand Type     |            Range                   |
| -----------------  | ---------------------------------- |
| Destination0       | 11-bits, R00-YGPRAM\_015. |
| Source0 Literal    | 16-bits, #0x0000-#0xffff. |
| Destination1       | 11-bits, R00-YGPRAM\_015. |
| Source1 Literal    | 16-bits, #0x0000-#0xffff. |


#### Quad Length Register Range:
Length four literal value moves are 32-bits each, and can be different for each
data path.
|   Operand Type     |            Range                   |
| -----------------  | ---------------------------------- |
| Destination0       | 11-bits, R00-YGPRAM\_015.          |
| Source0 Literal    | 32-bits, #0x00000000-#0xffffffff   |
| Destination1       | 11-bits, R00-YGPRAM\_015.          |
| Source1 Literal    | 32-bits, #0x00000000-#0xffffffff   |


### MOVX A\_REG, OFFSET Register Ranges:

#### Single Length Register Range:
##### Single Data Path (literal offset):
|   Operand Type     |            Range                                |
| ------------------ | ----------------------------------------------- |
| Source/Dest Reg    | 5-bits, R00-A\_R7\_MDFR.                        |
| Source/Dest A\_Reg | A\_R0-A\_R7, literal 7-int offset, -0x40/+0x3f. |


##### Single Data Path (address modifier offset):
|   Operand Type     |            Range                             |
| ------------------ | -------------------------------------------- |
| Source/Dest Reg    | 5-bits, R00-A\_R7\_MDFR.                     |
| Source/Dest A\_Reg | A\_R0-A\_R7, A\_R0\_MDFR-A\_R7\_MDFR offset. |


#### Double Length Register Range:
##### Single Data Path (literal offset):
|   Operand Type     |            Range                                     |
| ------------------ | ---------------------------------------------------- |
| Source/Dest Reg    | 5-bits, R00-A\_R7\_MDFR.                             |
| Source/Dest A\_Reg | A\_R0-A\_R7, literal 17-int offset, -0x8000/+0x7fff. |


##### Single Data Path (address modifier offset):
|   Operand Type     |            Range                             |
| ------------------ | -------------------------------------------- |
| Source/Dest Reg    | 5-bits, R00-A\_R7\_MDFR.                     |
| Source/Dest A\_Reg | A\_R0-A\_R7, A\_R0\_MDFR-A\_R7\_MDFR offset. |

##### Dual Data Path (literal offset):
|   Operand Type        |            Range                                   |
| --------------------- | -------------------------------------------------- |
| Source/Dest Reg0      | 5-bits, R00-A\_R7\_MDFR.                           |
| Source\Dest A\_Reg\_X | A\_R0-A\_R7, literal 11-int offset, -0x400/+0x3ff. |
| Source/Dest Reg1      | 5-bits, R00-A\_R7\_MDFR.                           |
| Source\Dest A\_Reg\_Y | A\_R0-A\_R7, literal 11-int offset, -0x400/+0x3ff. |


##### Dual Data Path (address modifier offset):
|   Operand Type        |            Range                             |
| --------------------- | -------------------------------------------- |
| Source/Dest Reg0      | 5-bits, R00-A\_R7\_MDFR.                     |
| Source/Dest A\_Reg\_X | A\_R0-A\_R7, A\_R0\_MDFR-A\_R7\_MDFR offset. |
| Source/Dest Reg1      | 5-bits, R00-A\_R7\_MDFR.                     |
| Source/Dest A\_Reg\_Y | A\_R0-A\_R7, A\_R0\_MDFR-A\_R7\_MDFR offset. |


#### Quad Length Register Range:
Unknown.


### R = X Y Register Ranges:

#### Single Length Register range:
##### Single Data Path:
|  Operand Type  |            Range             |
| -------------- | ---------------------------- |
| R              | 5-bits, R00-A\_R7\_MDFR.     |
| X              | 5-bits, R00-A\_R7\_MDFR.     |
| Y              | 5-bits, R00-A\_R7\_MDFR.     |

##### Dual Data Path:
|  Operand Type     |            Range             |
| ----------------- | ---------------------------- |
| R0                | R04-R07, R12-R15.            |
| X0                | R00-R01, R04-R05.            |
| Y0                | R00-R03.                     |
| R1                | R04-R07, R12-R15.            |
| X1                | R08-R09, R12-R13.            |
| Y1                | R07-R11.                     |


##### Literal Value:
|  Operand Type  |        Range.                    |
| -------------- | -------------------------------- |
| R              | 4-bits, R00-R15.                 |
| X              | 4-bits, R00-R15.                 |
| Y              | 8-bit literal int, -0x80-0x7f.   |

#### Double Length Register Range:
##### Single Data Path:
|  Operand Type  |            Range             |
| -------------- | ---------------------------- |
| R              | 7-bits, R00-@A\_R7\_INC\_REG.|
| X              | 7-bits, R00-@A\_R7\_INC\_REG.|
| Y              | 7-bits, R00-@A\_R7\_INC\_REG.|


Dual data path:
|  Operand Type  |            Range             |
| -------------- | ---------------------------- |
| R0             |  R00-R15.                    |
| X0             |  R00-R07.                    |
| Y0             |  R00-R05, R12-R13.           |
| R1             |  R00-R15.                    |
| X1             |  R08-R15.                    |
| Y1             |  R04-R05, R08-R11, R12-R13.  |


##### Literal Address:
Literal operands take the form of `@#0x0000_X` or `@#0x0000_Y` for X and Y RAM, respectively.
|   Operand Type     |            Range                   |
| -----------------  | ---------------------------------- |
| R                  | 4-bits, R00-R15.                   |
| X                  | 16-bits, 0x0000-0xffff XRAM/YRAM.  |
| Y                  | 4-bits, R00-R15.                   |


##### Literal Value:
|   Operand Type     |            Range                    |
| -----------------  | ----------------------------------- |
| R                  | 11-bits, R00-YGPRAM\_015.           | 
| X                  | 11-bits, R00-YGPRAM\_015.           | 
| Y                  | 16-bit literal int, -0x8000-0x7fff. |


#### Quad Length Register Range:
##### Single Data Path:
|   Operand Type    |            Range              |
| ----------------- | ----------------------------- |
| R                 | 11-bits, R00-YGPRAM\_015.     |
| X                 | 11-bits, R00-YGPRAM\_015.     |
| Y                 | 11-bits, R00-YGPRAM\_015.     |


##### Dual Data Path (without parallel op):
|   Operand Type    |            Range              |
| ----------------- | ----------------------------- |
| R0                | 11-bits, R00-YGPRAM\_015.     |
| X0                | 11-bits, R00-YGPRAM\_015.     |
| Y0                | 11-bits, R00-YGPRAM\_015.     |
| R1                | 11-bits, R00-YGPRAM\_015.     |
| X1                | 11-bits, R00-YGPRAM\_015.     |
| Y1                | 11-bits, R00-YGPRAM\_015.     |

##### Dual Data Path (with parallel op):
In this case, the second data path's register values must be in the range of
a signed 10-bit int, or -0x200/+0x1ff.
|   Operand Type    |            Range                |
| ----------------- | ------------------------------- |
| R0                | 11-bits, R00-YGPRAM\_015.       |
| X0                | 11-bits, R00-YGPRAM\_015.       |
| Y0                | 11-bits, R00-YGPRAM\_015.       |
| R1                | Operand R0 + 10-bit int offset. |
| X1                | Operand X0 + 10-bit int offset. |
| Y1                | Operand Y0 + 10-bit int offset. |

##### Literal Value:
##### Single Data Path:
|   Operand Type     |            Range                   |
| ------------------ | ---------------------------------- |
| R0                 | 11-bits, R00-YGPRAM\_015.          |
| X0                 | 11-bits, R00-YGPRAM\_015.          |
| Y0                 | 32-bit int, -0x80000000-0x7fffffff |


##### Dual Data Path:
|   Operand Type     |            Range                   |
| ------------------ | ---------------------------------- |
| R0                 | 11-bits, R00-YGPRAM\_015.          |
| X0                 | 11-bits, R00-YGPRAM\_015.          |
| Y0                 | 32-bit int, -0x80000000-0x7fffffff |
| R1                 | Operand R0 + 10-bit int offset.    |
| X1                 | Operand X0 + 10-bit int offset.    |
| Y1                 | 32-bit int, -0x80000000-0x7fffffff |


### R = X Y A Register Ranges:

#### Single Length Register Range:
|  Operand Type  |            Range                    |
| -------------- | ----------------------------------- |
| R              | R00-R01, R04-R05, R08-R09, R12-R13. |
| X              | 4-bits, R00-R15.                    |
| Y              | 4-bits, R00-R15.                    |
| A              | 4-bits, R00-R15.                    |


#### Double Length Register Range:
##### Single Data Path:
|  Operand Type  |            Range             |
| -------------- | ---------------------------- |
| R              | 4-bits, R00-R15.             |
| X              | 4-bits, R00-R15.             |
| Y              | 4-bits, R00-R15.             |
| A              | 4-bits, R00-R15.             |


##### Dual Data Path:
|  Operand Type  |            Range             |
| -------------- | ---------------------------- |
| R0             |  R04-R07.                    |
| X0             |  R00-R05, R08-R09.           |
| Y0             |  R00-R07.                    |
| A0             |  R04-R07, R12-R15.           |
| R1             |  R12-R15.                    |
| X1             |  R00-R01, R08-R13.           |
| Y1             |  R08-R15.                    |
| A1             |  R04-R07, R12-R15.           |


#### Quad Length Register Range:
##### Single Data Path:
|   Operand Type    |            Range              |
| ----------------- | ----------------------------- |
| R                 | 11-bits, R00-YGPRAM\_015.     |
| X                 | 11-bits, R00-YGPRAM\_015.     |
| Y                 | 11-bits, R00-YGPRAM\_015.     |
| A                 | 11-bits, R00-YGPRAM\_015.     |


##### Dual Data Path (without parallel op):
|   Operand Type    |            Range              |
| ----------------- | ----------------------------- |
| R0                | 11-bits, R00-YGPRAM\_015.     |
| X0                | 11-bits, R00-YGPRAM\_015.     |
| Y0                | 11-bits, R00-YGPRAM\_015.     |
| A0                | 11-bits, R00-YGPRAM\_015.     |
| R1                | 11-bits, R00-YGPRAM\_015.     |
| X1                | 11-bits, R00-YGPRAM\_015.     |
| Y1                | 11-bits, R00-YGPRAM\_015.     |
| A1                | 11-bits, R00-YGPRAM\_015.     |

##### Dual Data Path (with parallel op):
In this case, the second data path's register values must be in the same 4-bit range
of the first data path's value. So, R15 cannot get A\_R0, it can only get the other R00-R14 range.
|   Operand Type    |            Range              |
| ----------------- | ----------------------------- |
| R0                | 11-bits, R00-YGPRAM\_015.     |
| X0                | 11-bits, R00-YGPRAM\_015.     |
| Y0                | 11-bits, R00-YGPRAM\_015.     |
| A0                | 11-bits, R00-YGPRAM\_015.     |
| R1                | Operand R0 + 4-bit offset.    |
| X1                | Operand X0 + 4-bit offset.    |
| Y1                | Operand Y0 + 4-bit offset.    |
| A1                | Operand Y0 + 4-bit offset.    |
