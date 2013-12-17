#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <sys/types.h>
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

int main(void) {
	int rc;

	const UnitTest tests[] = {
		unit_test(test_nwrap_passwd_group),
	};

	rc = run_tests(tests);

	return rc;
}
