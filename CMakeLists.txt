cmake_minimum_required(VERSION 3.1)
project(microcrunch C)

set(CMAKE_C_STANDARD 99)

set(HEADERS
	src/crnlib.h
	src/stb_dxt.h
	src/stb_image.h)

set(SOURCES
	src/main.c)

add_executable(microcrunch ${SOURCES} ${HEADERS})
target_compile_options(microcrunch PUBLIC -fno-strict-aliasing -fwrapv)
