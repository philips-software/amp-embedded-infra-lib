---
applyTo:
  - "services/**/*.proto"
  - "documents/modules/ROOT/pages/EchoServices/**"
  - "documents/modules/ROOT/partials/echoservices/**"
  - "documents/modules/ROOT/pages/EchoServices.adoc"
---

## Keep Echo Services docs in sync with the proto

The Echo Services documentation (`documents/modules/ROOT/pages/EchoServices/` and `documents/modules/ROOT/partials/echoservices/`) describes the interface defined in the `.proto` files under `services/`. When one side changes, the other usually needs to change too.

When editing any of the files above, check that these still line up:

- Every RPC method in the proto has a matching per-method sequence diagram, and every diagram references a method that exists in the proto.
- Every field reference in diagrams uses the proto field name verbatim (no invented shortcuts like `handle=` when the proto field is `value`).
- Every enum value shown in diagrams is defined in the proto.
- Sequence diagrams reflect the `Response Sequences`, `Expected response`, and `Additional responses` documented above each proto method.
- No diagram invokes a method outside its `Allowed states`.

If a service is added, renamed, or removed, also update:

- `documents/modules/ROOT/pages/EchoServices.adoc` (landing page)
- `documents/modules/ROOT/nav.adoc`

## Diagram conventions — general

- Participants: `App` (caller) and `Node` (endpoint that implements the service).
- Thin single-value wrapper messages (`BoolValue`, `Handle`, `UInt32Value`, `MtuSize`, `OperationResult`, etc.) are inlined; multi-field messages must use proto field names verbatim.
- Stack-dependent behavior is described generically in `Note over` blocks. Do not hard-code vendor-specific timing or values (e.g. avoid "ST timeout is 10.24 s" inside diagrams).
- Do not use `;` inside `Note` text — the Kroki mermaid renderer crashes on semicolons.

## Diagram conventions — BLE-specific

- In dual-role diagrams: aliases `PeripheralNode` / `CentralNode`, displayed as `Peripheral Node` / `Central Node`.
- Peer BLE devices are drawn as separate lifelines: `BLE Peripheral` or `BLE Central`.
- Link-layer arrows between `Node` and peer BLE devices use `-)` (async fire-and-forget). Every diagram that contains such arrows carries a top-of-diagram legend note stating these events are not App-controlled.
