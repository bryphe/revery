module Shaders = Revery_Shaders;
module Geometry = Revery_Geometry;
module Layout = Layout;
module LayoutTypes = Layout.LayoutTypes;

open Fontkit;

open Revery_Core;

open ViewNode;
open RenderPass;


class textNode (name: string, text: string) = {
    as _this;

    val quad = Geometry.Cube.create();
    val textureShader = FontShader.create();
    val font = Fontkit.load("Roboto-Regular.ttf", 24);

    inherit (class viewNode)(name) as _super;
            
    pub! draw = (pass: renderPass, layer: int) => {
        /* Draw background first */
        _super#draw(pass, layer);

        switch (pass) {
        | SolidPass(m) => {
            Shaders.CompiledShader.use(textureShader);
            Shaders.CompiledShader.setUniformMatrix4fv(textureShader, "uProjection", m);

            let dimensions = _super#measurements();
            let color = _super#getStyle().color;

            Shaders.CompiledShader.setUniform3fv(textureShader, "uColor", Color.toVec3(color));

            let render = (s: Fontkit.fk_shape, x: float, y: float) => {
                let glyph = FontRenderer.getGlyph(font, s.codepoint);

                let {width, height, bearingX, bearingY, advance, _} = glyph;

                let _ = FontRenderer.getTexture(font, s.codepoint);
                /* TODO: Bind texture */

                Shaders.CompiledShader.setUniform4f(textureShader, "uPosition", 
                    x +. float_of_int(bearingX),
                    y +. float_of_int(dimensions.height) -. float_of_int(bearingY),
                    float_of_int(width),
                    float_of_int(height));

                Geometry.draw(quad, textureShader);

                x +. float_of_int(advance) /. 64.0;
            };

            let shapedText = Fontkit.fk_shape(font, text);
            let startX = ref(float_of_int(dimensions.left));
            Array.iter(s => {
                let nextPosition = render(s, startX^, float_of_int(dimensions.top));
                startX := nextPosition;
            }, shapedText);
            }
        };
    };

    pub! getMeasureFunction = () => {
        let measure = (_mode, _width, _widthMeasureMode, _height, _heightMeasureMode) => {
                let d = FontRenderer.measure(font, text);
                let ret: Layout.LayoutTypes.dimensions = { LayoutTypes.width: d.width, height: d.height };
                ret;
        };
        Some(measure);
    };
};
