# Cake, CMake without *mess*

> Still exploring the possiblities...

[![asciicast](https://asciinema.org/a/0CtIOkmDnP5f6w7cS2x0K6cqO.svg)](https://asciinema.org/a/0CtIOkmDnP5f6w7cS2x0K6cqO)

Bring C++ the power of *Cargo*, based on *CMake*.

## Goals

- Make build system of C/C++ acceptable.
- Make package management of C/C++ easier.

## Non goals

- Replace cmake.
- Performance.

Including:

- [x] Proper build system (with cmake).
  - [x] [cake build](./docs/cake_build.md)
  - [x] [cake run](./docs/cake_run.md)
  - [ ] [cake manifest support](./docs/cake_manifest.md)
  - [ ] [cake docs](./docs/cake_docs.md)
- [x] Proper package management (with vcpkg, manifest mode).
- etc.

## Reference

- [json](https://github.com/nlohmann/json) : for json parser.
- [cxxopts](https://github.com/jarro2783/cxxopts) : for command line options parser.
