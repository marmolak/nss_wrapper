#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <shadow.h>
#include <stdint.h>
#include <unistd.h>

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

int main(void) {
	int rc;

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_nwrap_getspent),
	};

	rc = cmocka_run_group_tests(tests, NULL, NULL);

	return rc;
}
