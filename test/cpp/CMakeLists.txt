macro(add_utf_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name}
        processlib
        Boost::unit_test_framework
        Boost::disable_autolinking
    )
    add_test(NAME ${name} COMMAND ${name})
endmacro(add_utf_test)

add_utf_test(test_data)
add_utf_test(test_pool_thread_mgr)
