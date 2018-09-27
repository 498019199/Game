#include "../Core/Compenent/ICompenent.h"
#include "../Core/Compenent/CameraController.hpp"

extern void InitCompenetList(Context* pContext)
{
	FirstPersonCameraController::RegisterObject(pContext);
}

