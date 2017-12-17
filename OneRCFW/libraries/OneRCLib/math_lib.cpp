/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *       A amateur remote control software library. Use at your own risk.
 *
 * @file    math_lib.cpp
 * @brief
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <math.h>
#include "math_lib.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Public functions declaration
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Private functions declaration
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * Math_FastSqrt - Calculate square root with approximation method.
 *
 * https://en.wikipedia.org/wiki/Methods_of_computing_square_roots
 * http://www.codeproject.com/Articles/69941/Best-Square-Root-Method-Algorithm-Function-Precisi
 *
 * Comparison (Simulated with Octave):
 *    sqrt(123.503524017)           = 11.1132139373360399
 *    sqrt_approx(123.503524017)    = 11.4220952987670898
 *
 * @param   [in]        val     The input value.
 *
 * @return  [float]     The square root of input value.
 *
 */
float Math_FastSqrt(float val)
{
    int32_t val_int = *((int32_t *)&val);   /* Same bits, but as an int */

    if(val < 0.0)
        return NAN;

    val_int -= ((int32_t)1 << 23);          /* Subtract 2^m. */
    val_int >>= 1;                          /* Divide by 2. */
    val_int += ((int32_t)1 << 29);          /* Add ((b + 1) / 2) * 2^m. */
    val_int += (-0x4C000);                  /* Adjust approximation errors */

    return *((float *)&val_int);            /* Interpret again as float */
}

/**
 * Math_FastInvSqrt - Fast Inverse Square Root.
 *
 * http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
 *
 * @param   [in]        x       The input value.
 *
 * @return  [float]     The square root of input value.
 *
 */
float Math_FastInvSqrt(float x)
{
    float xhalf = 0.5f * x;
    int32_t i = *(int32_t *)&x;             /* get bits for floating value */

    i = 0x5f3759df - (i >> 1);              /* gives initial guess y0 */
    x = *(float *)&i;                       /* convert bits back to float */
    x = x * (1.5f - xhalf * x * x);         /* Newton step, repeating increases accuracy */

    return x;
}

/**
 * Math_Atan2Nvidia -
 *
 * http://http.developer.nvidia.com/Cg/atan2.html
 *
 * Comparison (Simulated with Octave):
 *    atan2(0.54035, 0.08464)           = 1.4154196679854725
 *    atan2_nvidia(0.54035, 0.08464)    = 1.4154194593429565
 *
 * Speed (Run on Arduino)
 *    Call AHRS_Atan2Nvidia(0.54035, 0.08464) 10,0000 time requires 17273148 us,
 *    17273148 / 10,0000 = 172.73 us
 *
 * @param   [in]        y   Value of y-axis.
 * @param   [in]        x   Value of x-axis.
 *
 * @return  [float]     The result of atan(y / x)
 *
 */
float Math_Atan2Nvidia(float y, float x)
{
    float t0, t1, t3, t4;

    t3 = fabsf(x);
    t1 = fabsf(y);
    t0 = fmaxf(t3, t1);
    t1 = fminf(t3, t1);
    t3 = float(1) / t0;
    t3 = t1 * t3;

    t4 = t3 * t3;
    t0 =         - float(0.013480470);
    t0 = t0 * t4 + float(0.057477314);
    t0 = t0 * t4 - float(0.121239071);
    t0 = t0 * t4 + float(0.195635925);
    t0 = t0 * t4 - float(0.332994597);
    t0 = t0 * t4 + float(0.999995630);
    t3 = t0 * t3;

    t3 = (fabsf(y) > fabsf(x)) ? float(1.570796327) - t3 : t3;
    t3 = (x < 0) ?  float(3.141592654) - t3 : t3;
    t3 = (y < 0) ? -t3 : t3;

    return t3;
}

/**
 * Math_Atan2Approx1 -
 *
 * https://gist.github.com/volkansalma/2972237
 *
 * Comparison (Simulated with Octave):
 *    atan2(0.54035, 0.08464)           = 1.4154196679854725
 *    atan2_approx1(0.54035, 0.08464)   = 1.4251055717468262
 *
 * Speed (Run on Arduino)
 *    Call AHRS_Atan2Approx1(0.54035, 0.08464) 10,0000 time requires 10394132 us,
 *    10394132 / 10,0000 = 103.94 us
 *
 * @param   [in]        y   Value of y-axis.
 * @param   [in]        x   Value of x-axis.
 *
 * @return  [float]     The result of atan(y / x)
 *
 */
float Math_Atan2Approx1(float y, float x)
{
    //http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
    //Volkan SALMA

    const float ONEQTR_PI = MATH_PI / 4.0;
	const float THRQTR_PI = 3.0 * MATH_PI / 4.0;
	float r, angle;
	float abs_y = fabs(y) + 1e-10f;      // kludge to prevent 0/0 conditio
	if ( x < 0.0f )
	{
		r = (x + abs_y) / (abs_y - x);
		angle = THRQTR_PI;
	}
	else
	{
		r = (x - abs_y) / (x + abs_y);
		angle = ONEQTR_PI;
	}
	angle += (0.1963f * r * r - 0.9817f) * r;

	if ( y < 0.0f )
		return( -angle );     // negate if in quad III or IV
	else
		return( angle );
}

/**
 * Math_Atan2Approx2 -
 *
 * https://gist.github.com/volkansalma/2972237
 * http://lists.apple.com/archives/perfoptimization-dev/2005/Jan/msg00051.html
 *
 * Comparison (Simulated with Octave):
 *    atan2(0.54035, 0.08464)           = 1.4154196679854725
 *    atan2_approx1(0.54035, 0.08464)   = 1.4152259826660156
 *
 * Speed (Run on Arduino)
 *    Call AHRS_Atan2Approx1(0.54035, 0.08464) 10,0000 time requires 9935108 us,
 *    9935108 / 10,0000 = 99.351 us
 *
 * @param   [in]        y   Value of y-axis.
 * @param   [in]        x   Value of x-axis.
 *
 * @return  [float]     The result of atan(y / x)
 *
 */
float Math_Atan2Approx2(float y, float x)
{
	if(x == 0.0f){
		if (y > 0.0f)
            return MATH_PI_HALF;
		if (y == 0.0f)
            return 0.0f;

		return -(MATH_PI_HALF);
	}

	float atan;
	float z = y / x;

	if(fabs(z) < 1.0f){
		atan = z / (1.0f + 0.28f * z * z);

		if(x < 0.0f){
			if (y < 0.0f)
                return atan - MATH_PI;

			return atan + MATH_PI;
		}
	}
	else{
		atan = MATH_PI_HALF - z / (z * z + 0.28f);

		if (y < 0.0f)
            return atan - MATH_PI;
	}

	return atan;
}

/**
 * Math_FastSin -
 *
 * http://lab.polygonal.de/2007/07/18/fast-and-accurate-sinecosine-approximation/
 *
 * High precision sine/cosine (~8x faster).
 *
 * @param   [in]        x   Input radian
 *
 * @return  [float]     The result of sin(x)
 *
 */
float Math_FastSin(float x)
{
    float sin;

    //always wrap input angle to -PI..PI
    if(x < -3.14159265)
        x += 6.28318531;
    else if(x >  3.14159265)
        x -= 6.28318531;

    //compute sine
    if(x < 0){
        sin = 1.27323954 * x + .405284735 * x * x;

        if(sin < 0)
            sin = .225 * (sin *-sin - sin) + sin;
        else
            sin = .225 * (sin * sin - sin) + sin;
    }
    else{
        sin = 1.27323954 * x - 0.405284735 * x * x;

        if(sin < 0)
            sin = .225 * (sin *-sin - sin) + sin;
        else
            sin = .225 * (sin * sin - sin) + sin;
    }

    return sin;
}

/**
 * Math_FastCos -
 *
 * http://lab.polygonal.de/2007/07/18/fast-and-accurate-sinecosine-approximation/
 *
 * High precision sine/cosine (~8x faster).
 *
 * @param   [in]        x   Input radian
 *
 * @return  [float]     The result of cos(x)
 *
 */
float Math_FastCos(float x)
{
    float cos;

    //compute cosine: sin(x + PI/2) = cos(x)
    x += 1.57079632;
    if(x >  3.14159265)
        x -= 6.28318531;

    if(x < 0){
        cos = 1.27323954 * x + 0.405284735 * x * x;

        if(cos < 0)
            cos = .225 * (cos *-cos - cos) + cos;
        else
            cos = .225 * (cos * cos - cos) + cos;
    }
    else{
        cos = 1.27323954 * x - 0.405284735 * x * x;

        if(cos < 0)
            cos = .225 * (cos *-cos - cos) + cos;
        else
            cos = .225 * (cos * cos - cos) + cos;
    }

    return cos;
}

/**
 * Math_FastSin2 -
 *
 * http://forum.devmaster.net/t/fast-and-accurate-sine-cosine/9648
 *
 * Fast and accurate sine/cosine.
 *
 * @param   [in]        x   Input radian
 *
 * @return  [float]     The result of sin(x)
 *
 */
float Math_FastSin2(float x)
{
    const float B = 4 / MATH_PI;
    const float C = -4 / (MATH_PI * MATH_PI);
    const float P = 0.225;
    float y;

    y = B * x + C * x * fabs(x);
    y = P * (y * fabs(y) - y) + y;   // Q * y + P * y * abs(y)

    return y;
}

/**
 * Math_FastCos2 -
 *
 * http://forum.devmaster.net/t/fast-and-accurate-sine-cosine/9648
 *
 * Fast and accurate sine/cosine.
 * cos(x) = sin(x + pi/2).
 *
 * @param   [in]        x   Input radian
 *
 * @return  [float]     The result of cos(x)
 *
 */
float Math_FastCos2(float x)
{
    const float B = 4 / MATH_PI;
    const float C = -4 / (MATH_PI * MATH_PI);
    const float P = 0.225;
    float y;

    x += MATH_PI_HALF;
    y = B * x + C * x * fabs(x);
    y = P * (y * fabs(y) - y) + y;   // Q * y + P * y * abs(y)

    return y;
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */
