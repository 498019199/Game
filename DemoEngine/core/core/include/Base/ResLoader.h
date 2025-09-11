#pragma once
#include <base/ZEngine.h>

namespace RenderWorker
{
class ThreadPool;

class ZENGINE_CORE_API ResLoadingDesc
{
    ZENGINE_NONCOPYABLE(ResLoadingDesc);
public:
    ResLoadingDesc() noexcept;
    virtual ~ResLoadingDesc() noexcept;

    virtual uint64_t Type() const = 0;

    virtual bool StateLess() const = 0;

    virtual std::shared_ptr<void> CreateResource()
    {
        return std::shared_ptr<void>();
    }
    virtual void SubThreadStage() = 0;
    virtual void MainThreadStage() = 0;

    virtual bool HasSubThreadStage() const = 0;

    virtual bool Match(ResLoadingDesc const & rhs) const = 0;
    virtual void CopyDataFrom(ResLoadingDesc const & rhs) = 0;
    virtual std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) = 0;

    virtual std::shared_ptr<void> Resource() const = 0;
};
using ResLoadingDescPtr = std::shared_ptr<ResLoadingDesc>;

class ZENGINE_CORE_API ResLoader final
{
    ZENGINE_NONCOPYABLE(ResLoader);
    friend class Context;
public:
    ResLoader() noexcept;
    ~ResLoader() noexcept;

    void Suspend();
	void Resume();

    void AddPath(std::string_view phy_path);
	void DelPath(std::string_view phy_path);
	bool IsInPath(std::string_view phy_path);
	std::string const& LocalFolder() const noexcept;

    void Mount(std::string_view virtual_path, std::string_view phy_path);
    void Unmount(std::string_view virtual_path, std::string_view phy_path);

    ResIdentifierPtr Open(std::string_view name);
    std::string Locate(std::string_view name);
    uint64_t Timestamp(std::string_view name);
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

    
	void Update();

	uint32_t NumLoadingResources() const noexcept;

private:
    void Init(ThreadPool& tp);
    void Destroy() noexcept;
    bool Valid() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

}