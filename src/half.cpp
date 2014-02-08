/*
 * utils.cpp
 *
 *  Created on: Feb 8, 2014
 *      Author: denia
 */

#include "half.h"

static union _FP32 half_to_float_full(union _FP16 h)
{
	union _FP32 o = { 0 };

	// From ISPC ref code
	if (h.Exponent == 0 && h.Mantissa == 0) { // (Signed) zero
		o.Sign = h.Sign;
	}
	else
	{
		if (h.Exponent == 0) // Denormal (will convert to normalized)
		{
			// Adjust mantissa so it's normalized (and keep track of exp adjust)
			int e = -1;
			uint32_t m = h.Mantissa;
			do
			{
				e++;
				m <<= 1;
			} while ((m & 0x400) == 0);

			o.Mantissa = (m & 0x3ff) << 13;
			o.Exponent = 127 - 15 - e;
			o.Sign = h.Sign;
		}
		else if (h.Exponent == 0x1f) // Inf/NaN
		{
			// NOTE: It's safe to treat both with the same code path by just truncating
			// lower Mantissa bits in NaNs (this is valid).
			o.Mantissa = int(h.Mantissa) << 13;
			o.Exponent = 255;
			o.Sign = h.Sign;
		}
		else // Normalized number
		{
			o.Mantissa = int(h.Mantissa) << 13;
			o.Exponent = 127 - 15 + h.Exponent;
			o.Sign = h.Sign;
		}
	}

	return o;
}

static union _FP16 float_to_half_full(union _FP32 f)
{
	union _FP16 o = { 0 };

	// Based on ISPC reference code (with minor modifications)
	if (f.Exponent == 0) // Signed zero/denormal (which will underflow)
		o.Exponent = 0;
	else if (f.Exponent == 255) // Inf or NaN (all exponent bits set)
	{
		o.Exponent = 31;
		o.Mantissa = f.Mantissa ? 0x200 : 0; // NaN->qNaN and Inf->Inf
	}
	else // Normalized number
	{
		// Exponent unbias the single, then bias the halfp
		int newexp = f.Exponent - 127 + 15;
		if (newexp >= 31) // Overflow, return signed infinity
			o.Exponent = 31;
		else if (newexp <= 0) // Underflow
		{
			if ((14 - newexp) <= 24) // Mantissa might be non-zero
			{
				uint16_t mant = f.Mantissa | 0x800000; // Hidden 1 bit
				o.Mantissa = mant >> (14 - newexp);
				if ((mant >> (13 - newexp)) & 1) // Check for rounding
					o.u++; // Round, might overflow into exp bit, but this is OK
			}
		}
		else
		{
			o.Exponent = newexp;
			o.Mantissa = f.Mantissa >> 13;
			if (f.Mantissa & 0x1000) // Check for rounding
				o.u++; // Round, might overflow to inf, this is OK
		}
	}

	o.Sign = f.Sign;
	return o;
}
