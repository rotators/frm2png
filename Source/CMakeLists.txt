cmake_minimum_required( VERSION 3.7.2 FATAL_ERROR )

project( frm2png LANGUAGES CXX )

set( BUILD_SHARED_LIBS            OFF)
set( CMAKE_BUILD_TYPE             "Release" )
set( CMAKE_CONFIGURATION_TYPES    "Release" )
set( CMAKE_CXX_STANDARD           14 )
set( CMAKE_CXX_STANDARD_REQUIRED  YES )
#set( CMAKE_VERBOSE_MAKEFILE      ON )

# prevent any dependencies from running any installation-related stuff
set( CMAKE_SKIP_INSTALL_RULES     TRUE )
macro( install )
endmacro()

function( compiler_flag type target flag var )
	include( CheckCXXCompilerFlag )
	check_cxx_compiler_flag( ${flag} ${var} )

	if( ${var} )
		if( "${type}" STREQUAL "compile" )
			target_compile_options( ${target} PRIVATE ${flag} )
		elseif( "${type}" STREQUAL "link" )
			target_link_libraries( ${target} PRIVATE ${flag} )
		else()
			message( FATAL_ERROR "Unknown flag type '${type}'" )
		endif()
	endif()
endfunction()

function( other_target target )
	if( MSVC )
		set_property( TARGET ${target} APPEND PROPERTY COMPILE_FLAGS /MT )
		set_property( TARGET ${target} APPEND PROPERTY LINK_FLAGS    /OPT:REF )
	endif()
endfunction()

function( frm2png_target target )
	other_target( ${target} )

	if( UNIX )
		if( EBUG ) # as in: cmake -DEBUG=1 Source/
			compiler_flag( "compile" ${target} -g COMPILER_FLAG_G )
		endif()

		compiler_flag( "compile" ${target} -Wall      COMPILER_FLAG_WALL )
		compiler_flag( "compile" ${target} -Werror    COMPILER_FLAG_WERROR )
		compiler_flag( "compile" ${target} -Wextra    COMPILER_FLAG_WEXTRA )
		compiler_flag( "compile" ${target} -Wpedantic COMPILER_FLAG_WPEDANTIC )

		compiler_flag( "compile" ${target} -Wconversion COMPILER_FLAG_WCONVERSION )

		compiler_flag( "link" ${target} -static-libgcc    COMPILER_FLAG_STATIC_LIBGCC )
		compiler_flag( "link" ${target} -static-libstdc++ COMPILER_FLAG_STATIC_LIBSTDCPP )
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

set_property( TARGET zlibstatic PROPERTY FOLDER "Libraries" )

set( ZLIB_INCLUDE_DIR "${DIR_ZLIB_BINARY}" )
add_custom_command( OUTPUT "${DIR_ZLIB_BINARY}/zlib.h" COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DIR_ZLIB_SOURCE}/zlib.h" "${DIR_ZLIB_BINARY}/zlib.h" )
add_custom_target( zlib_h ALL DEPENDS "${DIR_ZLIB_BINARY}/zlib.h" )
add_dependencies( zlib_h zlibstatic )

set_property( TARGET zlibstatic zlib_h PROPERTY FOLDER "Libraries" )

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

set_property( TARGET png_static genfiles PROPERTY FOLDER "Libraries" )

##
## libfalltergeist-mini
##

message( STATUS "Configuring libfalltergeist-mini..." )
add_subdirectory( libfalltergeist-mini EXCLUDE_FROM_ALL )
add_dependencies( falltergeist-mini zlib_h )

##
## clipp
##

# added minimal setup here, as subdirectory requires cmake v3.8
# add_subdirectory( clipp EXCLUDE_FROM_ALL )
message( STATUS "Configuring clipp..." )
add_library( clipp INTERFACE )
target_include_directories( clipp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/clipp/include" )

set_property( TARGET clipp PROPERTY FOLDER "Libraries" )

##
## frm2png
##

message( STATUS "Configuring frm2png..." )
add_executable( frm2png "" )
target_sources( frm2png
	PRIVATE
		ColorPal.cpp
		ColorPal.h
		Logging.cpp
		Logging.h
		PngGenerator.cpp
		PngGenerator.h
		PngImage.cpp
		PngImage.h
		PngWriter.cpp
		PngWriter.h

		frm2png.cpp
)

target_include_directories( frm2png PRIVATE "${DIR_LIBPNG_BINARY}" libfalltergeist-mini libpng-apng )
target_link_libraries( frm2png PRIVATE png_static falltergeist-mini clipp )

frm2png_target( frm2png )

##
## IDE
##

set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set_property( GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake" )

source_group( " "         REGULAR_EXPRESSION "\.([CcHh]|[CcHh][Pp][Pp])$" )
source_group( "CMake"     REGULAR_EXPRESSION "[Cc][Mm][Aa][Kk][Ee]" )
