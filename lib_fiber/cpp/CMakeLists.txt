cmake_minimum_required(VERSION 3.2.0)
#cmake_minimum_required(VERSION 2.8.0)
project(fiber_cpp)

#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Release CACHE STRING "set build type to release")

if (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	message(FATAL_ERROR "Please into another dir to build!")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Android")
	# xxx: -flto can't be used on NDK with gnustl_shared
	if (ANDROID_STL MATCHES "gnustl_shared")
		add_definitions("-Oz -g")
	else()
		add_definitions("-O3 -flto")
	endif()

	add_definitions("-DANDROID")
	add_definitions("-Wno-unused-command-line-argument")
	add_definitions("-fdata-sections -ffunction-sections")
	string(APPEND CMAKE_CXX_FLAGS "-Qunused-arguments")
	set(UNIX_OS true)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions("-O2")
	set(UNIX_OS true)
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_definitions("-O2")
	set(UNIX_OS true)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
	add_definitions("-Wno-invalid-source-encoding")
	add_definitions("-fdata-sections -ffunction-sections")
	add_definitions("-flto")
	add_definitions("-Os")
	set(UNIX_OS true)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(CMAKE_SHARED_LINKER_FLAGS "/DYNAMICBASE ws2_32.lib")
	set(WIN_OS true)
else()
	message(FATAL_ERROR "unknown CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
endif()

if (${UNIX_OS})
	add_definitions(
		"-W"
		"-fPIC"
		"-Wall"
		"-Werror"
		"-Wshadow"
		"-Wformat"
		"-Wpointer-arith"
		"-D_REENTRANT"
		"-D_USE_FAST_MACRO"
		"-Wno-long-long"
	#	"-Wuninitialized"
		"-D_POSIX_PTHREAD_SEMANTICS"
	#	"-Wno-invalid-source-encoding"
	#	"-Wno-unused-private-field"
		"-Wno-unused-parameter"
	#	"-Wno-unused-const-variable"
		"-fexceptions"
	)

	if (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
		add_definitions("-O3")
	else()
		add_definitions("-g")
	endif()
elseif (${WIN_OS})
	add_definitions(
		"-D_WINSOCK_DEPRECATED_NO_WARNINGS"
		"-DACL_WRITEABLE_CHECK"
		"-DACL_PREPARE_COMPILE")
	if (ACL_BUILD_SHARED MATCHES "YES")
		add_definitions("-DFIBER_CPP_DLL -DFIBER_CPP_EXPORTS"
			"-DACL_DLL -DHTTP_DLL -DICMP_DLL -DSMTP_DLL -DACL_CPP_DLL -DFIBER_DLL"
		)
	endif()
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
	message(STATUS "The compiler supports C++11.")
	add_definitions("-std=c++11")
endif()

#if (CMAKE_BUILD_TYPE STREQUAL "")
#	set(CMAKE_BUILD_TYPE "DEBUG")
#else()
#	string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
#endif()

#string(TOUPPER ${CMAKE_SYSTEM_NAME} CMAKE_SYSTEM_NAME)
if(CMAKE_SYSTEM_NAME MATCHES "Android")
	add_definitions("-DANDROID")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
else()
	message(FATAL_ERROR "unknown CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
endif()

##############################################################################

set(src ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(acl_path ${CMAKE_CURRENT_SOURCE_DIR}/../../lib_acl)
set(acl_include ${acl_path}/include)
set(master_include ${acl_path}/src/master)
set(acl_cpp_path ${CMAKE_CURRENT_SOURCE_DIR}/../../lib_acl_cpp)
set(acl_cpp_include ${acl_cpp_path}/include)
set(fiber_path ${CMAKE_CURRENT_SOURCE_DIR}/../c)
set(fiber_include ${fiber_path}/include)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${acl_include}
	${master_include}
	${acl_cpp_include}
	${fiber_include}
)

if (${WIN_OS})
	set(sources ${src} ${src}/detours)
else()
	set(sources ${src} ${src}/keeper)
endif()

foreach(iter ${sources})
	if (ACL_BUILD_VERBOSE MATCHES "YES")
		message(STATUS ">>add dir ${iter}")
	endif()
	aux_source_directory(${iter} lib_src)
endforeach()

if (CMAKE_SYSTEM_NAME MATCHES "Android")
	set(lib_output_path ${CMAKE_CURRENT_SOURCE_DIR}/../../android/lib/${ANDROID_ABI})
else()
	set(lib_output_path ${PROJECT_BINARY_DIR}/../lib)
endif()

set(LIBRARY_OUTPUT_PATH ${lib_output_path})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${lib_output_path})

add_library(fiber_cpp_static STATIC ${lib_src})

if (${WIN_OS})
	set_target_properties(fiber_cpp_static PROPERTIES
		OUTPUT_NAME "libfiber_cpp"
		ARCHIVE_OUTPUT_DIRECTORY ${lib_output_path}/static
		LIBRARY_OUTPUT_DIRECTORY ${lib_output_path}/static
	)
else()
	set_target_properties(fiber_cpp_static PROPERTIES OUTPUT_NAME "fiber_cpp")
endif()

link_directories(${lib_output_path})

if(CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
	install(TARGETS fiber_cpp_static
		EXPORT fiber_cpp_static-targets
		ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	)
endif()

if (CMAKE_INSTALL_INCLUDEDIR MATCHES "")
	set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/fiber/
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/acl-lib/fiber_cpp"
)

#if (NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND ACL_BUILD_SHARED MATCHES "YES")
if (ACL_BUILD_SHARED MATCHES "YES")
	if (CMAKE_SYSTEM_NAME MATCHES "Android")
		if (ANDROID_STL MATCHES "gnustl_shared")
			set(sys_ldflags "-shared -lz")
		else()
			set(sys_ldflags "-shared -O3 -flto -lz")
		endif()
		target_compile_options(fiber_cpp_static PRIVATE
			-fvisibility=hidden -fvisibility-inlines-hidden)
	elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
		set(sys_ldflags "-shared -liconv -lz -lpthread -ldl")
		target_compile_options(fiber_cpp_static PRIVATE
			-fvisibility=hidden -fvisibility-inlines-hidden)
		set(CMAKE_MACOSX_RPATH 1)
	elseif (${UNIX_OS})
		set(sys_ldflags "-shared -lz -lpthread -ldl")
		target_compile_options(fiber_cpp_static PRIVATE
			-fvisibility=hidden -fvisibility-inlines-hidden)
	endif()

	if (${WIN_OS})
		link_directories(${lib_output_path}/shared)
	endif()

	add_library(fiber_cpp_shared SHARED ${lib_src})

	if (${WIN_OS})
		set_target_properties(fiber_cpp_shared PROPERTIES
			OUTPUT_NAME "libfiber_cpp"
			RUNTIME_OUTPUT_DIRECTORY ${lib_output_path}/shared
			ARCHIVE_OUTPUT_DIRECTORY ${lib_output_path}/shared
			LIBRARY_OUTPUT_DIRECTORY ${lib_output_path}/shared
		)
		set(CMAKE_SHARED_LINKER_FLAGS "${lc_ldflags} ${sys_ldflags}")
		target_link_libraries(fiber_cpp_shared libacl_cpp libprotocol libacl libfiber Ws2_32)
	else()
		set_target_properties(fiber_cpp_shared PROPERTIES OUTPUT_NAME "fiber_cpp")
		set(lc_ldflags "-Wl,-rpath,${CMAKE_INSTALL_PREFIX}/lib -L${lib_output_path}")
		set(CMAKE_SHARED_LINKER_FLAGS "${lc_ldflags} ${sys_ldflags}")
		target_link_libraries(fiber_cpp_shared acl_cpp protocol acl fiber)
	endif()

	if(CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
		install(TARGETS fiber_cpp_shared
			EXPORT fiber_cpp_shared-targets
			LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
		)
	endif()
endif()

##############################################################################
