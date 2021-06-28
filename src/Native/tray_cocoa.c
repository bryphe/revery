#include "config.h"
#ifdef USE_COCOA
#include <stdio.h>

#import <Cocoa/Cocoa.h>

void *revery_setTrayTitle_cocoa(NSStatusItem* statusItem, const char *titleText) {
    NSString *nsTitle =
        [NSString stringWithCString:titleText encoding:NSUTF8StringEncoding];

    statusItem.button.image = NULL;
    statusItem.button.title = nsTitle;

    return statusItem;
}

void revery_removeStatusItem_cocoa(NSStatusItem* statusItem) {
    [[NSStatusBar systemStatusBar] removeStatusItem: statusItem];
}

#endif
