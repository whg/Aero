cmake_minimum_required( VERSION 3.0 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

project( Areo )

get_filename_component( CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Code/Cinder" ABSOLUTE )
get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../" ABSOLUTE )

include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

ci_make_app(
	SOURCES     ${APP_PATH}/src/AreoApp.cpp ${APP_PATH}/src/Mapping.hpp ${APP_PATH}/src/Mapping.cpp ${APP_PATH}/src/FrameSequence.cpp  ${APP_PATH}/src/FrameSequence.hpp ${APP_PATH}/src/Transport.cpp  ${APP_PATH}/src/Transport.hpp ${APP_PATH}/src/Output.cpp  ${APP_PATH}/src/Output.hpp ${APP_PATH}/src/AudioTrack.cpp  ${APP_PATH}/src/AudioTrack.hpp ${APP_PATH}/include/whelpersg/serial.hpp
	CINDER_PATH ${CINDER_PATH}
	BLOCKS      Cinder-ImGui
)

target_include_directories( "${PROJECT_NAME}" PUBLIC "${APP_PATH}/include" )
