// 2018年7月31日 移植klayGE代码 zhangbei
// 多线程异步加载文件类
#ifndef _RESLOADER_HPP_
#define _RESLOADER_HPP_
#pragma once

#include "../Core/predefine.h"
#include "../Core/Entity/Entity.h"
//#include "../Container/Thread.h"

#include <istream>
#include <vector>
#include <string>
#include <mutex>

//#include <boost/lockfree/spsc_queue.hpp>
#include "../Container/C++17/string_view.h"
#include <boost/noncopyable.hpp>
class  ResLoadingDesc : boost::noncopyable
{
public:
	virtual ~ResLoadingDesc(){}
	virtual uint64_t Type() const = 0;
	virtual bool StateLess() const = 0;
	virtual std::shared_ptr<void> CreateResource(){	return std::shared_ptr<void>();}
	virtual void SubThreadStage() = 0;
	virtual void MainThreadStage() = 0;
	virtual bool HasSubThreadStage() const = 0;
	virtual bool Match(ResLoadingDesc const & rhs) const = 0;
	virtual void CopyDataFrom(ResLoadingDesc const & rhs) = 0;
	virtual std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) = 0;
	virtual std::shared_ptr<void> Resource() const = 0;
};

class ResIdentifier
{
public:
	ResIdentifier(std::string_view name, uint64_t timestamp,
		std::shared_ptr<std::istream> const & is)
		: ResIdentifier(name, timestamp, is, std::shared_ptr<std::streambuf>())
	{
	}
	ResIdentifier(std::string_view name, uint64_t timestamp,
		std::shared_ptr<std::istream> const & is, std::shared_ptr<std::streambuf> const & streambuf)
		: m_strNameRes(name), m_nTimestamp(timestamp), m_IStream(is), m_Streambuf(streambuf)
	{
	}

	void ResName(std::string_view name)
	{
		m_strNameRes = std::string(name);
	}
	std::string const & ResName() const
	{
		return m_strNameRes;
	}

	void Timestamp(uint64_t ts)
	{
		m_nTimestamp = ts;
	}
	uint64_t Timestamp() const
	{
		return m_nTimestamp;
	}

	void read(void* p, size_t size)
	{
		m_IStream->read(static_cast<char*>(p), static_cast<std::streamsize>(size));
	}
	bool empty() const
	{
		return gcount() == 0;
	}
	int64_t gcount() const
	{
		return static_cast<int64_t>(m_IStream->gcount());
	}

	void seekg(int64_t offset, std::ios_base::seekdir way)
	{
		m_IStream->seekg(static_cast<std::istream::off_type>(offset), way);
	}

	int64_t tellg()
	{
		return static_cast<int64_t>(m_IStream->tellg());
	}

	void clear()
	{
		m_IStream->clear();
	}

	operator bool() const
	{
		return !m_IStream->fail();
	}

	bool operator!() const
	{
		return m_IStream->operator!();
	}

	std::istream& input_stream()
	{
		return *m_IStream;
	}

private:
	std::string m_strNameRes;
	uint64_t m_nTimestamp;
	std::shared_ptr<std::istream> m_IStream;
	std::shared_ptr<std::streambuf> m_Streambuf;
};

class ResLoader : public IEntityEx
{
	// 文件状态
	enum LoadingStatus
	{
		LS_Loading,
		LS_Complete,
		LS_CanBeRemoved
	};
public:
	STX_ENTITY(ResLoader, IEntityEx);

	ResLoader(Context* pContext);

	~ResLoader();

	static ResLoader* Instance();

	static void Destroy();
	void Suspend();
	void Resume();

	void AddPath(std::string_view phy_path);
	void DelPath(std::string_view phy_path);
	inline const std::string& LocalFolder() const;

	void Mount(std::string_view virtual_path, std::string_view phy_path);
	void Unmount(std::string_view virtual_path, std::string_view phy_path);

	ResIdentifierPtr Open(std::string_view name);
	std::string Locate(std::string_view name);
	std::string AbsPath(std::string_view path);

	std::shared_ptr<void> SyncQuery(ResLoadingDescPtr const & res_desc);
	std::shared_ptr<void> ASyncQuery(ResLoadingDescPtr const & res_desc);
	void Unload(std::shared_ptr<void> const & res);

	template <typename T>
	std::shared_ptr<T> SyncQueryT(ResLoadingDescPtr const & res_desc)
	{
		return std::static_pointer_cast<T>(this->SyncQuery(res_desc));
	}

	template <typename T>
	std::shared_ptr<T> ASyncQueryT(ResLoadingDescPtr const & res_desc)
	{
		return std::static_pointer_cast<T>(this->ASyncQuery(res_desc));
	}

	template <typename T>
	void Unload(std::shared_ptr<T> const & res)
	{
		this->Unload(std::static_pointer_cast<void>(res));
	}

	virtual bool OnInit() override;

	virtual bool OnShut() override;

	virtual void Update() override;

private:
	std::string RealPath(std::string_view path);
	std::string RealPath(std::string_view path,
		std::string& package_path, std::string& password, std::string& path_in_package);
	void DecomposePackageName(std::string_view path,
		std::string& package_path, std::string& password, std::string& path_in_package);

	void AddLoadedResource(ResLoadingDescPtr const & res_desc, std::shared_ptr<void> const & res);
	std::shared_ptr<void> FindMatchLoadedResource(ResLoadingDescPtr const & res_desc);
	void RemoveUnrefResources();

	void LoadingThreadFunc();

private:
	static std::unique_ptr<ResLoader> m_InstanceResLoaderPtr;
	// 执行exe路径
	std::string m_strExePath;
	// 资源路径前缀
	std::string m_strLocalPath;

	// 7z解压包文件互斥锁
	std::mutex m_PathsMutex;
	// 文件缓存互斥锁
	std::mutex m_LoadedMutex;
	// 文件加载互斥锁
	std::mutex m_LoadingMutex;

	// 多个文件缓存
	std::vector<std::pair<ResLoadingDescPtr, std::weak_ptr<void>>> m_LoadedResVec;
	// 多个文件状态记录
	std::vector<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>> m_LoadingRes;
	// 压缩文件数据信息列表
	std::vector<std::tuple<uint64_t, uint32_t, std::string, PackagePtr>> m_PathDataVec;

	//boost::lockfree::spsc_queue<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>,
	//	boost::lockfree::capacity<1024>> loading_res_queue_;
	//std::unique_ptr<joiner<void>> loading_thread_;
	
// 是否退出
	volatile bool m_bQuit;
};
#endif//_RESLOADER_HPP_
