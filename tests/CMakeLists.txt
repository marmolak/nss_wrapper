project(tests C)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMOCKA_INCLUDE_DIR}
)

set(TESTSUITE_LIBRARIES ${NWRAP_REQUIRED_LIBRARIES} ${CMOCKA_LIBRARY})

set(HOMEDIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(passwd.in ${CMAKE_CURRENT_BINARY_DIR}/passwd @ONLY)
configure_file(group.in ${CMAKE_CURRENT_BINARY_DIR}/group @ONLY)
configure_file(hosts.in ${CMAKE_CURRENT_BINARY_DIR}/hosts @ONLY)

set(TEST_ENVIRONMENT LD_PRELOAD=${CMAKE_BINARY_DIR}/src/libnss_wrapper.so;NSS_WRAPPER_PASSWD=${CMAKE_CURRENT_BINARY_DIR}/passwd;NSS_WRAPPER_GROUP=${CMAKE_CURRENT_BINARY_DIR}/group;NSS_WRAPPER_HOSTS=${CMAKE_CURRENT_BINARY_DIR}/hosts)

set(NWRAP_TESTS testsuite test_getaddrinfo test_getnameinfo test_gethostby_name_addr)

foreach(_NWRAP_TEST ${NWRAP_TESTS})
    add_cmocka_test(${_NWRAP_TEST} ${_NWRAP_TEST}.c ${TESTSUITE_LIBRARIES})
    set_property(
        TEST
            ${_NWRAP_TEST}
        PROPERTY
            ENVIRONMENT ${TEST_ENVIRONMENT})
endforeach()

# Test nwrap without wrapping so the libc functions are called
add_cmocka_test(test_nwrap_disabled test_nwrap_disabled.c ${TESTSUITE_LIBRARIES})
set_property(
    TEST
        test_nwrap_disabled
    PROPERTY
        ENVIRONMENT LD_PRELOAD=${CMAKE_BINARY_DIR}/src/libnss_wrapper.so)
