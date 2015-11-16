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
	struct addrinfo *res = NULL;
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
	assert_non_null(res);

	assert_non_null(res->ai_canonname);
	assert_string_equal(res->ai_canonname, "magrathea.galaxy.site");

	assert_int_equal(res->ai_family, AF_INET);

	sinp = (struct sockaddr_in *)res->ai_addr;

	assert_int_equal(sinp->sin_family, AF_INET);
	ip = inet_ntoa(sinp->sin_addr);

	assert_string_equal(ip, "127.0.0.11");

	freeaddrinfo(res);
	res = NULL;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	rc = getaddrinfo("::13", NULL, &hints, &res);
	assert_non_null(res);
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

static void test_nwrap_getaddrinfo_samba(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	int rc;

	(void) state; /* unused */

	/* IPv4 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Stream socket */
	hints.ai_flags = 0;    /* For wildcard IP address */
	hints.ai_protocol = IPPROTO_TCP;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	rc = getaddrinfo("127.0.0.21", NULL, &hints, &res);
	assert_int_equal(rc, 0);
	assert_non_null(res);

	rc = getaddrinfo("samba.example.com", NULL, &hints, &res);
	assert_int_equal(rc, 0);
	assert_non_null(res);

	rc = getaddrinfo("localdc", NULL, &hints, &res);
	assert_int_equal(rc, 0);
	assert_non_null(res);

	rc = getaddrinfo("localdc.samba.example.com", NULL, &hints, &res);
	assert_int_equal(rc, 0);
	assert_non_null(res);

	rc = getaddrinfo("fd00:0000:0000:0000:0000:0000:5357:5f15", NULL, &hints, &res);
	assert_int_equal(rc, 0);
	assert_non_null(res);

	freeaddrinfo(res);
}

static void test_nwrap_getaddrinfo_any(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res = NULL;
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
	assert_non_null(res);

	assert_int_equal(res->ai_family, AF_INET);
	assert_int_equal(res->ai_socktype, SOCK_STREAM);

	assert_null(res->ai_canonname);

	sinp = (struct sockaddr_in *)res->ai_addr;

	assert_int_equal(389, htons(sinp->sin_port));

	ip = inet_ntoa(sinp->sin_addr);

	assert_string_equal(ip, "0.0.0.0");

	freeaddrinfo(res);
	res = NULL;

	/* IPv4 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

	rc = getaddrinfo("::", "389", &hints, &res);
	assert_int_equal(rc, 0);
	assert_non_null(res);

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
	hints.ai_flags = 0;

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
	hints.ai_flags = 0;

	rc = getaddrinfo("maximegalon.galaxy.site", NULL, &hints, &res);
	assert_int_equal(rc, 0);
	assert_non_null(res);

	assert_non_null(res);
	assert_int_equal(res->ai_family, AF_INET);
	assert_int_equal(res->ai_socktype, SOCK_STREAM);

	assert_non_null(res->ai_canonname);
	assert_string_equal(res->ai_canonname, "maximegalon.galaxy.site");

	sinp = (struct sockaddr_in *)res->ai_addr;
	ip = inet_ntoa(sinp->sin_addr);

	assert_string_equal(ip, "127.0.0.12");

	freeaddrinfo(res);
	res = NULL;

	/* IPv4 */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;

	rc = getaddrinfo("MAGRATHEA", NULL, &hints, &res);
	assert_int_equal(rc, 0);

	assert_non_null(res);
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
	hints.ai_flags = 0;

	rc = getaddrinfo("magrathea", "wurst", &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	/* Check ldap port */
	rc = getaddrinfo("magrathea", "ldap", &hints, &res);
	assert_int_equal(rc, 0);
	assert_non_null(res);

	assert_int_equal(res->ai_family, AF_INET);
	assert_int_equal(res->ai_socktype, SOCK_STREAM);

	assert_non_null(res->ai_canonname);
	assert_string_equal(res->ai_canonname, "magrathea.galaxy.site");

	assert_non_null(res->ai_addr);
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
	hints.ai_flags = 0;

	rc = getaddrinfo(NULL, NULL, &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	/* Check dns service */
	rc = getaddrinfo(NULL, "domain", &hints, &res);
	assert_int_equal(rc, 0);

	assert_non_null(res);
	assert_null(res->ai_canonname);

	assert_int_equal(res->ai_family, AF_INET6);
	assert_int_equal(res->ai_socktype, SOCK_DGRAM);

	assert_non_null(res->ai_addr);
	sin6p = (struct sockaddr_in6 *)res->ai_addr;
	inet_ntop(AF_INET6, (void *)&sin6p->sin6_addr, ip6, sizeof(ip6));

	assert_string_equal(ip6, "::1");

	freeaddrinfo(res);
	res = NULL;

	/* Check dns service */
	rc = getaddrinfo("magrathea", "domain", NULL, &res);
	assert_non_null(res);
	assert_int_equal(rc, 0);

	assert_non_null(res->ai_canonname);

	freeaddrinfo(res);
}

static void test_nwrap_getaddrinfo_dot(void **state)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
	};
	struct addrinfo *res = NULL;
	struct sockaddr_in *sinp;
	char ip[INET_ADDRSTRLEN];
	int rc;

	(void) state; /* unused */

	/* Check with a dot at the end */
	rc = getaddrinfo("magrathea.galaxy.site.", NULL, &hints, &res);
	assert_int_equal(rc, 0);
	assert_non_null(res);

	assert_non_null(res->ai_next);
	assert_int_equal(res->ai_family, AF_INET);

	sinp = (struct sockaddr_in *)res->ai_addr;
	assert_int_equal(sinp->sin_family, AF_INET);
	inet_ntop(AF_INET, (void *)&sinp->sin_addr, ip, sizeof(ip));

	assert_string_equal(ip, "127.0.0.11");

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
	assert_non_null(res);

	assert_non_null(res->ai_next);
	assert_int_equal(res->ai_family, AF_INET6);

	sin6p = (struct sockaddr_in6 *)res->ai_addr;
	assert_int_equal(sin6p->sin6_family, AF_INET6);
	inet_ntop(AF_INET6, (void *)&sin6p->sin6_addr, ip6, sizeof(ip6));

	assert_string_equal(ip6, "::14");

	freeaddrinfo(res);
}

static void test_nwrap_getaddrinfo_multiple_mixed(void **state)
{
	struct addrinfo *res, *res_head;
	struct addrinfo hints;
	unsigned int ipv6_count = 0;
	unsigned int ipv4_count = 0;
	int rc;
	int p;

	struct sockaddr_in *r_addr;
	struct sockaddr_in6 *r_addr6;

	const char *result;
	const char *value;

	/* For inet_ntop call */
	char buf[4096];

	/* 2 - ipv4 and 3 ipv6 addresses */
	const char *ipvX_results[] = {"127.1.1.1", "127.0.0.66", "2666::22", "B00B:5::4", "DEAD:BEEF:1:2:3::4", NULL};

	(void) state; /* unused */

	 memset(&hints, '\0', sizeof(struct addrinfo));
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

	rc = getaddrinfo("pumpkin.bunny.net", NULL, &hints, &res_head);
	assert_return_code(rc, 0);
	assert_non_null(res_head);

	for (res = res_head; res != NULL; res = res->ai_next) {
		if (res->ai_family == AF_INET) {
			r_addr = (struct sockaddr_in *) res->ai_addr;
			assert_non_null(r_addr);
			++ipv4_count;
			result = inet_ntop(AF_INET,
					   &r_addr->sin_addr,
					   buf,
					   4096);
		} else if (res->ai_family == AF_INET6) {
			r_addr6 = (struct sockaddr_in6 *) res->ai_addr;
			assert_non_null(r_addr6);
			++ipv6_count;
			result = inet_ntop(AF_INET6,
					   &r_addr6->sin6_addr,
					   buf,
					   4096);
		} else {
			/* Unknown family type */
			assert_int_equal(1,0);
		}

		/* Important part */
		assert_non_null(result);

		/* This could be part of cmocka library */
		for (value = ipvX_results[0], p = 0; value != NULL; value = ipvX_results[++p]) {
			if (strcasecmp(value, result) == 0) {
				break;
			}
		}
		assert_non_null(value);
	}

	assert_int_equal(ipv6_count, 3);
	assert_int_equal(ipv4_count, 2);

	freeaddrinfo(res_head);
}

static void test_nwrap_getaddrinfo_flags_ai_numericserv(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res;
	int rc;

	(void) state; /* unused */

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;

	/*
	 * Calls with NULL name are handled by libc,
	 * even if nss_wrapper is enabled
	 */

	rc = getaddrinfo(NULL, "echo", &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	rc = getaddrinfo(NULL, "80", &hints, &res);
	assert_int_equal(rc, 0);

	/* Crippled input */
	rc = getaddrinfo(NULL, "80a1", &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	/*
	 * Calls with non-NULL name are handled by nwrap
	 */

	rc = getaddrinfo("magrathea.galaxy.site", "echo", &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	rc = getaddrinfo("magrathea.galaxy.site", "80", &hints, &res);
	assert_int_equal(rc, 0);

	/* Crippled input */
	rc = getaddrinfo("magrathea.galaxy.site", "80a1", &hints, &res);
	assert_int_equal(rc, EAI_NONAME);
}

static void test_nwrap_getaddrinfo_flags_ai_numerichost(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res;
	int rc;

	(void) state; /* unused */

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;  /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;

	/* IPv4 or IPv6 */

	hints.ai_family = AF_UNSPEC;

	rc = getaddrinfo("127.0.0.11", NULL, &hints, &res);
	assert_int_equal(rc, 0);
	freeaddrinfo(res);

	rc = getaddrinfo("::1", NULL, &hints, &res);
	assert_int_equal(rc, 0);
	freeaddrinfo(res);

	rc = getaddrinfo(NULL, "echo", &hints, &res);
	assert_int_equal(rc, 0);
	freeaddrinfo(res);

	rc = getaddrinfo("magrathea.galaxy.site", NULL, &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	rc = getaddrinfo("", NULL, &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	rc = getaddrinfo("fail.me", "echo", &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	/* IPv4 */

	hints.ai_family = AF_INET;

	rc = getaddrinfo("127.0.0.11", NULL, &hints, &res);
	assert_int_equal(rc, 0);
	freeaddrinfo(res);

	rc = getaddrinfo("::1", NULL, &hints, &res);
#ifdef EAI_ADDRFAMILY
	assert_int_equal(rc, EAI_ADDRFAMILY);
#else
	assert_int_equal(rc, EAI_FAMILY);
#endif

	rc = getaddrinfo(NULL, "echo", &hints, &res);
	assert_int_equal(rc, 0);
	freeaddrinfo(res);

	rc = getaddrinfo("magrathea.galaxy.site", NULL, &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	rc = getaddrinfo("", NULL, &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	rc = getaddrinfo("fail.me", "echo", &hints, &res);
	assert_int_equal(rc, EAI_NONAME);


	/* IPv6 */

	hints.ai_family = AF_INET6;

	rc = getaddrinfo("127.0.0.11", NULL, &hints, &res);
#ifdef EAI_ADDRFAMILY
	assert_int_equal(rc, EAI_ADDRFAMILY);
#else
	assert_int_equal(rc, EAI_FAMILY);
#endif

	rc = getaddrinfo("::1", NULL, &hints, &res);
	assert_int_equal(rc, 0);
	freeaddrinfo(res);

	rc = getaddrinfo(NULL, "echo", &hints, &res);
	assert_int_equal(rc, 0);
	freeaddrinfo(res);

	rc = getaddrinfo("magrathea.galaxy.site", NULL, &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	rc = getaddrinfo("", NULL, &hints, &res);
	assert_int_equal(rc, EAI_NONAME);

	rc = getaddrinfo("fail.me", "echo", &hints, &res);
	assert_int_equal(rc, EAI_NONAME);
}

int main(void) {
	int rc;

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_nwrap_getaddrinfo),
		cmocka_unit_test(test_nwrap_getaddrinfo_any),
		cmocka_unit_test(test_nwrap_getaddrinfo_local),
		cmocka_unit_test(test_nwrap_getaddrinfo_name),
		cmocka_unit_test(test_nwrap_getaddrinfo_service),
		cmocka_unit_test(test_nwrap_getaddrinfo_null),
		cmocka_unit_test(test_nwrap_getaddrinfo_dot),
		cmocka_unit_test(test_nwrap_getaddrinfo_ipv6),
		cmocka_unit_test(test_nwrap_getaddrinfo_multiple_mixed),
		cmocka_unit_test(test_nwrap_getaddrinfo_flags_ai_numericserv),
		cmocka_unit_test(test_nwrap_getaddrinfo_flags_ai_numerichost),
		cmocka_unit_test(test_nwrap_getaddrinfo_samba),
	};

	rc = cmocka_run_group_tests(tests, NULL, NULL);

	return rc;
}
