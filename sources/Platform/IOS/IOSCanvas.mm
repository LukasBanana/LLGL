/*
 * IOSCanvas.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "IOSCanvas.h"
#include "IOSDisplay.h"
#include "../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>

#import <MetalKit/MetalKit.h>


// VIEW CONTROLLER

static LLGL::Extent2D GetScaledResolution(CGSize size, CGFloat screenScale = 1.0f)
{
    LLGL::Extent2D resolution;
    resolution.width    = static_cast<std::uint32_t>(size.width  * screenScale);
    resolution.height   = static_cast<std::uint32_t>(size.height * screenScale);
    return resolution;
}

static LLGL::Extent2D GetScaledResolutionByDisplayScale(CGSize size, const LLGL::Display* display)
{
    if (display != nullptr)
    {
        UIScreen* screen = static_cast<const LLGL::IOSDisplay*>(display)->GetNative();
        return GetScaledResolution(size, [screen nativeScale]);
    }
    return GetScaledResolution(size);
}

@interface IOSCanvasViewController : UIViewController
@end

@implementation IOSCanvasViewController
{
    LLGL::IOSCanvas* canvas_;
}

- (nonnull instancetype)initWithCanvas:(nonnull LLGL::IOSCanvas*)canvas;
{
    self = [super init];
    if (self)
        canvas_ = canvas;
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator;
{
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    const LLGL::Extent2D resolution = GetScaledResolutionByDisplayScale(size, canvas_->FindResidentDisplay());
    canvas_->PostResize(resolution);
}

@end


namespace LLGL
{


/*static NSString* ToNSString(const wchar_t* s)
{
    return [[NSString alloc]
        initWithBytes: s
        length: sizeof(*s)*wcslen(s)
        encoding:NSUTF32LittleEndianStringEncoding
    ];
}*/

std::unique_ptr<Canvas> Canvas::Create(const CanvasDescriptor& desc)
{
    return MakeUnique<IOSCanvas>(desc);
}

static IOSCanvasViewController* CreateIOSCanvasViewController(IOSCanvas* canvas)
{
    IOSCanvasViewController* viewCtrl = [[IOSCanvasViewController alloc] initWithCanvas:canvas];
    return viewCtrl;
}

static UIWindow* CreateIOSUIWindow(UIViewController* viewCtrl)
{
    CGRect mainScreenBounds = [[UIScreen mainScreen] bounds];

    UIWindow* wnd = [[UIWindow alloc] initWithFrame:mainScreenBounds];

    //UINavigationController* navCtrl = [[UINavigationController alloc] initWithRootViewController:viewCtrl];

    wnd.backgroundColor = [UIColor greenColor]; //TEST

    wnd.rootViewController = viewCtrl;
    [wnd makeKeyAndVisible];

    return wnd;
}

IOSCanvas::IOSCanvas(const CanvasDescriptor& desc) :
    desc_           { desc                                },
    viewController_ { CreateIOSCanvasViewController(this) },
    wnd_            { CreateIOSUIWindow(viewController_)  }
{
}

IOSCanvas::~IOSCanvas()
{
    [viewController_ release];
    [wnd_ release];
}

bool IOSCanvas::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(NativeHandle))
    {
        NativeHandle* handle = reinterpret_cast<NativeHandle*>(nativeHandle);
        handle->view = wnd_;
        return true;
    }
    return false;
}

Extent2D IOSCanvas::GetContentSize() const
{
    return GetScaledResolutionByDisplayScale(wnd_.bounds.size, FindResidentDisplay());
}

void IOSCanvas::SetTitle(const UTF8String& title)
{
    //todo...
}

UTF8String IOSCanvas::GetTitle() const
{
    return {}; //todo...
}


/*
 * ======= Private: =======
 */

void IOSCanvas::OnProcessEvents()
{
    //TODO...
}

void IOSCanvas::ResetPixelFormat()
{
    // dummy
}


} // /namespace LLGL



// ================================================================================
