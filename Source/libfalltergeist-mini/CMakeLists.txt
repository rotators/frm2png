add_library( falltergeist-mini STATIC "" )
target_sources( falltergeist-mini
	PRIVATE
		Format/Enums.h

		Format/Base/Buffer.h

		Format/Dat/Entry.cpp
		Format/Dat/File.cpp
		Format/Dat/File.h
		Format/Dat/Item.cpp
		Format/Dat/Item.h
		Format/Dat/MiscFile.cpp
		Format/Dat/MiscFile.h
		Format/Dat/Stream.h
		Format/Dat/Stream.cpp

		Format/Frm/Direction.cpp
		Format/Frm/Direction.h
		Format/Frm/File.cpp
		Format/Frm/File.h
		Format/Frm/Frame.cpp
		Format/Frm/Frame.h

		Format/Pal/Color.h
		Format/Pal/Color.cpp
		Format/Pal/File.h
		Format/Pal/File.cpp
)

if( NOT ZLIB_INCLUDE_DIR )
	message( FATAL_ERROR "ZLIB_INCLUDE_DIR not set" )
endif()

target_include_directories( falltergeist-mini PRIVATE "${ZLIB_INCLUDE_DIR}" )
target_link_libraries( falltergeist-mini PRIVATE zlibstatic )
frm2png_target( falltergeist-mini )

source_group( " "         REGULAR_EXPRESSION "\.([CcHh]|[CcHh][Pp][Pp])$" )
source_group( "CMake"     REGULAR_EXPRESSION "[Cc][Mm][Aa][Kk][Ee]" )

file( GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.[CcHh]" "${CMAKE_CURRENT_SOURCE_DIR}/*.[CcHh][Pp][Pp]" )
source_group( TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCE_FILES} )
