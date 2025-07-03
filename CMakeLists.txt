cmake_minimum_required(VERSION 3.22)
project(qrustyquake LANGUAGES C HOMEPAGE_URL "https://github.com/cyanbun96/qrustyquake/" VERSION 0.5.1)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

file(GLOB QRUSTYQUAKE_HEADERS "${PROJECT_SOURCE_DIR}/src/*.h")
file(GLOB QRUSTYQUAKE_SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")

add_executable(qrustyquake)

if(WIN32 OR MSVC OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
	list(FILTER QRUSTYQUAKE_SOURCES EXCLUDE REGEX "net_udp.c")
	target_link_libraries(qrustyquake PRIVATE ws2_32)
else()
	list(FILTER QRUSTYQUAKE_SOURCES EXCLUDE REGEX "net_wins.c")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Haiku")
	target_link_libraries(qrustyquake PRIVATE network)
endif()

target_sources(qrustyquake PRIVATE ${QRUSTYQUAKE_SOURCES})

target_include_directories(qrustyquake PRIVATE "${PROJECT_SOURCE_DIR}/src/")
target_link_libraries(qrustyquake PRIVATE SDL3::SDL3)

find_library(libmath m)
if(libmath)
	target_link_libraries(qrustyquake PRIVATE ${libmath})
endif()
