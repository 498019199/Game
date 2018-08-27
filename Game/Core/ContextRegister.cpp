#include "Context.h"
#include "../Util/UtilTool.h"

#include "../Platform/DxGraphDevice.h"

#include "../System/Log.h"
#include "../System/ResLoader.h"

#include "../Render/SceneManager.h"
#include "../Render/ICamera.h"
#include "../Render/Renderable.h"
#include "../Render/Mesh.h"
#include "../Render/ITexture.h"
#include <boost/assert.hpp>

#include "../Tool/XMLDocument.h"
#include "../Container/Hash.h"
#define REGISTER_SUBSYSTEM(val) pContext->RegisterSubsystem(NEW val(pContext))
#define REGISTER_FACTORY(val) val::RegisterObject(pContext)

ConfigPath::ConfigPath()
{
	memset(szWorkPath, 0, sizeof(szWorkPath));
	memset(szCodePath, 0, sizeof(szCodePath));
	memset(szResourcePath, 0, sizeof(szResourcePath));
}

void ConfigPath::SetConfigPath(const char* szPath)
{
	SafePrint(szWorkPath, "%s", szPath);
	SafePrint(szCodePath, "%s\\Game\\", szPath);
	SafePrint(szResourcePath, "%s\\Res\\", szPath);
}

extern Context* InitCore(const IVarList& args)
{
	auto pContext =  Context::Instance();
	NULL_RETURN(pContext, nullptr);

	// 核心子系统
	REGISTER_SUBSYSTEM(Log);
	REGISTER_SUBSYSTEM(ResLoader);

	int nWidth = args.IntVal(0);
	int nHeight = args.IntVal(1);
	pContext->SetPath(args.StringVal(2));
	std::string strPath = pContext->GetResource();
	strPath += "config.xml";
	auto fp = ResLoader::Instance()->Open(strPath);
	bool bFullScreen = false;
	ITexture::PixelFormat format = ITexture::PixelFormat::RGBA8888;

	if (fp)
	{
		XMLDocument dot;
		auto pNode = dot.Parse(fp);
		NULL_RETURN(pNode, nullptr);
		auto context_node = pNode->FirstNode("context");
		NULL_RETURN(context_node, nullptr);
		auto graphics_node = pNode->FirstNode("graphics");
		NULL_RETURN(graphics_node, nullptr);

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
	}

	ITexture::setDefaultAlphaPixelFormat(format);
	pContext->SetHeight(nHeight);
	pContext->SetWidth(nWidth);
	return pContext;
}

extern void InitCoreList(Context* pContext)
{
	// 子系统
	REGISTER_SUBSYSTEM(DxGraphDevice);

	// 场景
	REGISTER_FACTORY(SceneManager);

	// 图形
	REGISTER_FACTORY(Renderable);
	REGISTER_FACTORY(ICamera);
	REGISTER_FACTORY(RenderModel);
	REGISTER_FACTORY(StaticMesh);
}

extern void EndCore()
{
#define REMOVE_SUBSYSTEM(val) Context::Instance()->RemoveSubsystem(val::GetTypeNameStatic().c_str());
	REMOVE_SUBSYSTEM(Log);
	REMOVE_SUBSYSTEM(ResLoader);
}