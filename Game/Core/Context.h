/* 2017/9/17 ESV���д�ģ���ʵ�嶼�ŵ�ȫ���У�
	2018/8/23 ����urho3d, �޸�
*/
#ifndef STX_CONTEXT_H_
#define STX_CONTEXT_H_
#pragma once

#include "predefine.h"
#include "Entity/Entity.h"
#include "App.h"
#include "../Container/hash_list.h"
#include "../Container/cvar_list.h"

#include <map>
#include "../Container/C++17/string_view.h"
#include <boost/noncopyable.hpp>

enum RenderType
{
	RENDER_TYPE_NONE = 0,
	RENDER_TYPE_WIREFRAME = 1, // ��Ⱦ�߿�
	RENDER_TYPE_TEXTURE = 2,	//��Ⱦ����
	RENDER_TYPE_COLOR = 4,		// ��Ⱦ��ɫ
};
struct WindowDesc
{
	RenderType m_nRenderType;

	bool	bHideWin;
	bool	bFullScreen;

	int		nLeft;
	int		nTop;
	int		nWidth;
	int		nHeight;

	bool bKeepScreenOn;
	WindowDesc()
		:m_nRenderType(RENDER_TYPE_NONE),bHideWin(false),bFullScreen(false)
		,nLeft(0),nTop(0),nWidth(0),nHeight(0)
		, bKeepScreenOn(false)
	{}
};

extern Context* InitCore(const IVarList& args);
extern void InitCoreList(Context* pContext);
extern void EndCore();

class Context : boost::noncopyable
{
	typedef hash_list<char, VariablePtr> HashMap;
public:
	// ʵ����
	static Context* Instance();

	// �˳�״̬
	void SetQuit();
	bool GetQuit()const ;
	void Close();

	//Ѱ��ȫ��ֵ
	bool FindGlobalValue(const char* szName);
	//��ȡȫ��ֵ����
	int GetGlobalCount();
	//����ȫ��ֵ
	bool SetGlobalValue(const char* szName, VariablePtr val);
	//��ȡȫ��ֵ
	VariablePtr GetGlobalValue(const char* szName);
	//ɾ��ȫ��ֵ
	bool RemoveGlobalValue(const char* szName);

	// ֡ѭ��
	void DisPlay(float fTimer);
	bool AddExecute(const PERSISTID obj);
	void RemoveExecute(const PERSISTID obj);
	void Test();

	// ��ȡ��������
	void SetScene(ScenePtr scene) { pCurScene = (scene); }
	ScenePtr ActiveScene() { return pCurScene; }
	WindowDesc& GetConfig() { return m_ConfigWinDesc; }
	void SetConfig(const WindowDesc& desc) { m_ConfigWinDesc = desc; }
	int GetWidth() { return m_ConfigWinDesc.nWidth; }
	int GetHeight() { return m_ConfigWinDesc.nHeight; }
	RenderType GetRenderType() { return m_ConfigWinDesc.m_nRenderType; }
	// ע��
	template <typename T> void RegisterFactory();
	void RegisterFactory(EnitityFactory* factory);
	// ʵ�崴��
	template <typename T> inline std::shared_ptr<T> CreateObject();
	std::shared_ptr<IEntity> CreateObject(const char* szName);
	template<typename T, typename... Args>
	std::shared_ptr<T> CreateObjectArgs(Args&&... args);

	// ��ϵͳ����
	template <typename T> T* GetSubsystem() const;
	void RegisterSubsystem(IEntityEx* pEntity);
	bool RemoveSubsystem(const char* szName);
	IEntity* GetSubsystem(const char* szName) const;
	// ʵ�����
	IEntityPtr GetEntity(const PERSISTID& obj);
	void AddEntity(const PERSISTID& obj, IEntityPtr pEntity);
	bool RemoveEntity(const PERSISTID& ob);

	void AppInstance(App* app){m_App = app;}
	bool AppValid() const{return m_App != nullptr;}
	App* AppInstance(){BOOST_ASSERT(m_App);return m_App;}
private:
	WindowDesc m_ConfigWinDesc;
	ScenePtr pCurScene;
	static std::unique_ptr<Context> m_InstanceContext;
	bool m_Quit;													// �Ƿ��˳�
	HashMap m_GlobalVar;									//ȫ����ʱֵ

	// �������̹����࣬
	App*		m_App;
	// ��ϵͳ
	hash_list<char, std::shared_ptr<IEntityEx>> m_SubSystemMrg;
	// ���������
	hash_list<char, std::shared_ptr<EnitityFactory>> m_FactoryMrg;
	// ʵ�����
	std::map<PERSISTID, IEntityPtr> m_EntityMrg;
};


template <typename T> void Context::RegisterFactory() { RegisterFactory(NEW EnitityFactoryImpl<T>(this)); }
template <typename T> inline std::shared_ptr<T>	Context::CreateObject(){return std::static_pointer_cast<T>(CreateObject(T::GetTypeNameStatic().c_str()));}
template <typename T> T* Context::GetSubsystem() const { return reinterpret_cast<T*>(GetSubsystem(T::GetTypeNameStatic().c_str())); }

template<typename T, typename... Args> std::shared_ptr<T>
	Context::CreateObjectArgs(Args&&... args)
	{
		auto it = m_FactoryMrg.find(T::GetTypeNameStatic().c_str());
		auto  pFunc = std::static_pointer_cast<EnitityFactoryImpl<T>>(it.get_data());
		return pFunc->CreateObjectArgs<T>(std::forward<Args>(args)...);
	}
#endif//STX_CONTEXT_H_
