/*
 * MacOSAppDelegate.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MacOSAppDelegate.h"

#import <Cocoa/Cocoa.h>


@interface MacOSAppDelegate : NSObject<NSApplicationDelegate>
@end

@implementation MacOSAppDelegate
@end


namespace LLGL
{


static bool g_appDelegateCreated = false;

void LoadNSAppDelegate()
{
    if (!g_appDelegateCreated)
    {
        /* Initialize Cocoa framework */
        [[NSAutoreleasePool alloc] init];
        [NSApplication sharedApplication];

        [NSApp setDelegate:(id<NSApplicationDelegate>)[
            [MacOSAppDelegate alloc]
            autorelease
        ]];

        [NSApp finishLaunching];

        g_appDelegateCreated = true;
    }
}


} // /namespace LLGL



// ================================================================================
