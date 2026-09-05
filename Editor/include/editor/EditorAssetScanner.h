#pragma once

#include <filesystem>
#include <unordered_set>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

namespace EditorWorker
{
struct ScannedAsset
{
    std::filesystem::path path;
    bool directory;
    uintmax_t size;
};

// One instance per scan. Do not follow links, even when a queued directory has
// been replaced by a link since it was enumerated.
class EditorAssetScanner
{
public:
    std::vector<ScannedAsset> ReadDirectory(std::filesystem::path const& path)
    {
        namespace fs = std::filesystem;
        std::vector<ScannedAsset> entries;
        std::error_code ec;
        if (IsLinkOrUnavailable(path) || !fs::is_directory(path, ec) || ec)
            return entries;
        auto canonical = fs::canonical(path, ec);
        if (ec || !visited_.insert(canonical).second)
            return entries;

        fs::directory_iterator it(path, fs::directory_options::skip_permission_denied, ec), end;
        while (!ec && it != end)
        {
            auto const entry_path = it->path();
            if (!IsLinkOrUnavailable(entry_path))
            {
                // Query the path again instead of relying on cached entry metadata.
                auto const status = fs::symlink_status(entry_path, ec);
                if (!ec && fs::is_directory(status))
                    entries.push_back({entry_path, true, 0});
                else if (!ec && fs::is_regular_file(status))
                {
                    auto const size = fs::file_size(entry_path, ec);
                    if (!ec)
                        entries.push_back({entry_path, false, size});
                }
            }
            // A missing/unreadable entry does not prevent scanning its siblings.
            ec.clear();
            it.increment(ec);
        }
        return entries;
    }

private:
    static bool IsLinkOrUnavailable(std::filesystem::path const& path)
    {
        std::error_code ec;
        auto const status = std::filesystem::symlink_status(path, ec);
        if (ec || !std::filesystem::exists(status) || std::filesystem::is_symlink(status))
            return true;
#ifdef _WIN32
        // Windows junctions and other reparse points are not all symlinks.
        auto const attributes = GetFileAttributesW(path.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
            return true;
#endif
        return false;
    }

    std::unordered_set<std::filesystem::path> visited_;
};
}
