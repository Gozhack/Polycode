/*
 *  PolyQuaternion.h
 *  TAU
 *
 *  Created by Ivan Safrin on 3/26/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
 
#pragma once
#include "PolyString.h"
#include "PolyGlobals.h"
#include "PolyBasics.h"
#include "PolyMatrix4.h"
#include <math.h>

// @package Math

namespace Polycode {

	class _PolyExport Quaternion {
		public:
			Quaternion(Number w, Number x, Number y, Number z) {
				set(w,x,y,z);
			}
			Quaternion();
			~Quaternion();
		
		inline void setFromMatrix(const Matrix4 &kRot) {
			Number fTrace = kRot.m[0][0]+kRot.m[1][1]+kRot.m[2][2];
			Number fRoot;
			
			if ( fTrace > 0.0 )
			{
				// |w| > 1/2, may as well choose w > 1/2
				fRoot = sqrtf(fTrace + 1.0);  // 2w
				w = 0.5*fRoot;
				fRoot = 0.5/fRoot;  // 1/(4w)
				x = (kRot.m[2][1]-kRot.m[1][2])*fRoot;
				y = (kRot.m[0][2]-kRot.m[2][0])*fRoot;
				z = (kRot.m[1][0]-kRot.m[0][1])*fRoot;
			}
			else
			{
				// |w| <= 1/2
				static size_t s_iNext[3] = { 1, 2, 0 };
				size_t i = 0;
				if ( kRot.m[1][1] > kRot.m[0][0] )
					i = 1;
				if ( kRot.m[2][2] > kRot.m[i][i] )
					i = 2;
				size_t j = s_iNext[i];
				size_t k = s_iNext[j];
				
				fRoot = sqrtf(kRot.m[i][i]-kRot.m[j][j]-kRot.m[k][k] + 1.0);
				Number* apkQuat[3] = { &x, &y, &z };
				*apkQuat[i] = 0.5*fRoot;
				fRoot = 0.5/fRoot;
				w = (kRot.m[k][j]-kRot.m[j][k])*fRoot;
				*apkQuat[j] = (kRot.m[j][i]+kRot.m[i][j])*fRoot;
				*apkQuat[k] = (kRot.m[k][i]+kRot.m[i][k])*fRoot;
			}			
		}
			
			static Quaternion Slerp(Number fT, const Quaternion& rkP, const Quaternion& rkQ, bool shortestPath=false);
			Number Dot(const Quaternion& rkQ) const;
	Quaternion Log () const;
    Quaternion Exp () const;	
    Number Norm () const;
    Number normalise(void);	
    Quaternion operator+ (const Quaternion& rkQ) const;
    Quaternion operator* (const Quaternion& rkQ) const;
    Quaternion operator* (Number fScalar) const;
		
		inline void lookAt(const Vector3 &D, const Vector3 &upVector) {
			/*
			Vector3 D;
			Vector3 back = D * -1;
			back.Normalize();
			
			Vector3 right = back.crossProduct(upVector) ;
			right.Normalize();
			right = right * -1;
			
			Vector3 up = back.crossProduct(right);
			
			set( y.z - z.y , z.x - x.z, x.y - y.x, tr + 1.0f ); 
			 */
		}
		
		void createFromMatrix(Matrix4 matrix) {
			Number  tr, s, q[4];
			int    i, j, k;
			
			static const int nxt[3] = {1, 2, 0};			
			
			tr = matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2];
			
			
			// check the diagonal
			if (tr > 0.0f)
			{
                s = sqrtf(tr + 1.0f);
                w = s / 2.0f;
                s = 0.5f / s;
                x = (matrix.m[1][2] - matrix.m[2][1]) * s;
                y = (matrix.m[2][0] - matrix.m[0][2]) * s;
                z = (matrix.m[0][1] - matrix.m[1][0]) * s;
			}
			else
			{
                // diagonal is negative
                i = 0;
                if (matrix.m[1][1] > matrix.m[0][0]) i = 1;
                if (matrix.m[2][2] > matrix.m[i][i]) i = 2;
                j = nxt[i];
                k = nxt[j];
				
                s = sqrtf((matrix.m[i][i] - (matrix.m[j][j] + matrix.m[k][k])) + 1.0f);
				
                q[i] = s * 0.5f;
				
                if (s != 0.0f) s = 0.5f / s;
				
                q[3] = (matrix.m[j][k] - matrix.m[k][j]) * s;
                q[j] = (matrix.m[i][j] + matrix.m[j][i]) * s;
                q[k] = (matrix.m[i][k] + matrix.m[k][i]) * s;
				
                x = q[0];
                y = q[1];
                z = q[2];
                w = q[3];
			}
			
		}


	inline bool operator== (const Quaternion& rhs) const
	{
			return (rhs.x == x) && (rhs.y == y) &&
				(rhs.z == z) && (rhs.w == w);
	}

	inline bool operator!= (const Quaternion& rhs) const
	{
			return (rhs.x != x) && (rhs.y != y) &&
				(rhs.z != z) && (rhs.w != w);
	}


    static Quaternion Squad(Number fT, const Quaternion& rkP, const Quaternion& rkA, const Quaternion& rkB, const Quaternion& rkQ, bool shortestPath);
	Quaternion Inverse () const;
	
    Quaternion operator- () const
    {
        return Quaternion(-w,-x,-y,-z);
    }
			
			void set(Number w, Number x, Number y, Number z) {
				this->w = w;
				this->x = x;
				this->y = y;
				this->z = z;												
			}
			
			Quaternion inverse() {
				Number fNorm = w*w+x*x+y*y+z*z;
				Number fInvNorm = 1.0/fNorm;
				return Quaternion(w*fInvNorm,-x*fInvNorm,-y*fInvNorm,-z*fInvNorm);

			}
			
	Number InvSqrt(Number x){
   Number xhalf = 0.5f * x;
   int i = *(int*)&x; // store Numbering-point bits in integer
   i = 0x5f3759d5 - (i >> 1); // initial guess for Newton's method
   x = *(Number*)&i; // convert new bits into Number
   x = x*(1.5f - xhalf*x*x); // One round of Newton's method
   return x;
}
		
	inline void fromAxes(Number az, Number ay, Number ax) {
		ax *= TORADIANS;
		ay *= TORADIANS;
		az *= TORADIANS;		
		
		Number c1 = cos(ay / 2.0f);
		Number c2 = cos(ax / 2.0f);
		Number c3 = cos(az / 2.0f);
		
		Number s1 = sin(ay / 2.0f);
		Number s2 = sin(ax / 2.0f);
		Number s3 = sin(az / 2.0f);
		
		w = (c1*c2*c3) - (s1*s2*s3);
		x = (s1*s2*c3) + (c1*c2*s3);
		y = (s1*c2*c3) + (c1*s2*s3);
		z = (c1*s2*c3) - (s1*c2*s3);		
	}
			
			
    void FromAngleAxis (const Number& rfAngle,
        const Vector3& rkAxis)
    {
        Number fHalfAngle ( 0.5*rfAngle );
        Number fSin = sin(fHalfAngle);
        w = cos(fHalfAngle);
        x = fSin*rkAxis.x;
        y = fSin*rkAxis.y;
        z = fSin*rkAxis.z;
    }
    //-----------------------------------------------------------------------
    void ToAngleAxis (Number& rfAngle, Vector3& rkAxis) 
    {
        // The quaternion representing the rotation is
        //   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

        Number fSqrLength = x*x+y*y+z*z;
        if ( fSqrLength > 0.0 )
        {
            rfAngle = 2.0*acos(w);
            Number fInvLength = InvSqrt(fSqrLength);
            rkAxis.x = x*fInvLength;
            rkAxis.y = y*fInvLength;
            rkAxis.z = z*fInvLength;
        }
        else
        {
            // angle is 0 (mod 2*pi), so any axis will do
            rfAngle = Number(0.0);
            rkAxis.x = 1.0;
            rkAxis.y = 0.0;
            rkAxis.z = 0.0;
        }
    }			
			
			void createFromAxisAngle(Number x, Number y, Number z, Number degrees);
			Matrix4 createMatrix();
			
			
			
			Quaternion operator *(Quaternion q);
			
			Number x;
			Number y;
			Number z;
			Number w;
			
		protected:
	};
}