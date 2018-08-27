#include "../Platform/Renderer.h"
#include "../Core/Context.h"
#include "../System/Log.h"
#include "../Render/ILight.h"
#include "../Render/ICamera.h"
#include "../Render/SceneManager.h"
#include "../Core/CHelper.h"
#include "../Render/ITexture.h"
#include "../Util/UtilTool.h"

#include <boost/assert.hpp>
void Renderer::Inilize(int nWidth, int nHeight, const char* szWorkPath, HINSTANCE hInstance)
{
	CVarList var;
	var << nWidth << nWidth << szWorkPath;
	auto pCore = InitCore(var);
	if (nullptr == pCore)
	{
		Log::Instance()->write(LOG_ERROR, "Core inlitial fail!") ;
	}
	m_IntPut.InitDevice(hInstance);
	InitCoreList(pCore);
}

void Renderer::SetFont(DxGraphDevice* device)
{
	m_TextPrint.InitFont(device, "微软雅黑");
}

void Renderer::ProccessWinMsg(std::size_t nParam1, std::size_t nParam2)
{

}

void Renderer::Render()
{
	// general rotation matrix
	static float4x4 mrot; 
	mrot.Identity();
	// rotation angle
	static float ang_y = 1.0f;     
	static float ang_x = 0.5f;
	// 读取键盘鼠标信息
	m_IntPut.Execute();
	if ('w' == m_IntPut.GetKeyCode())
	{
		ang_x += 0.1f;
	}
	if ('s' == m_IntPut.GetKeyCode())
	{
		ang_x -= 0.1f;
	}
	if ('a' == m_IntPut.GetKeyCode())
	{
		ang_y += 0.1f;
	}
	if ('d' == m_IntPut.GetKeyCode())
	{
		ang_y -= 0.1f;
	}
	if ((ang_y) >= 360.0)
	{
		ang_y = 0;
	}

	mrot = MathLib::MatrixRotateY(ang_y);
	mrot *= MathLib::MatrixScale(float3(ang_x, ang_x, ang_x));
	auto val = MakeSharedPtr<VariableFloat4x4>();
	*val = mrot;
	Context::Instance()->SetGlobalValue("control_move", val);

	auto pDevice = Context::Instance()->GetSubsystem<DxGraphDevice>();
	pDevice->BeginRender();
	Context::Instance()->ActiveScene()->RefreshRender(pDevice);

	pDevice->DrawTextGDI("Press ESC to exit.", 0, 0, RGB(0, 255, 0));
	char szBuffer[1024] = { 0 };
	sprintf_s(szBuffer, "FPS:%0.3f, NumFrame:%d，FrameTime:%0.3f", m_fFPS, n_nNumFrame, m_fFrameTime);
	pDevice->DrawTextGDI(szBuffer, 0, 20, RGB(255, 0, 0));
	sprintf_s(szBuffer, "DynTexture:%0.2f MB", (0.0f/*pRender->GetDynTextureSize()*/));
	pDevice->DrawTextGDI(szBuffer, 0, 40, RGB(255, 0, 0));
	sprintf_s(szBuffer, "Polys number: %d", 0/*pRender->GetPolysNum()*/);
	pDevice->DrawTextGDI(szBuffer, 0, 60, RGB(255, 0, 0));
	pDevice->EndRender();
}

void Renderer::Display(float second)
{
	Context::Instance()->Execute(second);
	Context::Instance()->Test();
}

void Renderer::Run()
{
	++m_nTotalNumFrames;
	m_fFrameTime = static_cast<float>(m_Timer.Elapsed());
	++n_nNumFrame;
	m_fAccumulate += m_fFrameTime;
	m_fAppTime += m_fFrameTime;

	Display(m_fFrameTime);
	if (m_fAccumulate > 1)
	{
		m_fFPS = n_nNumFrame / m_fAccumulate;

		m_fAccumulate = 0;
		n_nNumFrame = 0;
	}
	m_Timer.ReStart();

	static DWORD                start_clock_count;
	start_clock_count = GetTickCount();
	Render();
	while (::GetTickCount() - start_clock_count < 30);
}

float Renderer::GetFPS()
{
	//定义四个静态变量  
	static float  fps = 0; //我们需要计算的FPS值  
	static int    frameCount = 0;//帧数  
	static float  currentTime = 0.0f;//当前时间  
	static float  lastTime = 0.0f;//持续时间  

	frameCount++;//每调用一次Get_FPS()函数，帧数自增1  
	currentTime = timeGetTime()*0.001f;//获取系统时间，其中timeGetTime函数返回的是以毫秒为单位的系统时间，所以需要乘以0.001，得到单位为秒的时间  
	//如果当前时间减去持续时间大于了1秒钟，就进行一次FPS的计算和持续时间的更新，并将帧数值清零  
	if (currentTime - lastTime > 1.0f) //将时间控制在1秒钟  
	{
		fps = (float)frameCount / (currentTime - lastTime);//计算这1秒钟的FPS值  
		lastTime = currentTime; //将当前时间currentTime赋给持续时间lastTime，作为下一秒的基准时间  
		frameCount = 0;//将本次帧数frameCount值清零  
	}

	return fps;
}
