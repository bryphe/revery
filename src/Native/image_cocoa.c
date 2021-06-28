#include "config.h"
#ifdef USE_COCOA
#include <stdio.h>

#import <Cocoa/Cocoa.h>

void *revery_makeImageFromAbsolutePath_cocoa(const char *imagePath) {
    NSString *nsImagePath =
        [NSString stringWithCString:imagePath encoding:NSUTF8StringEncoding];

    NSImage *nsImage = [[NSImage alloc]initWithContentsOfFile:nsImagePath];

    return nsImage;
}

#endif
