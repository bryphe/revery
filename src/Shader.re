open Reglfw;
open Reglfw.Glfw;

type vertexShaderSource = string;
type fragmentShaderSource = string;

type vertexChannel = 
| Position
| Normal
| Color
| TextureCoordinate;

module ShaderPrecision {
    type t =
    | Default
    | Low
    | Medium
    | High;

    let toString = (v) => {
        switch((Environment.webGL, v)) {
        | (false, _) => ""
        | (true, p) => switch(p) {
            | Default => ""
            | Low => "lowp"
            | Medium => "mediump"
            | High => "highp"
            };
        };
    };
}


module ShaderDataType {
    type t =
    | Float
    | Vector2
    | Vector3
    | Vector4
    | Mat4
    | Sampler2D;

    let toString = (v) => {
        switch(v) {
        | Float => "float"
        | Vector2 => "vec2"
        | Vector3 => "vec3"
        | Vector4 => "vec4"
        | Mat4 => "mat4"
        | Sampler2D => "sampler2D"
        };
    };
}

type shaderUniformUsage =
| VertexShader
| FragmentShader
| VertexAndFragmentShader


module ShaderAttribute {
    type t = {
        channel: vertexChannel,
        dataType: ShaderDataType.t,
        name: string
    };

    let toString = (a) => {
        "attribute " ++ ShaderDataType.toString(a.dataType) ++ " " ++ a.name ++ ";"
    };
}

type shaderUniform = {
    dataType: ShaderDataType.t,
    usage: shaderUniformUsage,
    name: string
};

type shaderVarying = {
    dataType: ShaderDataType.t,
    precision: ShaderPrecision.t,
    name: string
};

type uncompiledShader = (list(shaderUniform), list(ShaderAttribute.t), list(shaderVarying), vertexShaderSource, fragmentShaderSource); 
type compiledShader = (list(shaderUniform), list(ShaderAttribute.t), list(shaderVarying), Glfw.program);

let generateAttributeVertexShaderBlock = (attributes: list(ShaderAttribute.t)) => {
    let strs: list(string) = List.map((s) => ShaderAttribute.toString(s), attributes);
    List.fold_left((prev, cur) => prev ++ "\n" ++ cur, "", strs);
};

let create = (~uniforms: list(shaderUniform), ~attributes: list(ShaderAttribute.t), ~varying: list(shaderVarying), ~vertexShader: vertexShaderSource, ~fragmentShader: fragmentShaderSource) => {

    let vertexShader = generateAttributeVertexShaderBlock(attributes) ++ vertexShader;

    (uniforms, attributes, varying, vertexShader, fragmentShader);
};

type shaderCompilationResult = 
| ShaderCompilationSuccess(compiledShader)
| ShaderCompilationFailure(string);

let compile = (shader: uncompiledShader) => {
    let (uniforms, attributes, varying, vs, fs) = shader;

    print_endline("creating shaders");
    let vertexShader = glCreateShader(Glfw.GL_VERTEX_SHADER);
    let fragmentShader = glCreateShader(Glfw.GL_FRAGMENT_SHADER);

    print_endline("shader source");
    let () = glShaderSource(vertexShader, vs);
    let () = glShaderSource(fragmentShader, fs);

    let vsResult = glCompileShader(vertexShader);
    let fsResult = glCompileShader(fragmentShader);

    let linkProgram = (vs, fs) => {
        let program = glCreateProgram();   
        let () = glAttachShader(program, vs);
        let () = glAttachShader(program, fs);

        let result = glLinkProgram(program);

        switch (result) {
        | LinkSuccess => ShaderCompilationSuccess((uniforms, attributes, varying, program))
        | LinkFailure(v) => ShaderCompilationFailure(v)
        }

    };

    switch ((vsResult, fsResult)) {
       | (CompilationSuccess, CompilationSuccess) => linkProgram(vertexShader, fragmentShader)
       | (CompilationFailure(v), CompilationSuccess) => ShaderCompilationFailure(v)
       | (CompilationSuccess, CompilationFailure(v)) => ShaderCompilationFailure(v)
       | (CompilationFailure(v1), CompilationFailure(v2)) => ShaderCompilationFailure(v1 ++ v2)
    }
};
