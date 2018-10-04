#include "Context.h"
#include "../Util/UtilTool.h"
#include "../Core/predefine.h"
#include "../Platform/DxGraphDevice.h"

#include "../System/Log.h"
#include "../System/ResLoader.h"
#include "../Platform/renderer.h"

#include "../Render/IScene.h"
#include "../Render/ICamera.h"
#include "../Render/Renderable.h"
#include "../Render/Mesh.h"
#include "../Render/ITexture.h"
#include <boost/assert.hpp>

#include "../Tool/XMLDocument.h"
#include "../Container/Hash.h"
extern void InitInputList(Context* pContext);
extern void InitCompenetList(Context* pContext);
#define REGISTER_SUBSYSTEM(val) pContext->RegisterSubsystem(NEW val(pContext))
#define REGISTER_FACTORY(val) val::RegisterObject(pContext)
extern Context* InitCore(const IVarList& args)
{
	auto pContext =  Context::Instance();
	NULL_RETURN(pContext, nullptr);

	// 核心子系统
	REGISTER_SUBSYSTEM(Log);
	REGISTER_SUBSYSTEM(ResLoader);

	int nWidth = args.IntVal(0);
	int nHeight = args.IntVal(1);
	auto fp = ResLoader::Instance()->Open(args.StringVal(3));
	bool bFullScreen = args.BoolVal(2);
	bool bKeepScreenOn = true;

	ITexture::PixelFormat format = ITexture::PixelFormat::RGBA8888;

	if (fp)
	{
		XMLDocument dot;
		auto pNode = dot.Parse(fp);
		NULL_RETURN(pNode, nullptr);
		auto context_node = pNode->FirstNode("context");
		NULL_RETURN(context_node, nullptr);


		auto graphics_node = pNode->FirstNode("graphics");
		if (nullptr != graphics_node)
		{
			auto frame_node = graphics_node->FirstNode("frame");
			auto attr = frame_node->Attrib("width");
			if (attr)
			{
				nWidth = attr->ValueInt();
			}
			attr = frame_node->Attrib("height");
			if (attr)
			{
				nHeight = attr->ValueInt();
			}

			std::string color_fmt_str = "ARGB8";
			attr = frame_node->Attrib("color_fmt");
			if (attr)
			{
				color_fmt_str = std::string(attr->ValueString());
			}
			size_t const color_fmt_str_hash = RT_HASH(color_fmt_str.c_str());
			if (CT_HASH("ARGB8") == color_fmt_str_hash)
			{
				format = ITexture::PixelFormat::RGBA8888;
			}
			else if (CT_HASH("ABGR8") == color_fmt_str_hash)
			{
				format = ITexture::PixelFormat::BGRA8888;
			}

			attr = frame_node->Attrib("fullscreen");
			if (attr)
			{
				bFullScreen = UtilString::as_bool(attr->ValueString());
			}
			attr = frame_node->Attrib("keep_screen_on");
			if (attr)
			{
				bKeepScreenOn = UtilString::as_bool(attr->ValueString());
			}
		}
	}

	WindowDesc config;
	config.bHideWin = false;
	config.nTop = 0;
	config.nLeft = 0;
	config.nWidth = nWidth;
	config.nHeight = nHeight;
	config.bFullScreen = bFullScreen;
	config.bKeepScreenOn = bKeepScreenOn;
	config.m_nRenderType = RENDER_TYPE_TEXTURE;
	pContext->SetConfig(config);
	ITexture::setDefaultAlphaPixelFormat(format);

	return pContext;
}

extern void InitCoreList(Context* pContext)
{
	// 子系统
	REGISTER_SUBSYSTEM(DxGraphDevice);
	REGISTER_SUBSYSTEM(Renderer);
	InitInputList(pContext);

	// 场景
	REGISTER_FACTORY(IScene);

	// 图形
	REGISTER_FACTORY(Renderable);
	REGISTER_FACTORY(ICamera);
	REGISTER_FACTORY(RenderModel);
	REGISTER_FACTORY(StaticMesh);

	InitCompenetList(pContext);
}

extern void EndCore()
{
#define REMOVE_SUBSYSTEM(val) Context::Instance()->RemoveSubsystem(val::GetTypeNameStatic().c_str());
	REMOVE_SUBSYSTEM(Log);
	REMOVE_SUBSYSTEM(ResLoader);
}