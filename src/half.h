/*
 * utils.h
 *
 *  Created on: Feb 8, 2014
 *      Author: denia
 */

#ifndef HALF_H_
#define HALF_H_

#include <stdint.h>

typedef short int16_t ;
typedef unsigned short uint16_t ;
//typedef unsigned long uint32_t ;


union _FP32 {
	uint32_t u;
	float f;
	struct {
		uint32_t Mantissa : 23;
		uint32_t Exponent : 8;
		uint32_t Sign : 1;
	};
};

union _FP16 {
	uint16_t u;
	struct {
		uint16_t Mantissa : 10;
		uint16_t Exponent : 5;
		uint16_t Sign : 1;
	};
};

union _FP32 half_to_float_full(union _FP16 h);
union _FP16 float_to_half_full(union _FP32 f);

#endif /* UTILS_H_ */
