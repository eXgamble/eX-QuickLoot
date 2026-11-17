# SkyUI stubs for compiling the scripts without CI

The release workflow compiles the Papyrus scripts with Caprica against SkyUI's SDK sources. Without CI, the Creation Kit's
PapyrusCompiler can build them against these compile-only stubs (generated from the SkyUI SDK signatures, defaults
included: Papyrus bakes default arguments into the caller, so they must match SkyUI's) plus SKSE's and PapyrusUtil's
script sources. The stubs never ship: SkyUI provides the real scripts at runtime.
