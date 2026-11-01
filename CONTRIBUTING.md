# Contributing

**Issues go to the main repository: <https://github.com/bssth/openciv4/issues>.** Bugs in
this DLL, problems porting a mod, questions about the engine: all of them there, with
`openciv4-dll` in the title. This repository does not track issues.

Pull requests with fixes to the port or the compat layer are welcome here. Keep the rules
the port follows ([docs/SDK-PORT.md](docs/SDK-PORT.md)):

* game results unchanged: the same seed plays the same game (identical per-turn sync
  checksums) before and after the change, unless the change fixes a bug that made it differ;
* the save format unchanged: original saves still load;
* each changed line in `CvGameCoreDLL/` marked with an `openciv4` comment, the SDK's code
  otherwise left as it is.
