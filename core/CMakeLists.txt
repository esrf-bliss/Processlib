add_library(core OBJECT
    src/GslErrorMgr.cpp
    src/PoolThreadMgr.cpp
    src/TaskEventCallback.cpp
    src/LinkTask.cpp
    src/SinkTask.cpp
    src/TaskMgr.cpp
    src/Metadata.cpp
)

if(NOT PROCESSLIB_ENABLE_GSL)
    target_compile_definitions(core
        PUBLIC PROCESSLIB_WITHOUT_GSL
    )
endif()

target_include_directories(core PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include/processlib>"
)

target_include_directories(core PRIVATE
    ${GSL_INCLUDE_DIRS})

# Headers only (used by tests)
add_library(core-headers INTERFACE)

target_include_directories(core-headers INTERFACE 
    "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>"
    include/
)
