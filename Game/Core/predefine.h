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
class CameraController;
typedef std::shared_ptr<CameraController> CameraControllerPtr;
class FirstPersonCameraController;
typedef std::shared_ptr<FirstPersonCameraController> FirstPersonCameraControllerPtr;
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


class InputEngine;
class InputDevice;
typedef std::shared_ptr<InputDevice> InputDevicePtr;
class InputKeyboard;
typedef std::shared_ptr<InputKeyboard> InputKeyboardPtr;
class InputMouse;
typedef std::shared_ptr<InputMouse> InputMousePtr;
class InputJoystick;
typedef std::shared_ptr<InputJoystick> InputJoystickPtr;
class InputTouch;
typedef std::shared_ptr<InputTouch> InputTouchPtr;
class InputSensor;
typedef std::shared_ptr<InputSensor> InputSensorPtr;
class InputFactory;
struct InputActionParam;
typedef std::shared_ptr<InputActionParam> InputActionParamPtr;
struct InputKeyboardActionParam;
typedef std::shared_ptr<InputKeyboardActionParam> InputKeyboardActionParamPtr;
struct InputMouseActionParam;
typedef std::shared_ptr<InputMouseActionParam> InputMouseActionParamPtr;
struct InputJoystickActionParam;
typedef std::shared_ptr<InputJoystickActionParam> InputJoystickActionParamPtr;
struct InputTouchActionParam;
typedef std::shared_ptr<InputTouchActionParam> InputTouchActionParamPtr;
struct InputSensorActionParam;
typedef std::shared_ptr<InputSensorActionParam> InputSensorActionParamPtr;
#endif //_PREDEFINE_H