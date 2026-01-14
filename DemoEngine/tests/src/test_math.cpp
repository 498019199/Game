#include "test.h"
#include <math/math.h>
using namespace RenderWorker;

TEST(MathTest, VectorContruct)
{
	// test contruct
	float3 v(1, 2, 3);
	float3 u = { 0,2,3 };
	float3 w = {1.f, 1.f, 1.f};
	EXPECT_EQ(float3(1,2,3), v);
	EXPECT_EQ(float3(0,2,3), u);
	EXPECT_EQ(float3(1.f,1.f,1.f), w);
}

TEST(MathTest, VectorOperater)
{
	int3 iv(1, 2, 3);
	float3 v(1.f, 2.f, 3.f);
	int3 iu(0, 2, 3);

	// -
	EXPECT_EQ(int3(-1, -2, -3), -iv);
	// +
	EXPECT_EQ(int3(1, 4, 6), (iv + iu));
	iv += iu;
	EXPECT_EQ(int3(1, 4, 6), iv);
	// -
	iv -= iu;
	EXPECT_EQ(int3(1, 2, 3), iv);
	EXPECT_EQ(int3(1, 0, 0), (iv - iu));

	// *
	v = int3(1, 2, 3);
	EXPECT_EQ(int3(1, 2, 3), iv);
	EXPECT_EQ(int3(5, 10, 15), 5 * iv);
	EXPECT_EQ(int3(5, 10, 15), iv * 5);

	v = float3(1.f, 2.f, 3.f);
	EXPECT_EQ(float3(1.f, 2.f, 3.f), v);
	EXPECT_EQ(float3(5.f, 10.f, 15.f), 3.f * v);
	EXPECT_EQ(float3(5, 10, 15), v * 5);

	// /
	v = float3(1.f, 2.f, 3.f);
	EXPECT_EQ(float3(0.333333f, 0.666667f, 1.000000f), (v / 3.f));

	// ==
	iv = { 1,2,3 };
	EXPECT_EQ(int3(5,10,15), (iv * 5));
	EXPECT_EQ(int3(7,14,21), (iv * 7));


	// test add,sub
	auto a = float3(1,2,3);
	auto b = float3(4,5,6);
	auto c = float3(7,-3,0);
	EXPECT_EQ(float3(5,7,9), (a + b));
	EXPECT_EQ(float3(-3,-3,-3), (a - b));
	EXPECT_EQ(float3(10,0,3), (b + c - a));

	//  inner product
	EXPECT_EQ(30, (int2(4,6) | int2(-3,7)));
	EXPECT_EQ(-15, (int3(3,-2,7) | int3(0,4,-1)));

	// cross
	EXPECT_EQ(int3(-3,6,-3), (int3(1,2,3) ^ int3(4,5,6)));
	EXPECT_EQ(int3(337,110,-919), (int3(1,22,3) ^ int3(42,5,16)));


	// test angle
	std::stringstream ss;
	ss << std::setiosflags(std::ios::fixed) 
		<< std::setprecision(2) 
		<< MathWorker::Angle(float3(1,2,3), float3(3,3,3)) ;
	EXPECT_LT(0, ss.str().compare("22.21"));
}

TEST(MathTest, NormalizeFloat2)
{
	float2 v(1, 2);
	v = MathWorker::normalize(v);
	EXPECT_LT(MathWorker::abs(MathWorker::length(v) - 1.0f), 1e-5f);
}

TEST(MathTest, NormalizeFloat3)
{
	float3 v(1, 2, 3);
	v = MathWorker::normalize(v);
	EXPECT_LT(MathWorker::abs(MathWorker::length(v) - 1.0f), 1e-5f);
}

TEST(MathTest, NormalizeFloat4)
{
	float4 v(1, 2, 3, 4);
	v = MathWorker::normalize(v);
	EXPECT_LT(MathWorker::abs(MathWorker::length(v) - 1.0f), 1e-5f);
}
