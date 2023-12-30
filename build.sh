cmake -S . -B out -DCMAKE_TOOLCHAIN_FILE=./packages/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1
cmake --build out
