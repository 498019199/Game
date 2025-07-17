#include <editor/EditorManagerD3D11.h>
#include <base/Context.h>

using namespace EditorWorker;

int main()
{
    RenderWorker::Context::Instance().LoadConfig("config.xml");
    EditorManagerD3D11 edtor;
    return 0;
}