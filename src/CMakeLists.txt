cmake_minimum_required( VERSION 3.7.2 FATAL_ERROR )

project( frm2png LANGUAGES CXX )

set( BUILD_SHARED_LIBS            OFF)
set( CMAKE_CONFIGURATION_TYPES    "Release" )
set( CMAKE_CXX_STANDARD           11 )
set( CMAKE_CXX_STANDARD_REQUIRED  YES )

# prevent any dependencies from running any install-related stuff
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

# prevent libpng from searching for zlib, as it most likely will find one installed on system
# this should be replaced with tweaking CMP0079 and using target_link_libraries() after switching to cmake 3.13 or newer
set( ZLIB_LIBRARY zlibstatic )

add_subdirectory( libfalltergeist-mini EXCLUDE_FROM_ALL )
add_subdirectory( png                  EXCLUDE_FROM_ALL )
add_subdirectory( zlib                 EXCLUDE_FROM_ALL )


add_executable( frm2png "" )
target_sources( frm2png
	PRIVATE
		main.cpp

		ColorPal.cpp
		ColorPal.h
		Exception.cpp
		Exception.h
		PngImage.cpp
		PngImage.h
		PngWriter.cpp
		PngWriter.h
)

target_include_directories( frm2png PRIVATE libfalltergeist-mini png )
target_link_libraries( frm2png PRIVATE png_static falltergeist-mini )

frm2png_target( frm2png )
