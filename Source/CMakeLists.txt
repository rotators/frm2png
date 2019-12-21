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
	if( UNIX )
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

# zlib MUST be configured as first
message( STATUS "Configuring zlib..." )
add_subdirectory( zlib EXCLUDE_FROM_ALL )
message( STATUS "Configuring zlib - ${ZLIB_INCLUDE_DIR}" )
get_property(DIR_ZLIB_BINARY TARGET zlibstatic PROPERTY BINARY_DIR)
get_property(DIR_ZLIB_SOURCE TARGET zlibstatic PROPERTY SOURCE_DIR)

# libpng is waaay too stubborn on using installed zlib instead of source...
message( STATUS "Configuring libpng-apng..." )
option( PNG_BUILD_ZLIB "" ON )
set( ZLIB_INCLUDE_DIR ${DIR_ZLIB_SOURCE} )
add_subdirectory( libpng-apng EXCLUDE_FROM_ALL )
target_include_directories( png_static PRIVATE ${DIR_ZLIB_BINARY} ${DIR_ZLIB_SOURCE} )

message( STATUS "Configuring libfalltergeist-mini..." )
add_subdirectory( libfalltergeist-mini EXCLUDE_FROM_ALL )

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

target_include_directories( frm2png PRIVATE libfalltergeist-mini libpng-apng )
target_link_libraries( frm2png PRIVATE png_static falltergeist-mini )

frm2png_target( frm2png )
