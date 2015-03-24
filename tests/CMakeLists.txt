project(tests C)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMOCKA_INCLUDE_DIR}
)

set(TESTSUITE_LIBRARIES ${NWRAP_REQUIRED_LIBRARIES} ${CMOCKA_LIBRARY})

add_library(nss_nwrap SHARED nss_nwrap.c)

set(HOMEDIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(passwd.in ${CMAKE_CURRENT_BINARY_DIR}/passwd @ONLY)
configure_file(group.in ${CMAKE_CURRENT_BINARY_DIR}/group @ONLY)
configure_file(hosts.in ${CMAKE_CURRENT_BINARY_DIR}/hosts @ONLY)
configure_file(shadow.in ${CMAKE_CURRENT_BINARY_DIR}/shadow @ONLY)

set(TEST_ENVIRONMENT LD_PRELOAD=${NSS_WRAPPER_LOCATION})
list(APPEND TEST_ENVIRONMENT NSS_WRAPPER_PASSWD=${CMAKE_CURRENT_BINARY_DIR}/passwd)
list(APPEND TEST_ENVIRONMENT NSS_WRAPPER_GROUP=${CMAKE_CURRENT_BINARY_DIR}/group)
list(APPEND TEST_ENVIRONMENT NSS_WRAPPER_SHADOW=${CMAKE_CURRENT_BINARY_DIR}/shadow)
list(APPEND TEST_ENVIRONMENT NSS_WRAPPER_HOSTS=${CMAKE_CURRENT_BINARY_DIR}/hosts)
list(APPEND TEST_ENVIRONMENT NSS_WRAPPER_MODULE_SO_PATH=${CMAKE_CURRENT_BINARY_DIR}/libnss_nwrap.so)
list(APPEND TEST_ENVIRONMENT NSS_WRAPPER_MODULE_FN_PREFIX=nwrap)

set(NWRAP_TESTS
    testsuite
    test_nwrap_vector
    test_getaddrinfo
    test_getnameinfo
    test_gethostby_name_addr
    test_gethostent)

if (HAVE_SHADOW_H)
    list(APPEND NWRAP_TESTS test_shadow)
endif (HAVE_SHADOW_H)

foreach(_NWRAP_TEST ${NWRAP_TESTS})
    add_cmocka_test(${_NWRAP_TEST} ${_NWRAP_TEST}.c ${TESTSUITE_LIBRARIES})
    set_property(
        TEST
            ${_NWRAP_TEST}
        PROPERTY
            ENVIRONMENT ${TEST_ENVIRONMENT})
endforeach()

if (HAVE_SHADOW_H)
    # This is needed to check the hash in tests/shadow.in
    target_link_libraries(test_shadow crypt)
endif (HAVE_SHADOW_H)

target_link_libraries(test_nwrap_vector ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test_gethostby_name_addr ${CMAKE_THREAD_LIBS_INIT})

if (BSD)
    add_definitions(-DBSD)
endif (BSD)

# Test nwrap without wrapping so the libc functions are called
add_cmocka_test(test_nwrap_disabled test_nwrap_disabled.c ${TESTSUITE_LIBRARIES})
set_property(
    TEST
        test_nwrap_disabled
    PROPERTY
        ENVIRONMENT LD_PRELOAD=${CMAKE_BINARY_DIR}/src/libnss_wrapper.so)
