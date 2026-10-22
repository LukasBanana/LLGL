# AGENTS.md — BrickPlay Example

## Project context

This folder contains a small 3D toy called "BrickPlay" that lets users build worlds using building blocks referred to as "bricks" of various dimensions and colors.
They are placed in a 3D grid encoded via a kd-Tree representation to record occupied cells, perform ray-casting for user selection, and resolve camera collision detection.

## File Formats

BrickPlay uses two primary custom file formats for loading its configuration and loading/saving user generated worlds.
Both of these file formats follow a similar syntax as the Wavefront Object 3D file format.
They are parsed line-by-line whereas empty lines and lines starting with the hashtag character `#` are ignored as whitespaces/comments.
Statement lines begin with a single character specifying a command, followed by their parameters.

All numeric fields for bricks, grid, and bounding volume are in signed integers, only the camera setting uses floating-point numbers.
Filenames must not be wrapped in quotes and can therefore not contain whitespaces; They are parsed via the C++ stream `>>` operators.

### Configuration File Format

The configuration file *BrickPlay.config.txt* describes brick types, their identifier, dimensions,
where to load their 3D models (.obj format only) and into which group to load them,
and optionally which cells of their bounding box to ignore to allow shapes beyond basic 3D boxes (such as approximated/voxelated slope bricks).

These are the configuration file commands:
- `b ID W H D FILE [GROUP]` defines a brick named `ID` of dimension WxHxD and loads its .obj file into the specified group (or default group),
  e.g. `b 1x1x3  1 3 1  Bricks/Brick1x1x3.obj` loads "Brick1x1x3.obj" into the default group for a brick identified as "1x1x3".
  These identifiers are used in the following scene file format. `H` (height) and `D` (depth) might be swapped between the
  identifier/filename and their dimensional description.
  Multiple of these statements can be used for the same identifier to load multiple 3D models into various groups for the same brick ID.
  Valid groups are:
  - `Default` (or omitted) for the primary rendering of the model.
  - `WallsOnly` for an alternative model when the brick is blocked in the grid top and bottom.
  - `Transparent` for an alternative model that looks more appealing with transparent materials.
    This is not required for transparent bricks, but they are preferred.
- `e ID X Y Z` excludes the local cell at (X,Y,Z) from the brick specified by ID,
  e.g. `e 1x1x3 0 1 0` excludes the center cell of the 1x1x3 brick defined above.
- `v W H D` defines the bounding volume of the scene grid,
  e.g. `v 100 50 80` defines the grid size as 100 cells wide by 50 cells high by 100 cells deep.
  This defines the limits of all further grid coordaintes into the half-open ranges X `[-100/2, +100/2)`, Y `[0, +50)`, Z `[-80/2, +80/2)`.
- `p GRAYSCALE`/`p RGB`/`p RGBA` adds an entry to the color palette, either as gray-scaled value (e.g. `FF` for white),
  RGB value (e.g. `00FF00` for green), or RGBA value (e.g. `FFFF0080` for transparent yellow). They *must* be written as hex codes.
  They will appear as coloring options to the user but are independent of the color palettes defined in the scene file format.

3D models must match the dimensions specified in the configuration file.
Their bounding volumes always conform to the following formula (in pseudocode):
```
min = (X := -floor(W/2), Y := 0, Z := -floor(D/2))
max = (X := ceil(W/2), Y := H, Z := ceil(D/2))
```
Slopes must always fall from a smaller to higher value in each dimension, e.g. the '2x2_roof_c' brick, which looks like a corner piece for a roof,
has its highest point in the 3D model at (X := -1, Z := -1) and the lowest point at (X := +1, Z := +1).

### Scene File Format

User generated worlds are saved/loaded from the *Scenes/* subfolder.

These are the scene file commands:
- `c X Y Z PITCH YAW` defines the camera position at position (X,Y,Z) and with rotation PITCH (X-axis) and YAW (Y-axis) in degrees.
  This *should* appear exactly once as it overrides the previous camera setting.
- `p` command uses the same syntax as described above in the configuration file format section.
  The order in which these appear in the scene files determines how they are referenced in the following instancing command `i`.
  These commands describe color palette entries for the scene itself, while the one described in the previous section
  describe what users can select to paint their scene in the user interface.
  If more colors from the configuration are added or removed, the scene files will be unaffected.
- `b ID` makes the brick ID current, e.g. `b 1x1x3` causes all subsequent `i` commands to produce brick instances identified as "1x1x3" from the configuration file.
- `i X Y Z ROTATION COLOR` places a new brick instance into the scene at grid position (X,Y,Z).
  - `ROTATION` must be `0`, `90`, `180`, or `270` for their respective Y-axis rotation (in degrees).
  - `COLOR` defines the instance color and must be defined in one of the following patterns:
    - `p<INDEX>`, e.g. `p0` for the first color palette entry from the beginning of the scene file.
    - `GRAYSCALE`/`RGB`/`RGBA` just like the color palette values.
      These instances will receive the color with the closest 'distance' to the existing color palette from the configuration file;
      No new color can be described this way, but it prevents scenes from breaking entirely when the color palette changes.
- `o X Y Z` is an optional command not used by the scene writer that defines an offset to all subsequent instancing commands `i`.
  The offset (X,Y,Z) is added to the grid positions and allows users to quickly modify their scenes in a text editor.

