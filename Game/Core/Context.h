/* 2017/9/17 ESV框架写的，把实体都放到全局中，
	2018/8/23 按照urho3d, 修改
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
	RENDER_TYPE_WIREFRAME = 1, // 渲染线框
	RENDER_TYPE_TEXTURE = 2,	//渲染纹理
	RENDER_TYPE_COLOR = 4,		// 渲染颜色
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
	// 实例化
	static Context* Instance();

	// 退出状态
	void SetQuit();
	bool GetQuit()const ;
	void Close();

	//寻找全局值
	bool FindGlobalValue(const char* szName);
	//获取全局值个数
	int GetGlobalCount();
	//设置全局值
	bool SetGlobalValue(const char* szName, VariablePtr val);
	//获取全局值
	VariablePtr GetGlobalValue(const char* szName);
	//删除全局值
	bool RemoveGlobalValue(const char* szName);

	// 帧循环
	void DisPlay(float fTimer);
	bool AddExecute(const PERSISTID obj);
	void RemoveExecute(const PERSISTID obj);
	void Test();

	// 获取配置数据
	void SetScene(ScenePtr scene) { pCurScene = (scene); }
	ScenePtr ActiveScene() { return pCurScene; }
	WindowDesc& GetConfig() { return m_ConfigWinDesc; }
	void SetConfig(const WindowDesc& desc) { m_ConfigWinDesc = desc; }
	int GetWidth() { return m_ConfigWinDesc.nWidth; }
	int GetHeight() { return m_ConfigWinDesc.nHeight; }
	RenderType GetRenderType() { return m_ConfigWinDesc.m_nRenderType; }
	// 注册
	template <typename T> void RegisterFactory();
	void RegisterFactory(EnitityFactory* factory);
	// 实体创建
	template <typename T> inline std::shared_ptr<T> CreateObject();
	std::shared_ptr<IEntity> CreateObject(const char* szName);
	template<typename T, typename... Args>
	std::shared_ptr<T> CreateObjectArgs(Args&&... args);

	// 子系统管理
	template <typename T> T* GetSubsystem() const;
	void RegisterSubsystem(IEntityEx* pEntity);
	bool RemoveSubsystem(const char* szName);
	IEntity* GetSubsystem(const char* szName) const;
	// 实体管理
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
	bool m_Quit;													// 是否退出
	HashMap m_GlobalVar;									//全局临时值

	// 窗口流程管理类，
	App*		m_App;
	// 子系统
	hash_list<char, std::shared_ptr<IEntityEx>> m_SubSystemMrg;
	// 工程类管理
	hash_list<char, std::shared_ptr<EnitityFactory>> m_FactoryMrg;
	// 实体管理
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
