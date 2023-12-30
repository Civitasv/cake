# cake-manifest

The `Cake.toml` file for each package is called its *manifest*.

## The Manifest Format

- `[package]` : Defines a package.
    - `name` : The name of the package.
    - `version` : The version of the package.
    - `authors` : The authors of the package.
    - `description` : A description of the package.
    - `license` : The license of the package.
    - `default-run` : Specift the default binary picked by `cake run`.
- `[[lib]]` : Library target settings.
    - `name` : The name of the library.
    - `build-type` : The build type of the library.
    - `linkage` : Either `static` or `shared`.
- `[[bin]]` : Binary target settings.
    - `name` : The name of the binary.
    - `build-type` : The build type of the binary.
    - `args` : The arguments passed to the binary.
- `[profile]` : Compiler settings and optimizations.
    - `compiler` : The compiler.
    - `linker` : The linker.
    - `build-type` : The global settings of build type, will be overrided by `[[lib]]` and `[[bin]]`.
    - `linkage` : The global settings of linkage, either `static` or `shared`, will be overrided by `[[lib]]`.
    - `compile_commands` : Whether geneate the compile commands json file.

## The manifest file for Cake itself

```toml
[package]
name = "cake"
version = "0.0.1"
authors = "Civitasv"
description = "Cake, CMake without a mess"
license = "MIT"
default-run = "cake"

[[lib]]

[[bin]]
name = "cake"
build-type = "release"

[profile]
compiler = "g++"
linker = "lld"
```
