add_library(win_compat STATIC
    pthread_cancelling.cpp
    pthread_cond.cpp
    pthread_key.cpp
    pthread_mutex.cpp
    pthread_rwlock.cpp
    pthread_thread.cpp
    stdio_compat.cpp
    time_compat.cpp)

target_include_directories(win_compat PUBLIC
    ${CMAKE_SOURCE_DIR}/core/include/
    ${CMAKE_SOURCE_DIR}/core/include/WindowSpecific)
