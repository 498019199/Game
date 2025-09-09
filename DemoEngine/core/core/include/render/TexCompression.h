


#include <render/Texture.h>

namespace RenderWorker
{

uint32_t BlockWidth(ElementFormat format);
uint32_t BlockHeight(ElementFormat format);
uint32_t BlockDepth(ElementFormat format);
uint32_t BlockBytes(ElementFormat format);
ElementFormat DecodedFormat(ElementFormat format);
}