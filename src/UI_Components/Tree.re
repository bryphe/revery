open Revery_UI;
open Revery_Core;

type status =
  | Open
  | Closed;

type content('a) = {
  data: 'a,
  id: int,
  status,
};

type tree('a) =
  | Empty
  | Node(content('a), list(tree('a)));

let component = React.component("Tree");

let defaultNodeStyles = Style.[flexDirection(`Row), marginVertical(5)];

/* let rec findNode = (nodeID, tr) => { */
/*   switch (tr) { */
/*   | Empty => None */
/*   | Node({id, _} as x, _) when nodeID == id => Some(x) */
/*   | Node(_, leaves) => */
/*     let found = findNode(nodeID, leaves); */
/*     switch (found) { */
/*     | Some(n) => Some(n) */
/*     | None => findNode(nodeID, rightTree) */
/*     }; */
/*   }; */
/* }; */

let default = (~indent, {data, status, _}) => {
  let isOpen =
    switch (status) {
    | Open => true
    | Closed => false
    };
  open Style;
  let textStyles = [fontSize(20), color(Colors.black)];
  let indentStr = String.make(indent * 2, ' ');
  let arrow = isOpen ? {||} : {||};
  <Clickable>
    <View style=defaultNodeStyles>
      <Text
        text={indentStr ++ arrow ++ " "}
        style=[fontFamily("FontAwesome5FreeSolid.otf"), ...textStyles]
      />
      <Text
        text=data
        style=[fontFamily("Roboto-Regular.ttf"), ...textStyles]
      />
    </View>
  </Clickable>;
};

/**
 * @param ~indent How much to indent the current (sub)tree.
 * @param ~renderer is a function which determines how each node is rendered
 * @param t The tree to convert to a tree of JSX elements.
 */
let rec renderTree = (~indent=0, ~nodeRenderer, ~emptyRenderer, t) => {
  let drawNode = nodeRenderer(~indent);
  let empty =
    switch (emptyRenderer) {
    | Some(r) => [r(indent)]
    | None => []
    };
  let createSubtree =
    renderTree(~indent=indent + 1, ~nodeRenderer, ~emptyRenderer);
  switch (t) {
  | Empty => empty
  /* If the node is closed OR has no children
     only draw the parent do not render its children */
  | Node({status: Open, _} as x, [])
  | Node({status: Closed, _} as x, _) => [drawNode(x)]
  | Node(current, [n, ...rest]) =>
    let grandChildren =
      List.fold_left(
        (accum, i) => {
          let subtreeNode = createSubtree(i);
          List.concat([accum, subtreeNode]);
        },
        [],
        rest,
      );
    let firstChild = createSubtree(n);
    let currentGeneration = [drawNode(current), ...grandChildren];
    List.concat([currentGeneration, firstChild]);
  };
};

let make = (~tree, ~nodeRenderer, ~emptyRenderer) =>
  component((_slots: React.Hooks.empty) => {
    let componentTree = renderTree(tree, ~nodeRenderer, ~emptyRenderer);
    <View> ...componentTree </View>;
  });

/*
   Cannot set a default argument for the node renderer as this will
   narrow down the type signature of the "tree" to whaterver type the
   default takes making it no longer generalisable
 */
let createElement =
    (~tree, ~nodeRenderer, ~emptyRenderer=None, ~children as _, ()) =>
  React.element(make(~tree, ~nodeRenderer, ~emptyRenderer));
