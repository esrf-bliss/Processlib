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


option(PROCESSLIB_ENABLE_PYTHON "compile python binding code?" ON)
# If processlib is subdired from Lima it must follow Lima policy for python option
if(LIMA_ENABLE_PYTHON)
  set(PROCESSLIB_ENABLE_PYTHON ON)
else()
  set(PROCESSLIB_ENABLE_PYTHON OFF)
endif()

if (WIN32)  # GSL is installed in exotic folders on Windows
    set(GSL_INCLUDE_DIR "/usr/include" CACHE PATH "Path to GSL includes")
    set(GSL_LIB_DIR "/usr/lib" CACHE PATH "Path to GSL lib folder")
    include_directories(${GSL_INCLUDE_DIR})
    link_directories(${GSL_LIB_DIR})
endif()

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

file(STRINGS "VERSION" processlib_ver)
add_library(processlib SHARED
            ${processlib_srcs})
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
    find_library(GSL libgsl ${GSL_LIB_DIR})
    find_library(GSLCBLAS libgslcblas ${GSL_LIB_DIR})
    target_link_libraries(processlib ${GSL} ${GSLCBLAS})
else()
    target_link_libraries(processlib gsl gslcblas)
endif()
if(WIN32)
    target_compile_definitions(processlib PRIVATE LIBPROCESSLIB_EXPORTS)
endif()

set_target_properties(processlib PROPERTIES VERSION ${processlib_ver} SOVERSION 1)
if(WIN32)
    set_target_properties(processlib PROPERTIES PREFIX "lib")
endif()
if(WIN32)
    install(TARGETS processlib
            DESTINATION lib)
else()
    install(TARGETS processlib
            LIBRARY DESTINATION lib)
endif()

if(CMAKE_SYSTEM_NAME EQUAL "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SIP_DISABLE_FEATURES "BPM_SUPPORT")
    endif()
endif()

if(PROCESSLIB_ENABLE_PYTHON)
  configure_file(${CMAKE_SOURCE_DIR}/third-party/Processlib/sip/processlib.sip sip/processlib.sip)
  set(SIP_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/tasks/sip")
  add_sip_python_module(processlib "${CMAKE_CURRENT_BINARY_DIR}/sip/processlib.sip")
  target_include_directories(python_module_processlib PRIVATE ${PYTHON_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/sip)
  target_link_libraries(python_module_processlib processlib)
endif()