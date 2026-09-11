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
- When observable behaviour is consistent across independent stack implementations, that behaviour is contract, not stack quirk. If the proto text and the stacks disagree, update the proto text to match the behaviour (do not "fix" the diagram to match a stale comment).

If a service is added, renamed, or removed, also update:

- `documents/modules/ROOT/pages/EchoServices.adoc` (landing page)
- `documents/modules/ROOT/nav.adoc`

## Diagram conventions - general

- Participants: `App` (caller) and `Node` (endpoint that implements the service).
- Thin single-value wrapper messages (`BoolValue`, `Handle`, `UInt32Value`, `MtuSize`, `OperationResult`, etc.) are inlined; multi-field messages must use proto field names verbatim.
- Every App-Node arrow must carry the fully qualified method or callback name (e.g. `GapCentral.PairAndBond()`, `GapCentralResponse.PairingResult(...)`). Never use bare method names on arrows.
- Stack-dependent behavior is described generically in `Note over` blocks. Do not hard-code vendor-specific timing or values (e.g. avoid "ST timeout is 10.24 s" inside diagrams).
- Do not use `;` inside `Note` text. The Kroki mermaid renderer crashes on semicolons.
- Do not use em-dashes (`—`) in prose or notes. Use plain hyphens, colons, or split into separate sentences.
- Note line length: keep each physical `<br>`-separated line under ~75 characters. Longer lines render past the participant span and collide with `alt`/`opt` frame borders, especially when the note is `Note over` a single narrow participant.
- Colour convention for `rect rgb(...)` bands:
  - `rgb(230, 230, 240)` (blue-lavender): major section / step headers.
  - `rgb(245, 240, 230)` (beige): LL / SMP detail bands (encryption setup, key distribution, SMP procedures).
  - `rgb(245, 230, 230)` (soft pink): peer disconnect / termination.

## Diagram conventions - BLE-specific

- In dual-role diagrams: aliases `PeripheralNode` / `CentralNode`, displayed as `Peripheral Node` / `Central Node`.
- Peer BLE devices are drawn as separate lifelines: `BLE Peripheral` or `BLE Central`.
- All peer-to-Node async events use `-)` (not just LL PDUs). This covers advertisements, scan responses, ATT notifications/indications, SMP PDUs, and LL PDUs. The dashed reply `-->>` and solid sync `->>` styles are reserved for App-Node arrows. Never use `--x` (mermaid's "lost message" glyph) for a graceful disconnect.
- Every diagram that contains peer-Node arrows carries a top-of-diagram legend note stating these events are not App-controlled and are observable only via Echo callbacks.
- Peer disconnects: wrap `Central-)Node: LL_TERMINATE_IND` (or `Peripheral-)Node: LL_TERMINATE_IND` when the Node is central) inside a `rect rgb(245, 230, 230)` band, matching the colour convention above.
- Prefer spec PDU names (`LL_TERMINATE_IND`, `ATT_HANDLE_VALUE_NTF`, `Pairing_Request`, `LL_ENC_REQ`, `CONNECT_IND`) over abstract labels (`Disconnect`, `Notification`, `Pair request`). A reader with a sniffer trace should see the same names on both sides.
- `Standby()` allowed-states notes must include every state the proto allows, including `standby` itself. Peripheral: `standby, advertising, connected`. Central: `standby, scanning, connected, initiating`. Calling from `standby` still yields `CurrentState(standby)`.
