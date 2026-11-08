#include <render/RenderView.h>

namespace RenderWorker
{
	enum CascadedShadowLayerType
	{
		CSLT_Auto,
		CSLT_PSSM,
		CSLT_SDSM
	};

    ZENGINE_CORE_API AABBox CalcFrustumExtents(Camera const & camera, float near_z, float far_z, float4x4 const & light_view_proj);
}