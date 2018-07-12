###########################################################################
# This file is part of LImA, a Library for Image Acquisition
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

# README -- README --README -- README --README -- README --README -- README
# To include Processlib as a sub-project within another project (e.g LIMA)
# set PROCESSLIB_EXTERNALLY_CONFIGURED to ON and supply value for following variables...
#
# - PROCESSLIB_EXTERNALLY_ENABLE_PYTHON:
#   Set this "ON" to create binding codes for python
#
# For installation purpose one can set the following variables...
#
# - CMAKE_INSTALL_PREFIX:
#  Set this to a installation directory, default for UNIX is /usr/local
#
# - PYTHON_SITE_PACKAGES_DIR:
# Set this to an alternative python site-packages directory,
# default is the python installation directory, for UNIX it
# can be /usr/lib/python2.7/dist-packages/ .


#--------------------------------------------------------------------------------
# Enable/Disable python binding but depending if project is included as a
# sub-project, see above for further instructions
#--------------------------------------------------------------------------------
if (NOT PROCESSLIB_EXTERNALLY_CONFIGURED)
  option(PROCESSLIB_ENABLE_PYTHON "compile python binding code?" OFF)
endif()
if (PROCESSLIB_EXTERNALLY_CONFIGURED)
  if (PROCESSLIB_EXTERNALLY_ENABLE_PYTHON)
    set(PROCESSLIB_ENABLE_PYTHON ON)
  else()
    set(PROCESSLIB_ENABLE_PYTHON OFF)
  endif()
endif()

#--------------------------------------------------------------------------------
# cmake/ directory contains tools
#--------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
include(ProcesslibTools)

#--------------------------------------------------------------------------------
# If Processlib is used as a standalone library set here everything required
#--------------------------------------------------------------------------------
if (NOT PROCESSLIB_EXTERNALLY_CONFIGURED)
  cmake_minimum_required(VERSION 2.8)
  project (processlib)

  #--------------------------------------------------------------------------------
  # Threads lib required
  #--------------------------------------------------------------------------------
  find_package(Threads REQUIRED)

  #--------------------------------------------------------------------------------
  # Enable python binding code compilation using sip generator
  #--------------------------------------------------------------------------------
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
    include(SIPMacros)

    set(PYTHON_SITE_PACKAGES_DIR ${_PYTHON_SITE_PACKAGES_DIR} CACHE PATH "where should python modules be installed?")

    find_package(NumPy)
    if(${NUMPY_FOUND})
      include_directories(${NUMPY_INCLUDE_DIRS})
      link_directories(${NUMPY_LIBRARIES})
    else()
        message(FATAL_ERROR "NumPy not found, cannot build python extensions")
    endif()
    if(WIN32)
      set(SIP_TAGS WIN32_PLATFORM)
    elseif(UNIX)
      set(SIP_TAGS POSIX_PLATFORM)
    endif(WIN32)
    set(SIP_EXTRA_OPTIONS -e -g)
  endif()

endif(NOT PROCESSLIB_EXTERNALLY_CONFIGURED)

set(processlib_srcs
    core/src/GslErrorMgr.cpp
    core/src/PoolThreadMgr.cpp
    core/src/TaskEventCallback.cpp
    core/src/LinkTask.cpp
    core/src/SinkTask.cpp
    core/src/TaskMgr.cpp
    core/src/HeaderContainer.cpp
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
    )
if(WIN32)
    list(APPEND processlib_srcs core/src/WindowSpecific/pthread_cancelling.cpp
                                core/src/WindowSpecific/pthread_cond.cpp
                                core/src/WindowSpecific/pthread_key.cpp
                                core/src/WindowSpecific/pthread_mutex.cpp
                                core/src/WindowSpecific/pthread_rwlock.cpp
                                core/src/WindowSpecific/pthread_thread.cpp
                                core/src/WindowSpecific/stdio_compat.cpp
                                core/src/WindowSpecific/time_compat.cpp
				)
endif()

find_package(GSL REQUIRED)

#--------------------------------------------------------------------------------
# Add the library processlib definition
#--------------------------------------------------------------------------------
file(STRINGS "VERSION" processlib_ver)
add_library(processlib SHARED
            ${processlib_srcs})
processlib_set_library_soversion(processlib "VERSION")

target_link_libraries(processlib GSL::gsl GSL::gslcblas)
target_include_directories(processlib PUBLIC ${GSL_INCLUDE_DIRS})

if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/core/include/WindowSpecific")
endif()
target_include_directories(processlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/include")
target_include_directories(processlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/tasks/include")
if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(PUBLIC processlib "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(processlib "${CMAKE_THREAD_LIBS_INIT}")
endif()

if(WIN32)
    target_compile_definitions(processlib PRIVATE LIBPROCESSLIB_EXPORTS)
    set_target_properties(processlib PROPERTIES PREFIX "lib")
endif()

if(WIN32)
    install(TARGETS processlib
            LIBRARY DESTINATION lib
            RUNTIME DESTINATION bin)
else()
    include(GNUInstallDirs)
    install(TARGETS processlib
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/core/include/processlib
    DESTINATION include/processlib
    FILES_MATCHING PATTERN "*.h")

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/tasks/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h")

#--------------------------------------------------------------------------------
# SIP generates binding code for python
#--------------------------------------------------------------------------------
if(PROCESSLIB_ENABLE_PYTHON)
  if(SIP_VERSION_STR VERSION_LESS "4.12")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sip/processlib_before_4_12.sip sip/processlib.sip)
  else()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sip/processlib.sip sip/processlib.sip)
  endif()
  set(SIP_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/tasks/sip")
  add_sip_python_module(processlib "${CMAKE_CURRENT_BINARY_DIR}/sip/processlib.sip")
  target_include_directories(python_module_processlib PRIVATE ${PYTHON_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/sip)
  target_link_libraries(python_module_processlib processlib)
endif()
