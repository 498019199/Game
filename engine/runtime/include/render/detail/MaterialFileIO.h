#pragma once

#include <common/ErrorHandling.h>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string_view>
#if defined(_WIN32)
#include <windows.h>
#endif

namespace RenderWorker::Detail
{
enum class MaterialSaveStage { Open, Write, Close, Replace };

// Internal transaction boundary. The per-call hook enables deterministic fault
// tests without global state or replacing the real filesystem implementation.
inline void WriteMaterialFileAtomically(std::filesystem::path const& requested, std::string_view bytes,
    std::function<void(MaterialSaveStage)> const& checkpoint = {}, bool replace_existing = true)
{
    namespace fs = std::filesystem;
    auto const target = fs::absolute(requested);
    auto const parent = target.parent_path();
    if (!fs::is_directory(parent) || fs::is_directory(target) || fs::is_symlink(target))
        TMSG("Invalid material save target");

    static std::atomic<uint64_t> sequence {0};
    fs::path staging;
    for (unsigned attempt = 0; attempt != 64; ++attempt)
    {
        auto candidate = parent / (".material-save-" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-" +
            std::to_string(sequence.fetch_add(1)));
        // Atomic reservation: never reuse/truncate another writer's temporary file.
        if (fs::create_directory(candidate)) { staging = std::move(candidate); break; }
    }
    if (staging.empty()) TMSG("Cannot reserve material save staging directory");
    struct Cleanup
    {
        fs::path file, directory;
        ~Cleanup()
        {
            std::error_code ec;
            fs::remove(file, ec);
            fs::remove(directory, ec);
        }
    } cleanup {staging / "material.tmp", staging};

    auto check = [&](MaterialSaveStage stage) { if (checkpoint) checkpoint(stage); };
    check(MaterialSaveStage::Open);
    std::ofstream out(cleanup.file, std::ios::binary | std::ios::trunc);
    if (!out) TMSG("Cannot open temporary material file");
    auto const halfway = bytes.size() / 2;
    out.write(bytes.data(), static_cast<std::streamsize>(halfway));
    check(MaterialSaveStage::Write);
    out.write(bytes.data() + halfway, static_cast<std::streamsize>(bytes.size() - halfway));
    out.flush();
    if (!out) TMSG("Cannot write temporary material file");
    check(MaterialSaveStage::Close);
    out.close();
    if (!out) TMSG("Cannot close temporary material file");
    check(MaterialSaveStage::Replace);
#if defined(_WIN32)
    if (!::MoveFileExW(cleanup.file.c_str(), target.c_str(), (replace_existing ? MOVEFILE_REPLACE_EXISTING : 0) | MOVEFILE_WRITE_THROUGH))
        TMSG("Cannot replace material file (Windows error " + std::to_string(::GetLastError()) + ")");
#else
    std::error_code ec;
    if (replace_existing) fs::rename(cleanup.file, target, ec);
    else fs::create_hard_link(cleanup.file, target, ec); // Atomic no-clobber commit; cleanup unlinks the temporary name.
    if (ec) TMSG("Cannot replace material file: " + ec.message());
#endif
}
}
