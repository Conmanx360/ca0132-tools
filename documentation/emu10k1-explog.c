/*
 * This file was taken from the emu10k1 archives. I've cleaned up the code a
 * bit and fixed some of the bugs, but other than that it's unchanged.
 */
/*  By: Daniel Bertrand
    Date: Nov 6th, 2000

    Description: A software implementation of creatives LOG and EXP instructions
    found in the emu10k1 processor. (Patent: WO 9901814 (A1))

    I'm not sure if this is even legal, being a software implementation of a patented hardware instruction,
    so I'm releasing it into the public domain (no license, no copyright).

Compiling:

exp:
 gcc  explog.c -o exp

usage:
 ./exp <log_data> <max_exponent_size> <Sign_control>

log:
 gcc  explog.c -DLOG -o log

usage:
 ./log <lin_data> <max_exponent_size> <Sign_control>


--
log converts linear to logarithmic
exp converts logarithmic to linear

Logarithmic representation:


| 31 |30     29-25|28-24        0|
|sign| exponent   |   Mantissa   |

exponent size varies between 2 and 5 bits,

<max_exp_size> is specified in actual value, thus 3 bits is <max_exp_size>=7

max_exp_size can be anyvalue between 2 and 31.
*/


#include <stdio.h>
#include <stdlib.h>

#include <linux/types.h>

__u32 log1(__u32 lin_data,__u32 max_exp,__u32 sign_ctrl)
{
	int exp = (int)max_exp + 1;
	__u32 result;
	__u8 sign;


	//FIX ME: mask inputs (i think this is what it does, haven't checked it yet):
	sign_ctrl &= 0x03;
	max_exp &= 0x1f;
	sign = (lin_data >> 31) & 0x01;

	/* Smallest exponent value is 2, if we're below that, default to 2. */
	if (max_exp < 2)
		max_exp = 2;

	/* If the value is negative, one's complement the bits. */
	if (sign)
		lin_data= ~lin_data;

	/*
	 * Continue to shift left until we either:
	 * - End up with the exponent being 0.
	 * - Or end up with the MSB set.
	 */
	while (!(lin_data & 0x80000000)) {
		lin_data <<= 1;
		exp--;
		if (!exp)
			break;
	}

	/*
	 * If the exponent is not 0, shift left by 1 to get rid of the
	 * implicit bit.
	 */
	if (exp)
		lin_data <<= 1;

	if (max_exp & 0x10)
		result = (exp << 26) | (lin_data >> 6);
	else if (max_exp & 0x8)
		result = (exp << 27) | (lin_data >> 5);
	else if (max_exp & 0x4)
		result = (exp << 28) | (lin_data >> 4);
	else
		result = (exp << 29) | (lin_data >> 3);


	printf("Exponent %d, mantissa 0x%08x, result 0x%08x.\n", exp, ((lin_data >> 1) + 0x80000000) >> 6, result);

	/*
	  Sign control
	  ------------

	  the result is complemented depending on the following conditions

	  |----------------|----------|---------|
	  | Sign Control   | sign=0   | sign=1  |
	  |----------------|----------|---------|
	  |    00          |    R     |  ~R     |
	  |----------------|----------|---------|
	  |    01          |    R     |   R     |
	  |----------------|----------|---------|
	  |    10          |   ~R     |  ~R     |
	  |----------------|----------|---------|
	  |    11          |   ~R     |   R     |
	  |----------------|----------|---------|
	*/

	switch( sign_ctrl){
	case 0:
		if (sign)
			result = ~result;
		break;

	case 2:
		result = ~result;
		break;

	case 3:
		if (!sign)
			result= ~result;
		break;

	default:
		break;
	}

	return result;
}

__u32 exp1(__u32 log_data,__u32 max_exp,__u32 sign_ctrl)
{
	int exp = (int)max_exp + 1;
	__u32 result;
	__u8 sign;

	sign_ctrl &= 0x03;
	max_exp &= 0x1f;
	sign = (log_data >> 31) & 0x01;

	/* Smallest exponent value is 2, if we're below that, default to 2. */
	if (max_exp < 2)
		max_exp = 2;

	/* If the value is negative, one's complement the bits. */
	if (sign)
		log_data = ~log_data;

	//seperate mantissa and exponent
	if (max_exp & 0x10) {
		exp = log_data >> 26;
		log_data <<= 6;
	} else if (max_exp & 0x8) {
		exp = log_data >> 27;
		log_data <<= 5;
	} else if (max_exp & 0x4) {
		exp = log_data >> 28;
		log_data <<= 4;
	} else {
		exp = log_data >> 29;
		log_data <<= 3;
	}

	//put back the implicit 1
	if (exp) {
		log_data >>= 1;
		log_data |= 0x80000000;
		exp--;
	}

	//shift mantissa back down
	result = log_data >> (max_exp - exp);

	//handle sign
	switch( sign_ctrl){

	case 0:
		if(sign!=0)
			result= ~result;
		return result;
	case 1:
		return result;
	case 2:
		return ~result;
	case 3:
		if(sign==0)
			result= ~result;
		return result;
	}

}

int  main (int argc, char *argv[])
{

	if(argc!=4){
		printf("usage: log <lin_value> <max exponent size> <sign control>");
		return -1;
	}



#ifdef LOG
	printf("0x%08x\n",log1(strtoul(argv[1],NULL,16),strtoul(argv[2],NULL,16),strtoul(argv[3],NULL,16) ));
#else
	printf("0x%08x\n",exp1(strtoul(argv[1],NULL,16),strtoul(argv[2],NULL,16),strtoul(argv[3],NULL,16) ));
#endif
	return 0;
}
