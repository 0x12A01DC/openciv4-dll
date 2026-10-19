# x64 build of the Civ4 BtS SDK game core (CvGameCoreDLL) against Python 2.7 + pybind11.

set(OC4_PYTHON27_DIR "${PROJECT_SOURCE_DIR}/external/python27" CACHE PATH "Python 2.7 x64 (include/, libs/python27.lib, python27.dll)")
set(OC4_PYBIND11_DIR "${PROJECT_SOURCE_DIR}/external/pybind11-2.9.2" CACHE PATH "pybind11 2.9.x, the last series supporting Python 2.7")

if(NOT EXISTS "${OC4_PYTHON27_DIR}/include/Python.h" OR NOT EXISTS "${OC4_PYBIND11_DIR}/include/pybind11/pybind11.h")
	message(FATAL_ERROR "Python 2.7 / pybind11 not found: set OC4_PYTHON27_DIR and OC4_PYBIND11_DIR (see README)")
endif()

add_library(oc4_python27 INTERFACE)
target_include_directories(oc4_python27 SYSTEM INTERFACE "${OC4_PYTHON27_DIR}/include" "${OC4_PYBIND11_DIR}/include")
target_link_libraries(oc4_python27 INTERFACE "${OC4_PYTHON27_DIR}/libs/python27.lib")

# The vanilla game core; its headers are what the host is compiled against.
# CvTextScreens.cpp is not part of the original project (references a missing CvTextMgr.h).
oc4_add_game_core(CvGameCoreDLL SOURCE_DIR CvGameCoreDLL EXCLUDE CvTextScreens.cpp PUBLIC_HEADERS)

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
