# x64 build of the Civ4 BtS SDK game core (CvGameCoreDLL) against Python 2.7 + pybind11.

set(OC4_PYTHON27_DIR "${PROJECT_SOURCE_DIR}/external/python27" CACHE PATH "Python 2.7 x64 (include/, libs/python27.lib, python27.dll)")
set(OC4_PYBIND11_DIR "${PROJECT_SOURCE_DIR}/external/pybind11-2.9.2" CACHE PATH "pybind11 2.9.x, the last series supporting Python 2.7")

if(NOT EXISTS "${OC4_PYTHON27_DIR}/include/Python.h" OR NOT EXISTS "${OC4_PYBIND11_DIR}/include/pybind11/pybind11.h")
	message(FATAL_ERROR "Python 2.7 / pybind11 not found: set OC4_PYTHON27_DIR and OC4_PYBIND11_DIR (see README)")
endif()

add_library(oc4_python27 INTERFACE)
target_include_directories(oc4_python27 SYSTEM INTERFACE "${OC4_PYTHON27_DIR}/include" "${OC4_PYBIND11_DIR}/include")
target_link_libraries(oc4_python27 INTERFACE "${OC4_PYTHON27_DIR}/libs/python27.lib")

file(GLOB GAMECORE_SOURCES CONFIGURE_DEPENDS CvGameCoreDLL/*.cpp)
# Not part of the original project (references a missing CvTextMgr.h).
list(REMOVE_ITEM GAMECORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/CvGameCoreDLL/CvTextScreens.cpp")
add_library(CvGameCoreDLL SHARED ${GAMECORE_SOURCES} compat/oc4_boost_python.cpp)
target_include_directories(CvGameCoreDLL PUBLIC CvGameCoreDLL compat)
target_link_libraries(CvGameCoreDLL PUBLIC oc4_python27 PRIVATE winmm)
target_compile_definitions(CvGameCoreDLL
	PUBLIC WIN32 _WINDOWS
	PRIVATE _USRDLL CVGAMECOREDLL_EXPORTS _CRT_SECURE_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS _SCL_SECURE_NO_WARNINGS
		$<$<NOT:$<CONFIG:Debug>>:NDEBUG FINAL_RELEASE>)
set_target_properties(CvGameCoreDLL PROPERTIES CXX_STANDARD 20 CXX_SCAN_FOR_MODULES OFF FOLDER "gamecore")
if(MSVC)
	# The SDK is VC7.1-era code: lenient conformance, its own code page, quiet legacy warnings.
	target_compile_options(CvGameCoreDLL PRIVATE /permissive /Zc:forScope- /W3 /EHsc /bigobj /MP
		/source-charset:windows-1252 /execution-charset:windows-1252
		/wd4244 /wd4267 /wd4305 /wd4800 /wd4996 /wd4251 /wd4018 /wd4805 /wd4838)
endif()
target_precompile_headers(CvGameCoreDLL PRIVATE CvGameCoreDLL/CvGameCoreDLL.h)

# Profile-guided optimisation of the game core, driven by tools\pgo.cmd: GEN links an
# instrumented DLL that counts what AI autoplay runs through, USE links the DLL optimised
# with those counts. Only the link step differs, so switching modes relinks the DLL.
set(OC4_PGO "OFF" CACHE STRING "Profile-guided optimisation of the game core: OFF, GEN or USE")
set_property(CACHE OC4_PGO PROPERTY STRINGS OFF GEN USE)
set(OC4_PGO_PGD "${CMAKE_BINARY_DIR}/pgo/CvGameCoreDLL.pgd" CACHE FILEPATH "Profile database for OC4_PGO")
if(MSVC AND NOT OC4_PGO STREQUAL "OFF")
	if(NOT CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE)
		message(FATAL_ERROR "OC4_PGO needs link-time code generation (OC4_LTO)")
	endif()
	get_filename_component(OC4_PGO_DIR "${OC4_PGO_PGD}" DIRECTORY)
	file(MAKE_DIRECTORY "${OC4_PGO_DIR}")
	if(OC4_PGO STREQUAL "GEN")
		target_link_options(CvGameCoreDLL PRIVATE "$<$<CONFIG:Release>:/GENPROFILE:PGD=${OC4_PGO_PGD}>")
	elseif(OC4_PGO STREQUAL "USE")
		if(NOT EXISTS "${OC4_PGO_PGD}")
			message(FATAL_ERROR "No profile at ${OC4_PGO_PGD}: run tools\\pgo.cmd")
		endif()
		target_link_options(CvGameCoreDLL PRIVATE "$<$<CONFIG:Release>:/USEPROFILE:PGD=${OC4_PGO_PGD}>")
		set_property(TARGET CvGameCoreDLL APPEND PROPERTY LINK_DEPENDS "${OC4_PGO_PGD}")
	else()
		message(FATAL_ERROR "OC4_PGO must be OFF, GEN or USE")
	endif()
endif()
set_source_files_properties(compat/oc4_boost_python.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
