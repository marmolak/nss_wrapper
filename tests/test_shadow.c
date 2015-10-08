#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <shadow.h>
#include <stdint.h>
#include <unistd.h>

#include <crypt.h>

static void test_nwrap_getspent(void **state)
{
	struct spwd *sp;
	uint32_t i;

	(void)state; /* unused */

	setspent();

	for (sp = getspent(), i = 0; sp != NULL; sp = getspent(), i++) {
		if (i == 0) {
			assert_string_equal(sp->sp_namp, "alice");
		} else {
			assert_string_equal(sp->sp_namp, "bob");
		}
	}

	endspent();
}

static void test_nwrap_getspnam(void **state)
{
	char *encrypted_password;
	struct spwd *sp;

	(void)state; /* unused */

	sp = getspnam("alice");
	assert_non_null(sp);

	assert_string_equal(sp->sp_namp, "alice");

	encrypted_password = crypt("secret", sp->sp_pwdp);
	assert_non_null(encrypted_password);

	assert_string_equal(encrypted_password, sp->sp_pwdp);

	sp = getspnam("bob");
	assert_non_null(sp);

	assert_string_equal(sp->sp_namp, "bob");

	encrypted_password = crypt("secret", sp->sp_pwdp);
	assert_non_null(encrypted_password);

	assert_string_equal(encrypted_password, sp->sp_pwdp);
}

int main(void) {
	int rc;

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_nwrap_getspent),
		cmocka_unit_test(test_nwrap_getspnam),
	};

	rc = cmocka_run_group_tests(tests, NULL, NULL);

	return rc;
}
