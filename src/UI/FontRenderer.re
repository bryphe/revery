open Reglfw.Glfw;
open Fontkit;

open Revery_Core;

let _getGlyph =
  Memoize.make(((font: Fontkit.fk_face, codepoint: int)) =>
    Fontkit.renderGlyph(font, codepoint)
  );

let getGlyph = (font: Fontkit.fk_face, codepoint: int) =>
  _getGlyph((font, codepoint));

let _getTexture = ((font: Fontkit.fk_face, codepoint: int)) => {
  let glyph = getGlyph(font, codepoint);

  let {image, _} = glyph;

  /* - Create texture atlas */
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  let texture = glCreateTexture();
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, image);
  texture;
};

let _memoizedGetTexture = Memoize.make(_getTexture);
let getTexture = (font: Fontkit.fk_face, codepoint: int) =>
  _memoizedGetTexture((font, codepoint));

type dimensions = {
  width: int,
  height: int,
};

let measure = (font: Fontkit.fk_face, text: string) => {
  let shapedText = Fontkit.fk_shape(font, text);
  let minY = ref(1000);
  let maxY = ref(-1000);
  let x = ref(0);

  Array.iter(
    shape => {
      let {height, bearingY, advance, _} = getGlyph(font, shape.codepoint);
      let top = - bearingY;
      let bottom = top + height;

      if (height > 0) {
        minY := minY^ < top ? minY^ : top;
        maxY := maxY^ > bottom ? maxY^ : bottom;
      };

      x := x^ + advance;
    },
    shapedText,
  );

  let d: dimensions = {
    height: maxY^ - minY^,
    width: int_of_float(float_of_int(x^) /. 64.0),
  };
  d;
};
