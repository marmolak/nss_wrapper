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

add_cmocka_test(testsuite testsuite.c ${TESTSUITE_LIBRARIES})
set_property(
    TEST
        testsuite
    PROPERTY
        ENVIRONMENT ${TEST_ENVIRONMENT})

add_cmocka_test(test_getaddrinfo test_getaddrinfo.c ${TESTSUITE_LIBRARIES})
set_property(
    TEST
        test_getaddrinfo
    PROPERTY
        ENVIRONMENT ${TEST_ENVIRONMENT})

add_cmocka_test(test_getnameinfo test_getnameinfo.c ${TESTSUITE_LIBRARIES})
set_property(
    TEST
        test_getnameinfo
    PROPERTY
        ENVIRONMENT ${TEST_ENVIRONMENT})

add_cmocka_test(test_gethostby_name_addr test_gethostby_name_addr.c ${TESTSUITE_LIBRARIES})
set_property(
    TEST
        test_gethostby_name_addr
    PROPERTY
        ENVIRONMENT ${TEST_ENVIRONMENT})
