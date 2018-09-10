add_library(core OBJECT
    src/GslErrorMgr.cpp
    src/PoolThreadMgr.cpp
    src/TaskEventCallback.cpp
    src/LinkTask.cpp
    src/SinkTask.cpp
    src/TaskMgr.cpp
    src/HeaderContainer.cpp)

target_include_directories(core PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include/processlib>")

target_include_directories(core PRIVATE
    ${GSL_INCLUDE_DIRS})

if(WIN32)
    add_subdirectory(WindowSpecific)
    target_link_libraries(core PRIVATE win_compat)
endif()
