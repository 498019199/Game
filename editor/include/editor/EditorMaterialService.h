#pragma once
#include <render/RenderMaterial.h>
#include <filesystem>
#include <functional>
#include <map>

namespace EditorWorker
{
std::string MaterialUtf8(std::filesystem::path const& path);
std::filesystem::path MaterialPath(std::string_view path);

struct MaterialParameters
{
    std::string name;
    RenderWorker::float4 albedo {0.8f, 0.8f, 0.8f, 1};
    RenderWorker::float3 emissive {0, 0, 0};
    float metalness = 0, glossiness = 0.5f, normal_scale = 1, occlusion_strength = 1;
};

class EditorMaterialDocument
{
public:
    std::string const& Path() const { return path_; }
    RenderWorker::RenderMaterial const& Material() const { return *working_; }
    MaterialParameters Parameters() const;
    bool Dirty() const;
    uint64_t Version() const { return version_; }
    void Edit(MaterialParameters const& values);
    void ResetParameters();
    void Revert();
private:
    friend class EditorMaterialService;
    std::string path_;
    RenderWorker::RenderMaterialPtr working_, saved_;
    uint64_t version_ = 0;
};
using MaterialDocumentPtr = std::shared_ptr<EditorMaterialDocument>;
enum class MaterialCloseChoice { Save, Discard, Cancel };

class EditorMaterialService
{
public:
    using TextureLoader = std::function<RenderWorker::ShaderResourceViewPtr(std::filesystem::path const&)>;
    explicit EditorMaterialService(std::filesystem::path assets, TextureLoader loader = {});
    std::filesystem::path Resolve(std::string_view path) const;
    std::string Identity(std::string_view path) const;
    MaterialDocumentPtr Open(std::string_view path);
    MaterialDocumentPtr Create(std::string_view path);
    MaterialDocumentPtr CreateInFolder(std::string_view folder, std::string_view filename);
    void Save(EditorMaterialDocument& doc);
    MaterialDocumentPtr SaveAs(EditorMaterialDocument const& doc, std::string_view path);
    void Reload(EditorMaterialDocument& doc); // Explicit discard of edits, fresh disk read.
    void SetTexture(EditorMaterialDocument& doc, RenderWorker::RenderMaterial::TextureSlot slot, std::string_view path);
    bool Close(MaterialDocumentPtr const& doc, MaterialCloseChoice choice);
    bool HasDirty() const;
    bool ResolveExit(MaterialCloseChoice choice);
    auto const& Documents() const { return documents_; }
    static bool IsTexture(std::filesystem::path const& path);
private:
    void Prepare(RenderWorker::RenderMaterial& material) const;
    MaterialDocumentPtr Register(std::string const& id, RenderWorker::RenderMaterialPtr material);
    std::filesystem::path assets_;
    TextureLoader texture_loader_;
    std::map<std::string, MaterialDocumentPtr> documents_;
};
}
