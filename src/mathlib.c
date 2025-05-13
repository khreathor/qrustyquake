// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// Copyright (C) 2007-2008 Kristian Duske
// GPLv3 See LICENSE for details.

// mathlib.c -- math primitives

#include "quakedef.h"

vec3_t vec3_origin = { 0, 0, 0 };
s32 nanmask = 255 << 23;

void VectorSubtract(const vec3_t a, const vec3_t b, vec3_t c)
{ c[0] = a[0] - b[0]; c[1] = a[1] - b[1]; c[2] = a[2] - b[2]; }
void VectorAdd(const vec3_t a, const vec3_t b, vec3_t c)
{ c[0] = a[0] + b[0]; c[1] = a[1] + b[1]; c[2] = a[2] + b[2]; }
void VectorCopy(const vec3_t a, vec3_t b)
{ b[0] = a[0]; b[1] = a[1]; b[2] = a[2]; }

vec_t VectorLength(vec3_t v) { return sqrt(DotProduct(v,v)); }

void ProjectPointOnPlane(vec3_t dst, const vec3_t p, const vec3_t normal)
{
	float d;
	vec3_t n;
	float inv_denom;

	inv_denom = 1.0F / DotProduct(normal, normal);

	d = DotProduct(normal, p) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

float anglemod(float a)
{
	a = (360.0 / 65536) * ((s32)(a * (65536 / 360.0)) & 65535);
	return a;
}


s32 BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, mplane_t *p)
{ // Returns 1, 2, or 1 + 2
	float dist1 = 0, dist2 = 0;
	switch (p->signbits) { // general case
	case 0:
	dist1=p->normal[0]*emaxs[0]+p->normal[1]*emaxs[1]+p->normal[2]*emaxs[2];
	dist2=p->normal[0]*emins[0]+p->normal[1]*emins[1]+p->normal[2]*emins[2];
	break;
	case 1:
	dist1=p->normal[0]*emins[0]+p->normal[1]*emaxs[1]+p->normal[2]*emaxs[2];
	dist2=p->normal[0]*emaxs[0]+p->normal[1]*emins[1]+p->normal[2]*emins[2];
	break;
	case 2:
	dist1=p->normal[0]*emaxs[0]+p->normal[1]*emins[1]+p->normal[2]*emaxs[2];
	dist2=p->normal[0]*emins[0]+p->normal[1]*emaxs[1]+p->normal[2]*emins[2];
	break;
	case 3:
	dist1=p->normal[0]*emins[0]+p->normal[1]*emins[1]+p->normal[2]*emaxs[2];
	dist2=p->normal[0]*emaxs[0]+p->normal[1]*emaxs[1]+p->normal[2]*emins[2];
	break;
	case 4:
	dist1=p->normal[0]*emaxs[0]+p->normal[1]*emaxs[1]+p->normal[2]*emins[2];
	dist2=p->normal[0]*emins[0]+p->normal[1]*emins[1]+p->normal[2]*emaxs[2];
	break;
	case 5:
	dist1=p->normal[0]*emins[0]+p->normal[1]*emaxs[1]+p->normal[2]*emins[2];
	dist2=p->normal[0]*emaxs[0]+p->normal[1]*emins[1]+p->normal[2]*emaxs[2];
	break;
	case 6:
	dist1=p->normal[0]*emaxs[0]+p->normal[1]*emins[1]+p->normal[2]*emins[2];
	dist2=p->normal[0]*emins[0]+p->normal[1]*emaxs[1]+p->normal[2]*emaxs[2];
	break;
	case 7:
	dist1=p->normal[0]*emins[0]+p->normal[1]*emins[1]+p->normal[2]*emins[2];
	dist2=p->normal[0]*emaxs[0]+p->normal[1]*emaxs[1]+p->normal[2]*emaxs[2];
	break;
	default:
		Sys_Error("BoxOnPlaneSide:  Bad signbits");
		break;
	}
	s32 sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;
	return sides;
}

void AngleVectors(vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float angle = angles[YAW] * (M_PI * 2 / 360);
	float sy = sin(angle);
	float cy = cos(angle);
	angle = angles[PITCH] * (M_PI * 2 / 360);
	float sp = sin(angle);
	float cp = cos(angle);
	angle = angles[ROLL] * (M_PI * 2 / 360);
	float sr = sin(angle);
	float cr = cos(angle);
	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;
	right[0] = (-1 * sr * sp * cy + -1 * cr * -sy);
	right[1] = (-1 * sr * sp * sy + -1 * cr * cy);
	right[2] = -1 * sr * cp;
	up[0] = (cr * sp * cy + -sr * -sy);
	up[1] = (cr * sp * sy + -sr * cy);
	up[2] = cr * cp;
}

void VectorMA(vec3_t veca, float scale, vec3_t vecb, vec3_t vecc)
{
	vecc[0] = veca[0] + scale * vecb[0];
	vecc[1] = veca[1] + scale * vecb[1];
	vecc[2] = veca[2] + scale * vecb[2];
}

void CrossProduct(vec3_t v1, vec3_t v2, vec3_t cross)
{
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

vec_t Length(vec3_t v)
{
	float length = 0;
	for (s32 i = 0; i < 3; i++)
		length += v[i] * v[i];
	length = sqrt(length); // FIXME
	return length;
}

float VectorNormalize(vec3_t v)
{
	float length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrt(length); // FIXME
	if (length) {
		float ilength = 1 / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
	return length;
}

void VectorInverse(vec3_t v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void VectorScale(vec3_t in, vec_t scale, vec3_t out)
{
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
}

void R_ConcatRotations(float in1[3][3], float in2[3][3], float out[3][3])
{
	out[0][0]=in1[0][0]*in2[0][0]+in1[0][1]*in2[1][0]+in1[0][2]*in2[2][0];
	out[0][1]=in1[0][0]*in2[0][1]+in1[0][1]*in2[1][1]+in1[0][2]*in2[2][1];
	out[0][2]=in1[0][0]*in2[0][2]+in1[0][1]*in2[1][2]+in1[0][2]*in2[2][2];
	out[1][0]=in1[1][0]*in2[0][0]+in1[1][1]*in2[1][0]+in1[1][2]*in2[2][0];
	out[1][1]=in1[1][0]*in2[0][1]+in1[1][1]*in2[1][1]+in1[1][2]*in2[2][1];
	out[1][2]=in1[1][0]*in2[0][2]+in1[1][1]*in2[1][2]+in1[1][2]*in2[2][2];
	out[2][0]=in1[2][0]*in2[0][0]+in1[2][1]*in2[1][0]+in1[2][2]*in2[2][0];
	out[2][1]=in1[2][0]*in2[0][1]+in1[2][1]*in2[1][1]+in1[2][2]*in2[2][1];
	out[2][2]=in1[2][0]*in2[0][2]+in1[2][1]*in2[1][2]+in1[2][2]*in2[2][2];
}

void R_ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4])
{
	out[0][0]=in1[0][0]*in2[0][0]+in1[0][1]*in2[1][0]+in1[0][2]*in2[2][0];
	out[0][1]=in1[0][0]*in2[0][1]+in1[0][1]*in2[1][1]+in1[0][2]*in2[2][1];
	out[0][2]=in1[0][0]*in2[0][2]+in1[0][1]*in2[1][2]+in1[0][2]*in2[2][2];
	out[0][3]=in1[0][0]*in2[0][3]+in1[0][1]*in2[1][3]+in1[0][2]*in2[2][3]+in1[0][3];
	out[1][0]=in1[1][0]*in2[0][0]+in1[1][1]*in2[1][0]+in1[1][2]*in2[2][0];
	out[1][1]=in1[1][0]*in2[0][1]+in1[1][1]*in2[1][1]+in1[1][2]*in2[2][1];
	out[1][2]=in1[1][0]*in2[0][2]+in1[1][1]*in2[1][2]+in1[1][2]*in2[2][2];
	out[1][3]=in1[1][0]*in2[0][3]+in1[1][1]*in2[1][3]+in1[1][2]*in2[2][3]+in1[1][3];
	out[2][0]=in1[2][0]*in2[0][0]+in1[2][1]*in2[1][0]+in1[2][2]*in2[2][0];
	out[2][1]=in1[2][0]*in2[0][1]+in1[2][1]*in2[1][1]+in1[2][2]*in2[2][1];
	out[2][2]=in1[2][0]*in2[0][2]+in1[2][1]*in2[1][2]+in1[2][2]*in2[2][2];
	out[2][3]=in1[2][0]*in2[0][3]+in1[2][1]*in2[1][3]+in1[2][2]*in2[2][3]+in1[2][3];
}

// Returns mathematically correct (floor-based) quotient and remainder for
// numer and denom, both of which should contain no fractional part. The
// quotient must fit in 32 bits.
void FloorDivMod(double numer, double denom, s32 *quotient, s32 *rem)
{
	s32 q, r;
	if (numer >= 0.0) {
		double x = floor(numer / denom);
		q = (s32)x;
		r = (s32)floor(numer - (x * denom));
	} else { // perform operations with positive values, and fix mod
		double x = floor(-numer / denom); // to make floor-based
		q = -(s32)x;
		r = (s32)floor(-numer - (x * denom));
		if (r != 0) {
			q--;
			r = (s32)denom - r;
		}
	}
	*quotient = q;
	*rem = r;
}

s32 GreatestCommonDivisor(s32 i1, s32 i2)
{
	if (i1 > i2) {
		if (i2 == 0)
			return (i1);
		return GreatestCommonDivisor(i2, i1 % i2);
	} else {
		if (i1 == 0)
			return (i2);
		return GreatestCommonDivisor(i1, i2 % i1);
	}
}
