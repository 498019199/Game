#ifndef _PREDEFINE_H
#define _PREDEFINE_H

#include <memory>
class String;
class WString;

class Context;
typedef std::shared_ptr<Context> ContextPtr;
class IEntity;
typedef std::shared_ptr<IEntity> IEntityPtr;
class EnitityFactory;
typedef std::shared_ptr<EnitityFactory> EnitityFactoryPtr;

class DataBuffer;
typedef std::shared_ptr<DataBuffer> DataBufferPtr;
class RenderConstantBuffer;
class IVarList;
class RenderVariable;
typedef std::unique_ptr<RenderVariable> RenderVariableUniPtr;
class RenderCVarlist;
typedef std::shared_ptr<RenderCVarlist> RenderCVarlistPtr;
class RenderCVarTemplate;
typedef std::shared_ptr<RenderCVarTemplate> RenderCVarTemplatePtr;
class RenderCVarParameter;
class ITexture;
typedef std::shared_ptr<ITexture> TexturePtr;

class DxGraphDevice;

class RenderMaterial;
typedef std::shared_ptr<RenderMaterial> RenderMaterialPtr;
class Renderable;
typedef std::shared_ptr<Renderable> VisBasePtr;
class IActor;
typedef std::shared_ptr<IActor> ActorPtr;
class ILight;
typedef std::shared_ptr<ILight> LightPtr;
class ICamera;
typedef std::shared_ptr<ICamera> CameraPtr;
class SceneManager;
typedef std::shared_ptr<SceneManager> ScenePtr;
class ResLoadingDesc;
typedef std::shared_ptr<ResLoadingDesc> ResLoadingDescPtr;
class ResIdentifier;
typedef std::shared_ptr<ResIdentifier> ResIdentifierPtr;
class ResLoader;
typedef std::shared_ptr<ResLoader> ResLoaderPtr;
class Package;
typedef std::shared_ptr<Package> PackagePtr;

class bad_join;
template <typename ResultType>
class joiner;
class threader;
class thread_pool;

class XMLDocument;
typedef std::shared_ptr<XMLDocument> XMLDocumentPtr;
class XMLNode;
typedef std::shared_ptr<XMLNode> XMLNodePtr;
class XMLAttribute;
typedef std::shared_ptr<XMLAttribute> XMLAttributePtr;

class RenderLayout;
typedef std::shared_ptr<RenderLayout> RenderLayoutPtr;
class RenderModel;
typedef std::shared_ptr<RenderModel> RenderModelPtr;
class StaticMesh;
typedef std::shared_ptr<StaticMesh> StaticMeshPtr;

class SceneObject;
typedef std::shared_ptr<SceneObject> SceneObjectPtr;

class Window;
typedef std::shared_ptr<Window> WindowPtr;
struct  WindowDesc;
#endif //_PREDEFINE_H