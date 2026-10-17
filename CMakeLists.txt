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
set_source_files_properties(compat/oc4_boost_python.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
