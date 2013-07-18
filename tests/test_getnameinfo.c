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

static void test_nwrap_getnameinfo(void **state)
{
	char host[256] = {0};
	char serv[256] = {0};
	struct sockaddr_in sin;
	struct sockaddr_in6 sin6;
	int flags = 0;
	int rc;

	(void) state; /* unused */

	/* IPv4 */
	sin.sin_family = AF_INET;
	sin.sin_port = htons(53);
	rc = inet_pton(AF_INET, "127.0.0.11", &sin.sin_addr);
	assert_int_equal(rc, 1);

	rc = getnameinfo((const struct sockaddr *)&sin,
			 sizeof(struct sockaddr_in),
			 host, sizeof(host),
			 serv, sizeof(serv),
			 flags);
	assert_int_equal(rc, 0);

	assert_string_equal(host, "magrathea.galaxy.site");
	assert_string_equal(serv, "domain");

	/* IPv6 */
	sin6.sin6_family = AF_INET6;
	sin6.sin6_port = htons(53);
	rc = inet_pton(AF_INET6, "::13", &sin6.sin6_addr);
	assert_int_equal(rc, 1);

	rc = getnameinfo((const struct sockaddr *)&sin6,
			 sizeof(struct sockaddr_in6),
			 host, sizeof(host),
			 serv, sizeof(serv),
			 flags);
	assert_int_equal(rc, 0);

	assert_string_equal(host, "beteigeuze.galaxy.site");
	assert_string_equal(serv, "domain");
}

static void test_nwrap_getnameinfo_numeric(void **state)
{
	char host[256] = {0};
	char serv[256] = {0};
	struct sockaddr_in sin;
	struct sockaddr_in6 sin6;
	int flags = 0;
	int rc;

	(void) state; /* unused */

	/* IPv4 */
	sin.sin_family = AF_INET;
	sin.sin_port = htons(53);
	rc = inet_pton(AF_INET, "127.0.0.11", &sin.sin_addr);
	assert_int_equal(rc, 1);

	flags = NI_NUMERICHOST;

	rc = getnameinfo((const struct sockaddr *)&sin,
			 sizeof(struct sockaddr_in),
			 host, sizeof(host),
			 serv, sizeof(serv),
			 flags);
	assert_int_equal(rc, 0);

	assert_string_equal(host, "127.0.0.11");
	assert_string_equal(serv, "domain");

	/* IPv6 */
	sin6.sin6_family = AF_INET6;
	sin6.sin6_port = htons(53);
	rc = inet_pton(AF_INET6, "::13", &sin6.sin6_addr);
	assert_int_equal(rc, 1);

	flags = NI_NUMERICSERV;

	rc = getnameinfo((const struct sockaddr *)&sin6,
			 sizeof(struct sockaddr_in6),
			 host, sizeof(host),
			 serv, sizeof(serv),
			 flags);
	assert_int_equal(rc, 0);

	assert_string_equal(host, "beteigeuze.galaxy.site");
	assert_string_equal(serv, "53");
}

int main(void) {
	int rc;

	const UnitTest tests[] = {
		unit_test(test_nwrap_getnameinfo),
		unit_test(test_nwrap_getnameinfo_numeric),
	};

	rc = run_tests(tests);

	return rc;
}
