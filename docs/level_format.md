Level format
===========

This file describes the simple ASCII and JSON level formats supported by the engine's LevelLoader.

ASCII format (levels/*.lvl)
---------------------------
Each non-comment line is whitespace separated tokens. Lines starting with `#` are comments.

Base syntax:

- `cube px py pz [rot rx ry rz] [sx sy sz] [r g b]`
- `plane px py pz [rot rx ry rz] [sx sy sz] [r g b]`
- `sphere px py pz [rot rx ry rz] [sx sy sz] [r g b]`
- `cylinder px py pz [rot rx ry rz] [sx sy sz] [r g b]`
- `mesh <path> px py pz [rot rx ry rz] [sx sy sz] [r g b]`
- `door/window/platform px py pz [rot rx ry rz] [sx sy sz] [r g b]`

Notes:
- `rot` is an optional keyword followed by Euler angles in degrees (`rx ry rz`). Rotation is converted into radians internally.
- If `sx sy sz` are omitted a uniform scale of `1` is used.
- If `r g b` color values are omitted a default gray (0.8,0.8,0.8) is used.
- `mesh` paths are resolved relative to the level file location.

JSON format (levels/*.json)
---------------------------
The loader also accepts a simple JSON array of objects. Each object is a flat map of properties. Example object keys:

- `type`: string (cube, plane, sphere, cylinder, mesh, door, window, platform)
- `path`: string (only for `mesh`)
- `px, py, pz`: numbers
- `sx, sy, sz`: numbers (scale)
- `rot`: array of three numbers [rx, ry, rz] in degrees
- `r, g, b`: numbers (color)

Example:

[
  { "type": "plane", "px": 0, "py": 0, "pz": 0, "sx": 20, "sy": 1, "sz": 20 },
  { "type": "sphere", "px": 1, "py": 1, "pz": 0, "r": 1, "g": 0, "b": 0 }
]

Extensibility
-------------
- New primitive mesh generators (sphere, cylinder, wedge, stairs, torus) are included.
- A small OBJ loader is included to bring in custom meshes via the `mesh` type.
- Logical components `DoorComponent`, `WindowComponent`, and `PlatformComponent` are created when `door`, `window`, or `platform` entries are used. Game logic for these components should be implemented separately in systems.
- Rotation, scale and color are parsed and applied to `TransformComponent` and `MeshComponent`.

Limitations
-----------
- The JSON loader now uses `nlohmann::json` for robust parsing, but you must have the header available in your include path (or vendor the single-header library into `third_party`).
- The OBJ loader is minimal and supports triangular faces with `v`, `vt`, `vn` and basic `f` lines. Complex OBJ features are not handled.

If you'd like, I can: improve the OBJ loader, add more primitive generators (e.g. torus detail controls, parametric stairs), or implement simple game systems for `DoorComponent`/`PlatformComponent`.
