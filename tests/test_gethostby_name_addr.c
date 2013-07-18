#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

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

int main(void) {
	int rc;

	const UnitTest tests[] = {
		unit_test(test_nwrap_gethostbyname),
		unit_test(test_nwrap_gethostbyaddr),
	};

	rc = run_tests(tests);

	return rc;
}

