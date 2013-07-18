#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define NSS_WRAPPER_HOSTNAME_ENV "NSS_WRAPPER_HOSTNAME"

static void test_nwrap_gethostname(void **state)
{
	const char *hostname = "milliways";
	char sys_host[256] = {0};
	char host[16] = {0};
	int rc;

	(void) state; /* unused */

	rc = setenv(NSS_WRAPPER_HOSTNAME_ENV, hostname, 1);
	assert_int_equal(rc, 0);

	rc = gethostname(host, sizeof(host));
	assert_int_equal(rc, 0);

	assert_string_equal(host, hostname);

	rc = setenv(NSS_WRAPPER_HOSTNAME_ENV, "this_hostname_is_too_long", 1);
	assert_int_equal(rc, 0);

	rc = gethostname(host, sizeof(host));
	assert_int_equal(rc, -1);
	assert_int_equal(errno, ENAMETOOLONG);

	unsetenv(NSS_WRAPPER_HOSTNAME_ENV);

	rc = gethostname(sys_host, sizeof(sys_host));
	assert_int_equal(rc, 0);

}

static void test_nwrap_gethostbyname(void **state)
{
	char ip[INET_ADDRSTRLEN];
	struct hostent *he;
	const char *a;

	(void) state; /* unused */

	he = gethostbyname("magrathea.galaxy.site");
	assert_non_null(he);

	assert_string_equal(he->h_name, "magrathea.galaxy.site");
	assert_int_equal(he->h_addrtype, AF_INET);

	a = inet_ntop(AF_INET, he->h_addr_list[0], ip, sizeof(ip));
	assert_non_null(a);

	assert_string_equal(ip, "127.0.0.11");
}

static void test_nwrap_gethostbyaddr(void **state)
{
	struct hostent *he;
	struct in_addr in;
	int rc;

	(void) state; /* unused */

	rc = inet_aton("127.0.0.11", &in);
	assert_int_equal(rc, 1);

	he = gethostbyaddr(&in, sizeof(struct in_addr), AF_INET);
	assert_non_null(he);

	assert_string_equal(he->h_name, "magrathea.galaxy.site");
	assert_int_equal(he->h_addrtype, AF_INET);
	assert_memory_equal(&in, he->h_addr_list[0], he->h_length);
}

#ifdef HAVE_GETHOSTBYNAME_R
static void test_nwrap_gethostbyname_r(void **state)
{
	char buf[1024] = {0};
	char ip[INET_ADDRSTRLEN];
	struct hostent hb, *he;
	const char *a;
	int herr = 0;
	int rc;

	(void) state; /* unused */

	rc = gethostbyname_r("magrathea.galaxy.site",
			     &hb,
			     buf, sizeof(buf),
			     &he,
			     &herr);
	assert_int_equal(rc, 0);
	assert_non_null(he);

	assert_string_equal(he->h_name, "magrathea.galaxy.site");
	assert_int_equal(he->h_addrtype, AF_INET);

	a = inet_ntop(AF_INET, he->h_addr_list[0], ip, sizeof(ip));
	assert_non_null(a);

	assert_string_equal(ip, "127.0.0.11");
}
#endif

#ifdef HAVE_GETHOSTBYADDR_R
static void test_nwrap_gethostbyaddr_r(void **state)
{
	char buf[1024] = {0};
	struct hostent hb, *he;
	struct in_addr in;
	int herr = 0;
	int rc;

	(void) state; /* unused */

	rc = inet_aton("127.0.0.11", &in);
	assert_int_equal(rc, 1);

	rc = gethostbyaddr_r(&in, sizeof(struct in_addr),
			     AF_INET,
			     &hb,
			     buf, sizeof(buf),
			     &he,
			     &herr);
	assert_int_equal(rc, 0);
	assert_non_null(he);

	assert_string_equal(he->h_name, "magrathea.galaxy.site");
	assert_int_equal(he->h_addrtype, AF_INET);
	assert_memory_equal(&in, he->h_addr_list[0], he->h_length);
}
#endif

int main(void) {
	int rc;

	const UnitTest tests[] = {
		unit_test(test_nwrap_gethostname),
		unit_test(test_nwrap_gethostbyname),
		unit_test(test_nwrap_gethostbyaddr),
#ifdef HAVE_GETHOSTBYNAME_R
		unit_test(test_nwrap_gethostbyname_r),
#endif
#ifdef HAVE_GETHOSTBYADDR_R
		unit_test(test_nwrap_gethostbyaddr_r),
#endif
	};

	rc = run_tests(tests);

	return rc;
}

