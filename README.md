# gamecore

`CvGameCoreDLL/` is the Civilization IV: Beyond the Sword 3.19 SDK source (the game DLL
Firaxis released for modding), imported unmodified in the first commit that adds it and
then ported to x64 / modern MSVC / Python 2.7 for openciv4. It remains under Firaxis'
SDK terms; see the game's SDK documentation.

`compat/` holds what replaces the SDK's bundled third-party code:

* `compat/boost/python*.hpp`: the subset of Boost.Python 1.32 used by the SDK (`class_`,
  `bases`, `def`, `enum_`, return value policies, `tuple`/`list`), implemented on pybind11,
  so the `Cy*Interface*.cpp` binding files and mod DLLs compile unchanged.
* `compat/hash_map`: `stdext::hash_map` as an alias of `std::unordered_map`.

Porting notes (what differs from the original DLL):

* `stdext::hash_map` iteration order differs from VC7.1's, which may change the order some
  loops visit vote sources, events and turn scores.
* Floating point is SSE2 (x64) instead of x87, so float-based AI evaluations can round
  differently from the original 32-bit DLL.
