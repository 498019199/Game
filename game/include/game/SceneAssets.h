#pragma once
#include <common/JsonDom.h>
#include <filesystem>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace SceneAssets
{
using CommonWorker::JsonValue;
using CommonWorker::JsonValueType;
struct MaterialReference { uint32_t model, slot; std::string asset; };
inline uint32_t Index(JsonValue const* value)
{
    if (!value) throw std::runtime_error("Scene material reference is missing an index");
    if (value->Type() == JsonValueType::UInt) return value->ValueUInt();
    if (value->Type() == JsonValueType::Int && value->ValueInt() >= 0) return static_cast<uint32_t>(value->ValueInt());
    throw std::runtime_error("Scene material index must be a non-negative integer");
}
inline std::filesystem::path Resolve(std::filesystem::path const& assets, std::string const& name)
{
    namespace fs = std::filesystem;
    fs::path path(std::u8string(reinterpret_cast<char8_t const*>(name.data()), name.size()));
    if (name.empty() || name.find('\0') != std::string::npos || name.find(':') != std::string::npos ||
        name.find('\\') != std::string::npos || path.is_absolute() || path.extension() != ".mat")
        throw std::runtime_error("Expected an Assets-relative .mat reference");
    for (auto const& part : path) if (part == "..") throw std::runtime_error("Material reference escapes Assets");
    auto root = fs::canonical(assets), resolved = fs::weakly_canonical(root / path);
    auto relative = resolved.lexically_relative(root);
    if (relative.empty() || *relative.begin() == ".." || !fs::is_regular_file(resolved))
        throw std::runtime_error("Missing or escaping material: " + name);
    return resolved;
}
inline std::vector<MaterialReference> Read(JsonValue const& entry)
{
    std::vector<MaterialReference> result;
    if (entry.Type() != JsonValueType::Object) return result;
    auto refs = entry.Member("MaterialOverrides");
    if (!refs) return result;
    if (refs->Type() != JsonValueType::Array) throw std::runtime_error("MaterialOverrides must be an array");
    std::set<std::pair<uint32_t, uint32_t>> used;
    for (auto const& item : refs->ValueArray()) {
        if (item.Type() != JsonValueType::Object) throw std::runtime_error("Invalid material override");
        auto asset = item.Member("Asset");
        MaterialReference ref {Index(item.Member("Model")), Index(item.Member("Slot")), {}};
        if (!asset || asset->Type() != JsonValueType::String || asset->ValueString().empty() || !used.emplace(ref.model, ref.slot).second)
            throw std::runtime_error("Invalid or duplicate scene material reference");
        ref.asset = asset->ValueString(); result.push_back(std::move(ref));
    }
    return result;
}
inline void Set(JsonValue& entry, uint32_t model, uint32_t slot, std::string const& asset)
{
    if (entry.Type() == JsonValueType::String) {
        std::string prefab(entry.ValueString()); entry = JsonValue(JsonValueType::Object);
        entry.AppendValue("Prefab", JsonValue(std::string_view(prefab)));
    }
    if (entry.Type() != JsonValueType::Object) throw std::runtime_error("Invalid scene object");
    if (!entry.Member("MaterialOverrides")) entry.AppendValue("MaterialOverrides", JsonValue(JsonValueType::Array));
    auto& refs = *entry.Member("MaterialOverrides");
    (void)Read(entry); // Reject malformed data rather than silently replacing it.
    for (size_t i = 0; i < refs.ValueArray().size(); ++i) {
        JsonValue item(refs.ValueArray()[i]);
        if (Index(item.Member("Model")) == model && Index(item.Member("Slot")) == slot) {
            item.Member("Asset")->Value(std::string_view(asset)); refs.ValueIndex(static_cast<uint32_t>(i), std::move(item)); return;
        }
    }
    JsonValue item(JsonValueType::Object);
    item.AppendValue("Model", JsonValue(model)); item.AppendValue("Slot", JsonValue(slot));
    item.AppendValue("Asset", JsonValue(std::string_view(asset))); refs.AppendValue(std::move(item));
}
}
