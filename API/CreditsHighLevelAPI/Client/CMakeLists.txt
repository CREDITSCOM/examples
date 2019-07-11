cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_definitions(-DNOMINMAX)
endif()

add_definitions(-DUSE_STD_THREAD)
if (MSVC)
  option(BUILD_COMPILER "" OFF)
endif(MSVC)
option(WITH_BOOST_STATIC "" ON)
option(BUILD_TESTING "" OFF)
option(BUILD_TUTORIALS "" OFF)
option(BUILD_EXAMPLES "" OFF)
option(BUILD_PYTHON "" OFF)
option(WITH_SHARED_LIB "" OFF)
option(WITH_STATIC_LIB "" ON)
option(WITH_C_GLIB "" OFF)
option(WITH_JAVA "" OFF)
option(WITH_PYTHON "" OFF)
option(WITH_QT4 "" OFF)
option(WITH_QT5 "" OFF)
option(WITH_ZLIB "" OFF)
option(WITH_STDTHREADS "" ON)
option(WITH_LIBEVENT "" OFF)
option(WITH_OPENSSL "" OFF)
add_subdirectory(thrift)

find_package(Boost 1.70 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories("./thrift/lib/cpp/src")

add_executable(main
	client.h
	client.cpp
	./hlapi/hlapi_constants.h
	./hlapi/hlapi_types.h
	./hlapi/API.h
	./hlapi/hlapi_constants.cpp
	./hlapi/hlapi_types.cpp
	./hlapi/Api.cpp
	main.cpp
)

target_link_libraries(main ${Boost_LIBRARIES})
target_link_libraries(main thrift_static)
