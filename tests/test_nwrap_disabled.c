#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <sys/types.h>
#include <netdb.h>
#include <pwd.h>
#include <grp.h>
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

int main(void) {
	int rc;

	const UnitTest tests[] = {
		unit_test(test_nwrap_passwd_group),
		unit_test(test_nwrap_hostent),
	};

	rc = run_tests(tests);

	return rc;
}
