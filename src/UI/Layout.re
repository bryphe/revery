open Revery_Core;

open Flex;

module Node = {
  type context = ref(int);
  /* Ignored - only needed to create the dummy instance. */
  let nullContext = {contents: 0};
};

let rootContext = {contents: 0};

module Encoding = FixedEncoding;
module LayoutTestUtils = LayoutTestUtils.Create(Node, Encoding);
module Layout = Layout.Create(Node, Encoding);
module LayoutPrint = LayoutPrint.Create(Node, Encoding);
module LayoutSupport = Layout.LayoutSupport;
module LayoutTypes = LayoutTypes.Create(Node, Encoding);

let defaultStyle = LayoutSupport.defaultStyle;
let createNode = (children, style) =>
  LayoutSupport.createNode(
    ~withChildren=children,
    ~andStyle=style,
    rootContext,
  );
let createNodeWithMeasure = (children, style, measure) =>
  LayoutSupport.createNode(
    ~withChildren=children,
    ~andStyle=style,
    ~andMeasure=measure,
    rootContext,
  );
let updateCachedNode = (node: LayoutTypes.node) => {
        node
       /* ...node, */
        /* isDirty: true, */
        /* isDirty: true, */
        /* selfRef: Nativeint.zero, */
        /* hasNewLayout: true, */
        /* isDirty: true, */
        /* lineIndex: 0, */
        /* layout: LayoutSupport.createLayout(), */
        /* parent: LayoutSupport.theNullNode, */
        /* nextChild: LayoutSupport.theNullNode, */
}


let layout = (node, pixelRatio, scaleFactor) =>
  Performance.bench("layout", () => {
    let layoutNode = node#toLayoutNode(pixelRatio, scaleFactor);
    switch (layoutNode) {
    | None => ()
    | Some(v) =>
      Layout.layoutNode(v, Encoding.cssUndefined, Encoding.cssUndefined, Ltr)
    };
  });
let printCssNode = root =>
  LayoutPrint.printCssNode((
    root,
    {printLayout: true, printChildren: true, printStyle: true},
  ));
