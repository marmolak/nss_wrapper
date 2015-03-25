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

static void test_nwrap_gethostent(void **state)
{
	struct hostent *he;
	uint32_t i;

	(void)state; /* unused */

	sethostent(0);

	for (he = gethostent(); he != NULL; he = gethostent()) {
		assert_non_null(he->h_addr_list);
		assert_non_null(he->h_aliases);

		for (i = 0; he->h_addr_list[i] != NULL; i++) {
			char buf[INET6_ADDRSTRLEN];
			uint32_t j;
			const char *ip;
			
			ip = inet_ntop(he->h_addrtype,
				       he->h_addr_list[i],
				       buf,
				       sizeof(buf));

			printf("ip: %s\n", ip);

			for (j = 0; he->h_aliases[j] != NULL; j++) {
				printf("alias: %s\n", he->h_aliases[j]);
			}
		}
	}

	endhostent();
}

int main(void) {
	int rc;

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_nwrap_gethostent),
	};

	rc = cmocka_run_group_tests(tests, NULL, NULL);

	return rc;
}
