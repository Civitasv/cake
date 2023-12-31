add_executable(cake cake.cc utility/common.cc cmake/file_api.cc log/log.cc)
target_include_directories(cake PUBLIC ${CMAKE_SOURCE_DIR}/include)

