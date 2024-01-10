add_executable(TheModule)
target_sources(TheModule PUBLIC
	TheModule.cpp
	PUBLIC
	FILE_SET CXX_MODULES
	FILES
	${CMAKE_CURRENT_SOURCE_DIR}/modules/TheModule.cpp)
