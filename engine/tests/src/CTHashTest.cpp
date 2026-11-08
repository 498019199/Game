#include "test.h"
#include <base/ZEngine.h>
#include <common/Hash.h>
using namespace RenderWorker;

TEST(CTHashTest, Basic)
{
	EXPECT_EQ(CtHash("ABCD"), RtHash("ABCD"));
	EXPECT_EQ(CtHash("KlayGE"), RtHash("KlayGE"));
	EXPECT_EQ(CtHash("Test"), RtHash("Test"));
	EXPECT_EQ(CtHash("min_linear_mag_point_mip_linear"), RtHash("min_linear_mag_point_mip_linear"));
}
