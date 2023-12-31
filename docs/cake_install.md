# cake-install

## NAME

cake-install -- Install library using vcpkg.

## SYNOPSIS

`cake install [options]`

## DESCRIPTION

Install specific library using vcpkg.

## OPTIONS

### Library Selection

`--port <library>` : Add specific library to vcpkg.json.

`--sync` : Install all libraries in vcpkg.json.

### Common Options

`--config` *KEY=VALUE*

All options will send to vcpkg, using `-DKEY=VALUE`.

`--help`: Prints help information.

## ENVIRONMENT

## EXAMPLES

