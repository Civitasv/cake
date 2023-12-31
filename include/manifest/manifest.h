#ifndef CAKE_MANIFEST_H_
#define CAKE_MANIFEST_H_

#include "cake.h"
#include "utility/toml.hpp"

#define MANIFEST_FILE "Cake.toml"

using Manifest = toml::table;

Manifest ParseManifest();

BuildConfig ParseBuildConfigFromManifest();

RunConfig ParseRunConfigFromManifest();

InstallConfig ParseInstallConfigFromManifest();

DebugConfig ParseDebugConfigFromManifest();

#endif // CAKE_MANIFEST_H_
