# gamecore

`CvGameCoreDLL/` is the Civilization IV: Beyond the Sword 3.19 SDK source (the game DLL
Firaxis released for modding), imported unmodified in the first commit that adds it and
then ported to x64 / modern MSVC / Python 2.7 for openciv4. It remains under Firaxis'
SDK terms; see the game's SDK documentation.

`compat/` holds what replaces the SDK's bundled third-party code:

* `compat/boost/python*.hpp`: the subset of Boost.Python 1.32 used by the SDK (`class_`,
  `bases`, `def`, `enum_`, return value policies, `tuple`/`list`), implemented on pybind11,
  so the `Cy*Interface*.cpp` binding files and mod DLLs compile unchanged.
* `compat/hash_map`: `stdext::hash_map` behaving like the VC7.1 one the original DLL was
  built with (linear hashing, `key ^ 0xDEADBEEF`, keys ascending within a bucket), so maps
  iterate in the original order: `CvPlayer::doEvents` expires events in that order, and
  saves list maps in it. Reconstructed from original saves and checked against them by
  `tests/hash_map_test.cpp`; a BtS save loaded and saved again comes out byte-identical
  apart from what the DLL itself changes on load (session counter, reload replay message).

Boost.Python behaviours the SDK depends on and the compat layer reproduces: enums are
`int` subclasses with `names`/`values` (picklable as ints), and `make_tuple` copies the
object behind a raw pointer (`CyPlayer::firstCity` deletes it right after).

Porting notes (what differs from the original DLL):

* String-keyed `stdext::hash_map`s (info and type lookups) hash with FNV-1a instead of
  VC7.1's string hash; the game never iterates them, so only lookup speed differs.
* Floating point is SSE2 (x64) instead of x87, so float-based AI evaluations can round
  differently from the original 32-bit DLL.
* The DLL tests some Python callback results without initialising them (event trigger
  checks such as `PythonCanDoCity` read `lResult` after callbacks that may return
  `None`); the engine must always write the result. The host converts like
  `PyInt_AsLong`, so `None` gives -1 (true), as the original evidently does.

Changes for speed (game results unchanged: identical per-turn sync checksums, also across
save and load):

* Built with link-time code generation (`OC4_LTO`, on by default): the AI spends much of a
  turn in small accessors defined in other files. About -40% turn time on its own.
* `pathValid` tests the danger of the node being expanded once per path search instead of
  once per neighbour (`g_iPathSearch`, `CvPlayerAI::AI_isPlotDangerous`, which also stops
  at the first danger found).
* `CvGame::countCivTeamsEverAlive` counts with a flag array instead of building a
  `std::set` on every call.
* `stdext::hash_map<std::string, ...>::find(const char*)` does not build a `std::string`
  (`CvGlobals::getDefineINT`, `getInfoTypeForString`).
