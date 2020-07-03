/**
    Discovery.re

*/
open Skia;
exception Font_not_found;

let fontManager = FontManager.makeDefault();

let find =
    (
      ~weight=FontWeight.Normal,
      ~width=FontWidth.Normal,
      ~italic=false,
      family,
    ) => {
  let style =
    FontStyle.make(
      FontWeight.toInt(weight),
      FontWidth.toInt(width),
      italic ? Italic : Upright,
    );
  FontManager.matchFamilyStyle(fontManager, family, style);
};
