#include <stdio.h>

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>

#include "caml_values.h"

#include "config.h"
#ifdef USE_WIN32
#include "ReveryWin32.h"
#elif USE_COCOA
#include "ReveryCocoa.h"
#import <Cocoa/Cocoa.h>
#elif USE_GTK
#include "ReveryGtk.h"
#endif

#include "utilities.h"

CAMLprim value revery_makeTrayHandle(value vImagePath) {
    CAMLparam1(vImagePath);
    CAMLlocal1(result);

#ifdef USE_COCOA
    NSStatusItem *statusItem = [NSStatusBar.systemStatusBar statusItemWithLength:NSVariableStatusItemLength];
    [(NSObject *)statusItem retain];

    if (vImagePath != Val_none) {
        const char *imagePath = String_val(Some_val(vImagePath));

        NSImage *nsImage = revery_makeImageFromAbsolutePath_cocoa(imagePath);

        statusItem.button.image = nsImage;

        UNUSED(imagePath);
    }

    result = revery_wrapPointer(statusItem);

    CAMLreturn(result);
#elif USE_WIN32
    result = caml_alloc(sizeof(NULL), Abstract_tag);
#else
    result = caml_alloc(sizeof(NULL), Abstract_tag);
#endif
    CAMLreturn(result);
}

CAMLprim value revery_setTrayTitle(value vTrayHandle, value vTitle) {
    CAMLparam2(vTrayHandle, vTitle);
    CAMLlocal1(result);
#ifdef USE_COCOA
    void* statusItem = revery_unwrapPointer(vTrayHandle);

    const char *title = String_val(vTitle);

    revery_setTrayTitle_cocoa(statusItem, title);

    result = revery_wrapPointer(statusItem);

    CAMLreturn(result);
#elif USE_WIN32
    result = caml_alloc(sizeof(NULL), Abstract_tag);
#else
    result = caml_alloc(sizeof(NULL), Abstract_tag);
#endif
    CAMLreturn(result);
}

void revery_removeTrayItem(value vTrayHandle) {
    CAMLparam1(vTrayHandle);
#ifdef USE_COCOA
    void* statusItem = revery_unwrapPointer(vTrayHandle);

    revery_removeStatusItem_cocoa(statusItem);

    CAMLreturn0;
#elif USE_WIN32
    CAMLreturn0;
#else
    CAMLreturn0;
#endif
}
