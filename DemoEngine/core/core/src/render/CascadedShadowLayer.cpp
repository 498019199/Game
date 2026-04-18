#include <render/CascadedShadowLayer.h>
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <render/RenderEngine.h>

namespace RenderWorker
{
    AABBox CalcFrustumExtents(Camera const & camera, float near_z, float far_z, float4x4 const & light_view_proj)
	{
		float const inv_scale_x = 1.0f / camera.ProjMatrix()(0, 0);
		float const inv_scale_y = 1.0f / camera.ProjMatrix()(1, 1);

		float4x4 const view_to_light_proj = camera.InverseViewMatrix() * light_view_proj;

		float3 corners[8];

		float near_x = inv_scale_x * near_z;
		float near_y = inv_scale_y * near_z;
		corners[0] = float3(-near_x, +near_y, near_z);
		corners[1] = float3(+near_x, +near_y, near_z);
		corners[2] = float3(-near_x, -near_y, near_z);
		corners[3] = float3(+near_x, -near_y, near_z);

		float far_x = inv_scale_x * far_z;
		float far_y = inv_scale_y * far_z;
		corners[4] = float3(-far_x, +far_y, far_z);
		corners[5] = float3(+far_x, +far_y, far_z);
		corners[6] = float3(-far_x, -far_y, far_z);
		corners[7] = float3(+far_x, -far_y, far_z);

		for (uint32_t i = 0; i < 8; ++ i)
		{
			corners[i] = MathWorker::transform_coord(corners[i], view_to_light_proj);
		}

		return MathWorker::compute_aabbox(corners, corners + 8);
	}
}