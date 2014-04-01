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

static void test_nwrap_getaddrinfo(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct sockaddr_in *sinp;
	struct sockaddr_in6 *sin6p;
	char ip6[INET6_ADDRSTRLEN];
	char *ip;
	int rc;

	(void) state; /* unused */

	/* IPv4 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	rc = getaddrinfo("127.0.0.11", NULL, &hints, &res);
	assert_int_equal(rc, 0);

	assert_non_null(res->ai_canonname);
	assert_string_equal(res->ai_canonname, "magrathea.galaxy.site");

	assert_int_equal(res->ai_family, AF_INET);

	sinp = (struct sockaddr_in *)res->ai_addr;

	assert_int_equal(sinp->sin_family, AF_INET);
	ip = inet_ntoa(sinp->sin_addr);

	assert_string_equal(ip, "127.0.0.11");

	freeaddrinfo(res);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	rc = getaddrinfo("::13", NULL, &hints, &res);
	assert_int_equal(rc, 0);

	assert_non_null(res->ai_canonname);
	assert_string_equal(res->ai_canonname, "beteigeuze.galaxy.site");

	assert_int_equal(res->ai_family, AF_INET6);

	sin6p = (struct sockaddr_in6 *)res->ai_addr;

	assert_int_equal(sin6p->sin6_family, AF_INET6);
	inet_ntop(AF_INET6, (void *)&sin6p->sin6_addr, ip6, sizeof(ip6));

	assert_string_equal(ip6, "::13");

	freeaddrinfo(res);
}

static void test_nwrap_getaddrinfo_any(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct sockaddr_in *sinp;
	struct sockaddr_in6 *sin6p;
	char ip6[INET6_ADDRSTRLEN];
	char *ip;
	int rc;

	(void) state; /* unused */

	/* IPv4 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

	rc = getaddrinfo("0.0.0.0", "389", &hints, &res);
	assert_int_equal(rc, 0);

	assert_int_equal(res->ai_family, AF_INET);
	assert_int_equal(res->ai_socktype, SOCK_STREAM);

	assert_null(res->ai_canonname);

	sinp = (struct sockaddr_in *)res->ai_addr;

	assert_int_equal(389, htons(sinp->sin_port));

	ip = inet_ntoa(sinp->sin_addr);

	assert_string_equal(ip, "0.0.0.0");

	freeaddrinfo(res);

	/* IPv4 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

	rc = getaddrinfo("::", "389", &hints, &res);
	assert_int_equal(rc, 0);

	assert_int_equal(res->ai_family, AF_INET6);
	assert_int_equal(res->ai_socktype, SOCK_STREAM);

	assert_null(res->ai_canonname);

	sin6p = (struct sockaddr_in6 *)res->ai_addr;

	assert_int_equal(389, htons(sin6p->sin6_port));

	inet_ntop(AF_INET6, (void *)&sin6p->sin6_addr, ip6, sizeof(ip6));

	assert_string_equal(ip6, "::");

	freeaddrinfo(res);
}

static void test_nwrap_getaddrinfo_local(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct sockaddr_in *sinp;
	char *ip;
	int rc;

	(void) state; /* unused */

	/* IPv4 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;

	rc = getaddrinfo("127.0.0.1", NULL, &hints, &res);
	assert_int_equal(rc, 0);

	assert_int_equal(res->ai_family, AF_INET);
	assert_int_equal(res->ai_socktype, SOCK_STREAM);

	assert_null(res->ai_canonname);

	sinp = (struct sockaddr_in *)res->ai_addr;
	ip = inet_ntoa(sinp->sin_addr);

	assert_string_equal(ip, "127.0.0.1");

	freeaddrinfo(res);
}

static void test_nwrap_getaddrinfo_name(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	struct sockaddr_in *sinp;
	char *ip;
	int rc;

	(void) state; /* unused */

	/* IPv4 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;

	rc = getaddrinfo("maximegalon.galaxy.site", NULL, &hints, &res);
	assert_int_equal(rc, 0);

	assert_int_equal(res->ai_family, AF_INET);
	assert_int_equal(res->ai_socktype, SOCK_STREAM);

	assert_non_null(res->ai_canonname);
	assert_string_equal(res->ai_canonname, "maximegalon.galaxy.site");

	sinp = (struct sockaddr_in *)res->ai_addr;
	ip = inet_ntoa(sinp->sin_addr);

	assert_string_equal(ip, "127.0.0.12");

	freeaddrinfo(res);

	/* IPv4 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;

	rc = getaddrinfo("MAGRATHEA", NULL, &hints, &res);
	assert_int_equal(rc, 0);

	assert_int_equal(res->ai_family, AF_INET);
	assert_int_equal(res->ai_socktype, SOCK_STREAM);

	assert_non_null(res->ai_canonname);
	assert_string_equal(res->ai_canonname, "magrathea.galaxy.site");

	sinp = (struct sockaddr_in *)res->ai_addr;
	ip = inet_ntoa(sinp->sin_addr);

	assert_string_equal(ip, "127.0.0.11");

	freeaddrinfo(res);
}

static void test_nwrap_getaddrinfo_service(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	struct sockaddr_in *sinp;
	char *ip;
	int rc;

	(void) state; /* unused */

	/* IPv4 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;

	rc = getaddrinfo("magrathea", "wurst", &hints, &res);
	assert_int_equal(rc, EAI_SERVICE);

	/* Check ldap port */
	rc = getaddrinfo("magrathea", "ldap", &hints, &res);
	assert_int_equal(rc, 0);

	assert_int_equal(res->ai_family, AF_INET);
	assert_int_equal(res->ai_socktype, SOCK_STREAM);

	assert_non_null(res->ai_canonname);
	assert_string_equal(res->ai_canonname, "magrathea.galaxy.site");

	sinp = (struct sockaddr_in *)res->ai_addr;
	ip = inet_ntoa(sinp->sin_addr);

	assert_string_equal(ip, "127.0.0.11");

	assert_int_equal(ntohs(sinp->sin_port), 389);

	freeaddrinfo(res);
}

static void test_nwrap_getaddrinfo_null(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	struct sockaddr_in6 *sin6p;
	char ip6[INET6_ADDRSTRLEN];
	int rc;

	(void) state; /* unused */

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 17;
	hints.ai_flags = AI_ADDRCONFIG;

	rc = getaddrinfo(NULL, NULL, &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	/* Check dns service */
	rc = getaddrinfo(NULL, "domain", &hints, &res);
	assert_int_equal(rc, 0);

	assert_null(res->ai_canonname);

	assert_int_equal(res->ai_family, AF_INET6);
	assert_int_equal(res->ai_socktype, SOCK_DGRAM);

	sin6p = (struct sockaddr_in6 *)res->ai_addr;
	inet_ntop(AF_INET6, (void *)&sin6p->sin6_addr, ip6, sizeof(ip6));

	assert_string_equal(ip6, "::1");

	freeaddrinfo(res);

	/* Check dns service */
	rc = getaddrinfo("magrathea", "domain", NULL, &res);
	assert_int_equal(rc, 0);

	assert_non_null(res->ai_canonname);

	freeaddrinfo(res);
}

static void test_nwrap_getaddrinfo_ipv6(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	struct sockaddr_in6 *sin6p;
	char ip6[INET6_ADDRSTRLEN];
	int rc;

	(void) state; /* unused */

	/*
	 * krikkit.galaxy has an IPv4 and IPv6 address, this should only
	 * return the IPv6 address.
	 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;

	rc = getaddrinfo("krikkit.galaxy.site", NULL, &hints, &res);
	assert_int_equal(rc, 0);

	assert_non_null(res->ai_next);
	assert_int_equal(res->ai_family, AF_INET6);

	sin6p = (struct sockaddr_in6 *)res->ai_addr;
	assert_int_equal(sin6p->sin6_family, AF_INET6);
	inet_ntop(AF_INET6, (void *)&sin6p->sin6_addr, ip6, sizeof(ip6));

	assert_string_equal(ip6, "::14");

	freeaddrinfo(res);
}

int main(void) {
	int rc;

	const UnitTest tests[] = {
		unit_test(test_nwrap_getaddrinfo),
		unit_test(test_nwrap_getaddrinfo_any),
		unit_test(test_nwrap_getaddrinfo_local),
		unit_test(test_nwrap_getaddrinfo_name),
		unit_test(test_nwrap_getaddrinfo_service),
		unit_test(test_nwrap_getaddrinfo_null),
		unit_test(test_nwrap_getaddrinfo_ipv6),
	};

	rc = run_tests(tests);

	return rc;
}
