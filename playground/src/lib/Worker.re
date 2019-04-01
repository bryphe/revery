open Revery;
open Revery.UI;

open Js_of_ocaml;

open Types;

let renderFunction =
  ref(() =>
    <View
      style=Style.[backgroundColor(Colors.red), width(100), height(100)]
    />
  );

let _pendingUpdates: ref(list(updates)) = ref([]);
let clearUpdates = () => _pendingUpdates := [];
let queueUpdate = (update: updates) => {
  _pendingUpdates := [update, ..._pendingUpdates^];
};

class proxyViewNode (()) = {
  as _this;
  inherit (class viewNode)() as super;
  pub! setStyle = style => {
    queueUpdate(SetStyle(super#getInternalId(), style));
  };
  pub! addChild = child => {
    print_endline("WORKER: ADD CHILD");
    queueUpdate(AddChild(super#getInternalId(), child#getInternalId()));
    super#addChild(child);
  };
  pub! removeChild = child => {
    print_endline("WORKER: REMOVE CHILD");
    queueUpdate(RemoveChild(super#getInternalId(), child#getInternalId()));
    super#removeChild(child);
  };
  initializer {
    queueUpdate(NewNode(super#getInternalId(), View));
  };
};

class proxyTextNode (text) = {
  as _this;
  inherit (class textNode)(text) as super;
  pub! setStyle = style => {
    queueUpdate(SetStyle(super#getInternalId(), style));
  };
  pub! addChild = child => {
    print_endline("WORKER: ADD TEXT CHILD");
    queueUpdate(AddChild(super#getInternalId(), child#getInternalId()));
    super#addChild(child);
  };
  pub! removeChild = child => {
    print_endline("WORKER: REMOVE TEXT CHILD");
    queueUpdate(RemoveChild(super#getInternalId(), child#getInternalId()));
    super#removeChild(child);
  };
  pub! setText = text => {
    print_endline("TEXT: Set text: " ++ text);
    queueUpdate(SetText(super#getInternalId(), text));
  };
  initializer {
    print_endline("TEXT: Initial text: " ++ text);
    queueUpdate(NewNode(super#getInternalId(), Text));
    queueUpdate(SetText(super#getInternalId(), text));
  };
};

class proxyImageNode (src) = {
  as _this;
  inherit (class imageNode)(src) as super;
  pub! setStyle = style => {
    queueUpdate(SetStyle(super#getInternalId(), style));
  };
  pub! addChild = child => {
    print_endline("WORKER: ADD TEXT CHILD");
    queueUpdate(AddChild(super#getInternalId(), child#getInternalId()));
    super#addChild(child);
  };
  pub! removeChild = child => {
    print_endline("WORKER: REMOVE TEXT CHILD");
    queueUpdate(RemoveChild(super#getInternalId(), child#getInternalId()));
    super#removeChild(child);
  };
  initializer {
    print_endline("IMAGE: Initial src: " ++ src);
    queueUpdate(NewNode(super#getInternalId(), Image));
    queueUpdate(SetImageSrc(super#getInternalId(), src));
  };
};

let rootNode = (new proxyViewNode)();
queueUpdate(RootNode(rootNode#getInternalId()));
let container = ref(Container.create(rootNode));

let idToNode: Hashtbl.t(int, viewNode) = Hashtbl.create(100);

let registerNode = node => {
  Hashtbl.add(idToNode, node#getInternalId(), Obj.magic(node));
};
registerNode(rootNode);

let getNodeById = id => {
  switch (Hashtbl.find_opt(idToNode, id)) {
  | Some(v) => v
  | None => failwith("Unable to find node with id: " ++ string_of_int(id))
  };
};

let proxyNodeFactory: nodeFactory = {
  createViewNode: () => {
    let ret = (new proxyViewNode)();
    registerNode(ret);
    ret;
  },
  createTextNode: text => {
    let ret = (new proxyTextNode)(text);
    registerNode(ret);
    ret;
  },
  createImageNode: src => {
    let ret = (new proxyImageNode)(src);
    registerNode(ret);
    ret;
  },
};

setNodeFactory(proxyNodeFactory);

let sendMessage = msg => {
  Worker.post_message(msg);
};

let setRenderFunction = fn => {
  /* print_endline ("setting render function4"); */
  /* let testVal = fn(); */
  /* let marshalledData = Marshal.to_string(testVal, [Marshal.Closures]); */
  /* print_endline ("Marshalled data: " ++ marshalledData); */
  /* let unmarshalledData = Obj.magic(Marshal.from_string(marshalledData)); */

  renderFunction := fn;

  print_endline(
    "before render function - childCount: "
    ++ string_of_int(List.length(rootNode#getChildren())),
  );
  container := Container.update(container^, fn());
  print_endline(
    "Set render function - childCount: "
    ++ string_of_int(List.length(rootNode#getChildren())),
  );

  print_endline("Trying to post...");
  /* let _derp = Js.string("hi"); */
  let updatesToSend = _pendingUpdates^ |> List.rev;
  Types.showAll(updatesToSend);
  sendMessage(Protocol.ToRenderer.Updates(updatesToSend));
  /* Worker.post_message(updatesToSend); */
  print_endline("Posted!" ++ string_of_int(List.length(updatesToSend)));
  clearUpdates();
};

let log = v => print_endline("[Worker] " ++ v);

let start = exec => {
  Worker.set_onmessage((updates: Protocol.ToWorker.t) => {
    log("WORKER: GOT UPDATES");
    switch (updates) {
    | SourceCodeUpdated(v) =>
      log("got source code update");

      sendMessage(Protocol.ToRenderer.Compiling);
      let _ = exec(v);
      sendMessage(Protocol.ToRenderer.Ready);
    | Measurements(v) =>
      log("applying measurements");

      let f = (measurement: Protocol.ToWorker.nodeMeasurement) => {
        let nodeId = measurement.id;
        log("Applying measurement for node: " ++ string_of_int(nodeId));
        let measurements = measurement.dimensions;

        let node = getNodeById(nodeId);
        log("forcing measurement");
        node#forceMeasurements(measurements);
        log("forcing measurement done");
      };

      List.iter(f, v);
      rootNode#recalculate();

      log("measurements applied");
    | _ => log("unknown update")
    };
  });

  log("Initialized");
  sendMessage(Protocol.ToRenderer.Ready);
  /* PlaygroundLib.startPlayground(); */
};
