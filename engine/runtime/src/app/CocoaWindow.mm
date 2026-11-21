#import <AppKit/AppKit.h>
#include <base/Window.h>
#include <render/RenderEngine.h>
namespace RenderWorker
{
void RunCocoaLoop(Window &window, RenderEngine &renderer)
{
    while (!window.Closed())
    {
        @autoreleasepool
        {
            Window::PumpEvents();
            if (!window.Closed())
                renderer.Refresh();
        }
    }
}
void AttachDesktopChild(void *parent, void *child)
{
    [(NSWindow *)parent addChildWindow:(NSWindow *)child ordered:NSWindowAbove];
}
}
