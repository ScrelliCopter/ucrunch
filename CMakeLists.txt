cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
set(TARGET ucrunch)
project(${TARGET}
	HOMEPAGE_URL https://github.com/ScrelliCopter/ucrunch
	LANGUAGES C)
set(CMAKE_C_STANDARD 99)

set(HEADERS
	src/crnlib.h
	src/stb_dxt.h
	src/stb_image.h)
set(SOURCES
	src/main.c)

add_executable(${TARGET} ${SOURCES} ${HEADERS})
target_compile_options(${TARGET} PUBLIC -fno-strict-aliasing -fwrapv)
