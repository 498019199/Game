#include <editor/EditorMaterialService.h>
#include <common/ErrorHandling.h>
#include <algorithm>
#include <cmath>
#include <cctype>

namespace EditorWorker
{
using namespace RenderWorker;
namespace fs = std::filesystem;
std::string MaterialUtf8(fs::path const& path)
{
    auto s = path.generic_u8string();
    return {reinterpret_cast<char const*>(s.data()), s.size()};
}
fs::path MaterialPath(std::string_view s)
{
    return fs::path(std::u8string(reinterpret_cast<char8_t const*>(s.data()), s.size()));
}
namespace
{
std::string Lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
bool Same(MaterialParameters const& a, MaterialParameters const& b)
{
    if (a.name != b.name || a.metalness != b.metalness || a.glossiness != b.glossiness ||
        a.normal_scale != b.normal_scale || a.occlusion_strength != b.occlusion_strength) return false;
    for (int i = 0; i < 4; ++i) if (a.albedo[i] != b.albedo[i]) return false;
    for (int i = 0; i < 3; ++i) if (a.emissive[i] != b.emissive[i]) return false;
    return true;
}
MaterialParameters Read(RenderMaterial const& m)
{
    return {m.Name(), m.Albedo(), m.Emissive(), m.Metalness(), m.Glossiness(), m.NormalScale(), m.OcclusionStrength()};
}
void Range(float v, float lo, float hi)
{
    if (!std::isfinite(v) || v < lo || v > hi) TMSG("Material parameter is non-finite or outside its supported range.");
}
void CheckMaterialPath(fs::path const& p)
{
    if (Lower(MaterialUtf8(p.extension())) != ".mat") TMSG("Only ordinary .mat materials can be edited.");
}
}
MaterialParameters EditorMaterialDocument::Parameters() const { return Read(*working_); }
bool EditorMaterialDocument::Dirty() const
{
    if (!Same(Read(*working_), Read(*saved_))) return true;
    for (int i = 0; i < RenderMaterial::TS_NumTextureSlots; ++i)
        if (working_->TextureName(static_cast<RenderMaterial::TextureSlot>(i)) != saved_->TextureName(static_cast<RenderMaterial::TextureSlot>(i))) return true;
    return false;
}
void EditorMaterialDocument::Edit(MaterialParameters const& p)
{
    if (p.name.find_first_of("\r\n\t") != std::string::npos || p.name.find('\0') != std::string::npos)
        TMSG("Material name must not contain control characters.");
    for (int i = 0; i < 4; ++i) Range(p.albedo[i], 0, 1);
    for (int i = 0; i < 3; ++i) Range(p.emissive[i], 0, 64);
    Range(p.metalness, 0, 1); Range(p.glossiness, 0, 1);
    Range(p.normal_scale, 0, 4); Range(p.occlusion_strength, 0, 1);
    if (Same(Parameters(), p)) return;
    working_->Name(p.name); working_->Albedo(p.albedo); working_->Emissive(p.emissive);
    working_->Metalness(p.metalness); working_->Glossiness(p.glossiness);
    working_->NormalScale(p.normal_scale); working_->OcclusionStrength(p.occlusion_strength);
    ++version_;
}
void EditorMaterialDocument::ResetParameters()
{
    MaterialParameters defaults;
    defaults.name = working_->Name();
    Edit(defaults); // Texture references and unexposed advanced fields deliberately remain intact.
}
void EditorMaterialDocument::Revert()
{
    if (!Dirty()) return;
    working_ = saved_->Clone();
    ++version_;
}
EditorMaterialService::EditorMaterialService(fs::path assets, TextureLoader loader)
    : assets_(fs::canonical(assets)), texture_loader_(std::move(loader)) {}
fs::path EditorMaterialService::Resolve(std::string_view input) const
{
    if (input.empty() || input.find('\0') != std::string_view::npos) TMSG("An Assets-relative path is required.");
    auto p = MaterialPath(input);
    if (p.is_absolute() || p.has_root_name() || input.find(':') != std::string_view::npos || input.find('\\') != std::string_view::npos)
        TMSG("Use an Assets-relative path with forward slashes.");
    for (auto const& part : p)
    {
        auto s = MaterialUtf8(part);
        if (s == ".." || s.find_first_of("<>\"|?*\r\n\t") != std::string::npos ||
            (s != "." && !s.empty() && (s.back() == '.' || s.back() == ' '))) TMSG("Invalid or escaping asset path.");
    }
    auto result = fs::weakly_canonical(assets_ / p);
    auto relative = result.lexically_relative(assets_);
    if (relative.empty() || relative == "." || *relative.begin() == "..") TMSG("Asset path escapes Assets or names its root.");
    return result;
}
std::string EditorMaterialService::Identity(std::string_view path) const
{
    auto id = MaterialUtf8(Resolve(path).lexically_relative(assets_));
#ifdef _WIN32
    id = Lower(id);
#endif
    return id;
}
bool EditorMaterialService::IsTexture(fs::path const& p)
{
    auto e = Lower(MaterialUtf8(p.extension()));
    return e == ".png" || e == ".jpg" || e == ".jpeg" || e == ".tga" || e == ".dds";
}
void EditorMaterialService::Prepare(RenderMaterial& m) const
{
    for (int i = 0; i < RenderMaterial::TS_NumTextureSlots; ++i)
    {
        auto slot = static_cast<RenderMaterial::TextureSlot>(i);
        auto const name = m.TextureName(slot);
        if (name.empty()) continue;
        auto path = Resolve(name);
        if (!IsTexture(path) || !fs::is_regular_file(path)) TMSG("Missing or unsupported texture: " + name);
        m.TextureName(slot, MaterialUtf8(path.lexically_relative(assets_)));
        if (texture_loader_)
        {
            auto srv = texture_loader_(path);
            if (!srv) TMSG("Texture loader returned no image: " + name);
            m.Texture(slot, std::move(srv));
        }
    }
}
MaterialDocumentPtr EditorMaterialService::Register(std::string const& id, RenderMaterialPtr m)
{
    auto d = std::make_shared<EditorMaterialDocument>();
    d->path_ = id; d->working_ = std::move(m); d->saved_ = d->working_->Clone();
    documents_.emplace(id, d);
    return d;
}
MaterialDocumentPtr EditorMaterialService::Open(std::string_view path)
{
    CheckMaterialPath(MaterialPath(path));
    auto id = Identity(path);
    if (auto it = documents_.find(id); it != documents_.end()) return it->second;
    auto m = LoadRenderMaterialFile(Resolve(path), false);
    Prepare(*m);
    return Register(id, std::move(m));
}
MaterialDocumentPtr EditorMaterialService::Create(std::string_view path)
{
    CheckMaterialPath(MaterialPath(path));
    auto id = Identity(path);
    auto target = Resolve(path);
    if (fs::exists(target) || documents_.contains(id)) TMSG("Material already exists; choose a new filename.");
    auto m = std::make_shared<RenderMaterial>();
    m->Name(MaterialUtf8(target.stem())); m->Template("EditorLitV1");
    m->Albedo(float4(0.8f, 0.8f, 0.8f, 1)); m->Metalness(0); m->Glossiness(0.5f);
    m->Emissive(float3(0, 0, 0)); m->NormalScale(1); m->OcclusionStrength(1);
    SaveRenderMaterial(m, MaterialUtf8(target), false);
    return Register(id, std::move(m));
}
MaterialDocumentPtr EditorMaterialService::CreateInFolder(std::string_view folder, std::string_view filename)
{
    if (filename.empty() || filename.find_first_of("/\\") != std::string_view::npos)
        TMSG("Enter a filename, not a directory path.");
    return Create(MaterialUtf8(MaterialPath(folder) / MaterialPath(filename)));
}
void EditorMaterialService::Save(EditorMaterialDocument& d)
{
    auto snapshot = d.working_->Clone();
    SaveRenderMaterial(d.working_, MaterialUtf8(Resolve(d.path_)));
    d.saved_ = std::move(snapshot);
}
MaterialDocumentPtr EditorMaterialService::SaveAs(EditorMaterialDocument const& d, std::string_view path)
{
    CheckMaterialPath(MaterialPath(path));
    auto id = Identity(path);
    auto target = Resolve(path);
    if (fs::exists(target) || documents_.contains(id)) TMSG("Save As never overwrites an existing asset; choose a new filename.");
    auto m = d.working_->Clone();
    SaveRenderMaterial(m, MaterialUtf8(target), false);
    return Register(id, std::move(m));
}
void EditorMaterialService::Reload(EditorMaterialDocument& d)
{
    auto m = LoadRenderMaterialFile(Resolve(d.path_), false);
    Prepare(*m);
    auto snapshot = m->Clone();
    d.working_ = std::move(m); d.saved_ = std::move(snapshot); ++d.version_;
}
void EditorMaterialService::SetTexture(EditorMaterialDocument& d, RenderMaterial::TextureSlot slot, std::string_view input)
{
    if (slot < 0 || slot >= RenderMaterial::TS_NumTextureSlots || slot == RenderMaterial::TS_Height) TMSG("Unsupported editable texture slot.");
    std::string name;
    ShaderResourceViewPtr srv;
    if (!input.empty())
    {
        auto path = Resolve(input);
        if (!IsTexture(path) || !fs::is_regular_file(path)) TMSG("Select an existing png/jpg/jpeg/tga/dds texture inside Assets.");
        name = MaterialUtf8(path.lexically_relative(assets_));
        if (name == d.working_->TextureName(slot)) return;
        if (texture_loader_)
        {
            srv = texture_loader_(path);
            if (!srv) TMSG("Texture could not be decoded; previous texture was kept.");
        }
    }
    else if (d.working_->TextureName(slot).empty()) return;
    d.working_->Texture(slot, std::move(srv));
    d.working_->TextureName(slot, name);
    ++d.version_;
}
bool EditorMaterialService::Close(MaterialDocumentPtr const& d, MaterialCloseChoice choice)
{
    if (choice == MaterialCloseChoice::Cancel) return false;
    if (d->Dirty() && choice == MaterialCloseChoice::Save) Save(*d);
    if (choice == MaterialCloseChoice::Discard) d->Revert();
    documents_.erase(d->Path());
    return true;
}
bool EditorMaterialService::HasDirty() const
{
    for (auto const& [path, d] : documents_) if (d->Dirty()) return true;
    return false;
}
bool EditorMaterialService::ResolveExit(MaterialCloseChoice choice)
{
    if (choice == MaterialCloseChoice::Cancel) return false;
    // Save all first; if any save fails do not discard any remaining edits or exit.
    if (choice == MaterialCloseChoice::Save)
        for (auto const& [path, d] : documents_) if (d->Dirty()) Save(*d);
    if (choice == MaterialCloseChoice::Discard)
        for (auto const& [path, d] : documents_) d->Revert();
    return true;
}
}
