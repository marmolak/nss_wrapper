#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <unistd.h>

static void test_nwrap_passwd_group(void **state)
{
	struct passwd *pwd;
	struct group *grp;

	(void) state; /* unused */

	pwd = getpwuid(getuid());
	assert_non_null(pwd);

	pwd = getpwnam(pwd->pw_name);
	assert_non_null(pwd);

	grp = getgrgid(pwd->pw_gid);
	assert_non_null(grp);

	grp = getgrnam(grp->gr_name);
	assert_non_null(grp);
}

/* Test libnsl */
static void test_nwrap_hostent(void **state)
{
	struct hostent *he;

	(void) state; /* unused */

	sethostent(0);

	he = gethostent();
	assert_non_null(he);

	endhostent();
}

static void test_nwrap_gethostname(void **state)
{
	char host[256] = {0};
	int rc;

	(void) state; /* unused */

	rc = gethostname(host, sizeof(host));
	assert_int_equal(rc, 0);

	assert_true(strlen(host) > 1);
}

static void test_nwrap_getaddrinfo_local(void **state)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct sockaddr_in *sinp;
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

	assert_int_equal(ntohl(sinp->sin_addr.s_addr), INADDR_LOOPBACK);

	freeaddrinfo(res);
}

int main(void) {
	int rc;

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_nwrap_passwd_group),
		cmocka_unit_test(test_nwrap_hostent),
		cmocka_unit_test(test_nwrap_gethostname),
		cmocka_unit_test(test_nwrap_getaddrinfo_local),
	};

	rc = cmocka_run_group_tests(tests, NULL, NULL);

	return rc;
}
