cmake_minimum_required( VERSION 3.7.2 FATAL_ERROR )

project( frm2png LANGUAGES CXX )

set( BUILD_SHARED_LIBS            OFF)
set( CMAKE_BUILD_TYPE             "Release" )
set( CMAKE_CONFIGURATION_TYPES    "Release" )
set( CMAKE_CXX_STANDARD           11 )
set( CMAKE_CXX_STANDARD_REQUIRED  YES )
#set( CMAKE_VERBOSE_MAKEFILE      ON )

# prevent any dependencies from running any installation-related stuff
set( CMAKE_SKIP_INSTALL_RULES     TRUE )
macro( install )
endmacro()

# strict compilation
function( frm2png_target target )
	if( MSVC )
		set_property( TARGET ${target} APPEND PROPERTY COMPILE_FLAGS /MT )
		set_property( TARGET ${target} APPEND PROPERTY LINK_FLAGS /OPT:REF )
	elseif( UNIX )
		set( CMAKE_BUILD_TYPE RelWithDebInfo )

		include( CheckCXXCompilerFlag )
		check_cxx_compiler_flag( -Wall      COMPILER_FLAG_WALL )
		check_cxx_compiler_flag( -Werror    COMPILER_FLAG_WERROR )
		check_cxx_compiler_flag( -Wextra    COMPILER_FLAG_WEXTRA )
		check_cxx_compiler_flag( -Wpedantic COMPILER_FLAG_WPEDANTIC )

		if( COMPILER_FLAG_WALL )
			target_compile_options( ${target} PRIVATE -Wall )
		endif()
		if( COMPILER_FLAG_WERROR )
			target_compile_options( ${target} PRIVATE -Werror )
		endif()
		if( COMPILER_FLAG_WEXTRA )
			target_compile_options( ${target} PRIVATE -Wextra )
		endif()
		if( COMPILER_FLAG_WPEDANTIC )
			target_compile_options( ${target} PRIVATE -Wpedantic )
		endif()

		#check_cxx_compiler_flag( -Wconversion COMPILER_FLAG_WCONVERSION )

		#if( COMPILER_FLAG_WCONVERSION )
		#	target_compile_options( ${target} PRIVATE -Wconversion )
		#endif()

		check_cxx_compiler_flag( -static-libgcc    COMPILER_FLAG_STATIC_LIBGCC )
		check_cxx_compiler_flag( -static-libstdc++ COMPILER_FLAG_STATIC_LIBSTDCPP )

		if( COMPILER_FLAG_STATIC_LIBGCC )
			target_link_libraries( ${target} PRIVATE -static-libgcc )
		endif()
		if( COMPILER_FLAG_STATIC_LIBSTDCPP )
			target_link_libraries( ${target} PRIVATE -static-libstdc++ )
		endif()
	endif()
endfunction()

function( other_target target )
	if( MSVC )
		set_property( TARGET ${target} APPEND PROPERTY COMPILE_FLAGS /MT )
		set_property( TARGET ${target} APPEND PROPERTY LINK_FLAGS /OPT:REF )
	endif()
endfunction()

##
## zlib
##

message( STATUS "Configuring zlib..." )
add_subdirectory( zlib EXCLUDE_FROM_ALL )
get_property(DIR_ZLIB_BINARY TARGET zlibstatic PROPERTY BINARY_DIR)
get_property(DIR_ZLIB_SOURCE TARGET zlibstatic PROPERTY SOURCE_DIR)
other_target( zlibstatic )

set( ZLIB_INCLUDE_DIR "${DIR_ZLIB_BINARY}" )
add_custom_command( OUTPUT "${DIR_ZLIB_BINARY}/zlib.h" COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DIR_ZLIB_SOURCE}/zlib.h" "${DIR_ZLIB_BINARY}/zlib.h" )
add_custom_target( zlib_h ALL DEPENDS "${DIR_ZLIB_BINARY}/zlib.h" )
add_dependencies( zlib_h zlibstatic )

##
## libpng-apng
##

message( STATUS "Configuring libpng-apng..." )
option( PNG_BUILD_ZLIB "" ON )
add_subdirectory( libpng-apng EXCLUDE_FROM_ALL )
get_property(DIR_LIBPNG_BINARY TARGET png_static PROPERTY BINARY_DIR)
other_target( png_static )

target_include_directories( png_static PRIVATE "${ZLIB_INCLUDE_DIR}" )
add_dependencies( genfiles zlib_h )

##
## libfalltergeist-mini
##

message( STATUS "Configuring libfalltergeist-mini..." )
add_subdirectory( libfalltergeist-mini EXCLUDE_FROM_ALL )
add_dependencies( falltergeist-mini zlib_h )

##
## frm2png
##

message( STATUS "Configuring frm2png..." )
add_executable( frm2png "" )
target_sources( frm2png
	PRIVATE
		ColorPal.cpp
		ColorPal.h
		Exception.cpp
		Exception.h
		PngImage.cpp
		PngImage.h
		PngWriter.cpp
		PngWriter.h

		frm2png.cpp
)

target_include_directories( frm2png PRIVATE "${DIR_LIBPNG_BINARY}" libfalltergeist-mini libpng-apng )
target_link_libraries( frm2png PRIVATE png_static falltergeist-mini )

frm2png_target( frm2png )
