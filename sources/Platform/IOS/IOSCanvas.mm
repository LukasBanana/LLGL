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
    LLGL::IOSCanvas*    canvas_;
    CGPoint             oldPanLocation_;
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

- (void)allocGestureRecognizers:(UIWindow*)wnd
{
    UITapGestureRecognizer* tapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTapGesture:)];
    [wnd addGestureRecognizer:tapGestureRecognizer];

    UIPanGestureRecognizer* panGestureRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(handlePanGesture:)];
    [wnd addGestureRecognizer:panGestureRecognizer];
}

static LLGL::Offset2D MapUIGestureLocation(UIGestureRecognizer* recognizer, UIView* view)
{
    CGPoint location = [recognizer locationInView:view];
    return LLGL::Offset2D{ static_cast<std::int32_t>(location.x), static_cast<std::int32_t>(location.y) };
}

- (void)handleTapGesture:(UITapGestureRecognizer*)recognizer
{
    UIView* view = canvas_->GetUIWindow();
    const std::uint32_t numTouches = static_cast<std::uint32_t>([recognizer numberOfTouches]);
    canvas_->PostTapGesture(MapUIGestureLocation(recognizer, view), numTouches);
}

- (void)handlePanGesture:(UIPanGestureRecognizer*)recognizer
{
    UIView* view = canvas_->GetUIWindow();

    const std::uint32_t numTouches = static_cast<std::uint32_t>([recognizer numberOfTouches]);
    CGPoint nativePosition = [recognizer locationInView:view];
    const LLGL::Offset2D position{ static_cast<std::int32_t>(nativePosition.x), static_cast<std::int32_t>(nativePosition.y) };

    switch ([recognizer state])
    {
        case UIGestureRecognizerStateBegan:
        {
            canvas_->PostPanGesture(position, numTouches, 0.0f, 0.0f, LLGL::EventAction::Began);
        }
        break;

        case UIGestureRecognizerStateChanged:
        {
            CGPoint velocity = CGPointMake(nativePosition.x - oldPanLocation_.x, nativePosition.y - oldPanLocation_.y);
            canvas_->PostPanGesture(position, numTouches, velocity.x, velocity.y, LLGL::EventAction::Changed);
        }
        break;

        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateEnded:
        {
            canvas_->PostPanGesture(position, numTouches, 0.0f, 0.0f, LLGL::EventAction::Ended);
        }
        break;

        default:
        {
            // don't forward states that are unrecognized by LLGL
        }
        break;
    }

    oldPanLocation_ = nativePosition;
}

@end


namespace LLGL
{


/*
 * Surface class
 */

bool Surface::ProcessEvents()
{
    return true; // dummy
}


/*
 * Canvas class
 */

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


/*
 * IOSCanvas class
 */

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

    wnd.rootViewController = viewCtrl;

    /* Always show UIWindow as there is no show/hide functionality for Canvas */
    [wnd makeKeyAndVisible];

    [(IOSCanvasViewController*)viewCtrl allocGestureRecognizers:wnd];

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


} // /namespace LLGL



// ================================================================================
