cmake -S . -B out -DCMAKE_TOOLCHAIN_FILE=./packages/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DVCPKG_MANIFEST_DIR=./packages -DVCPKG_MANIFEST_INSTALL=OFF -DVCPKG_INSTALLED_DIR=./packages/vcpkg_packages
cmake --build out
