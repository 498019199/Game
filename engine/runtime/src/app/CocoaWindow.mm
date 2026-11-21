#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#include <base/Input.h>
#include <base/Window.h>
#include <render/RenderEngine.h>

namespace RenderWorker
{
namespace
{
int KeyCode(unsigned short code)
{
    switch (code)
    {
    case kVK_ANSI_A: return KS_A;
    case kVK_ANSI_B: return KS_B;
    case kVK_ANSI_C: return KS_C;
    case kVK_ANSI_D: return KS_D;
    case kVK_ANSI_E: return KS_E;
    case kVK_ANSI_F: return KS_F;
    case kVK_ANSI_G: return KS_G;
    case kVK_ANSI_H: return KS_H;
    case kVK_ANSI_I: return KS_I;
    case kVK_ANSI_J: return KS_J;
    case kVK_ANSI_K: return KS_K;
    case kVK_ANSI_L: return KS_L;
    case kVK_ANSI_M: return KS_M;
    case kVK_ANSI_N: return KS_N;
    case kVK_ANSI_O: return KS_O;
    case kVK_ANSI_P: return KS_P;
    case kVK_ANSI_Q: return KS_Q;
    case kVK_ANSI_R: return KS_R;
    case kVK_ANSI_S: return KS_S;
    case kVK_ANSI_T: return KS_T;
    case kVK_ANSI_U: return KS_U;
    case kVK_ANSI_V: return KS_V;
    case kVK_ANSI_W: return KS_W;
    case kVK_ANSI_X: return KS_X;
    case kVK_ANSI_Y: return KS_Y;
    case kVK_ANSI_Z: return KS_Z;
    case kVK_ANSI_0: return KS_0;
    case kVK_ANSI_1: return KS_1;
    case kVK_ANSI_2: return KS_2;
    case kVK_ANSI_3: return KS_3;
    case kVK_ANSI_4: return KS_4;
    case kVK_ANSI_5: return KS_5;
    case kVK_ANSI_6: return KS_6;
    case kVK_ANSI_7: return KS_7;
    case kVK_ANSI_8: return KS_8;
    case kVK_ANSI_9: return KS_9;
    case kVK_ANSI_Minus: return KS_Minus;
    case kVK_ANSI_Equal: return KS_Equals;
    case kVK_ANSI_LeftBracket: return KS_LeftBracket;
    case kVK_ANSI_RightBracket: return KS_RightBracket;
    case kVK_ANSI_Backslash: return KS_BackSlash;
    case kVK_ANSI_Semicolon: return KS_Semicolon;
    case kVK_ANSI_Quote: return KS_Apostrophe;
    case kVK_ANSI_Grave: return KS_Grave;
    case kVK_ANSI_Comma: return KS_Comma;
    case kVK_ANSI_Period: return KS_Period;
    case kVK_ANSI_Slash: return KS_Slash;
    case kVK_ISO_Section: return KS_OEM_102;
    case kVK_Escape: return KS_Escape;
    case kVK_Space: return KS_Space;
    case kVK_Return: return KS_Enter;
    case kVK_Tab: return KS_Tab;
    case kVK_Delete: return KS_BackSpace;
    case kVK_ForwardDelete: return KS_Delete;
    case kVK_Home: return KS_Home;
    case kVK_End: return KS_End;
    case kVK_PageUp: return KS_PageUp;
    case kVK_PageDown: return KS_PageDown;
    case kVK_LeftArrow: return KS_LeftArrow;
    case kVK_RightArrow: return KS_RightArrow;
    case kVK_UpArrow: return KS_UpArrow;
    case kVK_DownArrow: return KS_DownArrow;
    case kVK_Shift: return KS_LeftShift;
    case kVK_RightShift: return KS_RightShift;
    case kVK_Control: return KS_LeftCtrl;
    case kVK_RightControl: return KS_RightCtrl;
    case kVK_Option: return KS_LeftAlt;
    case kVK_RightOption: return KS_RightAlt;
    case kVK_Command: return KS_LeftWin;
    case kVK_RightCommand: return KS_RightWin;
    case kVK_CapsLock: return KS_CapsLock;
    case kVK_F1: return KS_F1;
    case kVK_F2: return KS_F2;
    case kVK_F3: return KS_F3;
    case kVK_F4: return KS_F4;
    case kVK_F5: return KS_F5;
    case kVK_F6: return KS_F6;
    case kVK_F7: return KS_F7;
    case kVK_F8: return KS_F8;
    case kVK_F9: return KS_F9;
    case kVK_F10: return KS_F10;
    case kVK_F11: return KS_F11;
    case kVK_F12: return KS_F12;
    case kVK_F13: return KS_F13;
    case kVK_F14: return KS_F14;
    case kVK_F15: return KS_F15;
    case kVK_ANSI_Keypad0: return KS_NumPad0;
    case kVK_ANSI_Keypad1: return KS_NumPad1;
    case kVK_ANSI_Keypad2: return KS_NumPad2;
    case kVK_ANSI_Keypad3: return KS_NumPad3;
    case kVK_ANSI_Keypad4: return KS_NumPad4;
    case kVK_ANSI_Keypad5: return KS_NumPad5;
    case kVK_ANSI_Keypad6: return KS_NumPad6;
    case kVK_ANSI_Keypad7: return KS_NumPad7;
    case kVK_ANSI_Keypad8: return KS_NumPad8;
    case kVK_ANSI_Keypad9: return KS_NumPad9;
    case kVK_ANSI_KeypadClear: return KS_NumLock;
    case kVK_ANSI_KeypadDecimal: return KS_NumPadPeriod;
    case kVK_ANSI_KeypadMultiply: return KS_NumPadStar;
    case kVK_ANSI_KeypadPlus: return KS_NumPadPlus;
    case kVK_ANSI_KeypadDivide: return KS_NumPadSlash;
    case kVK_ANSI_KeypadMinus: return KS_NumPadMinus;
    case kVK_ANSI_KeypadEnter: return KS_NumPadEnter;
    case kVK_ANSI_KeypadEquals: return KS_NumPadEquals;
    default: return 0;
    }
}

int Modifiers(NSEvent* event)
{
    auto const flags = event.modifierFlags;
    int mods = 0;
    if (flags & NSEventModifierFlagShift) mods |= WindowInput::Shift;
    if (flags & NSEventModifierFlagControl) mods |= WindowInput::Control;
    if (flags & NSEventModifierFlagOption) mods |= WindowInput::Alt;
    if (flags & NSEventModifierFlagCommand) mods |= WindowInput::Meta;
    if (flags & NSEventModifierFlagCapsLock) mods |= WindowInput::CapsLock;
    return mods;
}

int ModifierForKey(int key)
{
    switch (key)
    {
    case KS_LeftShift: case KS_RightShift: return WindowInput::Shift;
    case KS_LeftCtrl: case KS_RightCtrl: return WindowInput::Control;
    case KS_LeftAlt: case KS_RightAlt: return WindowInput::Alt;
    case KS_LeftWin: case KS_RightWin: return WindowInput::Meta;
    case KS_CapsLock: return WindowInput::CapsLock;
    default: return 0;
    }
}
}

struct CocoaWindowEvents
{
    static void Resize(Window& window, NSWindow* native)
    {
        auto const size = native.contentView.bounds.size;
        window.width_ = static_cast<uint32_t>(size.width);
        window.height_ = static_cast<uint32_t>(size.height);
        window.UpdateDpiScale(static_cast<float>(native.backingScaleFactor));
    }

    static void Move(Window& window, NSWindow* native)
    {
        auto const desktop = NSScreen.screens.firstObject.frame;
        window.left_ = static_cast<int>(NSMinX(native.frame));
        window.top_ = static_cast<int>(NSMaxY(desktop) - NSMaxY(native.frame));
    }

    static void Close(Window& window, NSWindow* native)
    {
        if (window.RequestClose())
            [native orderOut:nil];
    }

    static void Text(Window& window, NSString* value)
    {
        for (NSUInteger i = 0; i < value.length; ++i)
            window.DispatchText([value characterAtIndex:i]);
    }

    static void Send(Window& window, NSWindow* native, NSEvent* event)
    {
        int const mods = Modifiers(event);
        auto const type = event.type;
        if (type == NSEventTypeKeyDown || type == NSEventTypeKeyUp || type == NSEventTypeFlagsChanged)
        {
            int const key = KeyCode(event.keyCode);
            bool const released = type == NSEventTypeKeyUp || (type == NSEventTypeFlagsChanged
                && (!(mods & ModifierForKey(key)) || window.KeyDown(key)));
            bool const repeated = type == NSEventTypeKeyDown && event.isARepeat;
            window.DispatchInput({WindowInput::Key, key, released ? WindowInput::Released
                : repeated ? WindowInput::Repeated : WindowInput::Pressed, mods});
            return;
        }

        switch (type)
        {
        case NSEventTypeMouseMoved: case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged: case NSEventTypeOtherMouseDragged:
        case NSEventTypeLeftMouseDown: case NSEventTypeRightMouseDown: case NSEventTypeOtherMouseDown:
        case NSEventTypeLeftMouseUp: case NSEventTypeRightMouseUp: case NSEventTypeOtherMouseUp:
        case NSEventTypeScrollWheel:
            break;
        default:
            return;
        }
        NSView* view = native.contentView;
        NSPoint pos = [view convertPoint:event.locationInWindow fromView:nil];
        pos.y = NSMaxY(view.bounds) - pos.y;
        switch (type)
        {
        case NSEventTypeScrollWheel:
        {
            double const unit = event.hasPreciseScrollingDeltas ? 10.0 : 1.0;
            window.DispatchInput({WindowInput::Scroll, 0, 0, mods, 0, pos.x, pos.y,
                event.scrollingDeltaX / unit, event.scrollingDeltaY / unit});
            break;
        }
        case NSEventTypeLeftMouseDown: case NSEventTypeRightMouseDown: case NSEventTypeOtherMouseDown:
        case NSEventTypeLeftMouseUp: case NSEventTypeRightMouseUp: case NSEventTypeOtherMouseUp:
        {
            bool const up = type == NSEventTypeLeftMouseUp || type == NSEventTypeRightMouseUp || type == NSEventTypeOtherMouseUp;
            window.DispatchInput({WindowInput::Button, static_cast<int>(event.buttonNumber),
                up ? WindowInput::Released : WindowInput::Pressed, mods, 0, pos.x, pos.y});
            break;
        }
        default:
            window.DispatchInput({WindowInput::Motion, 0, 0, mods, 0, pos.x, pos.y});
            break;
        }
    }
};
}

@interface ZEngineContentView : NSView <NSTextInputClient>
@property(nonatomic, assign) RenderWorker::Window* owner;
@end

@implementation ZEngineContentView
{
    NSString* marked_;
    NSRange selection_;
}

- (BOOL)acceptsFirstResponder { return YES; }
- (void)keyDown:(NSEvent*)event { [self interpretKeyEvents:@[event]]; }
- (void)doCommandBySelector:(SEL)command { (void)command; }
- (void)insertText:(id)text replacementRange:(NSRange)range
{
    (void)range;
    [self unmarkText];
    NSString* value = [text isKindOfClass:NSAttributedString.class] ? [text string] : text;
    if (self.owner)
        RenderWorker::CocoaWindowEvents::Text(*self.owner, value);
}
- (void)setMarkedText:(id)text selectedRange:(NSRange)selection replacementRange:(NSRange)range
{
    (void)range;
    NSString* value = [text isKindOfClass:NSAttributedString.class] ? [text string] : text;
    marked_ = [value copy];
    selection_ = selection;
}
- (void)unmarkText { marked_ = nil; }
- (BOOL)hasMarkedText { return marked_.length > 0; }
- (NSRange)markedRange { return self.hasMarkedText ? NSMakeRange(0, marked_.length) : NSMakeRange(NSNotFound, 0); }
- (NSRange)selectedRange { return self.hasMarkedText ? selection_ : NSMakeRange(NSNotFound, 0); }
- (NSArray<NSAttributedStringKey>*)validAttributesForMarkedText { return @[]; }
- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actual
{
    if (actual) *actual = NSMakeRange(NSNotFound, 0);
    if (!self.hasMarkedText || range.location >= marked_.length)
        return nil;
    range.length = std::min(range.length, marked_.length - range.location);
    if (actual) *actual = range;
    return [[NSAttributedString alloc] initWithString:[marked_ substringWithRange:range]];
}
- (NSUInteger)characterIndexForPoint:(NSPoint)point { (void)point; return NSNotFound; }
- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actual
{
    if (actual) *actual = range;
    NSRect const rect = NSMakeRect(NSMinX(self.bounds), NSMaxY(self.bounds), 0, 0);
    return [self.window convertRectToScreen:[self convertRect:rect toView:nil]];
}
@end

@interface ZEngineCocoaWindow : NSWindow <NSWindowDelegate>
@property(nonatomic, assign) RenderWorker::Window* owner;
@end

@implementation ZEngineCocoaWindow
- (BOOL)canBecomeKeyWindow { return YES; }
- (BOOL)canBecomeMainWindow { return YES; }
- (void)sendEvent:(NSEvent*)event
{
    if (self.owner)
        RenderWorker::CocoaWindowEvents::Send(*self.owner, self, event);
    [super sendEvent:event];
}
- (BOOL)windowShouldClose:(NSWindow*)sender
{
    if (self.owner)
        RenderWorker::CocoaWindowEvents::Close(*self.owner, sender);
    return NO;
}
- (void)windowDidResize:(NSNotification*)notification
{
    if (self.owner)
        RenderWorker::CocoaWindowEvents::Resize(*self.owner, notification.object);
}
- (void)windowDidMove:(NSNotification*)notification
{
    if (self.owner)
        RenderWorker::CocoaWindowEvents::Move(*self.owner, notification.object);
}
- (void)windowDidChangeBackingProperties:(NSNotification*)notification
{
    [self windowDidResize:notification];
}
- (void)windowDidBecomeKey:(NSNotification*)notification
{
    (void)notification;
    if (self.owner) self.owner->Active(true);
}
- (void)windowDidResignKey:(NSNotification*)notification
{
    (void)notification;
    if (self.owner) self.owner->Active(false);
}
@end

namespace RenderWorker
{
void Window::InitializeDesktop()
{
    static bool const initialized = [] {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp finishLaunching];
        return true;
    }();
    (void)initialized;
}

Window::Window(std::string const& name, RenderSettings const& settings, void* native_wnd)
    : left_(settings.left), top_(settings.top), width_(settings.width), height_(settings.height),
      dpi_scale_(1), effective_dpi_scale_(1), win_rotation_(WR_Identity), active_(false), ready_(false),
      closed_(false), keep_screen_on_(settings.keep_screen_on)
{
    InitializeDesktop();
    NSScreen* screen = NSScreen.screens.firstObject;
    NSRect const desktop = screen.frame;
    NSRect const rect = settings.full_screen && screen ? desktop : NSMakeRect(settings.left,
        NSMaxY(desktop) - settings.top - settings.height, settings.width, settings.height);
    auto const style = settings.full_screen ? NSWindowStyleMaskBorderless
        : NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
    ZEngineCocoaWindow* native = [[ZEngineCocoaWindow alloc] initWithContentRect:rect styleMask:style
        backing:NSBackingStoreBuffered defer:NO];
    if (!native)
        TMSG("Unable to create the Cocoa window");

    native.owner = this;
    native.delegate = native;
    native.releasedWhenClosed = NO;
    native.acceptsMouseMovedEvents = YES;
    native.title = [NSString stringWithUTF8String:name.c_str()] ?: @"ZEngine";
    ZEngineContentView* view = [[ZEngineContentView alloc] initWithFrame:NSMakeRect(0, 0, rect.size.width, rect.size.height)];
    view.owner = this;
    native.contentView = view;
    [native makeFirstResponder:view];
    if (native_wnd)
        [(__bridge NSWindow*)native_wnd addChildWindow:native ordered:NSWindowAbove];

    window_ = (__bridge_retained void*)native;
    CocoaWindowEvents::Resize(*this, native);
    CocoaWindowEvents::Move(*this, native);
    ready_ = true;
    if (!settings.hide_win)
        SetVisible(true);
}

Window::~Window()
{
    ZEngineCocoaWindow* native = (__bridge_transfer ZEngineCocoaWindow*)window_;
    native.owner = nullptr;
    native.delegate = nil;
    ZEngineContentView* view = (ZEngineContentView*)native.contentView;
    view.owner = nullptr;
    [native.parentWindow removeChildWindow:native];
    [native close];
}

void* Window::NativeHandle() const { return window_; }

void Window::PumpEvents()
{
    @autoreleasepool
    {
        while (NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:NSDate.distantPast
            inMode:NSDefaultRunLoopMode dequeue:YES])
            [NSApp sendEvent:event];
        [NSApp updateWindows];
    }
}

int2 Window::PrimaryDisplaySize(bool fullscreen)
{
    InitializeDesktop();
    NSScreen* screen = NSScreen.screens.firstObject;
    auto const size = (fullscreen ? screen.frame : screen.visibleFrame).size;
    return int2(static_cast<int>(size.width), static_cast<int>(size.height));
}

void Window::SetVisible(bool visible)
{
    NSWindow* native = (__bridge NSWindow*)window_;
    if (visible)
    {
        [native makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
    }
    else
        [native orderOut:nil];
}

int2 Window::FramebufferSize() const
{
    NSView* view = ((__bridge NSWindow*)window_).contentView;
    NSRect const rect = [view convertRectToBacking:view.bounds];
    return int2(static_cast<int>(rect.size.width), static_cast<int>(rect.size.height));
}

bool Window::Minimized() const { return ((__bridge NSWindow*)window_).miniaturized; }
bool Window::IsMaximized() const { return ((__bridge NSWindow*)window_).zoomed; }
void Window::Minimize() { [(__bridge NSWindow*)window_ miniaturize:nil]; }
void Window::Maximize()
{
    if (!IsMaximized()) [(__bridge NSWindow*)window_ zoom:nil];
}
void Window::Restore()
{
    auto* native = (__bridge NSWindow*)window_;
    if (Minimized()) [native deminiaturize:nil];
    else if (IsMaximized()) [native zoom:nil];
}
void Window::SetContentSize(int width, int height)
{
    [(__bridge NSWindow*)window_ setContentSize:NSMakeSize(width, height)];
}

void RunCocoaLoop(Window& window, RenderEngine& renderer)
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
}
