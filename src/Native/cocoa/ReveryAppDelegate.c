#include "config.h"
#ifdef USE_COCOA
#import "ReveryAppDelegate.h"
#import <Cocoa/Cocoa.h>

#import "utilities.h"

#include "ReveryAppDelegate_func.h"

// Implementation of ReveryAppDelegate
@implementation ReveryAppDelegate

/* init - initializes the AppDelegate
  Assigns a mutable dictionary to notificationActions
*/
- (id)initWithSDLDelegate:(SDLAppDelegate *)sdlDelegate {
    self = [super init];
    if (self) {
        _notificationActions = [NSMutableDictionary new];
        _sdlDelegate = sdlDelegate;
    }

    NSString *valueToSave = @"true";
    [[NSUserDefaults standardUserDefaults] setObject:valueToSave forKey:@"ApplePressAndHoldEnabled"];
    [[NSUserDefaults standardUserDefaults] setObject:valueToSave forKey:@"AppleMomentumScrollSupported"];
    [[NSUserDefaults standardUserDefaults] synchronize];

    return self;
}

+(id)newWithSDLDelegate:(SDLAppDelegate *)sdlDelegate {
    return [[ReveryAppDelegate alloc] initWithSDLDelegate:sdlDelegate];
}


/* applicationDidFinishLaunching
  Assigns self as the notification center delegate
*/
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [_sdlDelegate applicationDidFinishLaunching:notification];
}

/* didActivateNotification
  Gets the long from the NSDictionary from the notification's identifier and calls it
*/
- (void)userNotificationCenter:(NSUserNotificationCenter *)center
    didActivateNotification:(NSUserNotification *)notification {
    UNUSED(center);
    NSNumber *num = _notificationActions[[notification identifier]];
    long ocamlFunc = [num longValue];
    revery_caml_call(ocamlFunc);
}

/* shouldPresentNotification
  Always presents the notification
*/
- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center
    shouldPresentNotification:(NSUserNotification *)notification {
    UNUSED(center);
    UNUSED(notification);
    return YES;
}

/* openFile
  We call into the CAML function `revery_dispatchFileOpen`
  Unfortunately because of namespacing issues with `alloc`,
  we have to put the function in a separate file. Both OCaml
  and Objective-C have `alloc` selector/functions, which
  causes either build errors or runtime errors, neither of
  which are preferable :).
*/
- (BOOL)application:(NSApplication *)sender
    openFile:(NSString *)filename {
    UNUSED(sender);

    revery_appDelegate_openFile([filename UTF8String]);
    return YES;
}
@end

#endif
