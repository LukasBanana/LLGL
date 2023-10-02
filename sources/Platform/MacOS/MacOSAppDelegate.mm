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

static NSString* FindMacOSAppName()
{
    /* Search NSBundle for key entries to determine application name */
    NSDictionary* bundleInfo = [[NSBundle mainBundle] infoDictionary];
    NSString* bundleNameKeys[] = { @"CFBundleDisplayName", @"CFBundleName", @"CFBundleExecutable" };

    for (NSString* key : bundleNameKeys)
    {
        id name = bundleInfo[key];
        if (name != nil && [name isKindOfClass:[NSString class]])
        {
            NSString* nameStr = (NSString*)name;
            if (![nameStr isEqualToString:@""])
                return nameStr;
        }
    }

    /* Fallback to default name */
    return @"LLGL";
}

static void CreateDefaultNSMenuBar()
{
    NSString* appName = FindMacOSAppName();

    NSMenu* menu = [[NSMenu alloc] init];
    [NSApp setMainMenu:menu];

    NSMenuItem* appMenuItem = [menu addItemWithTitle:@"" action:nil keyEquivalent:@""];
    NSMenu* appMenu = [[NSMenu alloc] init];
    [appMenuItem setSubmenu:appMenu];

    [appMenu
        addItemWithTitle:   [NSString stringWithFormat:@"About %@", appName]
        action:             @selector(orderFrontStandardAboutPanel:)
        keyEquivalent:      @""
    ];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu
        addItemWithTitle:   [NSString stringWithFormat:@"Hide %@", appName]
        action:             @selector(hide:)
        keyEquivalent:      @"h"
    ];
    NSMenuItem* appMenuItemHideOthers = [appMenu
        addItemWithTitle:   @"Hide Others"
        action:             @selector(hideOtherApplications:)
        keyEquivalent:      @"h"
    ];
    [appMenuItemHideOthers setKeyEquivalentModifierMask:NSEventModifierFlagOption | NSEventModifierFlagCommand];
    [appMenu
        addItemWithTitle:   @"Show All"
        action:             @selector(unhideAllApplications:)
        keyEquivalent:      @""
    ];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu
        addItemWithTitle:   [NSString stringWithFormat:@"Quit %@", appName]
        action:             @selector(terminate:)
        keyEquivalent:      @"q"
    ];
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
    CreateDefaultNSMenuBar();
}

@end


namespace LLGL
{


static bool g_appDelegateCreated = false;

static void AllocDefaultNSAppDelegate()
{
    /* Initialize Cocoa framework */
    [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];

    [NSApp setDelegate:(id<NSApplicationDelegate>)[
        [MacOSAppDelegate alloc]
        autorelease
    ]];

    [NSApp finishLaunching];
}

void LoadNSAppDelegate()
{
    if (!g_appDelegateCreated)
    {
        /* Create default NSAppDelegate with standard window menu if no shared application is define by the client */
        if (NSApp == nil)
            AllocDefaultNSAppDelegate();
        g_appDelegateCreated = true;
    }
}


} // /namespace LLGL



// ================================================================================
