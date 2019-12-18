cmake_minimum_required( VERSION 3.7.2 FATAL_ERROR )

project( frm2png LANGUAGES CXX )

set( BUILD_SHARED_LIBS            OFF)
set( CMAKE_CONFIGURATION_TYPES    "Release" )
set( CMAKE_CXX_STANDARD           11 )
set( CMAKE_CXX_STANDARD_REQUIRED  YES )

set( CMAKE_SKIP_INSTALL_RULES     TRUE )
macro( install )
endmacro()

set( ZLIB_LIBRARY      zlibstatic )
add_subdirectory( png  EXCLUDE_FROM_ALL )
add_subdirectory( zlib EXCLUDE_FROM_ALL )

add_executable( frm2png "" )
target_sources( frm2png
	PRIVATE
		main.cpp

		Color.cpp
		Color.h
		Exception.cpp
		Exception.h
		FalloutFile.cpp
		FalloutFile.h
		FrmFalloutFile.cpp
		FrmFalloutFile.h
		FrmFrame.cpp
		FrmFrame.h
		PngImage.cpp
		PngImage.h
		PngWriter.cpp
		PngWriter.h
		pallete.h
)

target_include_directories( frm2png PRIVATE png )
target_link_libraries( frm2png PRIVATE png_static )
