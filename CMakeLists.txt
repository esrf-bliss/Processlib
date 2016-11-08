IF (WIN32)  # GSL is installed in exotic folders on Windows
    SET(GSL_INCLUDE_DIR "/usr/include" CACHE PATH "Path to GSL includes")
    SET(GSL_LIB_DIR "/usr/lib" CACHE PATH "Path to GSL lib folder")
    include_directories(${GSL_INCLUDE_DIR})
    link_directories(${GSL_LIB_DIR})
ENDIF()

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

file(STRINGS "VERSION" processlib_ver)
add_library(processlib SHARED
            ${processlib_srcs})
IF(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/core/include/WindowSpecific")
ENDIF()
TARGET_INCLUDE_DIRECTORIES(processlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/include")
TARGET_INCLUDE_DIRECTORIES(processlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/tasks/include")
if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(PUBLIC processlib "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(processlib "${CMAKE_THREAD_LIBS_INIT}")
endif()
IF(WIN32)
    FIND_LIBRARY(GSL libgsl ${GSL_LIB_DIR})
    FIND_LIBRARY(GSLCBLAS libgslcblas ${GSL_LIB_DIR})
    TARGET_LINK_LIBRARIES(processlib ${GSL} ${GSLCBLAS})
ELSE()
    TARGET_LINK_LIBRARIES(processlib gsl gslcblas)
ENDIF()
SET_TARGET_PROPERTIES(processlib PROPERTIES VERSION ${processlib_ver} SOVERSION 1)
if(WIN32)
    INSTALL(TARGETS processlib
            DESTINATION lib)
else()
    INSTALL(TARGETS processlib
            LIBRARY DESTINATION lib)
endif()

if(CMAKE_SYSTEM_NAME EQUAL "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SIP_DISABLE_FEATURES "BPM_SUPPORT")
    endif()
endif()

set(SIP_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/tasks/sip")
ADD_SIP_PYTHON_MODULE(processlib sip/processlib.sip)
TARGET_INCLUDE_DIRECTORIES(python_module_processlib PRIVATE ${PYTHON_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/sip)
TARGET_LINK_LIBRARIES(python_module_processlib processlib)
