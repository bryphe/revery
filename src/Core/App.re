%import
"../Native/config.h";

module AppLog = (val Log.withNamespace("Revery.App"));
module SdlLog = (val Log.withNamespace("Revery.SDL2"));

module Log = AppLog;

type delegatedFunc = unit => unit;
type unsubscribe = unit => unit;

type quitResponse =
  | AllowQuit
  | PreventQuit;

let mergeQuitResponse = (a: quitResponse, b: quitResponse) => {
  switch (a, b) {
  | (AllowQuit, AllowQuit) => AllowQuit
  | _ => PreventQuit
  };
};

type t = {
  mutable idleCount: int,
  mutable isFirstRender: bool,
  mutable isQuitting: bool,
  windows: Hashtbl.t(int, Window.t),
  onIdle: Event.t(unit),
  onBeforeQuit: Event.Fanout.t(int, quitResponse),
  onFileOpen: Event.t(string),
  mutable canIdle: unit => bool,
};

let framesToIdle = 10;

let getWindows = (app: t) => {
  Hashtbl.to_seq_values(app.windows) |> List.of_seq;
};

let getWindowById = (app: t, id: int) => {
  Hashtbl.find_opt(app.windows, id);
};

let _tryToClose = (app: t, window: Window.t) => {
  let uniqueId = Window.getUniqueId(window);
  if (Window.canQuit(window)) {
    Log.debugf(m => m("canQuit is true for window %i", uniqueId));
    Hashtbl.remove(app.windows, uniqueId);
  } else {
    Log.debugf(m => m("canQuit is false for window %i", uniqueId));
  };
};

let _tryToCloseAll = (app: t) => {
  let windows = Hashtbl.to_seq_values(app.windows);
  Seq.iter(w => _tryToClose(app, w), windows);
};

let quit = (~askNicely=false, ~code=0, app: t) => {
  if (askNicely) {
    _tryToCloseAll(app);
  };

  if (Hashtbl.length(app.windows) == 0 || !askNicely) {
    Revery_Native.uninitApp();

    // Verify [quit] wasn't called recursively from a beforeQuit handler
    if (!app.isQuitting) {
      Log.info("onBeforeQuit");
      app.isQuitting = true;
      let quitResponse =
        Event.Fanout.dispatch(
          app.onBeforeQuit,
          mergeQuitResponse,
          AllowQuit,
          code,
        );
      app.isQuitting = false;

      switch (quitResponse) {
      | AllowQuit =>
        Log.info("Quitting");
        exit(code);
      | PreventQuit => Log.info("Quit prevented by event handler")
      };
    };
  };
};

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

  // If we're 'idle' - in a [waitTimeout], dispatch an event to wake up the main thread
  Sdl2.Event.push();
};

let _anyPendingMainThreadJobs = () => {
  _anyPendingWork^;
};

let setCanIdle = (f, app: t) => {
  app.canIdle = f;
};

let onBeforeQuit = app => Event.Fanout.subscribe(app.onBeforeQuit);
let onIdle = app => Event.subscribe(app.onIdle);
let onFileOpen = app => Event.subscribe(app.onFileOpen);

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
  let uniqueId = Window.getUniqueId(w);
  Hashtbl.add(app.windows, uniqueId, w);
  w;
};

let _anyWindowsDirty = (app: t) =>
  List.fold_left(
    (prev, w) => prev || Window.isDirty(w),
    false,
    getWindows(app),
  );

let initConsole = () =>
  if (Sys.win32) {
    // First, try attaching to an existing console.
    let attachResult = Sdl2.Platform.win32AttachConsole();

    // If that wasn't available - try to allocate a new one.
    let _code =
      if (attachResult == 0) {
        Sdl2.Platform.win32AllocConsole();
      } else {
        attachResult;
      };
    ();
  };

let handleKeymapChanged = () => {
  AppLog.info("Keymap changed");

  if (Sys.win32) {
    // Workaround for https://github.com/onivim/oni2/issues/1657
    // When changing the keyboard layout, on Windows 10, the
    // modifier keys like the Windows/GUI key 'stick'...
    // ...this resets the state of the modifiers.
    Sdl2.Keymod.setState(
      Sdl2.Keymod.none,
    );
  };
};

%ifdef
USE_GTK;

let runGtkIteration = () =>
  if (Revery_Native.Gtk.eventsPending()) {
    AppLog.debug("Running Gtk iteration");
    let _quit: bool = Revery_Native.Gtk.mainIteration();
    ();
  };

[%%else];

let runGtkIteration = () => ();

[%%endif];

let start = init => {
  let appInstance: t = {
    windows: Hashtbl.create(1),
    idleCount: 0,
    isFirstRender: true,
    isQuitting: false,
    onBeforeQuit: Event.create(),
    onIdle: Event.create(),
    onFileOpen: Event.create(),
    canIdle: () => true,
  };

  Sdl2.Log.setOutputFunction((_category, priority, message) =>
    switch (priority) {
    | Verbose
    | Debug => SdlLog.trace(message)
    | Info => SdlLog.info(message)
    | Warn => SdlLog.warn(message)
    | Error
    | Critical => SdlLog.error(message)
    }
  );

  let _ = Sdl2.init();

  let _dispose = init(appInstance);

  let _ = Sdl2.ScreenSaver.enable();

  let _handleEvent = evt => {
    appInstance.idleCount = 0;
    let handleEvent = windowID => {
      let window = getWindowById(appInstance, windowID);
      switch (window) {
      | Some(win) => Window.handleEvent(evt, win)
      | None =>
        Log.errorf(m =>
          m(
            "Unable to find window with ID: %i - event: %s",
            windowID,
            Sdl2.Event.show(evt),
          )
        )
      };
    };
    switch (evt) {
    | Sdl2.Event.MouseButtonUp({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.MouseButtonDown({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.MouseMotion({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.MouseWheel({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.KeyDown({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.KeyUp({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.TextInput({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.TextEditing({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.KeymapChanged => handleKeymapChanged()
    | Sdl2.Event.WindowResized({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.WindowSizeChanged({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.WindowExposed({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.WindowFocusGained({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.WindowFocusLost({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.WindowMaximized({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.WindowFullscreen({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.WindowMinimized({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.WindowRestored({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.WindowMoved({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.WindowEnter({windowID}) => handleEvent(windowID)
    | Sdl2.Event.WindowLeave({windowID}) => handleEvent(windowID)
    | Sdl2.Event.WindowClosed({windowID, _}) =>
      Log.debugf(m => m("Got WindowClosed event for %i", windowID));
      handleEvent(windowID);
      switch (getWindowById(appInstance, windowID)) {
      | None => ()
      | Some(win) => _tryToClose(appInstance, win)
      };
    | Sdl2.Event.DropBegin({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.DropFile({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.DropComplete({windowID, _}) => handleEvent(windowID)
    | Sdl2.Event.Quit =>
      // Sometimes, on Mac, we could get a 'quit' without a
      // corresponding WindowClosed event - this can happen
      // if Command+Q is pressed. In that case, we'll try
      // closing all the windows - and if they all close,
      // we'll exit the app.
      quit(~askNicely=true, ~code=0, appInstance)
    | _ => ()
    };
  };

  let _flushEvents = () => {
    let processingEvents = ref(true);

    while (processingEvents^) {
      let evt = Sdl2.Event.poll();
      switch (evt) {
      | None => processingEvents := false
      | Some(v) => _handleEvent(v)
      };
    };
  };

  let dispatchFileOpen = Event.dispatch(appInstance.onFileOpen);
  Callback.register("revery_dispatchFileOpen", dispatchFileOpen);

  Revery_Native.initApp();

  AppLog.infof(m => m("Operating in locale : %s", Environment.userLocale));

  let appLoop = () => {
    _flushEvents();

    runGtkIteration();

    Tick.Default.pump();

    if (appInstance.isFirstRender
        || _anyWindowsDirty(appInstance)
        || _anyPendingMainThreadJobs()
        || !appInstance.canIdle()) {
      if (appInstance.idleCount > 0) {
        Log.debug("Upshifting into active state.");
      };

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
        Log.debug("Downshifting into idle state...");
        let _: unit = Event.dispatch(appInstance.onIdle, ());
        ();
      };

      let evt = Sdl2.Event.waitTimeout(250);
      switch (evt) {
      | None => ()
      | Some(evt) => _handleEvent(evt)
      };
    };

    Environment.yield();

    false;
  };

  Sdl2.renderLoop(appLoop);
};
let start = init => Sdl2.main(() => start(init));
