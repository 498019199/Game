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
	m_TextPrint.InitFont(device, "΢���ź�");
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
	// ��ȡ���������Ϣ
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
	sprintf_s(szBuffer, "FPS:%0.3f, NumFrame:%d��FrameTime:%0.3f", m_fFPS, n_nNumFrame, m_fFrameTime);
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
	//�����ĸ���̬����  
	static float  fps = 0; //������Ҫ�����FPSֵ  
	static int    frameCount = 0;//֡��  
	static float  currentTime = 0.0f;//��ǰʱ��  
	static float  lastTime = 0.0f;//����ʱ��  

	frameCount++;//ÿ����һ��Get_FPS()������֡������1  
	currentTime = timeGetTime()*0.001f;//��ȡϵͳʱ�䣬����timeGetTime�������ص����Ժ���Ϊ��λ��ϵͳʱ�䣬������Ҫ����0.001���õ���λΪ���ʱ��  
	//�����ǰʱ���ȥ����ʱ�������1���ӣ��ͽ���һ��FPS�ļ���ͳ���ʱ��ĸ��£�����֡��ֵ����  
	if (currentTime - lastTime > 1.0f) //��ʱ�������1����  
	{
		fps = (float)frameCount / (currentTime - lastTime);//������1���ӵ�FPSֵ  
		lastTime = currentTime; //����ǰʱ��currentTime��������ʱ��lastTime����Ϊ��һ��Ļ�׼ʱ��  
		frameCount = 0;//������֡��frameCountֵ����  
	}

	return fps;
}
