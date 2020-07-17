#pragma once

#include <math.h>
#include <windows.h>


struct VECTOR2
{
	float		x;
	float		y;

	inline	BOOL		operator==(const VECTOR2& v);
	inline	VECTOR2		operator +(const VECTOR2& v);
	inline	VECTOR2		operator -(const VECTOR2& v);
	inline	VECTOR2		operator *(float f);
	inline	VECTOR2		operator /(float f);
	inline	VECTOR2		VECTOR2::operator *(const VECTOR2 &v);
	inline	void		Set(float in_x, float in_y);
};

inline	BOOL		VECTOR2::operator==(const VECTOR2& v)
{
	BOOL	bResult;
	if (this->x == v.x && this->y == v.y)
		bResult = TRUE;
	else
		bResult = FALSE;

	return	bResult;
}

inline void VECTOR2::Set(float in_x, float in_y)
{
	x = in_x;
	y = in_y;
}
inline VECTOR2 VECTOR2::operator +(const VECTOR2& v)
{
	VECTOR2	r;
	r.x = this->x + v.x;
	r.y = this->y + v.y;
	return r;
}

inline VECTOR2 VECTOR2::operator -(const VECTOR2& v)
{
	VECTOR2 r;
	r.x = this->x - v.x;
	r.y = this->y - v.y;
	return r;
}

inline VECTOR2 VECTOR2::operator *(float f)
{
	VECTOR2 r;
	r.x = this->x * f;
	r.y = this->y * f;
	return r;
}

inline VECTOR2 VECTOR2::operator /(float f)
{
	VECTOR2	r;
	r.x = this->x / f;
	r.y = this->y / f;
	return r;
}
// dot.
inline VECTOR2 VECTOR2::operator *(const VECTOR2& v)
{
	VECTOR2	r;
	r.x = this->x * v.x;
	r.y = this->y * v.y;
	return r;
}

struct MATRIX4;


struct VECTOR3
{
	float	x;
	float	y;
	float	z;

	inline	VECTOR3		operator +(const VECTOR3& v);
	inline	VECTOR3		operator -(const VECTOR3& v);
	inline	VECTOR3		operator *(float f);
	inline	VECTOR3		operator /(float f);
	inline	VECTOR3		operator *(const VECTOR3& v);
	inline	BOOL		operator==(const VECTOR3& v);
	inline	BOOL		operator!=(const VECTOR3& v);
	inline	BOOL		NearZero(float fE);

	inline void			Set(float in_x, float in_y, float in_z);

};

inline void VECTOR3::Set(float in_x, float in_y, float in_z)
{
	x = in_x;
	y = in_y;
	z = in_z;
}
inline VECTOR3		VECTOR3::operator +(const VECTOR3& v)
{
	VECTOR3	r;
	r.x = this->x + v.x;
	r.y = this->y + v.y;
	r.z = this->z + v.z;
	return r;
}

inline VECTOR3		VECTOR3::operator -(const VECTOR3& v)
{
	VECTOR3	r;
	r.x = this->x - v.x;
	r.y = this->y - v.y;
	r.z = this->z - v.z;
	return r;
}

inline VECTOR3 VECTOR3::operator *(float f)
{
	VECTOR3	r;
	r.x = this->x * f;
	r.y = this->y * f;
	r.z = this->z * f;
	return r;
}

inline VECTOR3 VECTOR3::operator /(float f)
{
	VECTOR3 r;
	r.x = this->x / f;
	r.y = this->y / f;
	r.z = this->z / f;
	return r;
}

inline VECTOR3 VECTOR3::operator *(const VECTOR3& v)
{
	VECTOR3	r;
	r.x = this->x * v.x;
	r.y = this->y * v.y;
	r.z = this->z * v.z;
	return r;
}

inline	BOOL VECTOR3::operator==(const VECTOR3& v)
{
	BOOL	bResult;
	if (this->x == v.x && this->y == v.y && this->z == v.z)
		bResult = TRUE;
	else
		bResult = FALSE;

	return	bResult;
}
inline	BOOL VECTOR3::operator!=(const VECTOR3& v)
{
	BOOL	bResult;
	if (this->x != v.x || this->y != v.y || this->z != v.z)
		bResult = TRUE;
	else
		bResult = FALSE;

	return	bResult;
}


inline BOOL VECTOR3::NearZero(float fE)
{
	if (this->x > -fE && this->x < fE && this->y > -fE && this->y < fE && this->z > -fE && this->z < fE)
		return TRUE;
	else
		return FALSE;
}



struct VECTOR4
{
	float x;
	float y;
	float z;
	float w;

	inline		void Set(float in_x, float in_y, float in_z, float in_w);

	inline	VECTOR4		operator +(const VECTOR4& v);
	inline	VECTOR4		operator -(const VECTOR4& v);
	inline	VECTOR4		operator *(float f);
	inline	VECTOR4		operator /(float f);
	inline	float		operator *(const VECTOR4& v);			// dot.
	inline	BOOL		operator==(const VECTOR4& v);


};

inline VECTOR4 VECTOR4::operator +(const VECTOR4& v)
{
	VECTOR4	r;
	r.x = this->x + v.x;
	r.y = this->y + v.y;
	r.z = this->z + v.z;
	r.w = this->w + v.w;
	return r;
}

inline VECTOR4 VECTOR4::operator -(const VECTOR4 &v)
{
	VECTOR4	r;
	r.x = this->x - v.x;
	r.y = this->y - v.y;
	r.z = this->z - v.z;
	r.w = this->w - v.w;
	return r;
}

inline VECTOR4		VECTOR4::operator *(float	f)
{
	VECTOR4	r;
	r.x = this->x * f;
	r.y = this->y * f;
	r.z = this->z * f;
	r.w = this->w * f;
	return r;
}

inline VECTOR4		VECTOR4::operator /(float f)
{
	VECTOR4	r;
	r.x = this->x / f;
	r.y = this->y / f;
	r.z = this->z / f;
	r.w = this->w / f;
	return r;
}
// dot.
inline float VECTOR4::operator *(const VECTOR4& v)
{
	float r;
	r = this->x * v.x;
	r += this->y * v.y;
	r += this->z * v.z;
	r += this->w * v.w;
	return		r;
}

inline	BOOL VECTOR4::operator==(const VECTOR4& v)
{
	BOOL bResult;
	if (this->x == v.x && this->y == v.y && this->z == v.z && this->w == v.w)
		bResult = TRUE;
	else
		bResult = FALSE;

	return	bResult;
}


inline void VECTOR4::Set(float in_x, float in_y, float in_z, float in_w)
{
	x = in_x;
	y = in_y;
	z = in_z;
	w = in_w;

};
struct MATRIX3
{
	float	_11;
	float	_12;
	float	_13;
	float	_21;
	float	_22;
	float	_23;
	float	_31;
	float	_32;
	float	_33;
};
struct MATRIX4
{
	float	_11;
	float	_12;
	float	_13;
	float	_14;

	float	_21;
	float	_22;
	float	_23;
	float	_24;

	float	_31;
	float	_32;
	float	_33;
	float	_34;

	float	_41;
	float	_42;
	float	_43;
	float	_44;

};

struct MATRIX4x3
{
	float	_11;
	float	_12;
	float	_13;
	float	_14;

	float	_21;
	float	_22;
	float	_23;
	float	_24;

	float	_31;
	float	_32;
	float	_33;
	float	_34;
};

struct INT_VECTOR2
{
	int		x;
	int		y;

	inline	BOOL			operator==(const INT_VECTOR2& v);
	inline	INT_VECTOR2		operator +(const INT_VECTOR2& v);
	inline	INT_VECTOR2		operator -(const INT_VECTOR2& v);
	inline	INT_VECTOR2		operator *(const int a);
	inline	INT_VECTOR2		operator /(const int a);
	inline	int				operator *(const INT_VECTOR2 &v);			// dot.
	inline	void			Set(int in_x, int in_y);

};



inline	BOOL INT_VECTOR2::operator==(const INT_VECTOR2& v)
{
	BOOL	bResult;
	if (this->x == v.x && this->y == v.y)
		bResult = TRUE;
	else
		bResult = FALSE;

	return	bResult;
}

inline void INT_VECTOR2::Set(int in_x, int in_y)
{
	x = in_x;
	y = in_y;
}
inline INT_VECTOR2 INT_VECTOR2::operator +(const INT_VECTOR2 &v)
{
	INT_VECTOR2	r;
	r.x = this->x + v.x;
	r.y = this->y + v.y;
	return r;
}

inline INT_VECTOR2 INT_VECTOR2::operator -(const INT_VECTOR2 &v)
{
	INT_VECTOR2 r;
	r.x = this->x - v.x;
	r.y = this->y - v.y;
	return r;
}

inline INT_VECTOR2 INT_VECTOR2::operator *(const int a)
{
	INT_VECTOR2 r;
	r.x = this->x * a;
	r.y = this->y * a;
	return r;
}

inline INT_VECTOR2 INT_VECTOR2::operator /(const int a)
{
	INT_VECTOR2 r;
	r.x = this->x / a;
	r.y = this->y / a;
	return r;
}
// dot.
inline int INT_VECTOR2::operator *(const INT_VECTOR2 &v)
{
	int r = this->x * v.x;
	r += this->y * v.y;
	return r;
}



inline VECTOR3 CrossProduct(const VECTOR3& a, const VECTOR3& b)
{
	VECTOR3 r;
	r.x = a.y * b.z - a.z * b.y;
	r.y = a.z * b.x - a.x * b.z;
	r.z = a.x * b.y - a.y * b.x;
	return r;
}

inline float VECTOR3Length(const VECTOR3& v)
{
	float	r = (float)sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return r;
}

inline VECTOR3 Normalize(const VECTOR3& v)
{
	VECTOR3	r = { 0.0f,0.0f,0.0f };
	float len = VECTOR3Length(v);

	if (len != 0.0f)
	{
		r.x = v.x / len;
		r.y = v.y / len;
		r.z = v.z / len;
	}
	return r;
}

inline VECTOR3 CalcNormal(VECTOR3* pTriPointList)
{
	VECTOR3	u = pTriPointList[1] - pTriPointList[0];
	VECTOR3	v = pTriPointList[2] - pTriPointList[0];

	VECTOR3	r = CrossProduct(u, v);
	VECTOR3 n = Normalize(r);
}
