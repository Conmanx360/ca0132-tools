# Quartet DSP (X-Fi DSP)
The ca0132 codec is essentially the same hardware as the Sound Blaster X-Fi,
with the difference being that it sits behind an 8051 microcontroller which
handles the HDA interface.


Unless firmware is loaded, the DSP is disabled. If the firmware is uploaded,
the DSP is taken out of it's halt state and set to run. Once the DSP is running,
all audio streams are routed through it and it's DMA controllers.

## Sections:
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
  - [Main Instructions](#main-instructions)
    - [MOV](#mov-based-instructions)
    - [MOVX](#movx-based-instructions)
    - [CALL/JMP](#calljmps_jmps_call-instructions)
    - [RET/RETI](#retreti-instructions)
    - [INT/HALT](#interrupt-instructions)
    - [ADD/SUB/MUL/NMUL/FMAC](#addsubmulnmulfmac-instructions)
    - [FMA/FMS/NFMA](#fmafmsnfma-instructions)
    - [AND/OR/XOR](#andorxor-instructions)
    - [INV/ABS/CMPL](#invabscmpl-instructions)
    - [I\_TO\_F/F\_TO\_I](#i_to_ff_to_i-instructions)
    - [RR/RL/ARITH\_RR/ARITH\_RL](#rrrlarith_rrarith_rl-instructions)
    - [FFS/FFU/I\_ABS\_FFS](#ffsffui_abs_ffs-instructions)
    - [SET\_BIT/CLR\_BIT/TGL\_BIT](#set_bitclr_bittgl_bit-instructions)
    - [GET\_BITS/SET\_BITS](#get_bitsset_bits-instructions)
    - [POP/PUSH](#poppush-instructions)
  - [Parallel Instructions](#parallel-instructions)
    - [MOV\_P/MOV\_T1\_P](#movpmovt1p-instructions)
    - [EXEC\_COND\_P](#execcondp-instructions)
  - [Register Ranges](#register-ranges)
    - [MOV SRC, DST](#mov-src-dst-register-ranges)
    - [MOVX A\_REG, OFFSET](#movx-a_reg-offset-register-ranges)
    - [R = X Y](#r--x-y-register-ranges)
    - [R = X Y A](#r--x-y-a-register-ranges)


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

COND\_REG is the register that contains flags which are set after a math operation,
with 7 bits for each data path. I haven't done too much testing on these, but
they seem to be compared against with a bitmask when using conditionals.
Bits 0-7 represent data path 0's flags, and bits 8-14 represent data path 1's.
Even if data path 1 is unused, it's flags will still be set.


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

## Main Instructions:
According to Creative, there are 235 main instructions. I will detail the ones I have found out here.

### MOV Based Instructions:
MOV based instructions all have two operands, a source and a destination.


#### MOV/MOV\_T1/MOV\_T2:
The main MOV instructions come in three types.


- MOV is a regular move, and moves between all register normally.
- MOV\_T1 Moves the upper 32-bits of R04/R05/R12/R13 (accumulators.)
- MOV\_T2 moves the upper 8 bits of R04/R05/R12/R13.

#### MOV with source modifiers:
The main MOV instruction can also be used with source modifiers, which are:

- Increment, example: `MOV R00, R01++;`.
- Decrement, example: `MOV R00, R01--;`.
- Rotate right by 1, example: `MOV R00, R01 >> 1;`.
- Rotate left  by 1, example: `MOV R00, R01 << 1;`.

These can only be used with regular MOV instructions, so MOV\_T1 and MOV\_T2 are incompatible.
Length two literals are compatible.

#### MOV Literals:
MOV literals consistent of:

- MOV, which is a normal literal MOV. Also has MOV\_T1 and MOV\_T2 variants.
- MOV\_L, which moves only to the lower 16-bits. This should be used in 16-bit value sets. Has \_T1 and \_T2 variants.
- MOV\_U, which moves only to the upper 16-bits. This should be used in 16-bit value sets. Only has \_T1 variant, as \_T2 is only 8-bits.

### MOVX Based Instructions:

MOVX has \_T1 and regular MOV variants, but no \_T2 variant. No MOVX
instruction supports parallel instructions.

#### MOVX With Literal Integer Offset:
MOVX with literal integer offsets take an address register argument with a
literal integer offset. They can be source/destination swapped. In the case of
dual data path instructions, the first address register must be XRAM, and the
second must be YRAM. In single data path instructions, either can be used.

- Address register destination example: `MOVX @A_R0_X - 17, R02;`.
- Address register source example: `MOVX R03, @A_R6_Y + 18;`.
- Dual example `MOVX @A_R2_X + 5, R00 : @A_R2_Y + 2, R01;`.


#### MOVX With Address Modifier Offset:
Same behavior as above, except instead of taking a literal integer offset, it
takes an address modifier register.

- Address register destination example: `MOVX @A_R0_X + A_MD2, R02;`.
- Address register source example: `MOVX R03, @A_R6_Y + A_MD0;`.
- Dual example `MOVX @A_R2_X + A_MD3, R00 : @A_R2_Y + A_MD4, R01;`.


### CALL/JMP/S\_JMP/S\_CALL Instructions:
JMPC/JMPC\_T1 are not fully understood.


#### JMP/JMPC/JMPC\_T1/CALL:
These instructions take an address literal, or an address register to set the
PC. They all have an 8-bit conditional that is not yet fully understood.
Literal `#0x0f` seems to be always true.

- Call literal address example: `CALL #0x0f, #0xdf00;`.
- Call address register example: `CALL #0x0f, A_R0;`.
- Jump literal address example: `JMP #0x0f, #0x0e20;`.
- Jump address register example: `JMP #0x0f, A_R0;`.


#### S\_JMP/S\_JMPC/S\_JMPC\_T1/S\_CALL:
These instructions take a signed integer offset from the current PC. They also
take the same 8-bit conditional as the regular instructions.

- Call literal address example: `CALL #0x0f, #0xdf00;`.
- Jump literal address example: `JMP #0x0f, #0x0e20;`.


### RET/RETI Instructions:


#### RET.
Regular return, `RET;`. Decrements the PC\_STK\_PTR lower 16-bits.


#### RETI.
Interrupt return, `RETI;`. Decrements the PC\_STK\_PTR lower 16-bits, and the
STA\_S\_STK\_PTR, which stores the COND\_CODE register, and restores it's
value.


### Interrupt Instructions:

#### INT\_ENABLE/INT\_DISABLE:
These instructions enable/disable processor interrupts, `INT_ENABLE;`,
`INT_DISABLE;`.


#### INT\_CLR:
Clears the interrupt pin provided by the literal, `INT_CLR #0x00;` through
`INT_CLR #0x0f;`.


### ADD/SUB/MUL/NMUL/FMAC Instructions:
All of these instructions  have the format of `r = (x + y)` with different operator variations
between the two operands. Register ranges are [here.](#r--x-y-register-ranges)


#### ADD:
Basic addition, has format of `r = x + y`. Has `_T1` variant with unknown difference.

- `I_ADD R00, R02, R01;`, r00 = r02 + r01.
- `F_ADD R00, R02, R01;`, r00 = r02 + r01, but floating point.


#### SUB:
Basic subtraction, has format of `r = x - y`. Has `_T1` variant with unknown difference.

- `I_SUB R00, R02, R01;`, r00 = r02 - r01.
- `F_SUB R00, R02, R01;`, r00 = r02 - r01, but floating point.


#### MUL:
Basic multiplication, has format of `r = x * y`. Has `_T1` and `_T2` variants, with unknown differences.

- `I_MUL R00, R02, R01;`, r00 = r02 * r01.
- `F_MUL R00, R02, R01;`, r00 = r02 * r01, but floating point.

#### NMUL:
Basic multiplication, except product is negated. Has format of `r = -(x * y)`.

- `I_NMUL R00, R02, R01;`, r00 = -(r02 * r01).
- `F_NMUL R00, R02, R01;`, r00 = -(r02 * r01), but floating point.

#### FMAC:
Fused multiply and add to the accumulator, except in this case it's the destination register, so
`r += x * y`. Currently only a floating point variant is known.

- `F_FMAC R04, R02, R01;`, r04 += r02 * r01.


### FMA/FMS/NFMA Instructions:
FMA instructions all have the format of `r = (x * y) + a`, with different operator variations
between the operands. Register ranges are [here.](#r--x-y-a-register-ranges)

#### FMA:
Basic fused multiply add, `r = (x * y) + a`. `I_FMA` for integer, and `F_FMA` for
floating point. There is currently one known opcode that does FMA a little bit differently,
and I've marked this `_T2`. Example instruction:


- `I_FMA R04, R12, R02, R03;`, r04 = (r12 * r02) + r03.
- `F_FMA R04, R12, R02, R03;`, r04 = (r12 * r02) + r03, but floating point.


#### FMS:
Same basic format of FMA, except this time it's fused multiply and subtract, `r = (x * y) - a`.
Has a `_T1` variant, unknown difference.


- `I_FMS R04, R12, R02, R03;`, r04 = (r12 * r02) - r03.
- `F_FMS R04, R12, R02, R03;`, r04 = (r12 * r02) - r03, but floating point.


#### NFMA:
Basic FMA, except the product of the multiplication is negated, `r = -(x * y) + a`.
Has a `_T1` variant, unknown difference.


- `I_NFMA R04, R12, R02, R03;`, r04 = -(r12 * r02) + r03.
- `F_NFMA R04, R12, R02, R03;`, r04 = -(r12 * r02) + r03, but floating point.


### AND/OR/XOR Instructions:
All of these instructions  have the format of `r = (x & y)` with different operator variations
between the two operands. Register ranges are [here.](#r--x-y-register-ranges)

#### AND:
Basic bitwise AND instruction.

- `AND R00, R02, R03;`, r00 = r02 & r03.


#### OR:
Basic bitwise OR instruction.

- `OR R00, R02, R03;`, r00 = r02 | r03.


#### XOR:
Basic bitwise XOR instruction.

- `XOR R00, R02, R03;`, r00 = r02 ^ r03.


### INV/ABS/CMPL Instructions:
All of these instructions share the MOV layout, so `dst = src`, with different modifiers
applied to the source operand. Register ranges are [here.](#mov-src-dst-register-ranges)

#### INV:
Basic sign inversion instruction. Has float and integer version.

- `I_INV R00, R01;`, r00 = -r01.
- `F_INV R00, R01;`, r00 = -r01.


#### ABS:
Basic absolute value instruction. Has float and integer version.

- `I_ABS R00, R01;`, r00 = abs(r01).
- `F_ABS R00, R01;`, r00 = abs(r01).


#### CMPL:
Bitwise complement. Only one version, because this wouldn't make sense to have integer/float
versions.

- `CMPL R00, R01;`, r00 = ~r01.


### I\_TO\_F/F\_TO\_I Instructions:
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


### RR/RL/ARITH\_RR/ARITH\_RL Instructions:
These instructions perform bitshifts, and take the form `r = x >> y`, with either left or
right bitshifts. Register ranges are [here.](#r--x-y-register-ranges)

#### RR/RL:
Regular rotate instructions. Has a `_T1` variant, unknown difference. Examples:

- `RL R00, R02, #2;`, r00 = r02 << 2.
- `RL R00, R02, R01;`, r00 = r02 << r01.
- `RR R00, R02, #3;`, r00 = r02 >> 3.
- `RR R00, R02, R06;`, r00 = r02 >> r06.


#### ARITH\_RR/ARITH\_RL:
Arithmetic rotate instructions. These do sign extension of the MSB if it is set.
This allows the preservation of the sign of a bitshifted signed int. Examples:

- `ARITH_RL R00, R02, #2;`, r00 = r02 << 2.
- `ARITH_RL R00, R02, R01;`, r00 = r02 << r01.
- `ARITH_RR R00, R02, #3;`, r00 = r02 >> 3.
- `ARITH_RR R00, R02, R06;`, r00 = r02 >> r06.


An example of this behavior:
```
if r02 = 0x80000000,
ARITH_RR R00, R02, #1;

Will result in r00 = 0xc0000000, as the sign bit is extended.

if r02 = 0x00000000,
ARITH_RR R00, R02, #1;

Will result in r00 = 0x00000000, as there is no sign bit to extend.
```

### FFS/FFU/I\_ABS\_FFS Instructions:
These instructions find the first set/unset bit, starting from the MSB. These take a
source and destination register, like a MOV instruction. Register ranges are [here.](#mov-src-dst-register-ranges)

#### FFS:
Find first set instruction. Finds the first bit set in the source register, counting from the MSB.
Example:

- `FFS R00, CR_0x00000001;`, would have r00 = 0x1f.


#### FFU:
Find first unset instruction. Finds the first bit unset in the source register, counting from the MSB.
Example:

- `FFU R00, CR_0xfffffffe;`, would have r00 = 0x1f.


#### I\_ABS\_FFS:
This instruction gets the absolute value of the signed int in the source register before finding
the first set bit. Due to negative integers having a larger range, they count an extra bit.
Example:

- `I_ABS_FFS R00, CR_0x00000002;`, would have r00 = 0x1d, 29.
- `I_ABS_FFS R00, CR_0xfffffffe;`, would have r00 = 0x1e, 30.


As you can see, -2's first set bit is one larger than positive two, due to having different ranges.


### SET\_BIT/CLR\_BIT/TGL\_BIT Instructions:
These instructions set, clear, and toggle bits in a register. They take the form of `r = x | (1 << y)`.
There are also two special instructions for setting/clearing the SEMAPHORE\_G\_REG bits. Register ranges
are [here.](#r--x-y-register-ranges)


#### SET\_BIT:
This one takes the form of `r = x | (1 << y)`. If the bit is unset, it's set. Otherwise,
it's left alone. Examples:

- `SET_BIT R00, R01, R02;`, r00 = r01 | (1 << r02);
- `SET_BIT R00, R01, #2;`, r00 = r01 | (1 << 2);


#### CLR\_BIT:
This one takes the form of `r = x & ~(1 << y)`. If the bit is set, it's cleared. Otherwise,
it's left alone. Examples:

- `CLR_BIT R00, R01, R02;`, r00 = r01 & ~(1 << r02);
- `CLR_BIT R00, R01, #1;`, r00 = r01 & ~(1 << 1);


#### TGL\_BIT:
This one takes the form of `r = x ^ (1 << y)`. If the bit is 1, it's cleared, if it's 0, it's set.
Examples:

- `TGL_BIT R00, R01, R02;`, r00 = r01 & ~(1 << r02);
- `TGL_BIT R00, R01, #3;`, r00 = r01 & ~(1 << 3);


#### SET\_SEM\_G\_BIT/CLR\_SEM\_G\_BIT:
Does the same as the regular SET\_BIT instruction, except r in `r = x | (1 << y)` is
always the SEMAPHORE\_G\_REG, and the supplied r register operand is always cleared. Only
takes literal values for the Y operand. Examples:

- `SET_SEM_G_BIT R04, R01, #5;`, SEMAPHORE\_G\_REG = r01 | (1 << 5), r04 = 0.
- `CLR_SEM_G_BIT R04, R01, #5;`, SEMAPHORE\_G\_REG = r01 & ~(1 << 5), r04 = 0.


### GET\_BITS/SET\_BITS Instructions:
These instructions allow for extracting/setting bits within a register. Both GET and SET
require both data paths to function, as they take operands from both data paths. GET register
ranges are [here,](#r--x-y-register-ranges), SET register ranges are [here.](#r--x-y-a-register-ranges)

#### GET\_BITS:
This one takes the form of `r0 = (x0 >> y0) & ((1 << x1) - 1)`. r0 is the destination register,
x0 is the register to extract the bits from, y0 is the starting bit, and x1 is the bit count.
If r1 is set to a non-constant register, it stores the value of y0 + x1, which is the highest
bit of the extracted value. The y1 register seems to go unused.

Example:

- `GET_BITS R00, R01, R02 : R03, R06, CR_0x00000000;`, r00 = (r01 >> r02) & ((1 << r06) - 1). r03 = r02 + r06.


#### SET\_BITS:
This one takes the form of `r0 = ((a0 & ((1 << a1) - 1)) << x0) | y0`. This has three different variants.
The default behavior is to clear the bits to be set before setting them. The \_T1 behavior doesn't clear
the bits to set before setting them, making it essentially bitshift with OR. The \_T2 variant clears all
bits above the highest bit of our set value. Operands x1 and y1 seem to go unused. Operand r1 behaves
the same as GET\_BITS, where the starting bit and bit count are added together and stored into it.
Examples of each variant:


```
r01 = 0x10;
r02 = 0x01234567;
r03 = 0x06;
r06 = 0x08;

SET_BITS R00, R01, R02, R03 : CR_0x00000000, CR_0x00000000, CR_0x00000000, R06;
Results in r00 = 0x01064567;

SET_BITS_T1 R00, R01, R02, R03 : CR_0x00000000, CR_0x00000000, CR_0x00000000, R06;
Results in r00 = 0x01274567;

SET_BITS_T2 R00, R01, R02, R03 : CR_0x00000000, CR_0x00000000, CR_0x00000000, R06;
Results in r00 = 0x00064567;
```


### POP/PUSH Instructions:

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
