add_executable(cake cake.cc common.cc file_api.cc log.cc)
target_include_directories(cake PUBLIC ${CMAKE_SOURCE_DIR}/include)
