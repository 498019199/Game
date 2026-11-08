#include "DotnetHost.h"
#include <nethost.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>
#ifdef _WIN32
#include <windows.h>
#define HOST_TEXT(value) L##value
#else
#include <dlfcn.h>
#define HOST_TEXT(value) value
#endif
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace Gameplay
{
namespace
{
std::runtime_error HostError(const char* operation, int code)
{
    std::ostringstream message;
    message << operation << " failed (0x" << std::hex << static_cast<unsigned>(code) << ')';
    return std::runtime_error(message.str());
}

template <typename T> T Symbol(void* library, const char* name)
{
    #ifdef _WIN32
    auto address = GetProcAddress(static_cast<HMODULE>(library), name);
#else
    auto address = dlsym(library, name);
#endif
    if (!address) throw std::runtime_error(std::string("Missing hostfxr export: ") + name);
    return reinterpret_cast<T>(address);
}
}

RuntimeApi LoadGameRuntime(const std::filesystem::path& managed_directory,
                               const std::filesystem::path& dotnet_root)
{
    const auto directory = std::filesystem::canonical(managed_directory);
    const auto assembly = (directory / "Game.NativeBridge.dll").native();
    const auto config = (directory / "Game.NativeBridge.runtimeconfig.json").native();
    if (!std::filesystem::is_regular_file(assembly) || !std::filesystem::is_regular_file(config))
        throw std::runtime_error("Missing bridge DLL/runtimeconfig; build the managed target first.");
    const auto root = std::filesystem::canonical(dotnet_root).native();
    get_hostfxr_parameters parameters{sizeof(get_hostfxr_parameters), assembly.c_str(), root.c_str()};
    // Ask nethost for the required capacity instead of assuming PATH_MAX.
    std::size_t size = 0;
    get_hostfxr_path(nullptr, &size, &parameters);
    if (size == 0) throw std::runtime_error("Cannot locate hostfxr under DOTNET_ROOT: " + dotnet_root.string());
    std::vector<char_t> path(size);
    int rc = get_hostfxr_path(path.data(), &size, &parameters);
    if (rc != 0) throw HostError("get_hostfxr_path", rc);
    #ifdef _WIN32
    void* library = LoadLibraryW(path.data());
    if (!library) throw std::runtime_error("Cannot load hostfxr");
#else
    void* library = dlopen(path.data(), RTLD_NOW | RTLD_LOCAL);
    if (!library) throw std::runtime_error(std::string("Cannot load hostfxr: ") + dlerror());
#endif
    // Keep hostfxr loaded for the process lifetime, including outstanding managed
    // delegates. hostfxr_close releases a host context, not the running CLR.
    const auto initialize = Symbol<hostfxr_initialize_for_runtime_config_fn>(library, "hostfxr_initialize_for_runtime_config");
    const auto get_delegate = Symbol<hostfxr_get_runtime_delegate_fn>(library, "hostfxr_get_runtime_delegate");
    const auto close = Symbol<hostfxr_close_fn>(library, "hostfxr_close");
    hostfxr_initialize_parameters init_parameters{sizeof(hostfxr_initialize_parameters), nullptr, root.c_str()};
    hostfxr_handle context = nullptr;
    rc = initialize(config.c_str(), &init_parameters, &context);
    if (rc < 0 || !context)
    {
        if (context) close(context);
        throw HostError("hostfxr_initialize_for_runtime_config", rc);
    }
    void* delegate = nullptr;
    rc = get_delegate(context, hdt_load_assembly_and_get_function_pointer, &delegate);
    close(context);
    if (rc < 0 || !delegate) throw HostError("hostfxr_get_runtime_delegate", rc);
    const auto load = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(delegate);
    auto method = [&](const char_t* name) {
        void* function = nullptr;
        int result = load(assembly.c_str(), HOST_TEXT("Game.NativeBridge.EntryPoints, Game.NativeBridge"), name,
                          UNMANAGEDCALLERSONLY_METHOD, nullptr, &function);
        if (result != 0 || !function) throw HostError("load_assembly_and_get_function_pointer", result);
        return function;
    };
    return {reinterpret_cast<RuntimeApi::Create>(method(HOST_TEXT("Create"))),
            reinterpret_cast<RuntimeApi::Exchange>(method(HOST_TEXT("Exchange"))),
            reinterpret_cast<RuntimeApi::Release>(method(HOST_TEXT("Release"))),
            reinterpret_cast<RuntimeApi::Destroy>(method(HOST_TEXT("Destroy")))};
}
}
