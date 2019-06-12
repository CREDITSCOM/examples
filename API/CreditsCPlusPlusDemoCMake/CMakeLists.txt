cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(libsodium_dir ${CMAKE_CURRENT_SOURCE_DIR}/libsodium/)

if(WIN32)
  if(${MSVC_TOOLSET_VERSION} MATCHES 142 OR NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.20)
    set(libsodium_patch ${libsodium_dir}/../patches/msvc142_retargeting.patch)
  else()
    if(${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION} MATCHES 10.0.17763.0)
	    set(libsodium_patch ${libsodium_dir}/../patches/msvc141_retargeting17763.patch)
    else()
      set(libsodium_patch ${libsodium_dir}/../patches/msvc141_retargeting17134.patch)
    endif()
  endif()

  execute_process(COMMAND git apply --ignore-space-change ${libsodium_patch}
                  WORKING_DIRECTORY ${libsodium_dir})
				  
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(msbuild_property "/p:Configuration=Debug")
  else()
    set(msbuild_property "/p:Configuration=Release")
  endif()
  
  set(msbuild_parallel "/m")
  set(msbuild_platform "/p:platform=x64")
  set(proj_to_build "libsodium.sln")
  set(disable_post_build "/p:PostBuildEventUseInBuild=false")
  
  execute_process(COMMAND ${CMAKE_VS_MSBUILD_COMMAND} ${msbuild_parallel} ${proj_to_build} ${msbuild_property} ${msbuild_platform} ${disable_post_build}
				  WORKING_DIRECTORY ${libsodium_dir})
				  
  execute_process(COMMAND git apply --ignore-space-change ${libsodium_patch} -R
                  WORKING_DIRECTORY ${libsodium_dir})

  set(lb_lib_folder .)
else()

  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(libsodium_lib_dir ${libsodium_dir}/Build/Debug/x64/)
  else()
    set(libsodium_lib_dir ${libsodium_dir}/Build/Release/x64/)
  endif()

  if(EXISTS ${libsodium_lib_dir}/lib/libsodium${CMAKE_STATIC_LIBRARY_SUFFIX})
    message(STATUS "Libsodium lib has already been built")
  else()
    execute_process(COMMAND bash ./autogen.sh
                    WORKING_DIRECTORY ${libsodium_dir})
    execute_process(COMMAND ./configure --prefix=${libsodium_lib_dir}
                    WORKING_DIRECTORY ${libsodium_dir})
    execute_process(COMMAND make
                    WORKING_DIRECTORY ${libsodium_dir})
    execute_process(COMMAND make check
                    WORKING_DIRECTORY ${libsodium_dir})
    execute_process(COMMAND make install 
                    WORKING_DIRECTORY ${libsodium_dir})
  endif()
    
  set(lb_lib_folder lib)
endif()

add_library(libsodium STATIC IMPORTED GLOBAL)
set_property(TARGET libsodium PROPERTY IMPORTED_LOCATION_RELEASE
             ${libsodium_dir}/Build/Release/x64/${lb_lib_folder}/libsodium${CMAKE_STATIC_LIBRARY_SUFFIX})
set_property(TARGET libsodium PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO
             ${libsodium_dir}/Build/Release/x64/${lb_lib_folder}/libsodium${CMAKE_STATIC_LIBRARY_SUFFIX})
set_property(TARGET libsodium PROPERTY IMPORTED_LOCATION_DEBUG
             ${libsodium_dir}/Build/Debug/x64/${lb_lib_folder}/libsodium${CMAKE_STATIC_LIBRARY_SUFFIX})

set_property(TARGET libsodium PROPERTY IMPORTED_LOCATION_RELSPAM
             ${libsodium_dir}/Build/Release/x64/${lb_lib_folder}/libsodium${CMAKE_STATIC_LIBRARY_SUFFIX})
set_property(TARGET libsodium PROPERTY IMPORTED_LOCATION_RELMONITOR
             ${libsodium_dir}/Build/Release/x64/${lb_lib_folder}/libsodium${CMAKE_STATIC_LIBRARY_SUFFIX})
set_property(TARGET libsodium PROPERTY IMPORTED_LOCATION_RELWALLET
             ${libsodium_dir}/Build/Release/x64/${lb_lib_folder}/libsodium${CMAKE_STATIC_LIBRARY_SUFFIX})

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

find_package(Boost 1.68 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories("./thrift/lib/cpp/src")
include_directories("./libsodium/src/libsodium/include")

add_executable(main
	./api/api_constants.h
	./api/api_types.h
	./api/API.h
	./common/base58.h
	./api/general_constants.h
	./api/general_types.h
	keys.h
	ac.h
	./api/api_constants.cpp
	./api/api_types.cpp
	./api/API.cpp
	./common/base58.cpp
	./api/general_constants.cpp
	./api/general_types.cpp
	./common/variant_aux.cpp
	keys.cpp
	ac.cpp
	main.cpp
)

target_link_libraries(main ${Boost_LIBRARIES})
target_link_libraries(main libsodium)
target_compile_definitions(main PUBLIC SODIUM_STATIC)
target_link_libraries(main thrift_static)
