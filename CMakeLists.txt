cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
set(TARGET ucrunch)
project(${TARGET}
	HOMEPAGE_URL https://github.com/ScrelliCopter/ucrunch
	LANGUAGES C)
set(CMAKE_C_STANDARD 99)

set(HEADERS
	src/util.h
	src/crnlib.h
	src/stb/stb_dxt.h
	src/stb/stb_image.h
	src/stb/stb_image_resize.h
	src/dds.h
	src/cmdparse.h)
set(SOURCES
	src/util.c
	src/stb/stb_impl.c
	src/dds.c
	src/cmdparse.c
	src/main.c)

add_executable(${TARGET} ${SOURCES} ${HEADERS})
target_compile_options(${TARGET} PUBLIC -Wall -Wextra -fno-strict-aliasing -fwrapv)
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/stb)
target_link_libraries(${TARGET} m)
