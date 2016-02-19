
#ifndef VEC3D_H
#define VEC3D_H

#include <cmath>

//--�񎟌��x�N�g��--
struct vec3d
{
	double x, y, z;

	vec3d() : x(0.0), y(0.0), z(0.0) {}
	vec3d(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

	//���������߂�
	double length(){ return sqrt(x*x + y*y + z*z); }
	//��撷�������߂�
	double lengthsq(){ return x*x + y*y + z*z; }
	//���K���x�N�g�������߂�
	vec3d norm(){ double L = length(); return vec3d(x / L, y / L, z / L); }
	//���K��
	void normalize(){ *this /= this->length(); }

	//���Z�q�̃I�[�o�[���[�h�i�l�����Z���\�ɂ���j
	vec3d operator+(const vec3d& v)
	{
		return vec3d(x + v.x, y + v.y, z + v.z);
	}

	vec3d operator-(const vec3d& v)
	{
		return vec3d(x - v.x, y - v.y, z - v.z);
	}

	vec3d operator*(const double& a)
	{
		return vec3d(a*x, a*y, a*z);
	}

	vec3d operator/(const double& a)
	{
		return vec3d(x / a, y / a, z / a);
	}

	vec3d& operator+=(const vec3d& v)
	{
		*this = *this + v;
		return *this;
	}

	vec3d& operator-=(const vec3d& v)
	{
		*this = *this - v;
		return *this;
	}

	vec3d& operator*=(const double& a)
	{
		*this = *this * a;
		return *this;
	}

	vec3d& operator/=(const double& a)
	{
		*this = *this / a;
		return *this;
	}

	//vec3d���m��*�͓���
	double operator*(const vec3d& v)
	{
		return x * v.x + y * v.y, z * v.z;
	}
};

vec3d cross_vec3d(const vec3d& v1, const vec3d& v2)
{
	return vec3d(
		-v1.y*v2.z + v1.z*v1.y,
		-v1.z*v2.x + v1.x*v1.z,
		-v1.x*v2.y + v1.y*v1.x
	);
}

#endif
