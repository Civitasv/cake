# cake-build

## NAME

cake-build -- Compile the current package

## SYNOPSIS

`cake build [options]`

## DESCRIPTION

Compile local packages and all of their dependencies.

### CMake file api

Cake uses [cmake-file-api](https://cmake.org/cmake/help/latest/manual/cmake-file-api.7.html) to retrieve metadata of project.

- It uses v1 shared stateless query files.

## OPTIONS

### Target Selection

When no target selection options are given, `cake build` will build all binary and library targets.

`--lib`: Build the specified library.

`--bin`: Build the specified binary.

### Common Options

`--config` *KEY=VALUE*

All options will send to generation process to cmake, using `-DKEY=VALUE`.

`--help`: Prints help information.

## ENVIRONMENT

## EXAMPLES

