open Reglm;

module Shaders = Revery_Shaders;
module Geometry = Revery_Geometry;
module Layout = Layout;
module LayoutTypes = Layout.LayoutTypes;

open Fontkit;

class node (name: string) = {
    as _this;
    
    val _children: ref(list(node)) = ref([]);
    val _style = ref(Layout.defaultStyle);
    val _layoutNode = ref(Layout.createNode([||], Layout.defaultStyle));

    pub draw = (layer: int) => {
        print_endline("- Drawing: " ++ name ++ " | " ++ string_of_int(layer));
        List.iter((c) => c#draw(layer + 1), _children^)
    };

    pub measurements = () => {
        (_layoutNode^).layout;
    };

    pub setStyle = (style) => {
        _style := style;
    };

    pub addChild = (n: node) => {
        _children := List.append(_children^, [n]);
    };

    pub getMeasureFunction = () => {
        None;
    };

    pub toLayoutNode = () => {
        let childNodes = List.map((c) => c#toLayoutNode(), _children^);
        let node = switch (_this#getMeasureFunction()) {
        | None => Layout.createNode(Array.of_list(childNodes), _style^);
        | Some(m) => Layout.createNodeWithMeasure(Array.of_list(childNodes), _style^, m);
        };

        _layoutNode := node;
        node;
    };
};

class viewNode (name: string, color: Vec3.t) = {
    as _this;

    val _quad = Geometry.Cube.create();
    val solidShader = SolidShader.create();


    inherit (class node)(name) as _super;
            
    pub! draw = (layer: int) => {
        Shaders.CompiledShader.use(solidShader);
        let projection = Mat4.create();
        Mat4.ortho(projection, 0.0, 800.0, 600.0, 0.0, -0.01, -100.0);
        Shaders.CompiledShader.setUniformMatrix4fv(solidShader, "uProjection", projection);

        let dimensions = _super#measurements();

        print_endline ("**" ++ name ++ "**");
        print_endline ("- top: " ++ string_of_int(dimensions.top));
        print_endline ("- left: " ++ string_of_int(dimensions.left));
        print_endline ("- width: " ++ string_of_int(dimensions.width));
        print_endline ("- height: " ++ string_of_int(dimensions.height));

        Shaders.CompiledShader.setUniform3fv(solidShader, "uColor", color);
        Shaders.CompiledShader.setUniform4f(solidShader, "uPosition", 
            float_of_int(dimensions.left),
            float_of_int(dimensions.top),
            float_of_int(dimensions.width),
            float_of_int(dimensions.height));
        Geometry.draw(_quad, solidShader);

        _super#draw(layer);
    };
};

class textNode (name: string, text: string, color: Vec3.t) = {
    as _this;

    val _quad = Geometry.Cube.create();
    val textureShader = TextureShader.create();
    val font = Fontkit.load("Roboto-Regular.ttf", 24);

    inherit (class node)(name) as _super;
            
    pub! draw = (layer: int) => {
        Shaders.CompiledShader.use(textureShader);
        let projection = Mat4.create();

        Mat4.ortho(projection, 0.0, 800.0, 600.0, 0.0, -0.01, -100.0);
        Shaders.CompiledShader.setUniformMatrix4fv(textureShader, "uProjection", projection);

        let dimensions = _super#measurements();

        Shaders.CompiledShader.setUniform3fv(textureShader, "uColor", color);

        let render = (s: Fontkit.fk_shape, x: float, y: float) => {
            let glyph = FontRenderer.getGlyph(font, s.codepoint);

            let {width, height, bearingX, bearingY, advance, _} = glyph;

            let _ = FontRenderer.getTexture(font, s.codepoint);
            /* TODO: Bind texture */

            Shaders.CompiledShader.setUniform4f(textureShader, "uPosition", 
                x +. float_of_int(bearingX),
                y -. (float_of_int(height) -. float_of_int(bearingY)),
                float_of_int(width),
                float_of_int(height));

            Geometry.draw(_quad, textureShader);

            x +. float_of_int(advance) /. 64.0;
        };

        let shapedText = Fontkit.fk_shape(font, text);
        let startX = ref(float_of_int(dimensions.left));
        Array.iter(s => {
            let nextPosition = render(s, startX^, float_of_int(dimensions.top));
            startX := nextPosition;
        }, shapedText);

        _super#draw(layer);
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

let layout = (node) => {
    let rootLayoutNode = node#toLayoutNode();
    Layout.layoutNode(rootLayoutNode);
};

let render = (node) => {
    layout(node);
    node#draw(0);
};
