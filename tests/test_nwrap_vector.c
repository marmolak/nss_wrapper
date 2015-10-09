#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../src/nss_wrapper.c"

static void test_nwrap_vector_basic_add(void **state)
{
	struct nwrap_vector v;
	char string[] = "string!";
	char string2[] = "2string!";

	(void) state; /* unused */

	nwrap_vector_init(&v);
	assert_non_null(v.items);

	nwrap_vector_add_item(&v, string);
	assert_null(v.items[1]);
	assert_int_equal(v.count, 1);
	assert_string_equal(v.items[0], string);
	assert_int_equal(v.capacity, DEFAULT_VECTOR_CAPACITY);

	assert_string_equal(v.items[0], "string!");

	nwrap_vector_add_item(&v, string2);
	assert_null(v.items[2]);
	assert_int_equal(v.count, 2);
	assert_string_equal(v.items[0], string);
	assert_string_equal(v.items[1], string2);

	free(v.items);
}

static void test_nwrap_vector_merge_empty(void **state)
{
	struct nwrap_vector v1;
	struct nwrap_vector v2;
	char string[] = "string!";
	char string2[] = "2string!";

	(void) state; /* unused */

	nwrap_vector_init(&v1);
	assert_non_null(v1.items);
	assert_int_equal(v1.count, 0);

	nwrap_vector_init(&v2);
	assert_non_null(v2.items);
	assert_int_equal(v2.count, 0);

	nwrap_vector_merge(&v1, &v2);
	assert_int_equal(v1.count, 0);
	assert_null(v1.items[0]);

	nwrap_vector_add_item(&v1, string);
	nwrap_vector_add_item(&v1, string2);
	assert_int_equal(v1.count, 2);

	nwrap_vector_merge(&v1, &v2);
	assert_int_equal(v1.count, 2);
	assert_string_equal(v1.items[0], string);
	assert_string_equal(v1.items[1], string2);
	assert_null(v1.items[2]);

	nwrap_vector_merge(&v2, &v1);
	assert_int_equal(v2.count, 2);
	assert_string_equal(v2.items[0], string);
	assert_string_equal(v2.items[1], string2);
	assert_null(v2.items[2]);

	free(v1.items);
	free(v2.items);
}

static void test_nwrap_vector_merge(void **state)
{
	struct nwrap_vector v1;
	struct nwrap_vector v2;
	char string[] = "string!";
	char string2[] = "2string!";

	(void) state; /* unused */

	nwrap_vector_init(&v1);
	assert_non_null(v1.items);
	nwrap_vector_add_item(&v1, string);
	nwrap_vector_add_item(&v1, string2);
	assert_int_equal(v1.count, 2);

	nwrap_vector_init(&v2);
	assert_non_null(v2.items);
	nwrap_vector_add_item(&v2, string2);
	nwrap_vector_add_item(&v2, string);
	assert_int_equal(v2.count, 2);

	nwrap_vector_merge(&v1, &v2);
	assert_int_equal(v1.count, 4);
	assert_string_equal(v1.items[0], string);
	assert_string_equal(v1.items[1], string2);
	assert_string_equal(v1.items[2], string2);
	assert_string_equal(v1.items[3], string);
	assert_null(v1.items[4]);

	free(v1.items);
	free(v2.items);
}

static void test_nwrap_vector_merge_max(void **state)
{
	struct nwrap_vector v1;
	struct nwrap_vector v2;
	char string[] = "string!";
	char string2[] = "2string!";
	int p;

	(void) state; /* unused */

	nwrap_vector_init(&v1);
	assert_non_null(v1.items);

	for (p = 0; p < 64; ++p) {
		nwrap_vector_add_item(&v1, string);
	}
	assert_int_equal(v1.count, 64);

	nwrap_vector_init(&v2);
	assert_non_null(v2.items);

	nwrap_vector_merge(&v2, &v1);
	assert_int_equal(v2.count, 64);
	for (p = 0; p < 64; ++p) {
		assert_string_equal(v2.items[p], string);
	}

	nwrap_vector_add_item(&v2, string2);
	assert_string_equal(v2.items[64], string2);
	assert_int_equal(v2.count, 65);
	assert_null(v2.items[65]);

	free(v1.items);
	free(v2.items);
}

int main(void) {
	int rc;

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_nwrap_vector_basic_add),
		cmocka_unit_test(test_nwrap_vector_merge_empty),
		cmocka_unit_test(test_nwrap_vector_merge),
		cmocka_unit_test(test_nwrap_vector_merge_max),
	};

	rc = cmocka_run_group_tests(tests, NULL, NULL);

	return rc;
}

