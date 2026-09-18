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
- Every RPC comment line must start with a known convention keyword prefix (`Allowed states:`, `Description:`, `Pre-conditions:`, `Expected response:`, `Additional responses:`, `Response Sequences:`, `Triggered by:`, `Example:`) or be an indented continuation of one. Lines with no keyword prefix are silently dropped by the convention extractor. If in doubt, put the text under `Description:`.
- Use `Allowed states:` (short form) rather than `Allowed in link layer states:`. The extractor accepts both, but the short form is preferred for consistency across the proto.

If a service is added, renamed, or removed, also update:

- `documents/modules/ROOT/pages/EchoServices.adoc` (landing page)
- `documents/modules/ROOT/nav.adoc`

## Convention keywords and the two-carrier pattern

Each RPC's structured facts (allowed states, pre-conditions, expected response, response sequences, description) live in two carriers, and both must state the same fact:

1. The proto RPC comment carries the keyword-labelled form (e.g. `// Allowed states: standby`, `// Pre-conditions: optional SetSecurityMode / SetIoCapabilities`). This feeds `extract_conventions.py`, which surfaces the facts as labelled structured fields to downstream documentation renderers.
2. The adoc prose immediately above the per-method diagram on the service page carries the same fact in readable form (e.g. `*Allowed states:* standby. *Pre-conditions:* optional `SetSecurityMode` / `SetIoCapabilities``). This makes the fact visible in the Antora HTML rendering, above the diagram image.

The per-method mermaid diagram itself must NOT repeat these facts. Only the top-of-diagram multi-participant legend note (e.g. `Messages between Node and BLE Peripheral are link-layer events, not driven by the App...`) and mid-flow annotations that comment on specific arrows belong inside the mermaid. Do not add an opening `Note over App, Node:` fact block — the fact belongs above the `[mermaid]` block in the adoc, not inside it.

Both carriers must state the same fact - when updating one, update the other.

## Diagram conventions - general

- Participants: `App` (caller) and `Node` (endpoint that implements the service).
- Thin single-value wrapper messages (`BoolValue`, `Handle`, `UInt32Value`, `MtuSize`, `OperationResult`, etc.) are inlined; multi-field messages must use proto field names verbatim.
- Every App-Node arrow must carry the fully qualified method or callback name (e.g. `GapCentral.PairAndBond()`, `GapCentralResponse.PairingResult(...)`). Never use bare method names on arrows.
- Stack-dependent behavior is described generically in `Note over` blocks. Do not hard-code vendor-specific timing or values (e.g. avoid "ST timeout is 10.24 s" inside diagrams).
- Do not use `;` inside `Note` text. The Kroki mermaid renderer crashes on semicolons.
- Do not use em-dashes in prose or notes. Use plain hyphens, colons, or split into separate sentences.
- Note line length: keep each physical `<br>`-separated line no wider than the diagram's widest arrow label. For simple 1-2 arrow diagrams this typically means under ~45 characters; for composite flows with long RPC signatures it can go up to ~70. When in doubt, split more aggressively - render and inspect visually. Notes wider than the arrow span extrude past the participant boxes and, in `alt`/`opt` blocks, collide with the frame borders.
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
- SMP pairing failure does not automatically disconnect the link. On failure the middleware fires `PairingResult(pairedSuccessfully=false, reason=...)` and the link stays connected. `CurrentState(standby)` follows only via App-driven `Standby()` or a peer-driven disconnect - do not describe it as an automatic consequence of pairing failure.
