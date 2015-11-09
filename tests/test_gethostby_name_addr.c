#include "config.h"

#include <pthread.h>

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

static void *thread_test_gethostbyname(void *u)
{
	struct hostent *he;

	(void) u; /* unused */

	he = gethostbyname("magrathea");
	assert_non_null(he);
	assert_non_null(he->h_name);
	assert_string_equal(he->h_name, "magrathea.galaxy.site");
	pthread_exit(NULL);
}

static void test_nwrap_gethostbyname_thread(void **state)
{
	struct hostent *he;
	pthread_t th;

	(void) state; /* unused */

	he = gethostbyname("maximegalon.galaxy.site");
	assert_non_null(he);
	assert_non_null(he->h_name);
	assert_string_equal(he->h_name, "maximegalon.galaxy.site");

	pthread_create(&th, NULL, &thread_test_gethostbyname, NULL);
	pthread_join(th, NULL);

	assert_non_null(he);
	assert_non_null(he->h_name);
#ifdef BSD
	/*
	 * On *BSD (and Mac OS X) systems,
	 * data is stored in thread local storage.
	 */
	assert_string_equal(he->h_name, "maximegalon.galaxy.site");
#else
	/*
	 * Glibc doesn't store data in thread local storage, so calling
	 * gethostbyname from a thread overwrites the parent thread's data.
	 */
	assert_string_equal(he->h_name, "magrathea.galaxy.site");
#endif
}

static void test_nwrap_gethostbyname(void **state)
{
	char ip[INET_ADDRSTRLEN];
	struct hostent *he;
	const char *a;

	(void) state; /* unused */

	he = gethostbyname("magrathea.galaxy.site");
	assert_non_null(he);
	assert_non_null(he->h_name);
	assert_non_null(he->h_addr_list);

	assert_string_equal(he->h_name, "magrathea.galaxy.site");
	assert_int_equal(he->h_addrtype, AF_INET);

	a = inet_ntop(AF_INET, he->h_addr_list[0], ip, sizeof(ip));
	assert_non_null(a);

	assert_string_equal(ip, "127.0.0.11");
}

static void test_nwrap_gethostbyname_multiple(void **state)
{
	struct hostent *he;
	char **list;

	/* For inet_ntop call */
	char buf[4096];
	const char *result;
	char *p = buf;

	/* List of ips in hosts file - order matters */
	const char *const result_ips[] = { "127.0.0.11", "127.0.0.12", NULL };
	const char *actual_ip = result_ips[0];
	unsigned int ac;

	(void) state; /* unused */

	he = gethostbyname("magrathea.galaxy.site");
	assert_non_null(he);
	assert_non_null(he->h_name);
	assert_non_null(he->h_addr_list);

	list = he->h_addr_list;
	for (ac = 0; *list != NULL; ++ac, ++list) {
		actual_ip = result_ips[ac];
		/* When test fails here more records are returned */
		assert_non_null(actual_ip);
		result = inet_ntop(AF_INET, *list, p, 4096);
		assert_non_null(p);
		assert_string_equal(actual_ip, result);
	}
}

#ifdef HAVE_GETHOSTBYNAME2
static void test_nwrap_gethostbyname2(void **state)
{
	char ip[INET6_ADDRSTRLEN];
	struct hostent *he;
	const char *a;

	(void) state; /* unused */

	he = gethostbyname2("magrathea.galaxy.site", AF_INET6);
	assert_non_null(he);

	he = gethostbyname2("magrathea.galaxy.site", AF_INET);
	assert_non_null(he);

	/* Check ipv6 he */
	he = gethostbyname2("krikkit.galaxy.site", AF_INET6);
	assert_non_null(he);
	assert_non_null(he->h_name);
	assert_non_null(he->h_addr_list);

	assert_string_equal(he->h_name, "krikkit.galaxy.site");
	assert_int_equal(he->h_addrtype, AF_INET6);

	a = inet_ntop(AF_INET6, he->h_addr_list[0], ip, sizeof(ip));
	assert_non_null(a);

	assert_string_equal(ip, "::14");

	/* Check ipv4 he */
	he = gethostbyname2("krikkit.galaxy.site", AF_INET);
	assert_non_null(he);
	assert_non_null(he->h_name);
	assert_non_null(he->h_addr_list);

	assert_string_equal(he->h_name, "krikkit.galaxy.site");
	assert_int_equal(he->h_addrtype, AF_INET);

	a = inet_ntop(AF_INET, he->h_addr_list[0], ip, sizeof(ip));
	assert_non_null(a);

	assert_string_equal(ip, "127.0.0.14");
}
#endif /* HAVE_GETHOSTBYNAME2 */

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
	assert_non_null(he->h_name);
	assert_non_null(he->h_addr_list);

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
	assert_non_null(he->h_name);
	assert_non_null(he->h_addr_list);

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
	assert_non_null(he->h_name);
	assert_non_null(he->h_addr_list);

	assert_string_equal(he->h_name, "magrathea.galaxy.site");
	assert_int_equal(he->h_addrtype, AF_INET);
	assert_memory_equal(&in, he->h_addr_list[0], he->h_length);
}
#endif

int main(void) {
	int rc;

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_nwrap_gethostname),
		cmocka_unit_test(test_nwrap_gethostbyname),
		cmocka_unit_test(test_nwrap_gethostbyname_thread),
#ifdef HAVE_GETHOSTBYNAME2
		cmocka_unit_test(test_nwrap_gethostbyname2),
#endif
		cmocka_unit_test(test_nwrap_gethostbyaddr),
#ifdef HAVE_GETHOSTBYNAME_R
		cmocka_unit_test(test_nwrap_gethostbyname_r),
#endif
#ifdef HAVE_GETHOSTBYADDR_R
		cmocka_unit_test(test_nwrap_gethostbyaddr_r),
#endif
		cmocka_unit_test(test_nwrap_gethostbyname_multiple),
	};

	rc = cmocka_run_group_tests(tests, NULL, NULL);

	return rc;
}

