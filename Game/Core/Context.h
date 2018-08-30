/* 2017/9/17 ESV���д�ģ���ʵ�嶼�ŵ�ȫ���У�
	2018/8/23 ����urho3d, �޸�
*/
#ifndef STX_CONTEXT_H_
#define STX_CONTEXT_H_
#pragma once

#include "predefine.h"
#include "Entity/Entity.h"
#include "../Container/hash_list.h"
#include "../Container/cvar_list.h"

#include <map>
#include "../Container/C++17/string_view.h"
#include <boost/noncopyable.hpp>
struct ConfigPath
{
	ConfigPath();

	void SetConfigPath(const char* szPath);

	char szWorkPath[256];
	char szCodePath[256];
	char szResourcePath[256];
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

	// ·��
	void SetPath(const char* szPath);
	const char* GetResource();

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
	bool AddExecute(const PERSISTID obj);
	void RemoveExecute(const PERSISTID obj);
	void Execute(float fTime);
	void Test();

	//
	void SetScene(ScenePtr scene) { pCurScene = (scene); }
	ScenePtr ActiveScene() { return pCurScene; }
	int GetWidth() { return m_nWidth; }
	int GetHeight() { return m_nHeight; }
	void SetWidth(int nWidth) { m_nWidth = nWidth; }
	void SetHeight(int nHeight) { m_nHeight = nHeight; }

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
private:
	int m_nWidth;													// �����
	int m_nHeight;													// �����
	ScenePtr pCurScene;
	static std::unique_ptr<Context> m_InstanceContext;
	bool m_Quit;													// �Ƿ��˳�
	HashMap m_GlobalVar;									//ȫ����ʱֵ
	ConfigPath m_Path;											// ·��

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