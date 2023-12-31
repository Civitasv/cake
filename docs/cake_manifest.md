# cake-manifest

The `Cake.toml` file for each package is called its *manifest*.

Mainly used to replace `cmake-kits` and `cmake-variants` in cmake-tools of vscode.

## The Manifest Format

- `[package]` : Defines a package.
    - `name` : The name of the package.
    - `version` : The version of the package.
    - `authors` : The authors of the package.
    - `description` : A description of the package.
    - `license` : The license of the package.
    - `default-run` : Specift the default binary picked by `cake run`.
- `[profile]` : Compiler settings and optimizations.
    - `c_compiler` : The compiler for C.
    - `cxx_compiler` : The compiler for C++.
    - `linker` : The linker.
    - `debugger` : The debugger.
    - `vcpkg` : if support vcpkg.
    - `build-type` : The global settings of build type, including "Debug", "Release", "RelWithDebInfo", "MinSizeRel".
    - `build-directory` : The build directory.
    - `compile-commands` : Whether geneate the compile commands json file.

## The manifest file for Cake itself

```toml
[package]
name = "cake"
version = "0.0.1"
authors = "Civitasv"
description = "Cake, CMake without a mess"
license = "MIT"
default-run = "cake"

[profile]
compiler = "g++"
linker = "lld"
```
