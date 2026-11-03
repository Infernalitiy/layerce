# Primary sources and provenance

Checked during initial development in November 2026. Website documentation can
change; the build workflow pins toolchain release v15.0. Check the installed
package headers, linker script and actual emitted map when building.

- CE toolchain official v15.0 release and assets:
  https://github.com/CE-Programming/toolchain/releases/tag/v15.0
  Release notes identify LLVM/Clang 19 and GNU assembler/linker migration.
- Official Windows setup / structure / build instructions:
  https://ce-programming.github.io/toolchain/static/getting-started.html
- Published native memory regions:
  https://ce-programming.github.io/toolchain/static/faq.html
  The FAQ's compiler-version line is stale relative to v15 release notes;
  prefer the pinned package/release for compiler version.
- Official GraphX API, VRAM/buffering, clipping and palette:
  https://ce-programming.github.io/toolchain/libraries/graphx.html
- Official keypad API, continuous scanning, key names and restoration:
  https://ce-programming.github.io/toolchain/libraries/keypadc.html
- Official clock/timer guidance (use clock() rather than commandeering timers):
  https://ce-programming.github.io/toolchain/headers/sys/timers.html
- Official user RAM function semantics:
  https://ce-programming.github.io/toolchain/headers/ti/vars.html
- Official make options, compression distinction, map output:
  https://ce-programming.github.io/toolchain/static/makefile-options.html
- Matching CE libraries release:
  https://github.com/CE-Programming/libraries/releases/tag/v15.0
- arTIfiCE developer compatibility/tutorial:
  https://yvantt.github.io/arTIfiCE/
  v2.1 for CE OS through 5.8.4; v3 is a different route for 5.8.5.
- CEaShell developer installation and release repository:
  https://github.com/RoccoLoxPrograms/CEaShell
  https://github.com/RoccoLoxPrograms/CEaShell/releases
- Official CEmu repository and ROM requirement:
  https://github.com/CE-Programming/CEmu
- Build workflow actions:
  https://github.com/actions/checkout
  https://github.com/actions/upload-artifact

No downloaded third-party code, libraries, calculator ROM, OS bundle, launcher
binary or original BTD6 asset is redistributed here. Game source and geometric
visuals were authored for this project. Names referring to other products in
documentation identify compatibility/inspiration and do not imply affiliation.
