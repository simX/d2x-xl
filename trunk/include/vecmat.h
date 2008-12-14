/*
   THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
   SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
   END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
   ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
   IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
   SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
   FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
   CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
   AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
   COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
 */

#ifndef _VECMAT_H
#define _VECMAT_H

#include "maths.h"
#include <cstdlib>
#include <string.h>

namespace VecMat {

const size_t X = 0;
const size_t Y = 1;
const size_t Z = 2;
const size_t W = 3;

const size_t R = 0;
const size_t G = 1;
const size_t B = 2;
const size_t A = 3;

const size_t PA = 0;
const size_t BA = 1;
const size_t HA = 2;

const size_t RVEC = 0;
const size_t UVEC = 1;
const size_t FVEC = 2;
const size_t HVEC = 3;

// forward-declarations for ToFloat () and ToFix ();
class CFixVector;
class CFloatVector;
class CFloatVector3;
class CAngleVector;

class CFixMatrix;
class CFloatMatrix;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 * \class CFixVector
 * A 3 element fixed-point vector.
 */
class CFixVector {
	public:
		static const CFixVector ZERO;
		static const CFixVector XVEC;
		static const CFixVector YVEC;
		static const CFixVector ZVEC;

		static const CFixVector Create (fix f0, fix f1, fix f2);
		static const CFixVector Avg (const CFixVector& src0, const CFixVector& src1);
		static const CFixVector Avg (CFixVector& src0, CFixVector& src1,
								   CFixVector& src2, CFixVector& src3);
		static const CFixVector Cross (const CFixVector& v0, const CFixVector& v1);
		static CFixVector& Cross (CFixVector& dest, const CFixVector& v0, const CFixVector& v1);
		//computes the delta angle between two vectors.
		//vectors need not be normalized. if they are, call CFixVector::DeltaAngleNorm ()
		//the forward vector (third parameter) can be NULL, in which case the absolute
		//value of the angle in returned.  Otherwise the angle around that vector is
		//returned.
		static const fixang DeltaAngle (const CFixVector& v0, const CFixVector& v1, CFixVector *fVec);		//computes the delta angle between two normalized vectors.
		static const fixang DeltaAngleNorm (const CFixVector& v0, const CFixVector& v1, CFixVector *fVec);
		static const fix Dist (const CFixVector& vec0, const CFixVector& vec1);
		static const fix SSEDot (CFixVector *v0, CFixVector *v1);
		static const fix Dot (const CFixVector& v0, const CFixVector& v1);
		static const fix Dot (const fix x, const fix y, const fix z, const CFixVector& v);
		static const fix Normalize (CFixVector& v);
		static const CFixVector Perp (const CFixVector& p0, const CFixVector& p1, const CFixVector& p2);
		static CFixVector& Perp (CFixVector& dest, const CFixVector& p0, const CFixVector& p1, const CFixVector& p2);
		static const CFixVector Normal (const CFixVector& p0, const CFixVector& p1, const CFixVector& p2);
		static const CFixVector Random ();
		static const CFixVector Reflect (const CFixVector& d, const CFixVector& n);
		//return the normalized direction vector between two points
		//dest = normalized (end - start).  Returns Mag of direction vector
		//NOTE: the order of the parameters matches the vector subtraction
		static const fix NormalizedDir (CFixVector& dest, const CFixVector& end, const CFixVector& start);

		// access op for assignment
		fix& operator[] (size_t idx);
		// read-only access op
		const fix operator[] (size_t idx) const;

		bool operator== (const CFixVector& rhs) const;

		const CFixVector& Set (fix x, fix y, fix z);
		void Set (const fix *vec);

		bool IsZero () const;
		void SetZero ();
		const int Sign () const;

		fix SqrMag () const;
		fix Mag () const;
		CFixVector& Neg ();
		const CFixVector operator- () const;
		const bool operator== (const CFixVector& vec);
		const bool operator!= (const CFixVector& vec);
		const CFixVector& operator+= (const CFixVector& vec);
		const CFixVector& operator-= (const CFixVector& vec);
		const CFixVector& operator*= (const CFixVector& s);
		const CFixVector& operator*= (const fix s);
		const CFixVector& operator/= (const fix s);
		const CFixVector operator+ (const CFixVector& vec) const;
		const CFixVector operator- (const CFixVector& vec) const;
		CFixVector& Copy (const CFloatVector3& other);
		CFixVector& Copy (const CFloatVector& other);
		CFixVector& Copy (const CFixVector& other);

		// compute intersection of a line through a point a, with the line being orthogonal relative
		// to the plane given by the Normal n and a point p lieing in the plane, and store it in i.
		const CFixVector PlaneProjection (const CFixVector& n, const CFixVector& p) const;
		//compute the distance from a point to a plane.  takes the normalized Normal
		//of the plane (ebx), a point on the plane (edi), and the point to check (esi).
		//returns distance in eax
		//distance is signed, so Negative Dist is on the back of the plane
		const fix DistToPlane (const CFixVector& n, const CFixVector& p) const;
		//extract heading and pitch from a vector, assuming bank==0
		const CAngleVector ToAnglesVecNorm () const;
		//extract heading and pitch from a vector, assuming bank==0
		const CAngleVector ToAnglesVec () const;
		const CFloatVector ToFloat () const;
		const CFloatVector3 ToFloat3 () const;

	private:
		fix v [3];
};

inline const fix operator* (const CFixVector& v0, const CFixVector& v1);
inline const CFixVector operator* (const CFixVector& v, const fix s);
inline const CFixVector operator* (const fix s, const CFixVector& v);
inline const CFixVector operator/ (const CFixVector& v, const fix d);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/**
 * \class CFloatVector
 * A 4 element floating point vector class
 */
class CFloatVector {
	public:
		static const CFloatVector ZERO;
		static const CFloatVector ZERO4;
		static const CFloatVector XVEC;
		static const CFloatVector YVEC;
		static const CFloatVector ZVEC;

		static const CFloatVector Create (float f0, float f1, float f2, float f3=1.0f);

		static const CFloatVector Avg (const CFloatVector& src0, const CFloatVector& src1);
		static const CFloatVector Cross (const CFloatVector& v0, const CFloatVector& v1);
		static CFloatVector& Cross (CFloatVector& dest, const CFloatVector& v0, const CFloatVector& v1);

		static const float Dist (const CFloatVector& v0, const CFloatVector& v1);
		static const float Dot (const CFloatVector& v0, const CFloatVector& v1);
		static const float Normalize (CFloatVector& vec);
		static const CFloatVector Perp (const CFloatVector& p0, const CFloatVector& p1, const CFloatVector& p2);
		static CFloatVector& Perp (CFloatVector& dest, const CFloatVector& p0, const CFloatVector& p1, const CFloatVector& p2);
		static const CFloatVector Normal (const CFloatVector& p0, const CFloatVector& p1, const CFloatVector& p2);
		static const CFloatVector Reflect (const CFloatVector& d, const CFloatVector& n);

		// access op for assignment
		float& operator[] (size_t idx);
		// read-only access op
		const float operator[] (size_t idx) const;

		bool IsZero () const;

		void SetZero ();
		void Set (const float f0, const float f1, const float f2, const float f3=1.0f);
		void Set (const float *vec);
		const float SqrMag () const;
		const float Mag () const;
		CFloatVector& Neg ();
		CFloatVector3* V3 ();

		const CFloatVector operator- () const;
		const bool operator== (const CFloatVector& other);
		const bool operator!= (const CFloatVector& other);
		const CFloatVector& operator+= (const CFloatVector& other);
		const CFloatVector& operator-= (const CFloatVector& other);
		const CFloatVector& operator*= (const float s);
		const CFloatVector& operator/= (const float s);
		const CFloatVector  operator+ (const CFloatVector& other) const;
		const CFloatVector  operator- (const CFloatVector& other) const;
		CFloatVector& Copy (const CFloatVector3& other);
		CFloatVector& Copy (const CFloatVector& other);
		CFloatVector& Copy (const CFixVector& other);

		const CFixVector ToFix () const;

	private:
		float v [4];
};

const float operator* (const CFloatVector& v0, const CFloatVector& v1);
const CFloatVector operator* (const CFloatVector& v, const float s);
const CFloatVector operator* (const float s, const CFloatVector& v);
const CFloatVector operator/ (const CFloatVector& v, const float s);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/**
 * \class CFloatVector3
 * A 3 element floating point vector class
 */
class CFloatVector3 {
	private:
		float v [3];

	public:
		static const CFloatVector3 ZERO;
		static const CFloatVector3 XVEC;
		static const CFloatVector3 YVEC;
		static const CFloatVector3 ZVEC;

		static const CFloatVector3 Create (float f0, float f1, float f2);
		static const CFloatVector3 Avg (const CFloatVector3& src0, const CFloatVector3& src1);
		static const CFloatVector3 Cross (const CFloatVector3& v0, const CFloatVector3& v1);
		static CFloatVector3& Cross (CFloatVector3& dest, const CFloatVector3& v0, const CFloatVector3& v1);

		static const float Dist (const CFloatVector3& v0, const CFloatVector3& v1);
		static const float Dot (const CFloatVector3& v0, const CFloatVector3& v1);
		static const float Normalize (CFloatVector3& vec);
		static const CFloatVector3 Perp (const CFloatVector3& p0, const CFloatVector3& p1, const CFloatVector3& p2);
		static CFloatVector3& Perp (CFloatVector3& dest, const CFloatVector3& p0, const CFloatVector3& p1, const CFloatVector3& p2);
		static const CFloatVector3 Normal (const CFloatVector3& p0, const CFloatVector3& p1, const CFloatVector3& p2);
		static const CFloatVector3 Reflect (const CFloatVector3& d, const CFloatVector3& n);

		// access op for assignment
		float& operator[] (size_t idx);
		// read-only access op
		const float operator[] (size_t idx) const;

		bool IsZero () const;
		void SetZero ();
		void Set (const float f0, const float f1, const float f2);
		void Set (const float *vec);

		CFloatVector3& Neg ();
		const float Mag () const;
		const float SqrMag () const;

		const CFloatVector3 operator- () const;
		const bool operator== (const CFloatVector3& other);
		const bool operator!= (const CFloatVector3& other);
		const CFloatVector3& operator+= (const CFloatVector3& other);
		const CFloatVector3& operator-= (const CFloatVector3& other);
		const CFloatVector3& operator*= (const float s);
		const CFloatVector3& operator/= (const float s);
		const CFloatVector3 operator+ (const CFloatVector3& other) const;
		const CFloatVector3 operator- (const CFloatVector3& other) const;
		CFloatVector3& Copy (const CFloatVector3& other);
		CFloatVector3& Copy (const CFloatVector& other);
		CFloatVector3& Copy (const CFixVector& other);

		const CFixVector ToFix () const;
};

const float operator* (const CFloatVector3& v0, const CFloatVector3& v1);
const CFloatVector3 operator* (const CFloatVector3& v, float s);
const CFloatVector3 operator* (float s, const CFloatVector3& v);
const CFloatVector3 operator/ (const CFloatVector3& v, float s);

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

//Angle vector.  Used to store orientations

class CAngleVector {
public:
	static const CAngleVector ZERO;
	static const CAngleVector Create (const fixang p, const fixang b, const fixang h) {
		CAngleVector a;
		a [PA] = p; a [BA] = b; a [HA] = h;
		return a;
	}
	// access op for assignment
	fixang& operator[] (size_t idx) { return a [idx]; }
	// read-only access op
	const fixang operator[] (size_t idx) const { return a [idx]; }

	bool IsZero () const { return ! (a [PA] || a [HA] || a [BA]); }
	void SetZero () { memset (a, 0, 3*sizeof (fixang)); }

private:
    fixang a [3];
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// CFloatVector static inlines

inline const CFloatVector CFloatVector::Create (float f0, float f1, float f2, float f3) {
	CFloatVector vec;
	vec.Set (f0, f1, f2, f3);
	return vec;
}

inline const CFloatVector CFloatVector::Avg (const CFloatVector& src0, const CFloatVector& src1) {
	return Create ((src0 [X] + src1 [X]) / 2,
	               (src0 [Y] + src1 [Y]) / 2,
	               (src0 [Z] + src1 [Z]) / 2,
	               1);
}

inline CFloatVector& CFloatVector::Cross (CFloatVector& dest, const CFloatVector& v0, const CFloatVector& v1) {
	dest.Set (v0 [Y] * v1 [Z] - v0 [Z] * v1 [Y],
	          v0 [Z] * v1 [X] - v0 [X] * v1 [Z],
	          v0 [X] * v1 [Y] - v0 [Y] * v1 [X]);
	return dest;
}

inline const CFloatVector CFloatVector::Cross (const CFloatVector& v0, const CFloatVector& v1) {
	return Create (v0 [Y]*v1 [Z] - v0 [Z]*v1 [Y],
	               v0 [Z]*v1 [X] - v0 [X]*v1 [Z],
	               v0 [X]*v1 [Y] - v0 [Y]*v1 [X]);
}

inline const float CFloatVector::Dist (const CFloatVector& v0, const CFloatVector& v1) {
	return (v0-v1).Mag ();
}

inline const float CFloatVector::Dot (const CFloatVector& v0, const CFloatVector& v1) {
	return v0 [X]*v1 [X] + v0 [Y]*v1 [Y] + v0 [Z]*v1 [Z];
}

inline const float CFloatVector::Normalize (CFloatVector& vec) {
	float m = vec.Mag ();
	if (m)
		vec /= m;
	return m;
}

inline CFloatVector& CFloatVector::Perp (CFloatVector& dest, const CFloatVector& p0, const CFloatVector& p1, const CFloatVector& p2) {
	return Cross (dest, p1 - p0, p2 - p1);
}

inline const CFloatVector CFloatVector::Perp (const CFloatVector& p0, const CFloatVector& p1, const CFloatVector& p2) {
	return Cross (p1 - p0, p2 - p1);
}

inline const CFloatVector CFloatVector::Normal (const CFloatVector& p0, const CFloatVector& p1, const CFloatVector& p2) {
	CFloatVector v = 2.0f * Perp (p0, p1, p2);
	Normalize (v);
	return v;
}

inline const CFloatVector CFloatVector::Reflect (const CFloatVector& d, const CFloatVector& n) {
	return -2.0f * Dot (d, n) * n + d;
}

// -----------------------------------------------------------------------------
// CFloatVector member inlines

inline float& CFloatVector::operator[] (size_t idx) { return v [idx]; }

inline const float CFloatVector::operator[] (size_t idx) const { return v [idx]; }

inline bool CFloatVector::IsZero () const { return ! (v [X] || v [Y] || v [Z] || v [W]); }

inline void CFloatVector::SetZero () { memset (v, 0, 4*sizeof (float)); }

inline void CFloatVector::Set (const float f0, const float f1, const float f2, const float f3) {
	v [X] = f0; v [Y] = f1; v [Z] = f2; v [W] = f3;
}

inline void CFloatVector::Set (const float *vec) {
	v [X] = vec [0]; v [Y] = vec [1]; v [Z] = vec [2]; v [W] = 1.0f;
}

inline const float CFloatVector::SqrMag () const {
	return v [X]*v [X] + v [Y]*v [Y] + v [Z]*v [Z];
}

inline const float CFloatVector::Mag () const {
	return (const float) sqrt (SqrMag ());
}

inline CFloatVector& CFloatVector::Neg () { v [0] = -v [0], v [1] = -v [1], v [2] = -v [2]; return *this; }

inline CFloatVector3* CFloatVector::V3 () { return reinterpret_cast<CFloatVector3*> (v); }

inline const CFloatVector CFloatVector::operator- () const {
	return Create (-v [X], -v [Y], -v [Z]);
}

inline CFloatVector& CFloatVector::Copy (const CFloatVector3& other) {
	v [0] = other [0], v [1] = other [1], v [2] = other [2];
	return *this;
}

inline CFloatVector& CFloatVector::Copy (const CFloatVector& other) {
	v [0] = other [0], v [1] = other [1], v [2] = other [2];
	return *this;
}

inline CFloatVector& CFloatVector::Copy (const CFixVector& other) {
	v [0] = X2F (other [0]), v [1] = X2F (other [1]), v [2] = X2F (other [2]);
	return *this;
}

inline const bool CFloatVector::operator== (const CFloatVector& other) {
	return v [0] == other [0], v [1] == other [1], v [2] == other [2];
}

inline const bool CFloatVector::operator!= (const CFloatVector& other) {
	return v [0] != other [0] || v [1] != other [1] || v [2] != other [2];
}

inline const CFloatVector& CFloatVector::operator+= (const CFloatVector& other) {
	v [X] += other [X]; v [Y] += other [Y]; v [Z] += other [Z];
	return *this;
}

inline const CFloatVector& CFloatVector::operator-= (const CFloatVector& other) {
	v [X] -= other [X]; v [Y] -= other [Y]; v [Z] -= other [Z];
	return *this;
}

inline const CFloatVector& CFloatVector::operator*= (const float s) {
	v [0] *= s; v [1] *= s; v [2] *= s;
	return *this;
}

inline const CFloatVector& CFloatVector::operator/= (const float s) {
	v [0] /= s; v [1] /= s; v [2] /= s;
	return *this;
}

inline const CFloatVector CFloatVector::operator+ (const CFloatVector& other) const {
	return Create (v [0]+other [0], v [1]+other [1], v [2]+other [2], 1);
}

inline const CFloatVector CFloatVector::operator- (const CFloatVector& other) const {
	return Create (v [0]-other [0], v [1]-other [1], v [2]-other [2], 1);
}


// -----------------------------------------------------------------------------
// CFloatVector-related non-member ops

inline const float operator* (const CFloatVector& v0, const CFloatVector& v1) {
	return CFloatVector::Dot (v0, v1);
}

inline const CFloatVector operator* (const CFloatVector& v, const float s) {
	return CFloatVector::Create (v [X]*s, v [Y]*s, v [Z]*s, 1);
}

inline const CFloatVector operator* (const float s, const CFloatVector& v) {
	return CFloatVector::Create (v [X]*s, v [Y]*s, v [Z]*s, 1);
}

inline const CFloatVector operator/ (const CFloatVector& v, const float s) {
	return CFloatVector::Create (v [X]/s, v [Y]/s, v [Z]/s, 1);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// CFloatVector3 static inlines

inline const CFloatVector3 CFloatVector3::Create (float f0, float f1, float f2) {
	CFloatVector3 vec;
	vec.Set (f0, f1, f2);
	return vec;
}

inline const CFloatVector3 CFloatVector3::Avg (const CFloatVector3& src0, const CFloatVector3& src1) {
	return Create ((src0 [X] + src1 [X]) / 2,
	               (src0 [Y] + src1 [Y]) / 2,
	               (src0 [Z] + src1 [Z]) / 2);
}

inline CFloatVector3& CFloatVector3::Cross (CFloatVector3& dest, const CFloatVector3& v0, const CFloatVector3& v1) {
	dest.Set (v0 [Y]*v1 [Z] - v0 [Z]*v1 [Y],
	          v0 [Z]*v1 [X] - v0 [X]*v1 [Z],
	          v0 [X]*v1 [Y] - v0 [Y]*v1 [X]);
	return dest;
}

inline const CFloatVector3 CFloatVector3::Cross (const CFloatVector3& v0, const CFloatVector3& v1) {
	return Create (v0 [Y]*v1 [Z] - v0 [Z]*v1 [Y],
	               v0 [Z]*v1 [X] - v0 [X]*v1 [Z],
	               v0 [X]*v1 [Y] - v0 [Y]*v1 [X]);
}

inline const float CFloatVector3::Dist (const CFloatVector3& v0, const CFloatVector3& v1) {
	return (v0-v1).Mag ();
}

inline const float CFloatVector3::Dot (const CFloatVector3& v0, const CFloatVector3& v1) {
	return v0 [X]*v1 [X] + v0 [Y]*v1 [Y] + v0 [Z]*v1 [Z];
}

inline const float CFloatVector3::Normalize (CFloatVector3& vec) {
	float m = vec.Mag ();
	if (m)
		vec /= m;
	return m;
}

inline CFloatVector3& CFloatVector3::Perp (CFloatVector3& dest, const CFloatVector3& p0, const CFloatVector3& p1, const CFloatVector3& p2) {
	return Cross (dest, p1 - p0, p2 - p1);
}

inline const CFloatVector3 CFloatVector3::Perp (const CFloatVector3& p0, const CFloatVector3& p1, const CFloatVector3& p2) {
	return Cross (p1 - p0, p2 - p1);
}

inline const CFloatVector3 CFloatVector3::Normal (const CFloatVector3& p0, const CFloatVector3& p1, const CFloatVector3& p2) {
	CFloatVector3 v = 2.0f*Perp (p0, p1, p2);
	Normalize (v);
	return v;
}

inline const CFloatVector3 CFloatVector3::Reflect (const CFloatVector3& d, const CFloatVector3& n) {
	return -2.0f * Dot (d, n) * n + d;
}

// -----------------------------------------------------------------------------
// CFloatVector3 member inlines

inline float& CFloatVector3::operator[] (size_t idx) { return v [idx]; }

inline const float CFloatVector3::operator[] (size_t idx) const { return v [idx]; }

inline bool CFloatVector3::IsZero () const { return ! (v [0] || v [1] || v [2]); }

inline void CFloatVector3::SetZero () { memset (v, 0, 3*sizeof (float)); }

inline void CFloatVector3::Set (const float f0, const float f1, const float f2) {
	v [X] = f0; v [Y] = f1; v [Z] = f2;
}

inline void CFloatVector3::Set (const float *vec) {
	v [X] = vec [0]; v [Y] = vec [1]; v [Z] = vec [2];
}

inline CFloatVector3& CFloatVector3::Neg () { v [0] = -v [0], v [1] = -v [1], v [2] = -v [2]; return *this; }

inline const float CFloatVector3::SqrMag () const {
	return v [X]*v [X] + v [Y]*v [Y] + v [Z]*v [Z];
}

inline const float CFloatVector3::Mag () const {
	return (const float) sqrt (SqrMag ());
}

inline CFloatVector3& CFloatVector3::Copy (const CFloatVector3& other) {
	v [0] = other [0], v [1] = other [1], v [2] = other [2];
	return *this;
}

inline CFloatVector3& CFloatVector3::Copy (const CFloatVector& other) {
	v [0] = other [0], v [1] = other [1], v [2] = other [2];
	return *this;
}

inline CFloatVector3& CFloatVector3::Copy (const CFixVector& other) {
	v [0] = X2F (other [0]), v [1] = X2F (other [1]), v [2] = X2F (other [2]);
	return *this;
}

inline const bool CFloatVector3::operator== (const CFloatVector3& other) {
	return v [0] == other [0], v [1] == other [1], v [2] == other [2];
}

inline const bool CFloatVector3::operator!= (const CFloatVector3& other) {
	return v [0] != other [0] || v [1] != other [1] || v [2] != other [2];
}

inline const CFloatVector3 CFloatVector3::operator- () const {
	return Create (-v [X], -v [Y], -v [Z]);
}

inline const CFloatVector3& CFloatVector3::operator+= (const CFloatVector3& other) {
	v [0] += other [0]; v [1] += other [1]; v [2] += other [2];
	return *this;
}

inline const CFloatVector3& CFloatVector3::operator-= (const CFloatVector3& other) {
	v [0] -= other [0]; v [1] -= other [1]; v [2] -= other [2];
	return *this;
}

inline const CFloatVector3& CFloatVector3::operator*= (const float s) {
	v [0] *= s; v [1] *= s; v [2] *= s;
	return *this;
}

inline const CFloatVector3& CFloatVector3::operator/= (const float s) {
	v [0] /= s; v [1] /= s; v [2] /= s;
	return *this;
}

inline const CFloatVector3 CFloatVector3::operator+ (const CFloatVector3& other) const {
	return Create (v [0]+other [0], v [1]+other [1], v [2]+other [2]);
}

inline const CFloatVector3 CFloatVector3::operator- (const CFloatVector3& other) const {
	return Create (v [0]-other [0], v [1]-other [1], v [2]-other [2]);
}


// -----------------------------------------------------------------------------
// CFloatVector3-related non-member ops

inline const float operator* (const CFloatVector3& v0, const CFloatVector3& v1) {
	return CFloatVector3::Dot (v0, v1);
}

inline const CFloatVector3 operator* (const CFloatVector3& v, float s) {
	return CFloatVector3::Create (v [X]*s, v [Y]*s, v [Z]*s);
}

inline const CFloatVector3 operator* (float s, const CFloatVector3& v) {
	return CFloatVector3::Create (v [X]*s, v [Y]*s, v [Z]*s);
}

inline const CFloatVector3 operator/ (const CFloatVector3& v, float s) {
	return CFloatVector3::Create (v [X]/s, v [Y]/s, v [Z]/s);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// CFixVector static inlines

inline const CFixVector CFixVector::Create (fix f0, fix f1, fix f2) {
	CFixVector vec;
	vec.Set (f0, f1, f2);
	return vec;
}

inline const CFixVector CFixVector::Avg (const CFixVector& src0, const CFixVector& src1) {
	return Create ( (src0 [X] + src1 [X]) / 2,
	              (src0 [Y] + src1 [Y]) / 2,
	              (src0 [Z] + src1 [Z]) / 2);
}

inline const CFixVector CFixVector::Avg (CFixVector& src0, CFixVector& src1,
						   CFixVector& src2, CFixVector& src3) {
	return Create ( (src0 [X] + src1 [X] + src2 [X] + src3 [X]) / 4,
	 (src0 [Y] + src1 [Y] + src2 [Y] + src3 [Y]) / 4,
	 (src0 [Z] + src1 [Z] + src2 [Z] + src3 [Z]) / 4);

}

inline CFixVector& CFixVector::Cross (CFixVector& dest, const CFixVector& v0, const CFixVector& v1) {
	dest.Set ((fix) (((double) v0 [Y] * (double) v1 [Z] - (double) v0 [Z] * (double) v1 [Y]) / 65536.0),
	          (fix) (((double) v0 [Z] * (double) v1 [X] - (double) v0 [X] * (double) v1 [Z]) / 65536.0),
	          (fix) (((double) v0 [X] * (double) v1 [Y] - (double) v0 [Y] * (double) v1 [X]) / 65536.0));
	return dest;
}

inline const CFixVector CFixVector::Cross (const CFixVector& v0, const CFixVector& v1) {
	return Create ((fix) (((double) v0 [Y] * (double) v1 [Z] - (double) v0 [Z] * (double) v1 [Y]) / 65536.0),
	               (fix) (((double) v0 [Z] * (double) v1 [X] - (double) v0 [X] * (double) v1 [Z]) / 65536.0),
	               (fix) (((double) v0 [X] * (double) v1 [Y] - (double) v0 [Y] * (double) v1 [X]) / 65536.0));
}

//computes the delta angle between two vectors.
//vectors need not be normalized. if they are, call CFixVector::DeltaAngleNorm ()
//the forward vector (third parameter) can be NULL, in which case the absolute
//value of the angle in returned.  Otherwise the angle around that vector is
//returned.
inline const fixang CFixVector::DeltaAngle (const CFixVector& v0, const CFixVector& v1, CFixVector *fVec) {
	CFixVector t0=v0, t1=v1;

	CFixVector::Normalize (t0);
	CFixVector::Normalize (t1);
	return DeltaAngleNorm (t0, t1, fVec);
}

//computes the delta angle between two normalized vectors.
inline const fixang CFixVector::DeltaAngleNorm (const CFixVector& v0, const CFixVector& v1, CFixVector *fVec) {
	fixang a = FixACos (CFixVector::Dot (v0, v1));
	if (fVec) {
		CFixVector t = CFixVector::Cross (v0, v1);
		if (CFixVector::Dot (t, *fVec) < 0)
			a = -a;
	}
	return a;
}

inline const fix CFixVector::Dist (const CFixVector& vec0, const CFixVector& vec1) {
	return (vec0-vec1).Mag ();
}

inline const fix CFixVector::SSEDot (CFixVector *v0, CFixVector *v1) {
	#if ENABLE_SSE
	if (gameStates.render.bEnableSSE) {
			CFloatVector	v0h, v1h;

		VmVecFixToFloat (&v0h, v0);
		VmVecFixToFloat (&v1h, v1);
	#if defined (_WIN32)
		_asm {
			movups	xmm0,v0h
			movups	xmm1,v1h
			mulps		xmm0,xmm1
			movups	v0h,xmm0
			}
	#elif defined (__unix__)
		asm (
			"movups	%0,%%xmm0\n\t"
			"movups	%1,%%xmm1\n\t"
			"mulps	%%xmm1,%%xmm0\n\t"
			"movups	%%xmm0,%0\n\t"
			: "=m" (v0h)
			: "m" (v0h), "m" (v1h)
			);
	#endif
		return (fix) ( (v0h [X] + v0h [Y] + v0h [Z]) * 65536);
		}
	#else
	return 0;
	#endif
}

inline const fix CFixVector::Dot (const CFixVector& v0, const CFixVector& v1) {
		return (fix) ( ( (double) v0 [X] * (double) v1 [X] +
							 (double) v0 [Y] * (double) v1 [Y] +
							 (double) v0 [Z] * (double) v1 [Z])
						  / 65536.0);
}

inline const fix CFixVector::Dot (const fix x, const fix y, const fix z, const CFixVector& v) {
	return (fix) ( ( (double) x * (double) v [X] + (double) y * (double) v [Y] + (double) z * (double) v [Z]) / 65536.0);
}

inline const fix CFixVector::Normalize (CFixVector& v) {
	fix m = v.Mag ();
	v [X] = FixDiv (v [X], m);
	v [Y] = FixDiv (v [Y], m);
	v [Z] = FixDiv (v [Z], m);
	return m;
}

inline CFixVector& CFixVector::Perp (CFixVector& dest, const CFixVector& p0, const CFixVector& p1, const CFixVector& p2) {
	CFixVector t0 = p1 - p0, t1 = p2 - p1;
	Normalize (t0);
	Normalize (t1);
	return Cross (dest, t0, t1);
}

inline const CFixVector CFixVector::Perp (const CFixVector& p0, const CFixVector& p1, const CFixVector& p2) {
	CFixVector t0 = p1 - p0, t1 = p2 - p1;
	Normalize (t0);
	Normalize (t1);
	return Cross (t0, t1);
}

inline const CFixVector CFixVector::Normal (const CFixVector& p0, const CFixVector& p1, const CFixVector& p2) {
	CFixVector v = Perp (p0, p1, p2);
	Normalize (v);
	return v;
}

inline const CFixVector CFixVector::Random () {
	CFixVector v;
	int i = d_rand () % 3;

	if (i == 2) {
		v [X] = (d_rand () - 16384) | 1;	// make sure we don't create null vector
		v [Y] = d_rand () - 16384;
		v [Z] = d_rand () - 16384;
		}
	else if (i == 1) {
		v [Y] = d_rand () - 16384;
		v [Z] = d_rand () - 16384;
		v [X] = (d_rand () - 16384) | 1;	// make sure we don't create null vector
		}
	else {
		v [Z] = d_rand () - 16384;
		v [X] = (d_rand () - 16384) | 1;	// make sure we don't create null vector
		v [Y] = d_rand () - 16384;
	}
	Normalize (v);
	return v;
}

inline const CFixVector CFixVector::Reflect (const CFixVector& d, const CFixVector& n) {
	fix k = Dot (d, n) * 2;
	CFixVector r = n;
	r *= k;
	r -= d;
	r.Neg ();
	return r;
}

//return the normalized direction vector between two points
//dest = normalized (end - start).  Returns Mag of direction vector
//NOTE: the order of the parameters matches the vector subtraction
inline const fix CFixVector::NormalizedDir (CFixVector& dest, const CFixVector& end, const CFixVector& start) {
	dest = end - start;
	return CFixVector::Normalize (dest);
}

// -----------------------------------------------------------------------------
// CFixVector member inlines

inline fix& CFixVector::operator[] (size_t idx) { return v [idx]; }

inline const fix CFixVector::operator[] (size_t idx) const { return v [idx]; }

inline CFixVector& CFixVector::Copy (const CFloatVector3& other) {
	v [0] = F2X (other [0]), v [1] = F2X (other [1]), v [2] = F2X (other [2]);
	return *this;
}

inline CFixVector& CFixVector::Copy (const CFloatVector& other) {
	v [0] = F2X (other [0]), v [1] = F2X (other [1]), v [2] = F2X (other [2]);
	return *this;
}

inline CFixVector& CFixVector::Copy (const CFixVector& other) {
	v [0] = other [0], v [1] = other [1], v [2] = other [2];
	return *this;
}

inline bool CFixVector::operator== (const CFixVector& rhs) const {
	return v [X] == rhs [X], v [Y] == rhs [Y], v [Z] == rhs [Z];
}

inline const CFixVector& CFixVector::Set (fix x, fix y, fix z) { v [0] = x; v [1] = y; v [2] = z; return *this; }

inline void CFixVector::Set (const fix *vec) {
	v [X] = vec [0]; v [Y] = vec [1]; v [Z] = vec [2];
}

inline bool CFixVector::IsZero () const { return ! (v [X] || v [Y] || v [Z]); }

inline void CFixVector::SetZero () { memset (v, 0, 3*sizeof (fix)); }

inline const int CFixVector::Sign () const { return (v [X] * v [Y] * v [Z] < 0) ? -1 : 1; }

inline fix CFixVector::SqrMag () const {
	return FixMul (v [X], v [X]) + FixMul (v [Y], v [Y]) + FixMul (v [Z], v [Z]);
}

inline fix CFixVector::Mag () const {
	return F2X (sqrt (X2F (v [X])*X2F (v [X]) + X2F (v [Y])*X2F (v [Y]) + X2F (v [Z])*X2F (v [Z])));
}

inline CFixVector& CFixVector::Neg () { v [0] = -v [0], v [1] = -v [1], v [2] = -v [2]; return *this; }

inline const CFixVector CFixVector::operator- () const {
	return Create (-v [X], -v [Y], -v [Z]);
}

inline const bool CFixVector::operator== (const CFixVector& vec) {
	return v [0] == vec [0], v [1] == vec [1], v [2] == vec [2];
}

inline const bool CFixVector::operator!= (const CFixVector& vec) {
	return v [0] != vec [0] || v [1] != vec [1] || v [2] != vec [2];
}

inline const CFixVector& CFixVector::operator+= (const CFixVector& vec) {
	v [0] += vec [0]; v [1] += vec [1]; v [2] += vec [2];
	return *this;
}

inline const CFixVector& CFixVector::operator-= (const CFixVector& vec) {
	v [0] -= vec [0]; v [1] -= vec [1]; v [2] -= vec [2];
	return *this;
}

inline const CFixVector& CFixVector::operator*= (const fix s) {
	v [0] = FixMul (v [0], s); v [1] = FixMul (v [1], s); v [2] = FixMul (v [2], s);
	return *this;
}

inline const CFixVector& CFixVector::operator*= (const CFixVector& s) {
	v [0] = FixMul (v [0], s [0]); v [1] = FixMul (v [1], s [1]); v [2] = FixMul (v [2], s [2]);
	return *this;
}

inline const CFixVector& CFixVector::operator/= (const fix s) {
	v [0] = FixDiv (v [0], s); v [1] = FixDiv (v [1], s); v [2] = FixDiv (v [2], s);
	return *this;
}

inline const CFixVector CFixVector::operator+ (const CFixVector& vec) const {
	return Create (v [0]+vec [0], v [1]+vec [1], v [2]+vec [2]);
}

inline const CFixVector CFixVector::operator- (const CFixVector& vec) const {
	return Create (v [0]-vec [0], v [1]-vec [1], v [2]-vec [2]);
}


// compute intersection of a line through a point a, with the line being orthogonal relative
// to the plane given by the Normal n and a point p lieing in the plane, and store it in i.
inline const CFixVector CFixVector::PlaneProjection (const CFixVector& n, const CFixVector& p) const {
	CFixVector i;
	double l = (double) -CFixVector::Dot (n, p) / (double) CFixVector::Dot (n, *this);
	i [X] = (fix) (l * (double) v [X]);
	i [Y] = (fix) (l * (double) v [Y]);
	i [Z] = (fix) (l * (double) v [Z]);
	return i;
}

//compute the distance from a point to a plane.  takes the normalized Normal
//of the plane (ebx), a point on the plane (edi), and the point to check (esi).
//returns distance in eax
//distance is signed, so Negative Dist is on the back of the plane
inline const fix CFixVector::DistToPlane (const CFixVector& n, const CFixVector& p) const {
	CFixVector t = *this - p;
	return CFixVector::Dot (t, n);
}

//extract heading and pitch from a vector, assuming bank==0
inline const CAngleVector CFixVector::ToAnglesVecNorm () const {
	CAngleVector a;
	a [BA] = 0;		//always zero bank
	a [PA] = FixASin (-v [Y]);
	a [HA] = (v [X] || v [Z]) ? FixAtan2 (v [Z], v [X]) : 0;
	return a;
}

//extract heading and pitch from a vector, assuming bank==0
inline const CAngleVector CFixVector::ToAnglesVec () const	{
	CFixVector t = *this;

//			if (CFixVector::Normalize (t))
	CFixVector::Normalize (t);
	return t.ToAnglesVecNorm ();
}


// -----------------------------------------------------------------------------
// CFixVector-related non-member ops

inline const fix operator* (const CFixVector& v0, const CFixVector& v1) {
	return CFixVector::Dot (v0, v1);
}

inline const CFixVector operator* (const CFixVector& v, const fix s) {
	return CFixVector::Create (FixMul (v [0], s), FixMul (v [1], s), FixMul (v [2], s));
}

inline const CFixVector operator* (const fix s, const CFixVector& v) {
	return CFixVector::Create (FixMul (v [0], s), FixMul (v [1], s), FixMul (v [2], s));
}

inline const CFixVector operator/ (const CFixVector& v, const fix d) {
	return CFixVector::Create (FixDiv (v [0], d), FixMul (v [1], d), FixMul (v [2], d));
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
 * \class CFixMatrix
 *
 * A 3x3 rotation matrix.  Sorry about the numbering starting with one. Ordering
 * is across then down, so <m1,m2,m3> is the first row.
 */
typedef union tFixMatrixData {
	CFixVector	mat [3];
	float			vec [12];
} tFixMatrixData;


class CFixMatrix {
	private:
		tFixMatrixData	m_data;

	public:
		static const CFixMatrix IDENTITY;
		static const CFixMatrix Create (const CFixVector& r, const CFixVector& u, const CFixVector& f);
		static const CFixMatrix Create (fix sinp, fix cosp, fix sinb, fix cosb, fix sinh, fix cosh);
		//computes a matrix from a Set of three angles.  returns ptr to matrix
		static const CFixMatrix Create (const CAngleVector& a);
		//computes a matrix from a forward vector and an angle
		static const CFixMatrix Create (CFixVector *v, fixang a);
		static const CFixMatrix CreateF (const CFixVector& fVec);
		static const CFixMatrix CreateFU (const CFixVector& fVec, const CFixVector& uVec);
		static const CFixMatrix CreateFR (const CFixVector& fVec, const CFixVector& rVec);

		static CFixMatrix& Invert (CFixMatrix& m);
		static CFixMatrix& Transpose (CFixMatrix& m);
		static CFixMatrix& Transpose (CFixMatrix& dest, const CFixMatrix& source);
		static CFloatMatrix& Transpose (CFloatMatrix& dest, const CFixMatrix& src);

		fix& operator[] (size_t idx);

		const CFixVector operator* (const CFixVector& v) const;
		const CFixMatrix operator* (const CFixMatrix& m) const;

		const fix Det () const;
		const CFixMatrix Inverse () const;
		const CFixMatrix Transpose () const;

		//make sure matrix is orthogonal
		void CheckAndFix ();

		//extract angles from a matrix
		const CAngleVector ExtractAnglesVec () const;

		const CFloatMatrix ToFloat () const;

		const CFixMatrix& Copy (const CFixMatrix& other);
		const CFixMatrix& Copy (const CFloatMatrix& other);

		static float* ToOpenGL (float* dest, const CFixMatrix& src);

		inline CFixVector& RVec() { return m_data.mat [0]; }
		inline CFixVector& UVec() { return m_data.mat [1]; }
		inline CFixVector& FVec() { return m_data.mat [2]; }
};


// -----------------------------------------------------------------------------
// CFixMatrix static inlines

inline const CFixMatrix CFixMatrix::Create (const CFixVector& r, const CFixVector& u, const CFixVector& f) {
	CFixMatrix m;
	m.RVec() = r;
	m.UVec() = u;
	m.FVec() = f;
	return m;
}

inline const CFixMatrix CFixMatrix::Create (fix sinp, fix cosp, fix sinb, fix cosb, fix sinh, fix cosh) {
	CFixMatrix m;
	fix sbsh, cbch, cbsh, sbch;

	sbsh = FixMul (sinb, sinh);
	cbch = FixMul (cosb, cosh);
	cbsh = FixMul (cosb, sinh);
	sbch = FixMul (sinb, cosh);
	m.RVec()[X] = cbch + FixMul (sinp, sbsh);		//m1
	m.UVec()[Z] = sbsh + FixMul (sinp, cbch);		//m8
	m.UVec()[X] = FixMul (sinp, cbsh) - sbch;		//m2
	m.RVec()[Z] = FixMul (sinp, sbch) - cbsh;		//m7
	m.FVec()[X] = FixMul (sinh, cosp);			//m3
	m.RVec()[Y] = FixMul (sinb, cosp);			//m4
	m.UVec()[Y] = FixMul (cosb, cosp);			//m5
	m.FVec()[Z] = FixMul (cosh, cosp);			//m9
	m.FVec()[Y] = -sinp;							//m6
	return m;
}

//computes a matrix from a Set of three angles.  returns ptr to matrix
inline const CFixMatrix CFixMatrix::Create (const CAngleVector& a) {
	fix sinp, cosp, sinb, cosb, sinh, cosh;
	FixSinCos (a [PA], &sinp, &cosp);
	FixSinCos (a [BA], &sinb, &cosb);
	FixSinCos (a [HA], &sinh, &cosh);
	return Create (sinp, cosp, sinb, cosb, sinh, cosh);
}

//computes a matrix from a forward vector and an angle
inline const CFixMatrix CFixMatrix::Create (CFixVector *v, fixang a) {
	fix sinb, cosb, sinp, cosp;

	FixSinCos (a, &sinb, &cosb);
	sinp = - (*v) [Y];
	cosp = FixSqrt (f1_0 - FixMul (sinp, sinp));
	return Create (sinp, cosp, sinb, cosb, FixDiv ( (*v) [X], cosp), FixDiv ( (*v) [Z], cosp));
}


inline CFixMatrix& CFixMatrix::Invert (CFixMatrix& m) {
	// TODO implement?
	return m;
}

#if 1
inline CFixMatrix& CFixMatrix::Transpose (CFixMatrix& m) {
	fix t;
	t = m [1];  m [1] = m [4];  m [4] = t;
	t = m [2];  m [2] = m [8];  m [8] = t;
	t = m [5];  m [5] = m [9];  m [9] = t;
	return m;
}
#else
inline CFixMatrix& CFixMatrix::Transpose (CFixMatrix& m) {
	fix t;
	t = m.UVec()[X];  m.UVec()[X] = m.RVec()[Y];  m.RVec()[Y] = t;
	t = m.FVec()[X];  m.FVec()[X] = m.RVec()[Z];  m.RVec()[Z] = t;
	t = m.FVec()[Y];  m.FVec()[Y] = m.UVec()[Z];  m.UVec()[Z] = t;
	return m;
}
#endif

//------------------------------------------------------------------------------

CFixMatrix& CFixMatrix::Transpose (CFixMatrix& dest, const CFixMatrix& src)
{
dest [0] = src [0];
dest [3] = src [1];
dest [6] = src [2];
dest [1] = src [3];
dest [4] = src [4];
dest [7] = src [5];
dest [2] = src [6];
dest [5] = src [7];
dest [8] = src [8];
return dest;
}

// -----------------------------------------------------------------------------
CFloatMatrix& CFixMatrix::Transpose (CFloatMatrix& dest, const CFixMatrix& src)
{
dest [0] = X2F (src [0]);
dest [4] = X2F (src [1]);
dest [8] = X2F (src [2]);
dest [1] = X2F (src [3]);
dest [5] = X2F (src [4]);
dest [9] = X2F (src [5]);
dest [2] = X2F (src [6]);
dest [6] = X2F (src [7]);
dest [10] = X2F (src [8]);
dest [11] = dest [12] = dest [13] = dest [14] = 0;
dest [15] = 1.0f;
return dest;
}

// -----------------------------------------------------------------------------
// CFixMatrix member ops

inline fix& CFixMatrix::operator[] (size_t idx) {
	return m_data.vec [idx];
}

inline const CFixVector CFixMatrix::operator* (const CFixVector& v) const {
	return CFixVector::Create (CFixVector::Dot (v, mat.RVec()),
										CFixVector::Dot (v, mat.UVec()),
										CFixVector::Dot (v, mat.FVec()));
}

inline const CFixMatrix CFixMatrix::operator* (const CFixMatrix& m) const {
	CFixVector v;
	CFixMatrix r;
	v [X] = mat.RVec()[X];
	v [Y] = mat.UVec()[X];
	v [Z] = mat.FVec()[X];
	r.RVec()[X] = CFixVector::Dot (v, m.RVec());
	r.UVec()[X] = CFixVector::Dot (v, m.UVec());
	r.FVec()[X] = CFixVector::Dot (v, m.FVec());
	v [X] = mat.RVec()[Y];
	v [Y] = mat.UVec()[Y];
	v [Z] = mat.FVec()[Y];
	r.RVec()[Y] = CFixVector::Dot (v, m.RVec());
	r.UVec()[Y] = CFixVector::Dot (v, m.UVec());
	r.FVec()[Y] = CFixVector::Dot (v, m.FVec());
	v [X] = mat.RVec()[Z];
	v [Y] = mat.UVec()[Z];
	v [Z] = mat.FVec()[Z];
	r.RVec()[Z] = CFixVector::Dot (v, m.RVec());
	r.UVec()[Z] = CFixVector::Dot (v, m.UVec());
	r.FVec()[Z] = CFixVector::Dot (v, m.FVec());
	return r;
}

inline const fix CFixMatrix::Det () const {
	fix	xDet;
	//PrintLog ("            CalcDetValue (R: %d, %d, %d; F: %d, %d, %d; U: %d, %d, %d)\n", m->rVec [X], m->rVec [Y], m->rVec [Z], m->fVec [X], m->fVec [Y], m->fVec [Z], m->uVec [X], m->uVec [Y], m->uVec [Z]);
	//PrintLog ("               xDet = FixMul (m->rVec [X], FixMul (m->uVec [Y], m->fVec [Z]))\n");
	xDet = FixMul (mat.RVec()[X], FixMul (mat.UVec()[Y], mat.FVec()[Z]));
	//PrintLog ("               xDet -= FixMul (m->rVec [X], FixMul (m->uVec [Z], m->fVec [Y]))\n");
	xDet -= FixMul (mat.RVec()[X], FixMul (mat.UVec()[Z], mat.FVec()[Y]));
	//PrintLog ("               xDet -= FixMul (m->rVec [Y], FixMul (m->uVec [X], m->fVec [Z]))\n");
	xDet -= FixMul (mat.RVec()[Y], FixMul (mat.UVec()[X], mat.FVec()[Z]));
	//PrintLog ("               xDet += FixMul (m->rVec [Y], FixMul (m->uVec [Z], m->fVec [X]))\n");
	xDet += FixMul (mat.RVec()[Y], FixMul (mat.UVec()[Z], mat.FVec()[X]));
	//PrintLog ("               xDet += FixMul (m->rVec [Z], FixMul (m->uVec [X], m->fVec [Y]))\n");
	xDet += FixMul (mat.RVec()[Z], FixMul (mat.UVec()[X], mat.FVec()[Y]));
	//PrintLog ("               xDet -= FixMul (m->rVec [Z], FixMul (m->uVec [Y], m->fVec [X]))\n");
	xDet -= FixMul (mat.RVec()[Z], FixMul (mat.UVec()[Y], mat.FVec()[X]));
	return xDet;
	//PrintLog ("             m = %d\n", xDet);
}

inline const CFixMatrix CFixMatrix::Inverse () const {
	fix	xDet = Det ();
	CFixMatrix m;

	m.RVec()[X] = FixDiv (FixMul (mat.UVec()[Y], mat.FVec()[Z]) - FixMul (mat.UVec()[Z], mat.FVec()[Y]), xDet);
	m.RVec()[Y] = FixDiv (FixMul (mat.RVec()[Z], mat.FVec()[Y]) - FixMul (mat.RVec()[Y], mat.FVec()[Z]), xDet);
	m.RVec()[Z] = FixDiv (FixMul (mat.RVec()[Y], mat.UVec()[Z]) - FixMul (mat.RVec()[Z], mat.UVec()[Y]), xDet);
	m.UVec()[X] = FixDiv (FixMul (mat.UVec()[Z], mat.FVec()[X]) - FixMul (mat.UVec()[X], mat.FVec()[Z]), xDet);
	m.UVec()[Y] = FixDiv (FixMul (mat.RVec()[X], mat.FVec()[Z]) - FixMul (mat.RVec()[Z], mat.FVec()[X]), xDet);
	m.UVec()[Z] = FixDiv (FixMul (mat.RVec()[Z], mat.UVec()[X]) - FixMul (mat.RVec()[X], mat.UVec()[Z]), xDet);
	m.FVec()[X] = FixDiv (FixMul (mat.UVec()[X], mat.FVec()[Y]) - FixMul (mat.UVec()[Y], mat.FVec()[X]), xDet);
	m.FVec()[Y] = FixDiv (FixMul (mat.RVec()[Y], mat.FVec()[X]) - FixMul (mat.RVec()[X], mat.FVec()[Y]), xDet);
	m.FVec()[Z] = FixDiv (FixMul (mat.RVec()[X], mat.UVec()[Y]) - FixMul (mat.RVec()[Y], mat.UVec()[X]), xDet);
	return m;
}

inline const CFixMatrix CFixMatrix::Transpose () const {
	CFixMatrix dest;
	Transpose (dest, *this);
	return dest;
}

//make sure this matrix is orthogonal
inline void CFixMatrix::CheckAndFix () {
	*this = CreateFU (mat.FVec(), mat.UVec());
}

//extract angles from a matrix
inline const CAngleVector CFixMatrix::ExtractAnglesVec () const {
	CAngleVector a;
	fix sinh, cosh, cosp;

	if (mat.FVec()[X]==0, mat.FVec()[Z]==0)		//zero head
		a [HA] = 0;
	else
		a [HA] = FixAtan2 (mat.FVec()[Z], mat.FVec()[X]);
	FixSinCos (a [HA], &sinh, &cosh);
	if (abs (sinh) > abs (cosh))				//sine is larger, so use it
		cosp = FixDiv (mat.FVec()[X], sinh);
	else											//cosine is larger, so use it
		cosp = FixDiv (mat.FVec()[Z], cosh);
	if (cosp==0, mat.FVec()[Y]==0)
		a [PA] = 0;
	else
		a [PA] = FixAtan2 (cosp, -mat.FVec()[Y]);
	if (cosp == 0)	//the cosine of pitch is zero.  we're pitched straight up. say no bank
		a [BA] = 0;
	else {
		fix sinb, cosb;

		sinb = FixDiv (mat.RVec()[Y], cosp);
		cosb = FixDiv (mat.UVec()[Y], cosp);
		if (sinb==0, cosb==0)
			a [BA] = 0;
		else
			a [BA] = FixAtan2 (cosb, sinb);
		}
	return a;
}



/**
 * \class CFloatMatrix
 *
 * A 4x4 floating point transformation matrix
 */

typedef union tFloatMatrixData {
	CFloatVector	mat [4];
	float				vec [16];
} tFloatMatrixData;

class CFloatMatrix {
	private:
		tFloatMatrixData	m_data;

	public:
		static const CFloatMatrix IDENTITY;
		static const CFloatMatrix Create (const CFloatVector& r, const CFloatVector& u, const CFloatVector& f, const CFloatVector& w);
		static const CFloatMatrix Create (float sinp, float cosp, float sinb, float cosb, float sinh, float cosh);

		static CFloatMatrix& Invert (CFloatMatrix& m);
		static CFloatMatrix& Transpose (CFloatMatrix& m);
		static CFloatMatrix& Transpose (CFloatMatrix& dest, CFloatMatrix& source);

		float& operator[] (size_t idx);

		const CFloatVector operator* (const CFloatVector& v) const;
		const CFloatVector3 operator* (const CFloatVector3& v);
		const CFloatMatrix operator* (const CFloatMatrix& m) const;

		const float Det () const;
		const CFloatMatrix Inverse () const;
		const CFloatMatrix Transpose () const;

		const CFloatMatrix& Copy (const CFixMatrix& other);
		const CFloatMatrix& Copy (const CFloatMatrix& other);

		static float* Transpose (float* dest, const CFloatMatrix& src);

		inline CFloatVector& RVec() { return m_data.mat [RVEC]; }
		inline CFloatVector& UVec() { return m_data.mat [UVEC]; }
		inline CFloatVector& FVec() { return m_data.mat [FVEC]; }
		inline CFloatVector& HVec() { return m_data.mat [HVEC]; }

};


// -----------------------------------------------------------------------------
// CFloatMatrix static inlines

inline const CFloatMatrix CFloatMatrix::Create (const CFloatVector& r, const CFloatVector& u, const CFloatVector& f, const CFloatVector& w) {
	CFloatMatrix m;
	m.RVec() = r;
	m.UVec() = u;
	m.FVec() = f;
	m.HVec() = w;
	return m;
}

inline const CFloatMatrix CFloatMatrix::Create (float sinp, float cosp, float sinb, float cosb, float sinh, float cosh) {
	CFloatMatrix m;
	float sbsh, cbch, cbsh, sbch;

	sbsh = sinb * sinh;
	cbch = cosb * cosh;
	cbsh = cosb * sinh;
	sbch = sinb * cosh;
	m.RVec()[X] = cbch + sinp * sbsh;	//m1
	m.UVec()[Z] = sbsh + sinp * cbch;	//m8
	m.UVec()[X] = sinp * cbsh - sbch;	//m2
	m.RVec()[Z] = sinp * sbch - cbsh;	//m7
	m.FVec()[X] = sinh * cosp;		//m3
	m.RVec()[Y] = sinb * cosp;		//m4
	m.UVec()[Y] = cosb * cosp;		//m5
	m.FVec()[Z] = cosh * cosp;		//m9
	m.FVec()[Y] = -sinp;				//m6
	return m;
}

inline CFloatMatrix& CFloatMatrix::Invert (CFloatMatrix& m) {
	//TODO: implement?
	return m;
}

#if 1
inline CFloatMatrix& CFloatMatrix::Transpose (CFloatMatrix& m) {
	float t;
	t = m [1];  m [1] = m [4];  m [4] = t;
	t = m [2];  m [2] = m [8];  m [8] = t;
	t = m [5];  m [5] = m [9];  m [9] = t;
	return m;

#else
inline CFloatMatrix& CFloatMatrix::Transpose (CFloatMatrix& m) {
	float t;
	t = m.UVec()[X];  m.UVec()[X] = m.RVec()[Y];  m.RVec()[Y] = t;
	t = m.FVec()[X];  m.FVec()[X] = m.RVec()[Z];  m.RVec()[Z] = t;
	t = m.FVec()[Y];  m.FVec()[Y] = m.UVec()[Z];  m.UVec()[Z] = t;
	return m;
}
#endif

// -----------------------------------------------------------------------------
// CFloatMatrix member ops

inline float& CFloatMatrix::operator[] (size_t idx) {
	return data.vec [idx];
}

inline const CFloatVector CFloatMatrix::operator* (const CFloatVector& v) const {
	return CFloatVector::Create (CFloatVector::Dot (v, mat.RVec()),
										  CFloatVector::Dot (v, mat.UVec()),
										  CFloatVector::Dot (v, mat.FVec()));
}

inline const CFloatVector3 CFloatMatrix::operator* (const CFloatVector3& v) {
	return CFloatVector3::Create (CFloatVector3::Dot (v, *mat.RVec().V3 ()),
											CFloatVector3::Dot (v, *mat.UVec().V3 ()),
											CFloatVector3::Dot (v, *mat.FVec().V3 ()));
}

inline const float CFloatMatrix::Det () const {
	return mat.RVec()[X] * mat.UVec()[Y] * mat.FVec()[Z] -
			 mat.RVec()[X] * mat.UVec()[Z] * mat.FVec()[Y] -
			 mat.RVec()[Y] * mat.UVec()[X] * mat.FVec()[Z] +
			 mat.RVec()[Y] * mat.UVec()[Z] * mat.FVec()[X] +
			 mat.RVec()[Z] * mat.UVec()[X] * mat.FVec()[Y] -
			 mat.RVec()[Z] * mat.UVec()[Y] * mat.FVec()[X];
}

inline const CFloatMatrix CFloatMatrix::Inverse () const {
	float fDet = Det ();
	CFloatMatrix	m = *this;

	m.RVec()[X] = (mat.UVec()[Y] * mat.FVec()[Z] - mat.UVec()[Z] * mat.FVec()[Y]) / fDet;
	m.RVec()[Y] = (mat.RVec()[Z] * mat.FVec()[Y] - mat.RVec()[Y] * mat.FVec()[Z]) / fDet;
	m.RVec()[Z] = (mat.RVec()[Y] * mat.UVec()[Z] - mat.RVec()[Z] * mat.UVec()[Y]) / fDet;
	m.UVec()[X] = (mat.UVec()[Z] * mat.FVec()[X] - mat.UVec()[X] * mat.FVec()[Z]) / fDet;
	m.UVec()[Y] = (mat.RVec()[X] * mat.FVec()[Z] - mat.RVec()[Z] * mat.FVec()[X]) / fDet;
	m.UVec()[Z] = (mat.RVec()[Z] * mat.UVec()[X] - mat.RVec()[X] * mat.UVec()[Z]) / fDet;
	m.FVec()[X] = (mat.UVec()[X] * mat.FVec()[Y] - mat.UVec()[Y] * mat.FVec()[X]) / fDet;
	m.FVec()[Y] = (mat.RVec()[Y] * mat.FVec()[X] - mat.RVec()[X] * mat.FVec()[Y]) / fDet;
	m.FVec()[Z] = (mat.RVec()[X] * mat.UVec()[Y] - mat.RVec()[Y] * mat.UVec()[X]) / fDet;
	return m;
}

inline const CFloatMatrix CFloatMatrix::Transpose () const {
	CFloatMatrix dest;
	Transpose (dest, *this);
	return dest;
}


inline const CFloatMatrix CFloatMatrix::operator* (const CFloatMatrix& m) const {
	CFloatVector v;
	CFloatMatrix r;
	v [X] = mat.RVec()[X];
	v [Y] = mat.UVec()[X];
	v [Z] = mat.FVec()[X];
	r.RVec()[X] = CFloatVector::Dot (v, m.RVec());
	r.UVec()[X] = CFloatVector::Dot (v, m.UVec());
	r.FVec()[X] = CFloatVector::Dot (v, m.FVec());
	v [X] = mat.RVec()[Y];
	v [Y] = mat.UVec()[Y];
	v [Z] = mat.FVec()[Y];
	r.RVec()[Y] = CFloatVector::Dot (v, m.RVec());
	r.UVec()[Y] = CFloatVector::Dot (v, m.UVec());
	r.FVec()[Y] = CFloatVector::Dot (v, m.FVec());
	v [X] = mat.RVec()[Z];
	v [Y] = mat.UVec()[Z];
	v [Z] = mat.FVec()[Z];
	r.RVec()[Z] = CFloatVector::Dot (v, m.RVec());
	r.UVec()[Z] = CFloatVector::Dot (v, m.UVec());
	r.FVec()[Z] = CFloatVector::Dot (v, m.FVec());
	return r;
}

// -----------------------------------------------------------------------------
// misc conversion member ops

inline const CFloatVector CFixVector::ToFloat () const {
	CFloatVector d;
	d [X] = X2F (v [X]); d [Y] = X2F (v [Y]); d [Z] = X2F (v [Z]); d [W] = 1; return d;
}

inline const CFloatVector3 CFixVector::ToFloat3 () const {
	CFloatVector3 d;
	d [X] = X2F (v [X]); d [Y] = X2F (v [Y]); d [Z] = X2F (v [Z]); return d;
}

inline const CFixVector CFloatVector::ToFix () const {
	CFixVector d;
	d [X] = F2X (v [X]); d [Y] = F2X (v [Y]); d [Z] = F2X (v [Z]); return d;
}

inline const CFixVector CFloatVector3::ToFix () const {
	CFixVector d;
	d [X] = F2X (v [X]); d [Y] = F2X (v [Y]); d [Z] = F2X (v [Z]); return d;
}

inline const CFloatMatrix CFixMatrix::ToFloat () const {
	CFloatMatrix m;
	m.RVec() = mat.RVec().ToFloat ();
	m.UVec() = mat.UVec().ToFloat ();
	m.FVec() = mat.FVec().ToFloat ();
	m.HVec() = CFloatVector::ZERO;
	return m;
}

inline const CFloatMatrix& CFloatMatrix::Copy (const CFloatMatrix& other) { 
	mat.RVec() = other.RVec(); 
	mat.UVec() = other.UVec(); 
	mat.FVec() = other.FVec(); 
	mat.HVec() = other.HVec(); 
	return *this;
	}

inline const CFloatMatrix& CFloatMatrix::Copy (const CFixMatrix& other) { 
	mat.RVec().Copy (other.RVec()); 
	mat.UVec().Copy (other.UVec()); 
	mat.FVec().Copy (other.FVec()); 
	mat.HVec().Set (1, 1, 1);
	return *this;
	}

inline const CFixMatrix& CFixMatrix::Copy (const CFixMatrix& other) { 
	mat.RVec() = other.RVec(); 
	mat.UVec() = other.UVec(); 
	mat.FVec() = other.FVec(); 
	return *this;
	}

inline const CFixMatrix& CFixMatrix::Copy (const CFloatMatrix& other) { 
	mat.RVec().Copy (other.RVec()); 
	mat.UVec().Copy (other.UVec()); 
	mat.FVec().Copy (other.FVec()); 
	return *this;
	}

//------------------------------------------------------------------------------

CFloatMatrix& CFloatMatrix::Transpose (CFloatMatrix& dest, const CFloatMatrix& src)
{
dest [0] = src [0];
dest [4] = src [1];
dest [8] = src [2];
dest [1] = src [4];
dest [5] = src [5];
dest [9] = src [6];
dest [2] = src [8];
dest [6] = src [9];
dest [10] = src [10];
dest [11] = dest [12] = dest [13] = dest [14] = 0;
dest [15] = 1.0f;
return dest;
}

} //VecMat

// -----------------------------------------------------------------------------
// misc remaining C-style funcs

const int VmPointLineIntersection (CFixVector& hitP, const CFixVector& p1, const CFixVector& p2, const CFixVector& p3, int bClampToFarthest);
//const int VmPointLineIntersection (CFixVector& hitP, const CFixVector& p1, const CFixVector& p2, const CFixVector& p3, const CFixVector& vPos, int bClampToFarthest);
const fix VmLinePointDist (const CFixVector& a, const CFixVector& b, const CFixVector& p);
const int VmPointLineIntersection (CFloatVector& hitP, const CFloatVector& p1, const CFloatVector& p2, const CFloatVector& p3, const CFloatVector& vPos, int bClamp);
const int VmPointLineIntersection (CFloatVector& hitP, const CFloatVector& p1, const CFloatVector& p2, const CFloatVector& p3, int bClamp);
const int VmPointLineIntersection (CFloatVector3& hitP, const CFloatVector3& p1, const CFloatVector3& p2, const CFloatVector3& p3, CFloatVector3 *vPos, int bClamp);
const float VmLinePointDist (const CFloatVector& a, const CFloatVector& b, const CFloatVector& p, int bClamp);
const float VmLinePointDist (const CFloatVector3& a, const CFloatVector3& b, const CFloatVector3& p, int bClamp);
const float VmLineLineIntersection (const CFloatVector3& v1, const CFloatVector3& v2, const CFloatVector3& V3, const CFloatVector3& v4, CFloatVector3& va, CFloatVector3& vb);
const float VmLineLineIntersection (const CFloatVector& v1, const CFloatVector& v2, const CFloatVector& V3, const CFloatVector& v4, CFloatVector& va, CFloatVector& vb);

float TriangleSize (const CFixVector& p0, const CFixVector& p1, const CFixVector& p2);

// ------------------------------------------------------------------------

#endif //_VECMAT_H
