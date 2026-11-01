# What the engine expects of a DLL

openciv4's engine (`oc4core.exe`) treats the game DLL the way the original
`Civ4BeyondSword.exe` does: it is compiled against the SDK's headers, calls only what the DLL
exports, and gives the DLL the same surroundings the original process gave it. A DLL that
worked with the original engine and is ported to x64 (see [PORTING.md](PORTING.md)) therefore
needs no openciv4-specific code. This page lists the contract, so that you know what a mod
must not change and what the engine does for it.

Questions and reports: <https://github.com/bssth/openciv4/issues>.

## The interface

**Exports.** The engine imports about 450 functions from `CvGameCoreDLL.dll` by their x64
mangled names: the SDK's `DllExport` functions it uses, plus a few the original engine did
not use but openciv4 does (help texts for plots and units, for example). Each must exist with
the SDK's signature, under the SDK's name (directly or through a `.def` alias).
`tools/check-exports.ps1` checks a DLL against an engine.

**Classes the engine creates itself.** The engine constructs a few of the DLL's classes in
its own memory, sized by the SDK's headers, exactly as the original engine did:
`CvXMLLoadUtility`, `CvPopupInfo`, `CvTalkingHeadMessage`, `CvFractal`, `IDInfo`, and the
data classes it passes to the DLL (`CvWidgetDataStruct`, `CvGlobeLayerData`,
`CvPlotIndicatorData`, ...). A mod must not add members to these or change their layout
(the original engine had the same rule; a mod that broke it crashed there too).

**Inline accessors.** The engine reads a few classes through accessors defined in the headers,
compiled into the engine, so it depends on their layout: `CvInitCore`'s getters (the game
setup). Keep `CvInitCore`'s SDK members in place, with their types; add new ones after them.
`gDLL` is reached through the exported `getDLLIFaceNonInl()`, never through `CvGlobals`'
layout, so a mod may change `CvGlobals` freely (some replace it with a proxy object).

**Counts come from the DLL.** The numbers of players and teams (`GC.getMAX_PLAYERS()`,
`getMAX_CIV_PLAYERS()`, `getMAX_TEAMS()`), info counts and the like are asked from the DLL,
so mods with more civilization slots (50, 100) work.

**Dirty flags.** The engine redraws what the DLL marks dirty, as the original does: the
interface's dirty bits (`CvInterface` via `gDLL->getInterfaceIFace()->setDirty`), and per city
`isInfoDirty` (its billboard: name, size, production) and `isLayoutDirty` (its buildings, via
`getVisibleBuildings`). A mod that changes what a city shows must set these as the SDK code
does, or the map keeps the old picture until something else marks the city.

**Saves.** The engine writes and reads the `.CivBeyondSwordSave` container (header, the
staging room `CvInitCore`, the DLL's game data, the Python script data) in the original
format; everything inside the game data is the DLL's own `read`/`write`. The mod folder is
recorded as `Mods\<folder name>\`. On load the staging room is read into
`GC.getLoadedInitCore()` as well as the game's, as the original does (mods version their
saves by what is stored there). A load that does not read exactly what was written is
refused with an error.

## The process

The DLL runs as if it were inside the original game:

* **Working folder**: the `Beyond the Sword` folder of the install. Mods open
  `Mods\<mod>\Assets\XML\...` relative to it, and BUG finds its files from there.
* **`GetModuleFileName`** places the executable at `Beyond the Sword\Civ4BeyondSword.exe` and
  the DLL at `Beyond the Sword\Mods\<mod>\Assets\CvGameCoreDLL.dll` (or the full path of a
  mod outside the Mods folder), whatever their real location. `CvInitCore::getDLLPath`,
  modular XML loading and BUG use it; Python's `sys.executable` is the same path.
* **`GetACP`** answers Windows-1252, as on the Western systems mods were written on: mods
  that convert UTF-8 texts to the ANSI code page give the right characters on any system.
* **`MessageBox`** calls of the DLL (asserts, XML errors) go to the console and log, answered
  with the default button, instead of waiting for a click.
* **`SetUnhandledExceptionFilter`** of the DLL is ignored: the engine's crash handler stays and
  prints a stack trace (a mod's minidump writer would write into the game folder).

**Order of startup**, as in the original:

1. `setDLLIFace` first (some mods construct their globals there);
2. Python started, `DLLPublishToPython`, `CvAppInterface.init()`;
3. `CvGlobals::init`, then the XML: `LoadPlayerOptions`, `LoadGraphicOptions`,
   `SetGlobalDefines`, `SetGlobalTypes`, `LoadBasicInfos`, `LoadGlobalText`,
   `SetGlobalArtDefines`, `LoadPreMenuGlobals`, `SetPostGlobalsGlobalDefines`,
   `SetupGlobalLandscapeInfo`;
4. the staging room reset (`CvInitCore::init`), as between the original's menus (some mods
   add placeholder entries at the end of `LoadPreMenuGlobals` and remove them there);
5. `LoadPostMenuGlobals`;
6. for a game: new game setup, or a load followed by `CvAppInterface.onLoad`; then
   `CvAppInterface.preGameStart()` (BUG registers its event handlers there).

At shutdown, before the engine's interface object goes away, the DLL gets one that only
allocates and frees memory, for its static destructors.

## Python

The engine runs the game's Python 2.7 scripts and the mod's from the asset layers (mod, BtS,
Warlords, vanilla), with the engine-side classes the scripts expect (`CyInterface`,
`CyGInterfaceScreen`, `CyPopup`, `CyTranslator`, `CyFractal`, ...). What matters for a DLL:

* Bindings are Boost.Python 1.32's API on pybind11 (`compat/boost/python*.hpp`), with
  Boost.Python's behaviours: enums are `int` subclasses with `names`/`values`, `std::string` and
  `char*` results are `str` (not `unicode`), `make_tuple` copies the object behind a raw pointer.
  A mod's binding files compile unchanged apart from the fixes in
  [PORTING.md](PORTING.md#3-compile-errors).
* Callback results: the DLL often tests the result of a Python callback without initialising
  it. The engine always writes the result; `None` gives -1, as in the original.
* Engine enums (`NotifyCode`, `MouseFlags`, `InputTypes`, ...) have all the names the original
  registers, and `getSymbolID` numbers the game font's symbols as the original does (BUG
  computes symbols from others).
* A mod outside the game's Mods folder gets a `CvModFolder` module with its full path, so BUG
  finds the mod's own folder (configs, INI files) instead of an installed mod of the same name.

## The command line

The options of `oc4core.exe` that matter for a port:

| Option | What it does |
|---|---|
| `--mod <folder>` | the mod: `Mods\Name` under Beyond the Sword, or a full path |
| `--civ4 <folder>` | the install (the folder with `Assets`, `Warlords`, `Beyond the Sword`), when it is not found by itself |
| `--menu` | the main menu in a window (the default with no options) |
| `--play` | play in a window (with `--load`, or a new game) |
| `--load <save>` | continue a saved game |
| `--map <script> --size <SIZE> --seed <n> --players <n>` | a new game's setup |
| `--turns <n>` | AI autoplay for that many turns, a line per turn with the sync checksum |
| `--save <file> [--save-at <turn>]` | save at the end, or at that turn of the autoplay |
| `--view [--view-seconds <s>]` | the map in a window while the AI plays |
| `--log-dir <folder>` | the DLL's own logs (`xml.log`, ...) into that folder |
| `--rand-log` | every synchronised random number logged (with `--log-dir`) |
| `--probe-save <save>` | where reading each part of a save ends |
| `--profile` | where the autoplay's time goes |
