include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckStructHasMember)
include(CheckPrototypeDefinition)
include(TestBigEndian)

set(PACKAGE ${APPLICATION_NAME})
set(VERSION ${APPLICATION_VERSION})
set(DATADIR ${DATA_INSTALL_DIR})
set(LIBDIR ${LIB_INSTALL_DIR})
set(PLUGINDIR "${PLUGIN_INSTALL_DIR}-${LIBRARY_SOVERSION}")
set(SYSCONFDIR ${SYSCONF_INSTALL_DIR})

set(BINARYDIR ${CMAKE_BINARY_DIR})
set(SOURCEDIR ${CMAKE_SOURCE_DIR})

function(COMPILER_DUMPVERSION _OUTPUT_VERSION)
    # Remove whitespaces from the argument.
    # This is needed for CC="ccache gcc" cmake ..
    string(REPLACE " " "" _C_COMPILER_ARG "${CMAKE_C_COMPILER_ARG1}")

    execute_process(
        COMMAND
            ${CMAKE_C_COMPILER} ${_C_COMPILER_ARG} -dumpversion
        OUTPUT_VARIABLE _COMPILER_VERSION
    )

    string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1\\2"
           _COMPILER_VERSION "${_COMPILER_VERSION}")

    set(${_OUTPUT_VERSION} ${_COMPILER_VERSION} PARENT_SCOPE)
endfunction()

if(CMAKE_COMPILER_IS_GNUCC AND NOT MINGW AND NOT OS2)
    compiler_dumpversion(GNUCC_VERSION)
    if (NOT GNUCC_VERSION EQUAL 34)
        set(CMAKE_REQUIRED_FLAGS "-fvisibility=hidden")
        check_c_source_compiles(
"void __attribute__((visibility(\"default\"))) test() {}
int main(void){ return 0; }
" WITH_VISIBILITY_HIDDEN)
        set(CMAKE_REQUIRED_FLAGS "")
    endif (NOT GNUCC_VERSION EQUAL 34)
endif(CMAKE_COMPILER_IS_GNUCC AND NOT MINGW AND NOT OS2)

# HEADERS
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(pwd.h HAVE_PWD_H)
check_include_file(grp.h HAVE_GRP_H)
check_include_file(nss.h HAVE_NSS_H)
check_include_file(nss_common.h HAVE_NSS_COMMON_H)

# FUNCTIONS
check_function_exists(strncpy HAVE_STRNCPY)
check_function_exists(vsnprintf HAVE_VSNPRINTF)
check_function_exists(snprintf HAVE_SNPRINTF)

check_function_exists(getpwnam_r HAVE_GETPWNAM_R)
check_function_exists(getpwuid_r HAVE_GETPWUID_R)
check_function_exists(getpwent_r HAVE_GETPWENT_R)

check_function_exists(getgrnam_r HAVE_GETGRNAM_R)
check_function_exists(getgrgid_r HAVE_GETGRGID_R)
check_function_exists(getgrent_r HAVE_GETGRENT_R)

check_function_exists(getgrouplist HAVE_GETGROUPLIST)

check_function_exists(gethostbyaddr_r HAVE_GETHOSTBYADDR_R)
check_function_exists(gethostbyname_r HAVE_GETHOSTBYNAME_R)

if (WIN32)
    check_function_exists(_vsnprintf_s HAVE__VSNPRINTF_S)
    check_function_exists(_vsnprintf HAVE__VSNPRINTF)
    check_function_exists(_snprintf HAVE__SNPRINTF)
    check_function_exists(_snprintf_s HAVE__SNPRINTF_S)
endif (WIN32)

if (SOLARIS)
    check_function_exists(__posix_getpwnam_r HAVE___POSIX_GETPWNAM_R)
    check_function_exists(__posix_getpwuid_r HAVE___POSIX_GETPWUID_R)

    check_function_exists(__posix_getgrgid_r HAVE___POSIX_GETGRGID_R)
    check_function_exists(__posix_getgrnam_r HAVE___POSIX_GETGRNAM_R)
endif (SOLARIS)

check_function_exists(asprintf HAVE_ASPRINTF)
if (UNIX AND HAVE_ASPRINTF)
    add_definitions(-D_GNU_SOURCE)
endif (UNIX AND HAVE_ASPRINTF)

check_prototype_definition(getpwent_r
    "struct passwd *getpwent_r(struct passwd *src, char *buf, int buflen)"
    "NULL"
    "unistd.h;pwd.h"
    HAVE_SOLARIS_GETPWENT_R)

check_prototype_definition(getgrent_r
    "struct group *getgrent_r(struct group *src, char *buf, int buflen)"
    "NULL"
    "unistd.h;grp.h"
    HAVE_SOLARIS_GETGRENT_R)

check_prototype_definition(getgrnam_r
    "int getgrnam_r(const char *name, struct group *grp, char *buf, int buflen, struct group **pgrp)"
    "-1"
    "unistd.h;grp.h"
    HAVE_SOLARIS_GETGRNAM_R)

check_prototype_definition(sethostent
    "int sethostent(void)"
    "-1"
    "unistd.h;netdb.h"
    HAVE_SOLARIS_SETHOSTENT)

check_prototype_definition(endhostent
    "int endhostent(void)"
    "-1"
    "unistd.h;netdb.h"
    HAVE_SOLARIS_ENDHOSTENT)

check_prototype_definition(gethostname
    "int gethostname(char *name, int len)"
    "-1"
    "unistd.h;netdb.h"
    HAVE_SOLARIS_GETHOSTNAME)

check_prototype_definition(setgrent
    "int setgrent(void)"
    "-1"
    "unistd.h;grp.h"
    HAVE_BSD_SETGRENT)

check_prototype_definition(getnameinfo
    "int getnameinfo (const struct sockaddr *sa, socklen_t salen, char *host, socklen_t __hostlen, char *serv, socklen_t servlen, int flags)"
    "-1"
    "unistd.h;netdb.h"
    HAVE_LINUX_GETNAMEINFO)

# STRUCT MEMBERS
check_struct_has_member("struct sockaddr" sa_len "sys/socket.h netinet/in.h" HAVE_STRUCT_SOCKADDR_SA_LEN)

# IPV6
check_c_source_compiles("
    #include <stdlib.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <net/if.h>

int main(void) {
    struct sockaddr_storage sa_store;
    struct addrinfo *ai = NULL;
    struct in6_addr in6addr;
    int idx = if_nametoindex(\"iface1\");
    int s = socket(AF_INET6, SOCK_STREAM, 0);
    int ret = getaddrinfo(NULL, NULL, NULL, &ai);
    if (ret != 0) {
        const char *es = gai_strerror(ret);
    }

    freeaddrinfo(ai);
    {
        int val = 1;
#ifdef HAVE_LINUX_IPV6_V6ONLY_26
#define IPV6_V6ONLY 26
#endif
        ret = setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY,
                         (const void *)&val, sizeof(val));
    }

    return 0;
}" HAVE_IPV6)

check_library_exists(dl dlopen "" HAVE_LIBDL)
if (HAVE_LIBDL)
    find_library(DLFCN_LIBRARY dl)
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${DLFCN_LIBRARY})
endif (HAVE_LIBDL)

# ENDIAN
if (NOT WIN32)
    test_big_endian(WORDS_BIGENDIAN)
endif (NOT WIN32)

set(NSSWRAP_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} CACHE INTERNAL "nsswrap required system libraries")
