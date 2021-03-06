cmake_minimum_required( VERSION 2.4 )

if( COMMAND cmake_policy )
	cmake_policy( SET CMP0003 NEW )
endif( COMMAND cmake_policy )

include( CheckCXXSourceCompiles )
include( CheckFunctionExists )
include( CheckCXXCompilerFlag )
include( CheckLibraryExists )
include( FindPkgConfig )
include( FindOpenGL )

if( ZD_CMAKE_COMPILER_IS_GNUCXX_COMPATIBLE )
	option( NO_STRIP "Do not strip Release or MinSizeRel builds" )
	# At least some versions of Xcode fail if you strip with the linker
	# instead of the separate strip utility.
	if( APPLE )
		set( NO_STRIP ON )
	endif( APPLE )
endif( ZD_CMAKE_COMPILER_IS_GNUCXX_COMPATIBLE )

option( DYN_FLUIDSYNTH "Dynamically load fluidsynth" ON )

if( APPLE )
    option( OSX_COCOA_BACKEND "Use native Cocoa backend instead of SDL" ON )
endif( APPLE )

if( CMAKE_SIZEOF_VOID_P MATCHES "8" )
	set( X64 64 )
endif( CMAKE_SIZEOF_VOID_P MATCHES "8" )

# You can either use "make install" on the FMOD distribution to put it
# in standard system locations, or you can unpack the FMOD distribution
# in the root of the zdoom tree. e.g.:
# zdoom
#   docs
#   fmodapi<version>linux[64] -or simply- fmod
#   jpeg-6b
#   ...
# The recommended method is to put it in the zdoom tree, since its
# headers are unversioned. Especially now that we can't work properly
# with anything newer than 4.26.xx, you probably don't want to use
# a system-wide version.

if( NOT ZDOOM_LIBS )
	set( ZDOOM_LIBS "" )
endif( NOT ZDOOM_LIBS )

if( WIN32 )
	if( X64 )
		set( WIN_TYPE Win64 )
		set( XBITS x64 )
	else( X64 )
		set( WIN_TYPE Win32 )
		set( XBITS x86 )
	endif( X64 )

	add_definitions( -D_WIN32 )

	find_path( D3D_INCLUDE_DIR d3d9.h
		PATHS ENV DXSDK_DIR
		PATH_SUFFIXES Include )
	if( NOT D3D_INCLUDE_DIR )
		message( SEND_ERROR "Could not find DirectX 9 header files" )
	else( NOT D3D_INCLUDE_DIR )
		include_directories( ${D3D_INCLUDE_DIR} )
	endif( NOT D3D_INCLUDE_DIR )

	find_path( XINPUT_INCLUDE_DIR xinput.h
		PATHS ENV DXSDK_DIR
		PATH_SUFFIXES Include )
	if( NOT XINPUT_INCLUDE_DIR )
		message( WARNING "Could not find xinput.h. XInput will be disabled." )
		add_definitions( -DNO_XINPUT )
	else( NOT XINPUT_INCLUDE_DIR )
		include_directories( ${XINPUT_INCLUDE_DIR} )
	endif( NOT XINPUT_INCLUDE_DIR )

	find_library( DX_dxguid_LIBRARY dxguid
		PATHS ENV DXSDK_DIR
		PATH_SUFFIXES Lib Lib/${XBITS} )
	find_library( DX_dinput8_LIBRARY dinput8
		PATHS ENV DXSDK_DIR
		PATH_SUFFIXES Lib Lib/${XBITS} )

	set( DX_LIBS_FOUND YES )
	if( NOT DX_dxguid_LIBRARY )
		set( DX_LIBS_FOUND NO )
	endif( NOT DX_dxguid_LIBRARY )
	if( NOT DX_dinput8_LIBRARY )
		set( DX_LIBS_FOUND NO )
	endif( NOT DX_dinput8_LIBRARY )

	if( NOT DX_LIBS_FOUND )
		message( FATAL_ERROR "Could not find DirectX 9 libraries" )
	endif( NOT DX_LIBS_FOUND )

	set( ZDOOM_LIBS
		wsock32
		winmm
		"${DX_dxguid_LIBRARY}"
		"${DX_dinput8_LIBRARY}"
		ole32
		user32
		gdi32
		comctl32
		comdlg32
		ws2_32
		setupapi
		oleaut32
		DelayImp )
else( WIN32 )
	if( APPLE )
		set( NO_GTK ON )
	else( APPLE )
		option( NO_GTK "Disable GTK+ dialogs (Not applicable to Windows)" )
		option( VALGRIND "Add special Valgrind sequences to self-modifying code" )

		# Use GTK+ for the IWAD picker, if available.
		if( NOT NO_GTK )
			pkg_check_modules( GTK2 gtk+-2.0 )
			if( GTK2_FOUND )
				set( ZDOOM_LIBS ${ZDOOM_LIBS} ${GTK2_LIBRARIES} )
				include_directories( ${GTK2_INCLUDE_DIRS} )
			else( GTK2_FOUND )
				set( NO_GTK ON )
			endif( GTK2_FOUND )
		endif( NOT NO_GTK )
	endif( APPLE )

	if( NO_GTK )
		add_definitions( -DNO_GTK=1 )
	endif( NO_GTK )

	# Non-Windows version also needs SDL except native OS X backend
	if( NOT APPLE OR NOT OSX_COCOA_BACKEND )
		find_package( SDL2 REQUIRED )
		include_directories( "${SDL2_INCLUDE_DIR}" )
		set( ZDOOM_LIBS ${ZDOOM_LIBS} "${SDL2_LIBRARY}" )
	endif( NOT APPLE OR NOT OSX_COCOA_BACKEND )

	find_path( FPU_CONTROL_DIR fpu_control.h )
	if( FPU_CONTROL_DIR )
		include_directories( ${FPU_CONTROL_DIR} )
		add_definitions( -DHAVE_FPU_CONTROL )
	endif( FPU_CONTROL_DIR )
endif( WIN32 )

# Check if we have OpenGL

if( NOT OPENGL_FOUND )
	message( FATAL_ERROR "OpenGL is required for building." )
endif( NOT OPENGL_FOUND )
if( NOT OPENGL_GLU_FOUND )
	message( FATAL_ERROR "OpenGL GLU is required for building." )
endif( NOT OPENGL_GLU_FOUND )

set( ZDOOM_LIBS ${ZDOOM_LIBS} ${OPENGL_LIBRARIES} )
include_directories( ${OPENGL_INCLUDE_DIR} )


# check for GLEW
find_path( GLEW_INCLUDE_DIR GL/glew.h
	PATHS "/usr/include"
	"/usr/local/include" )

if( GLEW_INCLUDE_DIR )
	message( STATUS "GLEW include files found at ${GLEW_INCLUDE_DIR}" )
else( GLEW_INCLUDE_DIR )
	message( SEND_ERROR "Could not find GLEW include files" )
endif( GLEW_INCLUDE_DIR )

# GLEW include directory
include_directories( "${GLEW_INCLUDE_DIR}" )

if( NOT WIN32 OR APPLE )
	find_library( GLEW_LIBRARY libGLEW.so )
else( NOT WIN32 OR APPLE )
	find_library( GLEW_LIBRARY glew32 )
endif( NOT WIN32 OR APPLE )

if( NOT GLEW_LIBRARY )
	message( SEND_ERROR "Could not find GLEW library files" )
endif( NOT GLEW_LIBRARY )

set( ZDOOM_LIBS ${ZDOOM_LIBS} ${GLEW_LIBRARY} )

find_package( OpenAL )
mark_as_advanced(CLEAR OPENAL_LIBRARY OPENAL_INCLUDE_DIR)
if( OPENAL_FOUND )
		include_directories( ${OPENAL_INCLUDE_DIR} )
		set( ZDOOM_LIBS ${OPENAL_LIBRARY} ${ZDOOM_LIBS} )
else( OPENAL_FOUND )
		message ( SEND_ERROR "OpenAL could not be found." )
endif( OPENAL_FOUND )

# Search for libSndFile

find_package( SndFile )

# Search for libmpg123

find_package( MPG123 )

# Search for FluidSynth

find_package( FluidSynth )

# Decide on SSE setup

set( SSE_MATTERS NO )

# with global use of SSE 2 we do not need special handling for selected files
if (NOT ZDOOM_USE_SSE2)
	# SSE only matters on 32-bit targets. We check compiler flags to know if we can do it.
	if( CMAKE_SIZEOF_VOID_P MATCHES "4" AND NOT CMAKE_OSX_ARCHITECTURES MATCHES ppc )
		CHECK_CXX_COMPILER_FLAG( "-msse2 -mfpmath=sse" CAN_DO_MFPMATH )
		CHECK_CXX_COMPILER_FLAG( -arch:SSE2 CAN_DO_ARCHSSE2 )
		if( CAN_DO_MFPMATH )
			set( SSE1_ENABLE "-msse -mfpmath=sse" )
			set( SSE2_ENABLE "-msse2 -mfpmath=sse" )
			set( SSE_MATTERS YES )
		elseif( CAN_DO_ARCHSSE2 )
			set( SSE1_ENABLE -arch:SSE )
			set( SSE2_ENABLE -arch:SSE2 )
			set( SSE_MATTERS YES )
		endif( CAN_DO_MFPMATH )
	endif( CMAKE_SIZEOF_VOID_P MATCHES "4" AND NOT CMAKE_OSX_ARCHITECTURES MATCHES ppc )
endif (NOT ZDOOM_USE_SSE2)

if( SSE_MATTERS )
	if( WIN32 )
		set( BACKPATCH 1 CACHE BOOL "Enable backpatching." )
	else( WIN32 )
		CHECK_FUNCTION_EXISTS(mprotect HAVE_MPROTECT)
		if( HAVE_MPROTECT )
			set( BACKPATCH 1 CACHE BOOL "Enable backpatching." )
		else( HAVE_MPROTECT )
			set( BACKPATCH 0 )
		endif( HAVE_MPROTECT )
	endif( WIN32 )
	set( SSE 1 CACHE BOOL "Build SSE and SSE2 versions of key code." )
else( SSE_MATTERS )
	set( BACKPATCH 0 )
endif( SSE_MATTERS )

# Set up flags for GCC

if( ZD_CMAKE_COMPILER_IS_GNUCXX_COMPATIBLE )
	if( PROFILE )
		set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pg" )
		set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg" )
		set( CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -pg" )
		set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pg" )
	endif( PROFILE )

	set( REL_CXX_FLAGS "-fno-rtti" )
	if( NOT PROFILE AND NOT APPLE )
		# On OS X frame pointers are required for exception handling, at least with Clang
		set( REL_CXX_FLAGS "${REL_CXX_FLAGS} -fomit-frame-pointer" )
	endif( NOT PROFILE AND NOT APPLE )
	set( CMAKE_CXX_FLAGS_RELEASE "${REL_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}" )
	set( CMAKE_CXX_FLAGS_MINSIZEREL "${REL_CXX_FLAGS} ${CMAKE_CXX_FLAGS_MINSIZEREL}" )
	set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${REL_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )

	set( CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused -Wno-unused-parameter -Wno-missing-field-initializers ${CMAKE_CXX_FLAGS}" )

	# Remove extra warnings when using the official DirectX headers.
	# Also, TDM-GCC 4.4.0 no longer accepts glibc-style printf formats as valid,
	# which is a royal pain. The previous version I had been using was fine with them.
	if( WIN32 )
		set( CMAKE_CXX_FLAGS "-Wno-unknown-pragmas -Wno-comment -Wno-format ${CMAKE_CXX_FLAGS}" )
	endif( WIN32 )

	if( NOT NO_STRIP )
		set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s" )
		set (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} -s" )
	endif( NOT NO_STRIP )
endif( ZD_CMAKE_COMPILER_IS_GNUCXX_COMPATIBLE )

# Check for functions that may or may not exist.

CHECK_FUNCTION_EXISTS( filelength FILELENGTH_EXISTS )
if( FILELENGTH_EXISTS )
	add_definitions( -DHAVE_FILELENGTH=1 )
endif( FILELENGTH_EXISTS )

CHECK_FUNCTION_EXISTS( strupr STRUPR_EXISTS )
if( NOT STRUPR_EXISTS )
	add_definitions( -DNEED_STRUPR=1 )
endif( NOT STRUPR_EXISTS )

CHECK_FUNCTION_EXISTS( stricmp STRICMP_EXISTS )
if( NOT STRICMP_EXISTS )
	add_definitions( -Dstricmp=strcasecmp )
endif( NOT STRICMP_EXISTS )

CHECK_FUNCTION_EXISTS( strnicmp STRNICMP_EXISTS )
if( NOT STRNICMP_EXISTS )
	add_definitions( -Dstrnicmp=strncasecmp )
endif( NOT STRNICMP_EXISTS )

if( NOT MSVC )
	add_definitions( -D__forceinline=inline )
endif( NOT MSVC )

if( UNIX )
	CHECK_LIBRARY_EXISTS( rt clock_gettime "" CLOCK_GETTIME_IN_RT )
	if( NOT CLOCK_GETTIME_IN_RT )
		CHECK_FUNCTION_EXISTS( clock_gettime CLOCK_GETTIME_EXISTS )
		if( NOT CLOCK_GETTIME_EXISTS )
			message( STATUS "Could not find clock_gettime. Timing statistics will not be available." )
			add_definitions( -DNO_CLOCK_GETTIME )
		endif( NOT CLOCK_GETTIME_EXISTS )
	else( NOT CLOCK_GETTIME_IN_RT )
		set( ZDOOM_LIBS ${ZDOOM_LIBS} rt )
	endif( NOT CLOCK_GETTIME_IN_RT )
endif( UNIX )

CHECK_CXX_SOURCE_COMPILES(
	"#include <stdarg.h>
	int main() { va_list list1, list2; va_copy(list1, list2); return 0; }"
	HAS_VA_COPY )
if( NOT HAS_VA_COPY )
	CHECK_CXX_SOURCE_COMPILES(
		"#include <stdarg.h>
		int main() { va_list list1, list2; __va_copy(list1, list2); return 0; }"
		HAS___VA_COPY )
	if( HAS___VA_COPY )
		add_definitions( -Dva_copy=__va_copy )
	else( HAS___VA_COPY )
		add_definitions( -DNO_VA_COPY )
	endif( HAS___VA_COPY )
endif( NOT HAS_VA_COPY )

# Flags

if( BACKPATCH )
	add_definitions( -DBACKPATCH )
endif( BACKPATCH )

# Update gitinfo.h

#get_target_property( UPDATEREVISION_EXE updaterevision LOCATION )

#add_custom_target( revision_check ALL
#	COMMAND ${UPDATEREVISION_EXE} src/gitinfo.h
#	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#	DEPENDS updaterevision )

# Libraries ZDoom needs

message( STATUS "Fluid synth libs: ${FLUIDSYNTH_LIBRARIES}" )
set( ZDOOM_LIBS ${ZDOOM_LIBS} "${ZLIB_LIBRARIES}" "${JPEG_LIBRARIES}" "${BZIP2_LIBRARIES}" "${GME_LIBRARIES}" )
include_directories( "${ZLIB_INCLUDE_DIR}" "${BZIP2_INCLUDE_DIR}" "${LZMA_INCLUDE_DIR}" "${JPEG_INCLUDE_DIR}" "${GME_INCLUDE_DIR}" )

if( SNDFILE_FOUND )
    set( ZDOOM_LIBS ${ZDOOM_LIBS} "${SNDFILE_LIBRARIES}" )
    include_directories( "${SNDFILE_INCLUDE_DIRS}" )
endif( SNDFILE_FOUND )
if( MPG123_FOUND )
    set( ZDOOM_LIBS ${ZDOOM_LIBS} "${MPG123_LIBRARIES}" )
    include_directories( "${MPG123_INCLUDE_DIR}" )
endif( MPG123_FOUND )
if( NOT DYN_FLUIDSYNTH)
	if( FLUIDSYNTH_FOUND )
		set( ZDOOM_LIBS ${ZDOOM_LIBS} "${FLUIDSYNTH_LIBRARIES}" )
		include_directories( "${FLUIDSYNTH_INCLUDE_DIR}" )
	endif( FLUIDSYNTH_FOUND )
else( NOT DYN_FLUIDSYNTH )
	set( ZDOOM_LIBS ${ZDOOM_LIBS} ${CMAKE_DL_LIBS} )
endif( NOT DYN_FLUIDSYNTH )

# Start defining source files for ZDoom
set( PLAT_WIN32_SOURCES
	win32/eaxedit.cpp
	win32/fb_d3d9.cpp
	win32/fb_d3d9_wipe.cpp
	win32/fb_ddraw.cpp
	win32/hardware.cpp
	win32/helperthread.cpp
	win32/i_cd.cpp
	win32/i_crash.cpp
	win32/i_input.cpp
	win32/i_keyboard.cpp
	win32/i_mouse.cpp
	win32/i_dijoy.cpp
	win32/i_rawps2.cpp
	win32/i_xinput.cpp
	win32/i_main.cpp
	win32/i_movie.cpp
	win32/i_system.cpp
	win32/st_start.cpp
	win32/win32gliface.cpp
	win32/win32video.cpp )
set( PLAT_POSIX_SOURCES
	posix/i_cd.cpp
	posix/i_movie.cpp
	posix/i_steam.cpp
	posix/i_system.cpp
	posix/st_start.cpp )
set( PLAT_SDL_SOURCES
	posix/sdl/crashcatcher.c
	posix/sdl/hardware.cpp
	posix/sdl/i_gui.cpp
	posix/sdl/i_input.cpp
	posix/sdl/i_joystick.cpp
	posix/sdl/i_main.cpp
	posix/sdl/i_timer.cpp
	posix/sdl/sdlvideo.cpp
	posix/sdl/sdlglvideo.cpp)
set( PLAT_OSX_SOURCES
	posix/osx/iwadpicker_cocoa.mm
	posix/osx/zdoom.icns )
set( PLAT_COCOA_SOURCES
	posix/cocoa/hid/HID_Config_Utilities.c
	posix/cocoa/hid/HID_Error_Handler.c
	posix/cocoa/hid/HID_Name_Lookup.c
	posix/cocoa/hid/HID_Queue_Utilities.c
	posix/cocoa/hid/HID_Utilities.c
	posix/cocoa/hid/IOHIDDevice_.c
	posix/cocoa/hid/IOHIDElement_.c
	posix/cocoa/hid/ImmrHIDUtilAddOn.c
	posix/cocoa/critsec.cpp
	posix/cocoa/i_backend_cocoa.mm
	posix/cocoa/i_joystick.cpp
	posix/cocoa/i_timer.cpp )

if( WIN32 )
	set( SYSTEM_SOURCES_DIR win32 )
	set( SYSTEM_SOURCES ${PLAT_WIN32_SOURCES} )
	set( OTHER_SYSTEM_SOURCES ${PLAT_POSIX_SOURCES} ${PLAT_SDL_SOURCES} ${PLAT_OSX_SOURCES} ${PLAT_COCOA_SOURCES} )

	if( ZD_CMAKE_COMPILER_IS_GNUCXX_COMPATIBLE )
		# CMake is not set up to compile and link rc files with GCC. :(
		add_custom_command( OUTPUT zdoom-rc.o
			COMMAND windres -o zdoom-rc.o -i ${CMAKE_CURRENT_SOURCE_DIR}/win32/zdoom.rc
			DEPENDS win32/zdoom.rc )
		set( SYSTEM_SOURCES ${SYSTEM_SOURCES} zdoom-rc.o )
	else( ZD_CMAKE_COMPILER_IS_GNUCXX_COMPATIBLE )
		set( SYSTEM_SOURCES ${SYSTEM_SOURCES} win32/zdoom.rc )
	endif( ZD_CMAKE_COMPILER_IS_GNUCXX_COMPATIBLE )
elseif( APPLE )
	if( OSX_COCOA_BACKEND )
		set( SYSTEM_SOURCES_DIR posix posix/cocoa )
		set( SYSTEM_SOURCES ${PLAT_COCOA_SOURCES} )
		set( OTHER_SYSTEM_SOURCES ${PLAT_WIN32_SOURCES} ${PLAT_SDL_SOURCES} )
	else( OSX_COCOA_BACKEND )
		set( SYSTEM_SOURCES_DIR posix posix/sdl )
		set( SYSTEM_SOURCES ${PLAT_SDL_SOURCES} )
		set( PLAT_OSX_SOURCES ${PLAT_OSX_SOURCES} posix/sdl/i_system.mm )
		set( OTHER_SYSTEM_SOURCES ${PLAT_WIN32_SOURCES} ${PLAT_COCOA_SOURCES} )
	endif( OSX_COCOA_BACKEND )

	set( SYSTEM_SOURCES ${SYSTEM_SOURCES} ${PLAT_POSIX_SOURCES} ${PLAT_OSX_SOURCES} )

	set_source_files_properties( posix/osx/zdoom.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
else( WIN32 )
	set( SYSTEM_SOURCES_DIR posix posix/sdl )
	set( SYSTEM_SOURCES ${PLAT_POSIX_SOURCES} ${PLAT_SDL_SOURCES} )
	set( OTHER_SYSTEM_SOURCES ${PLAT_WIN32_SOURCES} ${PLAT_OSX_SOURCES} ${PLAT_COCOA_SOURCES} )
endif( WIN32 )

add_definitions( -DNOASM )

get_target_property( LEMON_EXE lemon LOCATION )
get_target_property( RE2C_EXE re2c LOCATION )

add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/xlat_parser.c ${CMAKE_CURRENT_BINARY_DIR}/xlat_parser.h
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/xlat/xlat_parser.y .
	COMMAND ${LEMON_EXE} xlat_parser.y
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS lemon ${CMAKE_CURRENT_SOURCE_DIR}/xlat/xlat_parser.y )

add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sc_man_scanner.h
	COMMAND ${RE2C_EXE} --no-generation-date -s -o ${CMAKE_CURRENT_BINARY_DIR}/sc_man_scanner.h ${CMAKE_CURRENT_SOURCE_DIR}/sc_man_scanner.re
	DEPENDS re2c ${CMAKE_CURRENT_SOURCE_DIR}/sc_man_scanner.re )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

if( SSE_MATTERS )
	if( SSE )
		set( X86_SOURCES nodebuild_classify_sse2.cpp )
		set_source_files_properties( nodebuild_classify_sse2.cpp PROPERTIES COMPILE_FLAGS "${SSE2_ENABLE}" )
	else( SSE )
		add_definitions( -DDISABLE_SSE )
	endif( SSE )
else( SSE_MATTERS )
	add_definitions( -DDISABLE_SSE )
	set( X86_SOURCES )
endif( SSE_MATTERS )

if( SNDFILE_FOUND )
    add_definitions( -DHAVE_SNDFILE )
endif( SNDFILE_FOUND )
if( MPG123_FOUND )
    add_definitions( -DHAVE_MPG123 )
endif( MPG123_FOUND )
if( DYN_FLUIDSYNTH )
	add_definitions( -DHAVE_FLUIDSYNTH -DDYN_FLUIDSYNTH )
elseif( FLUIDSYNTH_FOUND )
	add_definitions( -DHAVE_FLUIDSYNTH )
endif( DYN_FLUIDSYNTH )

# Project files should be aware of the header files. We can GLOB these since
# there's generally a new cpp for every header so this file will get changed
if( WIN32 )
	set( EXTRA_HEADER_DIRS win32/*.h )
elseif( APPLE )
	if( OSX_COCOA_BACKEND )
		set( EXTRA_HEADER_DIRS posix/*.h posix/cocoa/*.h )
	else( OSX_COCOA_BACKEND )
		set( EXTRA_HEADER_DIRS posix/*.h posix/sdl/*.h )
	endif( OSX_COCOA_BACKEND )
else( WIN32 )
	set( EXTRA_HEADER_DIRS posix/*.h posix/sdl/*.h )
endif( WIN32 )
file( GLOB HEADER_FILES
	${EXTRA_HEADER_DIRS}
	fragglescript/*.h
	g_doom/*.h
	g_heretic/*.h
	g_hexen/*.h
	g_raven/*.h
	g_shared/*.h
	g_strife/*.h
	intermission/*.h
	menu/*.h
	posix/*.h
	posix/cocoa/*.h
	posix/sdl/*.h
	r_data/*.h
	resourcefiles/*.h
	sfmt/*.h
	sound/*.h
	textures/*.h
	thingdef/*.h
	xlat/*.h
	gl/*.h
	gl/api/*.h
	gl/data/*.h
	gl/dynlights/*.h
	gl/hqnx/*.h
	gl/models/*.h
	gl/renderer/*.h
	gl/scene/*.h
	gl/shaders/*.h
	gl/system/*.h
	gl/textures/*.h
	gl/utility/*.h
	*.h
)

# These files will be flagged as "headers" so that they appear in project files
# without being compiled.
set( NOT_COMPILED_SOURCE_FILES
	${OTHER_SYSTEM_SOURCES}
	sc_man_scanner.h
	sc_man_scanner.re
	g_doom/a_arachnotron.cpp
	g_doom/a_archvile.cpp
	g_doom/a_bossbrain.cpp
	g_doom/a_bruiser.cpp
	g_doom/a_cacodemon.cpp
	g_doom/a_cyberdemon.cpp
	g_doom/a_demon.cpp
	g_doom/a_doomimp.cpp
	g_doom/a_doomweaps.cpp
	g_doom/a_fatso.cpp
	g_doom/a_keen.cpp
	g_doom/a_lostsoul.cpp
	g_doom/a_painelemental.cpp
	g_doom/a_possessed.cpp
	g_doom/a_revenant.cpp
	g_doom/a_scriptedmarine.cpp
	g_doom/a_spidermaster.cpp
	g_heretic/a_chicken.cpp
	g_heretic/a_dsparil.cpp
	g_heretic/a_hereticartifacts.cpp
	g_heretic/a_hereticimp.cpp
	g_heretic/a_hereticweaps.cpp
	g_heretic/a_ironlich.cpp
	g_heretic/a_knight.cpp
	g_heretic/a_wizard.cpp
	g_hexen/a_bats.cpp
	g_hexen/a_bishop.cpp
	g_hexen/a_blastradius.cpp
	g_hexen/a_boostarmor.cpp
	g_hexen/a_centaur.cpp
	g_hexen/a_clericflame.cpp
	g_hexen/a_clericholy.cpp
	g_hexen/a_clericmace.cpp
	g_hexen/a_clericstaff.cpp
	g_hexen/a_dragon.cpp
	g_hexen/a_fighteraxe.cpp
	g_hexen/a_fighterhammer.cpp
	g_hexen/a_fighterplayer.cpp
	g_hexen/a_fighterquietus.cpp
	g_hexen/a_firedemon.cpp
	g_hexen/a_flechette.cpp
	g_hexen/a_fog.cpp
	g_hexen/a_healingradius.cpp
	g_hexen/a_heresiarch.cpp
	g_hexen/a_hexenspecialdecs.cpp
	g_hexen/a_iceguy.cpp
	g_hexen/a_korax.cpp
	g_hexen/a_magecone.cpp
	g_hexen/a_magelightning.cpp
	g_hexen/a_magestaff.cpp
	g_hexen/a_pig.cpp
	g_hexen/a_serpent.cpp
	g_hexen/a_spike.cpp
	g_hexen/a_summon.cpp
	g_hexen/a_teleportother.cpp
	g_hexen/a_wraith.cpp
	g_strife/a_acolyte.cpp
	g_strife/a_alienspectres.cpp
	g_strife/a_coin.cpp
	g_strife/a_crusader.cpp
	g_strife/a_entityboss.cpp
	g_strife/a_inquisitor.cpp
	g_strife/a_oracle.cpp
	g_strife/a_programmer.cpp
	g_strife/a_reaver.cpp
	g_strife/a_rebels.cpp
	g_strife/a_sentinel.cpp
	g_strife/a_stalker.cpp
	g_strife/a_strifeitems.cpp
	g_strife/a_strifeweapons.cpp
	g_strife/a_templar.cpp
	g_strife/a_thingstoblowup.cpp
	g_shared/sbarinfo_commands.cpp
	xlat/xlat_parser.y
	xlat_parser.c
	xlat_parser.h
)

add_executable( gloome WIN32 MACOSX_BUNDLE
	${HEADER_FILES}
	${NOT_COMPILED_SOURCE_FILES}
	__autostart.cpp
	${SYSTEM_SOURCES}
	${X86_SOURCES}
	x86.cpp
	actorptrselect.cpp
	am_map.cpp
	b_bot.cpp
	b_func.cpp
	b_game.cpp
	b_move.cpp
	b_think.cpp
	bbannouncer.cpp
	c_bind.cpp
	c_cmds.cpp
	c_console.cpp
	c_consolebuffer.cpp
	c_cvars.cpp
	c_dispatch.cpp
	c_expr.cpp
	cmdlib.cpp
	colormatcher.cpp
	compatibility.cpp
	configfile.cpp
	ct_chat.cpp
	d_dehacked.cpp
	d_iwad.cpp
	d_main.cpp
	d_net.cpp
	d_netinfo.cpp
	d_protocol.cpp
	decallib.cpp
	dobject.cpp
	dobjgc.cpp
	dobjtype.cpp
	doomdef.cpp
	doomstat.cpp
	dsectoreffect.cpp
	dthinker.cpp
	f_wipe.cpp
	farchive.cpp
	files.cpp
	g_game.cpp
	g_acsresponder.cpp
	g_hub.cpp
	g_level.cpp
	g_mapinfo.cpp
	g_skill.cpp
	g_gameproperties.cpp
	gameconfigfile.cpp
	gi.cpp
	gitinfo.cpp
	hu_scores.cpp
	i_net.cpp
	info.cpp
	keysections.cpp
	lumpconfigfile.cpp
	m_alloc.cpp
	m_argv.cpp
	m_bbox.cpp
	m_cheat.cpp
	m_joy.cpp
	m_misc.cpp
	m_png.cpp
	m_random.cpp
	m_specialpaths.cpp
	memarena.cpp
	md5.cpp
	name.cpp
	nodebuild.cpp
	nodebuild_classify_nosse2.cpp
	nodebuild_events.cpp
	nodebuild_extract.cpp
	nodebuild_gl.cpp
	nodebuild_utility.cpp
	p_3dfloors.cpp
	p_3dmidtex.cpp
	p_acs.cpp
	p_buildmap.cpp
	p_ceiling.cpp
	p_conversation.cpp
	p_doors.cpp
	p_effect.cpp
	p_enemy.cpp
	p_floor.cpp
	p_glnodes.cpp
	p_interaction.cpp
	p_lights.cpp
	p_linkedsectors.cpp
	p_lnspec.cpp
	p_map.cpp
	p_maputl.cpp
	p_mobj.cpp
	p_pillar.cpp
	p_plats.cpp
	p_pspr.cpp
	p_saveg.cpp
	p_sectors.cpp
	p_setup.cpp
	p_sight.cpp
	p_slopes.cpp
	p_spec.cpp
	p_states.cpp
	p_switch.cpp
	p_teleport.cpp
	p_terrain.cpp
	p_things.cpp
	p_tick.cpp
	p_trace.cpp
	p_udmf.cpp
	p_usdf.cpp
	p_user.cpp
	p_writemap.cpp
	p_xlat.cpp
	parsecontext.cpp
	po_man.cpp
	r_swrenderer.cpp
	r_utility.cpp
	r_3dfloors.cpp
	r_bsp.cpp
	r_draw.cpp
	r_drawt.cpp
	r_main.cpp
	r_plane.cpp
	r_segs.cpp
	r_sky.cpp
	r_things.cpp
	s_advsound.cpp
	s_environment.cpp
	s_playlist.cpp
	s_sndseq.cpp
	s_sound.cpp
	sc_man.cpp
	st_stuff.cpp
	statistics.cpp
	stats.cpp
	stringtable.cpp
	strnatcmp.c
	tables.cpp
	teaminfo.cpp
	tempfiles.cpp
	v_blend.cpp
	v_collection.cpp
	v_draw.cpp
	v_font.cpp
	v_palette.cpp
	v_pfx.cpp
	v_text.cpp
	v_video.cpp
	w_wad.cpp
	wi_stuff.cpp
	zstrformat.cpp
	zstring.cpp
	g_doom/a_doommisc.cpp
	g_heretic/a_hereticmisc.cpp
	g_hexen/a_hexenmisc.cpp
	g_raven/a_artitele.cpp
	g_raven/a_minotaur.cpp
	g_strife/a_strifestuff.cpp
	g_strife/strife_sbar.cpp
	g_shared/a_action.cpp
	g_shared/a_armor.cpp
	g_shared/a_artifacts.cpp
	g_shared/a_bridge.cpp
	g_shared/a_camera.cpp
	g_shared/a_debris.cpp
	g_shared/a_decals.cpp
	g_shared/a_fastprojectile.cpp
	g_shared/a_flashfader.cpp
	g_shared/a_fountain.cpp
	g_shared/a_hatetarget.cpp
	g_shared/a_keys.cpp
	g_shared/a_lightning.cpp
	g_shared/a_mapmarker.cpp
	g_shared/a_morph.cpp
	g_shared/a_movingcamera.cpp
	g_shared/a_pickups.cpp
	g_shared/a_puzzleitems.cpp
	g_shared/a_quake.cpp
	g_shared/a_randomspawner.cpp
	g_shared/a_secrettrigger.cpp
	g_shared/a_sectoraction.cpp
	g_shared/a_setcolor.cpp
	g_shared/a_skies.cpp
	g_shared/a_soundenvironment.cpp
	g_shared/a_soundsequence.cpp
	g_shared/a_spark.cpp
	g_shared/a_specialspot.cpp
	g_shared/a_waterzone.cpp
	g_shared/a_weaponpiece.cpp
	g_shared/a_weapons.cpp
	g_shared/hudmessages.cpp
	g_shared/sbarinfo.cpp
	g_shared/sbar_mugshot.cpp
	g_shared/shared_hud.cpp
	g_shared/shared_sbar.cpp
	intermission/intermission.cpp
	intermission/intermission_parse.cpp
	menu/colorpickermenu.cpp
	menu/joystickmenu.cpp
	menu/listmenu.cpp
	menu/loadsavemenu.cpp
	menu/menu.cpp
	menu/menudef.cpp
	menu/menuinput.cpp
	menu/messagebox.cpp
	menu/optionmenu.cpp
	menu/playerdisplay.cpp
	menu/playermenu.cpp
	menu/readthis.cpp
	menu/videomenu.cpp
	gl/data/gl_data.cpp
	gl/data/gl_portaldata.cpp
	gl/data/gl_setup.cpp
	gl/data/gl_vertexbuffer.cpp
	gl/dynlights/a_dynlight.cpp
	gl/utility/gl_clock.cpp
	gl/utility/gl_cycler.cpp
	gl/utility/gl_geometric.cpp
	gl/renderer/gl_renderer.cpp
	gl/renderer/gl_renderstate.cpp
	gl/renderer/gl_lightdata.cpp
	gl/hqnx/init.cpp
	gl/hqnx/hq2x.cpp
	gl/hqnx/hq3x.cpp
	gl/hqnx/hq4x.cpp
	gl/textures/gl_hwtexture.cpp
	gl/textures/gl_texture.cpp
	gl/textures/gl_material.cpp
	gl/textures/gl_hirestex.cpp
	gl/textures/gl_bitmap.cpp
	gl/textures/gl_translate.cpp
	gl/textures/gl_hqresize.cpp
	gl/textures/gl_skyboxtexture.cpp
	gl/scene/gl_bsp.cpp
	gl/scene/gl_fakeflat.cpp
	gl/scene/gl_clipper.cpp
	gl/scene/gl_decal.cpp
	gl/scene/gl_drawinfo.cpp
	gl/scene/gl_flats.cpp
	gl/scene/gl_walls.cpp
	gl/scene/gl_sprite.cpp
	gl/scene/gl_skydome.cpp
	gl/scene/gl_renderhacks.cpp
	gl/scene/gl_weapon.cpp
	gl/scene/gl_scene.cpp
	gl/scene/gl_sky.cpp
	gl/scene/gl_portal.cpp
	gl/scene/gl_walls_draw.cpp
	gl/scene/gl_vertex.cpp
	gl/scene/gl_spritelight.cpp
	gl/dynlights/gl_dynlight.cpp
	gl/dynlights/gl_glow.cpp
	gl/dynlights/gl_dynlight1.cpp
	gl/dynlights/gl_lightbuffer.cpp
	gl/shaders/gl_shader.cpp
	gl/shaders/gl_texshader.cpp
	gl/system/gl_interface.cpp
	gl/system/gl_framebuffer.cpp
	gl/system/gl_menu.cpp
	gl/system/gl_wipe.cpp
	gl/models/gl_models_md3.cpp
	gl/models/gl_models_md2.cpp
	gl/models/gl_models.cpp
	gl/models/gl_voxels.cpp
	resourcefiles/ancientzip.cpp
	resourcefiles/file_7z.cpp
	resourcefiles/file_grp.cpp
	resourcefiles/file_lump.cpp
	resourcefiles/file_rff.cpp
	resourcefiles/file_wad.cpp
	resourcefiles/file_zip.cpp
	resourcefiles/file_pak.cpp
	resourcefiles/file_directory.cpp
	resourcefiles/resourcefile.cpp
	sfmt/SFMT.cpp
	sound/i_music.cpp
	sound/i_sound.cpp
	sound/mpg123_decoder.cpp
	sound/music_cd.cpp
	sound/music_dumb.cpp
	sound/music_gme.cpp
	sound/music_mus_midiout.cpp
	sound/music_smf_midiout.cpp
	sound/music_hmi_midiout.cpp
	sound/music_xmi_midiout.cpp
	sound/music_midistream.cpp
	sound/music_midi_base.cpp
	sound/music_midi_timidity.cpp
	sound/music_stream.cpp
	sound/music_fluidsynth_mididevice.cpp
	sound/music_softsynth_mididevice.cpp
	sound/music_timidity_mididevice.cpp
	sound/music_win_mididevice.cpp
	sound/oalsound.cpp
	sound/sndfile_decoder.cpp
	sound/music_pseudo_mididevice.cpp
	textures/animations.cpp
	textures/anim_switches.cpp
	textures/automaptexture.cpp
	textures/bitmap.cpp
	textures/buildtexture.cpp
	textures/canvastexture.cpp
	textures/ddstexture.cpp
	textures/flattexture.cpp
	textures/imgztexture.cpp
	textures/jpegtexture.cpp
	textures/multipatchtexture.cpp
	textures/patchtexture.cpp
	textures/pcxtexture.cpp
	textures/pngtexture.cpp
	textures/rawpagetexture.cpp
	textures/emptytexture.cpp
	textures/texture.cpp
	textures/texturemanager.cpp
	textures/tgatexture.cpp
	textures/warptexture.cpp
	thingdef/olddecorations.cpp
	thingdef/thingdef.cpp
	thingdef/thingdef_codeptr.cpp
	thingdef/thingdef_data.cpp
	thingdef/thingdef_exp.cpp
	thingdef/thingdef_expression.cpp
	thingdef/thingdef_function.cpp
	thingdef/thingdef_parse.cpp
	thingdef/thingdef_properties.cpp
	thingdef/thingdef_states.cpp
	timidity/common.cpp
	timidity/instrum.cpp
	timidity/instrum_dls.cpp
	timidity/instrum_font.cpp
	timidity/instrum_sf2.cpp
	timidity/mix.cpp
	timidity/playmidi.cpp
	timidity/resample.cpp
	timidity/timidity.cpp
	xlat/parse_xlat.cpp
	fragglescript/t_fspic.cpp
	fragglescript/t_func.cpp
	fragglescript/t_load.cpp
	fragglescript/t_oper.cpp
	fragglescript/t_parse.cpp
	fragglescript/t_prepro.cpp
	fragglescript/t_script.cpp
	fragglescript/t_spec.cpp
	fragglescript/t_variable.cpp
	fragglescript/t_cmd.cpp
	r_data/colormaps.cpp
	r_data/sprites.cpp
	r_data/voxels.cpp
	r_data/renderstyle.cpp
	r_data/r_interpolate.cpp
	r_data/r_translate.cpp
	zzautozend.cpp
)

set_source_files_properties( xlat/parse_xlat.cpp PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/xlat_parser.c" )
set_source_files_properties( sc_man.cpp PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/sc_man_scanner.h" )
set_source_files_properties( ${NOT_COMPILED_SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE )

if(${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")
	# [BL] Solaris requires these to be explicitly linked.
	set( ZDOOM_LIBS ${ZDOOM_LIBS} nsl socket)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")

target_link_libraries( gloome ${ZDOOM_LIBS} gdtoa dumb lzma )
include_directories( .
	g_doom
	g_heretic
	g_hexen
	g_raven
	g_strife
	g_shared
	sound
	textures
	thingdef
	timidity
	xlat
	../gdtoa
	../dumb/include
	${CMAKE_BINARY_DIR}/gdtoa
	${SYSTEM_SOURCES_DIR} )

add_dependencies( gloome revision_check )

# Due to some quirks, we need to do this in this order
if( NOT ZDOOM_OUTPUT_OLDSTYLE OR NO_GENERATOR_EXPRESSIONS )
	# RUNTIME_OUTPUT_DIRECTORY does not exist in CMake 2.4.
	# Linux distributions are slow to adopt 2.6. :(
	set_target_properties( gloome PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ZDOOM_OUTPUT_DIR} )
	set_target_properties( gloome PROPERTIES OUTPUT_NAME ${ZDOOM_EXE_NAME} )
else( NOT ZDOOM_OUTPUT_OLDSTYLE OR NO_GENERATOR_EXPRESSIONS )
	set_target_properties( gloome PROPERTIES
		RUNTIME_OUTPUT_NAME ${ZDOOM_EXE_NAME}
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${ZDOOM_OUTPUT_DIR}
		RUNTIME_OUTPUT_NAME_DEBUG ${ZDOOM_EXE_NAME}d
		RUNTIME_OUTPUT_DIRECTORY_DEBUG ${ZDOOM_OUTPUT_DIR}
		RUNTIME_OUTPUT_NAME_MINSIZEREL ${ZDOOM_EXE_NAME}msr
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${ZDOOM_OUTPUT_DIR}
		RUNTIME_OUTPUT_NAME_RELWITHDEBINFO ${ZDOOM_EXE_NAME}rd
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${ZDOOM_OUTPUT_DIR}
	)
endif( NOT ZDOOM_OUTPUT_OLDSTYLE OR NO_GENERATOR_EXPRESSIONS )

# if( MSVC )
	# option( ZDOOM_GENERATE_MAPFILE "Generate .map file for debugging." OFF )
	# if( ZDOOM_GENERATE_MAPFILE )
		# set_target_properties(zdoom PROPERTIES LINK_FLAGS "/MANIFEST:NO /DELAYLOAD:\"fmodex${X64}.dll\" /DELAYLOAD:\"openal32.dll\" /DELAYLOAD:\"libmpg123-0.dll\" /DELAYLOAD:\"libsndfile-1.dll\" /MAP")
	# else( ZDOOM_GENERATE_MAPFILE )
		# set_target_properties(zdoom PROPERTIES LINK_FLAGS "/MANIFEST:NO /DELAYLOAD:\"fmodex${X64}.dll\" /DELAYLOAD:\"openal32.dll\" /DELAYLOAD:\"libmpg123-0.dll\" /DELAYLOAD:\"libsndfile-1.dll\"")
	# endif( ZDOOM_GENERATE_MAPFILE )

	# if( NO_GENERATOR_EXPRESSIONS )
		# add_custom_command(TARGET zdoom POST_BUILD
			# COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\win32\\zdoom.exe.manifest\" -outputresource:\"$(TargetDir)$(TargetFileName)\"\;\#1
			# COMMENT "Adding manifest..."
		# )
	# else( NO_GENERATOR_EXPRESSIONS )
		# add_custom_command(TARGET zdoom POST_BUILD
			# COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\win32\\zdoom.exe.manifest\" -outputresource:\"$<TARGET_FILE:zdoom>\"\;\#1
			# COMMENT "Adding manifest..."
		# )
	# endif( NO_GENERATOR_EXPRESSIONS )

	# create_default_target_launcher( zdoom WORKING_DIRECTORY ${ZDOOM_OUTPUT_DIR} )
# endif( MSVC )

if( NOT WIN32 )
	FILE( WRITE ${CMAKE_CURRENT_BINARY_DIR}/link-make "if [ ! -e ${ZDOOM_OUTPUT_DIR}/${ZDOOM_EXE_NAME} ]; then ln -sf ${CMAKE_CURRENT_BINARY_DIR}/${ZDOOM_EXE_NAME} ${ZDOOM_OUTPUT_DIR}/${ZDOOM_EXE_NAME}; fi" )
	add_custom_command( TARGET gloome POST_BUILD
		COMMAND chmod +x ${CMAKE_CURRENT_BINARY_DIR}/link-make
		COMMAND /bin/sh -c ${CMAKE_CURRENT_BINARY_DIR}/link-make )
endif( NOT WIN32 )
if( ZD_CMAKE_COMPILER_IS_GNUCXX_COMPATIBLE )
	# Need to enable intrinsics for this file.
	if( SSE_MATTERS )
		set_source_files_properties( x86.cpp PROPERTIES COMPILE_FLAGS "-msse2 -mmmx" )
	endif( SSE_MATTERS )
endif( ZD_CMAKE_COMPILER_IS_GNUCXX_COMPATIBLE )

if( APPLE )
	set_target_properties(gloome PROPERTIES
		LINK_FLAGS "-framework Carbon -framework Cocoa -framework IOKit -framework OpenGL"
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/posix/osx/zdoom-info.plist" )
endif( APPLE )

source_group("Audio Files" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/sound/.+")
source_group("Audio Files\\Timidity\\Headers" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/timidity/.+\\.h$")
source_group("Audio Files\\Timidity\\Source" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/timidity/.+\\.cpp$")
source_group("Decorate++" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/thingdef/.+")
source_group("FraggleScript" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/fragglescript/.+")
source_group("Games\\Doom Game" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/g_doom/.+")
source_group("Games\\Heretic Game" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/g_heretic/.+")
source_group("Games\\Hexen Game" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/g_hexen/.+")
source_group("Games\\Raven Shared" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/g_raven/.+")
source_group("Games\\Strife Game" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/g_strife/.+")
source_group("Intermission" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/intermission/.+")
source_group("Menu" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/menu/.+")
source_group("OpenGL Renderer" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/gl/.+")
source_group("OpenGL Renderer\\Data" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/gl/data/.+")
source_group("OpenGL Renderer\\Dynamic Lights" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/gl/dynlights/.+")
source_group("OpenGL Renderer\\HQ Resize" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/gl/hqnx/.+")
source_group("OpenGL Renderer\\Models" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/gl/models/.+")
source_group("OpenGL Renderer\\Renderer" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/gl/renderer/.+")
source_group("OpenGL Renderer\\Scene" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/gl/scene/.+")
source_group("OpenGL Renderer\\Shaders" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/gl/shaders/.+")
source_group("OpenGL Renderer\\System" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/gl/system/.+")
source_group("OpenGL Renderer\\Textures" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/gl/textures/.+")
source_group("OpenGL Renderer\\Utilities" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/gl/utility/.+")
source_group("Render Core\\Render Headers" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/r_.+\\.h$")
source_group("Render Core\\Render Sources" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/r_.+\\.cpp$")
source_group("Render Data\\Resource Headers" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/r_data/.+\\.h$")
source_group("Render Data\\Resource Sources" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/r_data/.+\\.cpp$")
source_group("Render Data\\Textures" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/textures/.+")
source_group("Render Interface" FILES r_defs.h r_renderer.h r_sky.cpp r_sky.h r_state.h r_utility.cpp r_utility.h)
source_group("Resource Files" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/resourcefiles/.+")
source_group("POSIX Files" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/posix/.+")
source_group("Cocoa Files" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/posix/cocoa/.+")
source_group("OS X Files" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/posix/osx/.+")
source_group("SDL Files" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/posix/sdl/.+")
source_group("SFMT" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/sfmt/.+")
source_group("Shared Game" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/g_shared/.+")
source_group("Versioning" FILES version.h win32/zdoom.rc)
source_group("Win32 Files" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/win32/.+")
source_group("Xlat" REGULAR_EXPRESSION "^${CMAKE_CURRENT_SOURCE_DIR}/xlat/.+" FILES ${CMAKE_CURRENT_BINARY_DIR}/xlat_parser.c ${CMAKE_CURRENT_BINARY_DIR}/xlat_parser.h)
source_group("Source Files" FILES ${CMAKE_CURRENT_BINARY_DIR}/sc_man_scanner.h sc_man_scanner.re)
