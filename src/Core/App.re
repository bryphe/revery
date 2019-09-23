type delegatedFunc = unit => unit;
type idleFunc = unit => unit;
type canIdleFunc = unit => bool;
let noop = () => ();

type t = {
  mutable windows: list(Window.t),
  mutable idleCount: int,
  mutable isFirstRender: bool,
  onIdle: idleFunc,
  canIdle: ref(canIdleFunc),
};

type initFunc = t => unit;

let framesToIdle = 10;

type appInitFunc = t => unit;

let getWindows = (app: t) => app.windows;

let getWindowById = (app: t, id: int) =>
  app.windows |> List.filter(w => Window.getUniqueId(w) == id) |> List.hd;

let quit = (code: int) => exit(code);

let isIdle = (app: t) => app.idleCount >= framesToIdle;

let _mainThreadMutex = Mutex.create();
/* A list of pending functions the main thread will need to run */
let _mainThreadPendingFunctions: ref(list(delegatedFunc)) = ref([]);
let _anyPendingWork: ref(bool) = ref(false);
let runOnMainThread = f => {
  Mutex.lock(_mainThreadMutex);
  _mainThreadPendingFunctions := [f, ..._mainThreadPendingFunctions^];
  _anyPendingWork := true;
  Mutex.unlock(_mainThreadMutex);
};

let _anyPendingMainThreadJobs = () => {
  _anyPendingWork^;
};

let setCanIdle = (f: canIdleFunc, app: t) => {
  app.canIdle := f;
};

/* Execute any pending main thread jobs */
let _doPendingMainThreadJobs = () => {
  let jobs = {
    Mutex.lock(_mainThreadMutex);
    let ret = _mainThreadPendingFunctions^;
    _anyPendingWork := false;
    _mainThreadPendingFunctions := [];
    Mutex.unlock(_mainThreadMutex);
    ret;
  };

  jobs |> List.rev |> List.iter(f => f());
};

let flushPendingCallbacks = () => _doPendingMainThreadJobs();

let createWindow =
    (~createOptions=WindowCreateOptions.default, app: t, windowName) => {
  let w = Window.create(windowName, createOptions);
  /* Window.render(w) */
  app.windows = [w, ...app.windows];
  w;
};

let _anyWindowsDirty = (app: t) =>
  List.fold_left(
    (prev, w) => prev || Window.isDirty(w),
    false,
    getWindows(app),
  );

let _checkAndCloseWindows = (app: t) => {
  let currentWindows = getWindows(app);
  let windowsToClose =
    List.filter(w => Window.shouldClose(w), currentWindows);
  let windowsToKeep =
    List.filter(w => !Window.shouldClose(w), currentWindows);

  List.iter(w => Window.destroyWindow(w), windowsToClose);
  app.windows = windowsToKeep;
};

let start = (~onIdle=noop, initFunc: appInitFunc) => {
  let appInstance: t = {
    windows: [],
    idleCount: 0,
    isFirstRender: true,
    onIdle,
    canIdle: ref(() => true),
  };

  let _ = Sdl2.init();
  let _ = initFunc(appInstance);

  let appLoop = () => {
    let evt = Sdl2.Event.poll();
    switch (evt) {
    | None => () // prerr_endline ("No event");
    | Some(v) =>
      let handleEvent = (windowID) => {
        let window = getWindowById(appInstance, windowID);
        Window._handleEvent(v, window);
      };
      switch (v) {
      | Sdl2.Event.MouseButtonUp({windowID, _}) => handleEvent(windowID);
      | Sdl2.Event.MouseButtonDown({windowID, _}) => handleEvent(windowID);
      | Sdl2.Event.MouseMotion({windowID, _ }) => handleEvent(windowID);
      | Sdl2.Event.MouseWheel({windowID, _}) => handleEvent(windowID);
      | Sdl2.Event.KeyDown({windowID, _}) => handleEvent(windowID);
      | Sdl2.Event.KeyUp({windowID, _}) => handleEvent(windowID);
      | Sdl2.Event.TextInput({windowID, _}) => handleEvent(windowID);
      | Sdl2.Event.TextEditing({windowID, _}) => handleEvent(windowID);
      | Sdl2.Event.WindowResized({windowID, _}) => handleEvent(windowID);
      | Sdl2.Event.WindowSizeChanged({windowID, _}) => handleEvent(windowID);
      | Sdl2.Event.WindowMoved({windowID, _}) => handleEvent(windowID);
      | Sdl2.Event.Quit => exit(0)
      | _ => ()
      };
    };

    Tick.Default.pump();

    _checkAndCloseWindows(appInstance);

    if (appInstance.isFirstRender
        || _anyWindowsDirty(appInstance)
        || _anyPendingMainThreadJobs()
        || !appInstance.canIdle^()) {
      Performance.bench("_doPendingMainThreadJobs", () =>
        _doPendingMainThreadJobs()
      );
      Performance.bench("renderWindows", () => {
        List.iter(w => Window.render(w), getWindows(appInstance))
      });

      appInstance.idleCount = 0;
      appInstance.isFirstRender = false;
    } else {
      appInstance.idleCount = appInstance.idleCount + 1;

      if (appInstance.idleCount === framesToIdle) {
        appInstance.onIdle();
      };

      Environment.sleep(Milliseconds(1.));
    };

    Environment.yield();

    List.length(getWindows(appInstance)) == 0;
  };

  Sdl2.renderLoop(appLoop);
};
