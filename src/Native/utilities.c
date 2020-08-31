#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>
#include <caml/alloc.h>

#include "caml_values.h"
#include "utilities.h"

/* Sourced from Brisk's `BriskCocoa`
    Thank you @wokalski!
*/
void revery_caml_call_n(camlValue f, int argCount, camlValue *args) {
    caml_c_thread_register();
    caml_acquire_runtime_system();
    caml_callbackN(f, argCount, args);
    caml_release_runtime_system();
}

void revery_caml_call(camlValue f) {
    value args[] = {Val_unit};
    revery_caml_call_n(f, 1, args);
}

CAMLprim value revery_wrapPointer(void *data) {
    CAMLparam0();
    CAMLlocal1(result);

    result = caml_alloc(1, Abstract_tag);
    Store_field(data, 0, (value)data);

    CAMLreturn(result);
}

void *revery_extractPointer(camlValue data) {
    return (void *)Field(data, 0);
}
