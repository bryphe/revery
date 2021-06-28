type t;

/**
 * make
 *
 * Takes an optional [imagePath] which is an absolute path to a in image-file.
 * Returns a newly created Tray.t;
 *
 * Examples:
 *   Tray.make(~imagePath="/absolute/path/to/image.png", ()) |> ignore;
 *   Tray.make(~imagePath=Environment.getAssetPath("some_asset_image.png", ()) |> ignore;
 *   let tray = Tray.make();
 */
let make: (~imagePath: string=?, unit) => t;

/**
 * setTitle
 *
 * Takes a [title] of string and sets the tray's text to it.
 * Returns the updated tray item.
 *  *
 * Examples:
 *   tray |> Tray.setTitle(~text="Hello Revery!") |> ignore;
 *   let tray = Tray.make() |> Tray.setTitle(~text="Hello Revery!");
 *   Tray.setTitle(tray, ~text="Hello World!") |> ignore;
 */
let setTitle: (t, ~text: string) => t;

/**
 * remove
 *
 * Given a [t] removes the tray item from the tray.
 *  *
 * Example:
 *   tray |> Tray.remove;
 *   Tray.remove(tray);
 */
let remove: t => unit;
