macro(add_utf_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name}
        core-headers
        Boost::unit_test_framework
    )
    add_test(NAME ${name} COMMAND ${name})
endmacro(add_utf_test)

add_utf_test(test_typed_data)
add_utf_test(test_data)

# add_executable(test_core)
#     test_core.cpp
#     test_typed_data.cpp
#     test_data.cpp
# )
# 
# target_link_libraries(test_core
#     core-headers
#     Boost::unit_test_framework
# )
# 
# add_test(NAME test_core COMMAND test_core)
