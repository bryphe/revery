open Revery_Font;
open TestFramework;

let runCount = glyphStrings => List.length(glyphStrings);

let glyphCount = ((_typeface, glyphs)) =>
  // Each glyph is 2 bytes
  String.length(glyphs) / 2;

let run = (index, runs) => List.nth(runs, index);

let typefaceId = ((typeface, _glyphs)) =>
  typeface |> Skia.Typeface.getUniqueID |> Int32.to_int;

describe("FontCache", ({test, _}) => {
  let defaultFont =
    Family.default
    |> Family.resolve(~italic=false, Weight.Normal)
    |> Result.get_ok;

  let defaultFontId =
    defaultFont
    |> FontCache.getSkiaTypeface
    |> Skia.Typeface.getUniqueID
    |> Int32.to_int;

  test("empty string has empty shapes", ({expect, _}) => {
    let {glyphStrings}: ShapeResult.t = "" |> FontCache.shape(defaultFont);

    expect.int(glyphStrings |> runCount).toBe(0);
  });

  test(
    "fallback for all ASCII characters - including non-printable characters",
    ({expect, _}) => {
    for (ascii in 0 to 255) {
      let asciiCharacter = Zed_utf8.make(1, Uchar.of_int(ascii));
      let {glyphStrings}: ShapeResult.t =
        asciiCharacter |> FontCache.shape(defaultFont);

      expect.int(glyphStrings |> runCount).toBe(1);
      // TODO: Investigate why we sometimes get 2 glyph strings here?
      // expect.int(glyphStrings |> run(0) |> glyphCount).toBe(1);
    }
  });

  test("shape simple CJK text", ({expect, _}) => {
    let {glyphStrings}: ShapeResult.t =
      "腐" |> FontCache.shape(defaultFont);

    expect.int(glyphStrings |> runCount).toBe(1);
    expect.int(glyphStrings |> run(0) |> glyphCount).toBe(1);
  });

  test("shape simple ASCII string", ({expect, _}) => {
    let {glyphStrings}: ShapeResult.t =
      "abc" |> FontCache.shape(defaultFont);

    expect.int(glyphStrings |> runCount).toBe(1);
    expect.int(glyphStrings |> run(0) |> glyphCount).toBe(3);
  });

  test("shape string w/ fallback", ({expect, _}) => {
    let {glyphStrings}: ShapeResult.t =
      "a⌋" |> FontCache.shape(defaultFont);

    expect.int(glyphStrings |> runCount).toBe(2);
    expect.int(glyphStrings |> run(0) |> glyphCount).toBe(1);
    expect.int(glyphStrings |> run(0) |> typefaceId).toBe(defaultFontId);

    expect.int(glyphStrings |> run(1) |> glyphCount).toBe(1);
    expect.int(glyphStrings |> run(1) |> typefaceId).not.toBe(
      defaultFontId,
    );
  });

  test("fallback first, then shape", ({expect, _}) => {
    let {glyphStrings}: ShapeResult.t =
      "⌋a" |> FontCache.shape(defaultFont);

    expect.int(glyphStrings |> runCount).toBe(2);
    expect.int(glyphStrings |> run(0) |> glyphCount).toBe(1);
    expect.int(glyphStrings |> run(0) |> typefaceId).not.toBe(
      defaultFontId,
    );

    expect.int(glyphStrings |> run(1) |> glyphCount).toBe(1);
    expect.int(glyphStrings |> run(1) |> typefaceId).toBe(defaultFontId);
  });

  test("non-fallback surrounded by holes (onivim/oni2#2178)", ({expect, _}) => {
    let {glyphStrings}: ShapeResult.t =
      "⌋a⌋" |> FontCache.shape(defaultFont);

    expect.int(glyphStrings |> runCount).toBe(3);
    expect.int(glyphStrings |> run(0) |> glyphCount).toBe(1);
    expect.int(glyphStrings |> run(0) |> typefaceId).not.toBe(
      defaultFontId,
    );

    expect.int(glyphStrings |> run(1) |> glyphCount).toBe(1);
    expect.int(glyphStrings |> run(1) |> typefaceId).toBe(defaultFontId);

    expect.int(glyphStrings |> run(2) |> glyphCount).toBe(1);
    expect.int(glyphStrings |> run(2) |> typefaceId).not.toBe(
      defaultFontId,
    );
  });
});
