cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(iron CXX)
set(CMAKE_CXX_STANDARD 20)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    "ReleaseDebugLoopPerformance"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
    set(CMAKE_CXX_FLAGS_RELEASEDEBUGLOOPPERFORMANCE "")
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASEDEBUGLOOPPERFORMANCE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASEDEBUGLOOPPERFORMANCE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASEDEBUGLOOPPERFORMANCE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASEDEBUGLOOPPERFORMANCE "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    ".gitignore"
    "Config.cpp"
    "Config.h"
    "config.json"
    "iracing.cpp"
    "iracing.h"
    "irsdk/irsdk_diskclient.cpp"
    "irsdk/irsdk_diskclient.h"
    "LICENSE"
    "main.cpp"
    "Overlay.cpp"
    "Overlay.h"
    "OverlayCover.h"
    "OverlayDDU.h"
    "OverlayDebug.cpp"
    "OverlayDebug.h"
    "OverlayInputs.h"
    "OverlayRadar.h"
    "OverlayRelative.h"
    "OverlayStandings.h"
    "OverlayTurnNumber.h"
    "picojson.h"
    "README.md"
    "util.h"
)
source_group("" FILES ${no_group_source_files})

set(irsdk
    "irsdk/irsdk_client.cpp"
    "irsdk/irsdk_client.h"
    "irsdk/irsdk_defines.h"
    "irsdk/irsdk_utils.cpp"
    "irsdk/yaml_parser.cpp"
    "irsdk/yaml_parser.h"
)
source_group("irsdk" FILES ${irsdk})

set(ALL_FILES
    ${no_group_source_files}
    ${irsdk}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE iron)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASEDEBUGLOOPPERFORMANCE "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_RELEASE                     "TRUE"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
    $<$<CONFIG:Debug>:
        MultiThreadedDebug
    >
    $<$<CONFIG:ReleaseDebugLoopPerformance>:
        MultiThreaded
    >
    $<$<CONFIG:Release>:
        MultiThreaded
    >
    $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:ReleaseDebugLoopPerformance>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:ReleaseDebugLoopPerformance>:"
        "NDEBUG;"
        "DEBUG_OVERLAY_TIME"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "PICOJSON_USE_RVALUE_REFERENCE=0;"
    "NOMINMAX;"
    "_CRT_SECURE_NO_WARNINGS;"
    "_CONSOLE;"
    "_MBCS"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:ReleaseDebugLoopPerformance>:
            /Oi;
            /Gy
        >
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        /permissive-;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /INCREMENTAL
        >
        $<$<CONFIG:ReleaseDebugLoopPerformance>:
            /OPT:REF;
            /OPT:ICF;
            /PROFILE;
            /INCREMENTAL:NO
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF;
            /PROFILE;
            /INCREMENTAL:NO
        >
        /DEBUG;
        /SUBSYSTEM:CONSOLE
    )
endif()

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "kernel32;"
    "user32;"
    "gdi32;"
    "winspool;"
    "comdlg32;"
    "advapi32;"
    "shell32;"
    "ole32;"
    "oleaut32;"
    "uuid;"
    "odbc32;"
    "odbccp32"
)
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

