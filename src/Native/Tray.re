%import
"config.h";

type t;

open {
       external c_make: (~imagePath: string=?, unit) => t =
         "revery_makeTrayHandle";
       external c_setTitle: (t, ~text: string) => t = "revery_setTrayTitle";
       external c_remove: t => unit = "revery_removeTrayItem";
     };

%if
defined(USE_COCOA);

let make = (~imagePath=?, ()) => {
  let trayHandle = c_make(~imagePath?, ());
  Gc.finalise(NSObject.release, trayHandle);
  trayHandle;
};

[%%else];

let make = c_make;

[%%endif];

let setTitle = c_setTitle;
let remove = c_remove;
