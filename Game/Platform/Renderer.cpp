#include "../Platform/Renderer.h"
#include "../Platform/Window/Window.h"
#include "../Core/Context.h"
#include "../System/Log.h"
#include "../Render/ILight.h"
#include "../Render/ICamera.h"
#include "../Render/IScene.h"
#include "../Core/CHelper.h"
#include "../Render/ITexture.h"
#include "../Util/UtilTool.h"

#include <boost/assert.hpp>

Renderer::Renderer(Context* pContext)
	:IEntityEx(pContext)
{

}

bool Renderer::OnInit()
{
	return true;
}

bool Renderer::OnShut()
{
	return true;
}

void Renderer::Update()
{

}

void Renderer::SetFont(DxGraphDevice* device)
{
	m_TextPrint.InitFont(device, "Î¢ÈíÑÅºÚ");
}

void Renderer::ProccessWinMsg(std::size_t nParam1, std::size_t nParam2)
{

}

void Renderer::Refresh()
{
	Render();
}

void Renderer::Render()
{
	auto pDevice = Context::Instance()->GetSubsystem<DxGraphDevice>();
	auto pApp = Context::Instance()->AppInstance();
	auto pScene = Context::Instance()->ActiveScene();

	pDevice->BeginRender();
	Context::Instance()->ActiveScene()->Render();
	pDevice->DrawTextGDI("Press ESC to exit.", 0, 0, RGB(0, 255, 0));
	char szBuffer[1024] = { 0 };
	sprintf_s(szBuffer, "FPS:%0.3lf, NumFrame:%f£¬FrameTime:%0.3lf", pApp->FPS(), pApp->AppTime(), pApp->FrameTime());
	pDevice->DrawTextGDI(szBuffer, 0, 20, RGB(255, 0, 0));
	sprintf_s(szBuffer, "DynTexture:%0.2f MB", (0.0f/*pRender->GetDynTextureSize()*/));
	pDevice->DrawTextGDI(szBuffer, 0, 40, RGB(255, 0, 0));
	sprintf_s(szBuffer, "Polys number: %d", pScene->GetPolysNum());
	pDevice->DrawTextGDI(szBuffer, 0, 60, RGB(255, 0, 0));
	pDevice->EndRender();
}
