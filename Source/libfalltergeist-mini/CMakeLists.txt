add_library( falltergeist-mini STATIC "" )
target_sources( falltergeist-mini
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Format/Dat/File.h
		${CMAKE_CURRENT_SOURCE_DIR}/Format/Frm/Direction.h
		${CMAKE_CURRENT_SOURCE_DIR}/Format/Frm/File.h
		${CMAKE_CURRENT_SOURCE_DIR}/Format/Frm/Frame.h

		${CMAKE_CURRENT_SOURCE_DIR}/Format/Pal/Color.h
		${CMAKE_CURRENT_SOURCE_DIR}/Format/Pal/File.h
	PRIVATE
		Format/Enums.h

		Format/Base/Buffer.h

		Format/Dat/Entry.cpp
		Format/Dat/Entry.h
		Format/Dat/File.cpp
		Format/Dat/Item.cpp
		Format/Dat/Item.h
		Format/Dat/MiscFile.cpp
		Format/Dat/MiscFile.h
		Format/Dat/Stream.cpp
		Format/Dat/Stream.h

		Format/Frm/Direction.cpp
		Format/Frm/File.cpp
		Format/Frm/Frame.cpp

		Format/Pal/Color.cpp
		Format/Pal/File.cpp
)

if( NOT ZLIB_INCLUDE_DIR )
	message( FATAL_ERROR "ZLIB_INCLUDE_DIR not set" )
endif()

target_include_directories( falltergeist-mini PRIVATE "${ZLIB_INCLUDE_DIR}" )
target_link_libraries( falltergeist-mini PRIVATE zlibstatic )
frm2png_target( falltergeist-mini )
