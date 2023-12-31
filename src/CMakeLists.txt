add_executable(cake cake.cc utility/common.cc cmake/file_api.cc log/log.cc)
target_include_directories(cake PUBLIC ${CMAKE_SOURCE_DIR}/include)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(cake PRIVATE fmt::fmt)

find_path(TERMCOLOR_INCLUDE_DIRS "termcolor/termcolor.hpp")
target_include_directories(cake PRIVATE ${TERMCOLOR_INCLUDE_DIRS})
