#include <iostream>
#include <iomanip>
#include <sstream>
#include <math/math.h>
#include <math/vectorxd.h>
#include <math/matrix.h>
#include <math/quaternion.h>
#include <cassert>

using namespace RenderWorker;
using namespace RenderWorker::MathWorker;
void checkvecotr()
{
	// test contruct
	float3 v(1, 2, 3);
	float3 u = { 0,2,3 };
	float3 w = {1.f, 1.f, 1.f};
	assert(float3(1,2,3) == v);
	assert(float3(0,2,3) == u);
	assert(float3(1.f,1.f,1.f) == w);

	std::cout << sizeof(float3) << std::endl;
	// add
	std::cout << (-v) << std::endl;
	std::cout << (v + u) << std::endl;
	std::cout << (v - u) << std::endl;
	v += u;
	std::cout << v << std::endl;
	v -= u;
	std::cout << v << std::endl;
	std::cout << (v * 5) << std::endl;
	std::cout << (5 * v) << std::endl;
	// mul
	std::cout << (v * 3.f) << std::endl;
	std::cout << (2.5f * v) << std::endl;
	std::cout << (2.5 * float2{ 1.f,2.f }) << std::endl;
	std::cout << (4.f * v) << std::endl;
	v *= 5.f;
	std::cout << v << std::endl;
	std::cout << (v / 3.f) << std::endl;
	v /= 5.f;
	std::cout << v << std::endl;
	// normalize
	std::cout << normalize(v) << std::endl;
	v = normalize(v);
	std::cout << v << std::endl;
	std::cout << v[2] << std::endl;

	// cross
	std::cout << cross(v, u) << std::endl;
	// dot
	std::cout << dot(v, u) << std::endl;

	// lerp
	std::cout << lerp(v, u, 0.5) << std::endl;

	// veci
	int3 iv{ 1,2,3 };
	assert(int3(5,10,15) == (iv * 5));
	assert(int3(7,14,21) == (iv * 7));

	// project, perpendicular
	//std::cout << v.project(u.normalize()) << std::endl;
	//std::cout << v.perpendicular(u.normalize()) << std::endl;

	Vector_T<float, 3> vs[4];
	std::cout << vs[2] + vs[3] << std::endl;
	std::cout << float3(vs[0][0] + vs[1][0], vs[0][1] + vs[1][1], vs[0][2] + vs[1][2]) << std::endl;

	float3 ddd{ 1,2,3 };
	std::cout << (ddd + (-ddd)) << std::endl;

	// test angle
	std::stringstream ss;
	ss << std::setiosflags(std::ios::fixed) << std::setprecision(2) << Angle(float3(1,2,3), float3(3,3,3)) ;
	assert(0 == ss.str().compare("22.21"));

	// test add,sub
	auto a = float3(1,2,3);
	auto b = float3(4,5,6);
	auto c = float3(7,-3,0);
	assert(float3(5,7,9) == (a + b));
	assert(float3(-3,-3,-3) == (a - b));
	assert(float3(10,0,3) == (b + c - a));

	//  inner product
	assert(30 == (int2(4,6) | int2(-3,7)));
	assert(-15 == (int3(3,-2,7) | int3(0,4,-1)));

	// cross
	assert(int3(-3,6,-3) == (int3(1,2,3) ^ int3(4,5,6)));
	assert(int3(337,110,-919) == (int3(1,22,3) ^ int3(42,5,16)));
}

void checkmatrix()
{
    //std::cout << "checkmatrix sucess" << std::endl;
	// test contruct
	auto tmp1 = float4x4(11, 12, 13, 14,21, 22, 23, 24,31, 32, 33, 34,41, 42, 43, 44);
	auto tmp2 = float4x4(tmp1);
	auto tmp3 = float4x4(11, 12, 13, 14,21, 22, 23, 24,31, 32, 33, 34,41, 42, 43, 44);
	float4x4 tmp4 = {11, 12, 13, 14,21, 22, 23, 24,31, 32, 33, 34,41, 42, 43, 44};
	assert(tmp1 == tmp2);
	assert(tmp1 == tmp3);
	assert(tmp1 == tmp4);
	std::cout << "zero matrix " << std::endl << tmp1.Zero() << std::endl;
	std::cout << "Identity matrix " << std::endl << tmp1.Identity() << std::endl;

	// test matrix multiplication
	assert(float4x4(12,1,13,14, 12,0,12,15,  10,0,10,7, 2,0,2,2) == 
		mul(float4x4(1,2,3,4,  0,0,5,6,  0,2,1,3,  0,1,0,0), 
		float4x4(0,1,1,1,  2,0,2,2,  0,0,0,3,  2,0,2,0)));
	// test +,-
	assert(float4x4(3,3,3,3, 3,3,3,3,  3,3,3,3, 3,3,3,3) == 
		(float4x4(1,1,1,1,  1,1,1,1,  1,1,1,1,  1,1,1,1) + 
		float4x4(2,2,2,2, 2,2,2,2,  2,2,2,2, 2,2,2,2)));
	assert(float4x4(2,2,2,2, 2,2,2,2,  2,2,2,2, 2,2,2,2) == 
		(float4x4(3,3,3,3, 3,3,3,3,  3,3,3,3, 3,3,3,3) - 
		float4x4(1,1,1,1,  1,1,1,1,  1,1,1,1,  1,1,1,1)));
	
    // test matrix scalar
	assert(float4x4(2,2,2,2, 2,2,2,2,  2,2,2,2, 2,2,2,2) == 
		float4x4(1,1,1,1,  1,1,1,1,  1,1,1,1,  1,1,1,1) * 2);
	assert(float4x4(2,2,2,2, 2,2,2,2,  2,2,2,2, 2,2,2,2) / 2 == 
		float4x4(1,1,1,1,  1,1,1,1,  1,1,1,1,  1,1,1,1));


	// athmatic matrix
	auto m1 = float4x4(11, 12, 13, 14,
					21, 22, 23, 24,
					31, 32, 33, 34,
					41, 42, 43, 44);
	auto m2 = float4x4(11, 21, 31, 41,
					 12, 22, 32, 42,
					 13, 23, 33, 43,
					 14, 24, 34, 44);
	assert(transpose(m1) == m2);

	// move 1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1
    auto _1 = float4x4(1,0,0,0,  0,1,0,0,  0,0,1,0,  1,2,3,1);
	assert(_1 == translation(1.f, 2.f, 3.f));
	// Scale
	assert(float4x4(2,0,0,0,  0,0.5,0,0,  0,0,2,0,  0,0,0,1) == MatrixScale(2.f, 0.5f, 2.f));

	// test rotation matrix
    // rotation matrix
    assert(float4x4(1,0,0,0, 0,0.838670671f,0.544638991f,0,  0,-0.544638991f,0.838670671f,0, 0,0,0,1) == rotation_x(Deg2Rad(33)));
    assert(float4x4(0.500000477f,0,-0.866025388f,0, 0,1,0,0,  0.866025388f,0,0.500000477f,0, 0,0,0,1) == rotation_y(Deg2Rad(60)));
    assert(float4x4(0.500000477f,0.866025388f,0,0, -0.866025388f,0.500000477f,0,0,  0,0,1,0, 0,0,0,1) == rotation_z(Deg2Rad(60)));

    // assert(float4x4(0.970946252f,0.147502527f,-0.128133610f,0, 
	// 				-0.128133610f,0.970946252f,0.147502527f,0,  
	// 				0.147502527f,-0.128133610f,0.970946252f,0, 0,0,0,0) == MatrixRotate(float3(1.f,1.f,1.f), Deg2Rad(16)));
	
    LookAtRH(float3(0.f, 0.f, 0.f),
        float3(0.f, 0.f, 0.f),
        float3(0.f, 0.f, 0.f));
	std::cout << "test matrix contruct success" << std::endl << tmp1 << std::endl;
}

void checkquaternion()
{
    //std::cout << "checkquaternion sucess" << std::endl;
	// test contruct
	auto q1 = quater();
	auto q2 = quater(1,2,3, 4);
	auto v1 = float3(1.f, 2.f, 3.f);
	auto q3 = quater(v1, 4.f);
	auto q4 = quater(q2);
	assert(q4 == q3);
	std::cout << "test quaternion  contruct success " << q2 << std::endl;
}

void checkmath()
{

}

int main()
{
    checkvecotr();
	checkquaternion();
    checkmatrix();
	//checkmath();
}