#pragma once
#include <common/macro.h>
#include <common/Log.h>
#include <common/instance.h>
#include <common/defer.h>
#include <common/Util.h>
#include <common/ErrorHandling.h>
#include <common/ResIdentifier.h>
#include <common/Hash.h>
#include <common/StringUtil.h>
#include <common/com_ptr.h>

#include <vector>
#include <map>
#include <common/span.h>
#include <string>
#include <string_view>

#include <math/math.h>
namespace RenderWorker
{
    using ResIdentifier = CommonWorker::ResIdentifier;
    using ResIdentifierPtr = CommonWorker::ResIdentifierPtr;
}