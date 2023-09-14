###########################################################################
# This file is part of ProcessLib, a submodule of LImA project the
# Library for Image Acquisition
#
# Copyright (C) : 2009-2017
# European Synchrotron Radiation Facility
# CS40220 38043 Grenoble Cedex 9
# FRANCE
#
# Contact: lima@esrf.fr
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################

# For installation purpose one can set the following variables...
#
# - CMAKE_INSTALL_PREFIX:
#  Set this to a installation directory, default for UNIX is /usr/local
#
# - PYTHON_SITE_PACKAGES_DIR:
# Set this to an alternative python site-packages directory,
# default is the python installation directory, for UNIX it
# can be /usr/lib/python2.7/dist-packages/ .

cmake_minimum_required(VERSION 3.8)

project (processlib)
include(cmake/project_version.cmake)

# Include additional modules that are used unconditionally
include(GNUInstallDirs)
include(GenerateExportHeader)

# If conda build, always set lib dir to 'lib'
if($ENV{CONDA_BUILD})
  set(CMAKE_INSTALL_LIBDIR "lib")
endif()

# Set lower / upper case project names
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)

# Check if processlib is being used directly or via add_subdirectory
set(PROCESSLIB_MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(PROCESSLIB_MASTER_PROJECT ON)
endif()

# Enable C++11 and later
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC AND MSVC_VERSION GREATER 1500)
    # Compiler specific (parallel build)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# cmake directory for additional macros
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Threads lib required
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Enable GSL processing
option(PROCESSLIB_ENABLE_GSL "enable GSL related processing?" ON)
if(PROCESSLIB_ENABLE_GSL)
    find_package(GSL REQUIRED)
endif(PROCESSLIB_ENABLE_GSL)

# Enable python binding code compilation using sip generator
option(PROCESSLIB_ENABLE_PYTHON "compile python binding code?" ${LIMA_ENABLE_PYTHON})
if(PROCESSLIB_ENABLE_PYTHON)
    find_package(PythonInterp)
    find_package(PythonLibs)
    if(${PYTHONINTERP_FOUND})
      # python site-packages folder
      execute_process(
          COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print (get_python_lib())"
          OUTPUT_VARIABLE _PYTHON_SITE_PACKAGES_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    find_package(SIP)
    if(SIP_FOUND)
        include(SIPMacros)

        if(WIN32)
            set(SIP_TAGS WIN32_PLATFORM)
        elseif(UNIX)
            set(SIP_TAGS POSIX_PLATFORM)
        endif(WIN32)
        set(SIP_EXTRA_OPTIONS -e -g)
    else()
  	    message(FATAL_ERROR "python SIP not installed, please install or disable PROCESSLIB_ENABLE_PYTHON")
    endif()

    set(PYTHON_SITE_PACKAGES_DIR ${_PYTHON_SITE_PACKAGES_DIR} CACHE PATH "where should python modules be installed?")

    find_package(NumPy)
    if(${NUMPY_FOUND})
        include_directories(${NUMPY_INCLUDE_DIRS})
        link_directories(${NUMPY_LIBRARIES})
    else()
        message(FATAL_ERROR "NumPy not found, cannot build python extensions")
    endif()
endif()

set(processlib_srcs
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/project_version.cc # Add version info
    core/src/GslErrorMgr.cpp
    core/src/PoolThreadMgr.cpp
    core/src/TaskEventCallback.cpp
    core/src/LinkTask.cpp
    core/src/SinkTask.cpp
    core/src/TaskMgr.cpp
    tasks/src/BackgroundSubstraction.cpp
    tasks/src/FlatfieldCorrection.cpp
    tasks/src/RoiCounter.cpp
    tasks/src/Binning.cpp
    tasks/src/Flip.cpp
    tasks/src/SoftRoi.cpp
    tasks/src/Bpm.cpp
    tasks/src/Mask.cpp
    tasks/src/Roi2Spectrum.cpp
    tasks/src/Rotation.cpp
    tasks/src/PeakFinder.cpp
    tasks/src/RoiCollection.cpp
)
if(WIN32)
    list(APPEND processlib_srcs
        core/src/win/stdio_compat.cpp
        core/src/win/time_compat.cpp
    )
endif()

file(GLOB_RECURSE core_incs "core/include/*.h")
file(GLOB_RECURSE tasks_incs "tasks/include/*.h")

add_library(processlib SHARED
    ${processlib_srcs}
    ${core_incs}
    ${tasks_incs}
)

# Generate export macros
generate_export_header(processlib)

# Set version
set_target_properties(processlib PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}")

if(PROCESSLIB_ENABLE_GSL)
    target_link_libraries(processlib
        PRIVATE GSL::gsl
        PRIVATE GSL::gslcblas
    )
else()
    target_compile_definitions(processlib
        PUBLIC PROCESSLIB_WITHOUT_GSL
    )
endif(PROCESSLIB_ENABLE_GSL)

target_include_directories(processlib PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tasks/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(processlib PUBLIC Threads::Threads)

if(WIN32)
    find_library(PTHREAD_WIN32_LIB pthread)
    target_link_libraries(processlib PUBLIC ${PTHREAD_WIN32_LIB})
    target_compile_definitions(processlib PRIVATE LIBPROCESSLIB_EXPORTS)
    set_target_properties(processlib PROPERTIES PREFIX "lib")
    set_target_properties(processlib PROPERTIES IMPORT_PREFIX "lib")
endif()

# SIP generated binding code for python
if(PROCESSLIB_ENABLE_PYTHON)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sip/processlib.sip sip/processlib.sip COPYONLY)
    set(SIP_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tasks/sip")
    # Disable BPM is GLS is not enabled
    if(NOT PROCESSLIB_ENABLE_GSL)
        set(SIP_DISABLE_FEATURES "BPM_SUPPORT")
    endif(NOT PROCESSLIB_ENABLE_GSL)
    add_sip_python_module(processlib "${CMAKE_CURRENT_BINARY_DIR}/sip/processlib.sip")
    target_include_directories(python_module_processlib PRIVATE ${PYTHON_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/sip)
    target_link_libraries(python_module_processlib PRIVATE processlib)
endif()

## Installation

# Generate and install package config file and version
if(PROCESSLIB_MASTER_PROJECT)
    set(PROJECT_LIBRARIES processlib)
    set(SIP_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/sip/processlib)
    include(cmake/package_config.cmake)
endif()

install(
    TARGETS processlib
    EXPORT "${TARGETS_EXPORT_NAME}"
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}   # import library
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # .so files are libraries
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # .dll files are binaries
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}  # this does not actually install anything (but used by downstream projects)
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/project_version.h
        ${CMAKE_CURRENT_BINARY_DIR}/processlib_export.h
    COMPONENT devel
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/processlib
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/core/include/processlib
    COMPONENT devel
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.inl"
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tasks/include/processlib
    COMPONENT devel
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

if(WIN32)
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/core/include/win
        COMPONENT devel
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/processlib
        FILES_MATCHING PATTERN "*.h"
    )
endif()

if(PROCESSLIB_ENABLE_PYTHON AND PROCESSLIB_MASTER_PROJECT)
    install(
        FILES sip/processlib.sip
              tasks/sip/BackgroundSubstraction.sip
              tasks/sip/Binning.sip
              tasks/sip/Bpm.sip
              tasks/sip/FlatfieldCorrection.sip
              tasks/sip/Flip.sip
              tasks/sip/Mask.sip
              tasks/sip/PeakFinder.sip
              tasks/sip/Roi2Spectrum.sip
              tasks/sip/RoiCounter.sip
              tasks/sip/Rotation.sip
              tasks/sip/SoftRoi.sip
              tasks/sip/RoiCollection.sip
        COMPONENT devel
        DESTINATION ${SIP_INSTALL_DIR}
    )
endif()

## Tests
if(PROCESSLIB_ENABLE_TESTS)
    enable_testing()
    #add_subdirectory(test)
endif()

# Packages
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ESRF/BLISS") #required
set(CPACK_PACKAGE_CONTACT "lima@esrf.fr")
include(CPack)
