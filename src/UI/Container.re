type state = {
  rendered: React.RenderedElement.t(React.reveryNode, React.reveryNode),
  previousElement: React.element(React.reveryNode),
};

type t = {
  node: React.reveryNode,
  state: option(state),
};

let create: React.reveryNode => t = n => {node: n, state: None};

let update: (t, React.element(React.reveryNode)) => t =
  ({node, state}, element) => {
    let newRendered =
      switch (state) {
      | None =>
        let updates =
          React.RenderedElement.render(
            {
              node,
              insertNode: React.insertNode,
              deleteNode: React.deleteNode,
              moveNode: React.moveNode,
            },
            element,
          );
        React.RenderedElement.executeHostViewUpdates(updates) |> ignore;
        updates |> React.RenderedElement.executePendingEffects;
      | Some(s) =>
        let nextElement =
          React.RenderedElement.update(
            ~previousElement=s.previousElement,
            ~renderedElement=s.rendered,
            element,
          );
        let nextElement =
          React.RenderedElement.flushPendingUpdates(nextElement);

        React.RenderedElement.executeHostViewUpdates(nextElement) |> ignore;

        React.RenderedElement.executePendingEffects(nextElement);
      };

    let ret: t = {
      node,
      state: Some({rendered: newRendered, previousElement: element}),
    };
    ret;
  };
