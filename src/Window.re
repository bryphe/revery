open Reglfw.Glfw;

class window (name: string) = {
    as _this;

    val w = glfwCreateWindow(800, 600, name);

    val renderCallback = ref(None);

    pub setRenderCallback = (rc) => {
        renderCallback := Some(rc);
    };

    pub render = () => {
        glfwMakeContextCurrent(w);
        glClearColor(0.39, 0.58, 0.93, 1.0);

        switch(renderCallback^) {
        | None => ()
        | Some(r) => r();
        };

        glfwSwapBuffers(w);
    };

}

/* type window = Glfw.window; */

/* let create = (name, width, height) => { */
/*     let _ = Glfw.glfwInit(); */
/*     let w = Glfw.glfwCreateWindow(width, height, name); */
/*     print_endline("Hello, world!"); */
/*     w; */
/* }; */
