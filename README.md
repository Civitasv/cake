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
  - [x] [cake debug](./docs/cake_debug.md)
  - [x] [cake manifest support](./docs/cake_manifest.md)
  - [x] [cake docs](./docs/cake_docs.md)
- [x] Proper package management (with vcpkg, manifest mode).
  - [x] [cake install](./docs/cake_install.md)
  - `packages/vcpkg_packages` : store the packages installed by vcpkg.
  - `packages/other_packages` : vcpkg\_packages should use `find_package` to link, other\_packages should only use `target_include_directories` and `target_link_libraries` to install.
- [x] Create template.
  - [x] [cake create](./docs/cake_create.md)

## Reference

- [json](https://github.com/nlohmann/json) : for json parser.
- [cxxopts](https://github.com/jarro2783/cxxopts) : for command line options parser.
