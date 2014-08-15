#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

/*
 * Defining _POSIX_PTHREAD_SEMANTICS before including pwd.h and grp.h  gives us
 * the posix getpwnam_r(), getpwuid_r(), getgrnam_r and getgrgid_r calls on
 * Solaris
 */
#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS
#endif

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef NDEBUG
#define DEBUG(...)
#else
#define DEBUG(...) printf(__VA_ARGS__)
#endif

static void assert_passwd_equal(const struct passwd *p1,
				const struct passwd *p2)
{
	assert_string_equal(p1->pw_name, p2->pw_name);
	assert_string_equal(p1->pw_passwd, p2->pw_passwd);
	assert_int_equal(p1->pw_uid, p2->pw_uid);
	assert_int_equal(p1->pw_gid, p2->pw_gid);
	assert_string_equal(p1->pw_gecos, p2->pw_gecos);
	assert_string_equal(p1->pw_dir, p2->pw_dir);
	assert_string_equal(p1->pw_shell, p2->pw_shell);
}

static void assert_group_equal(const struct group *g1,
			       const struct group *g2)
{
	int i;

	assert_string_equal(g1->gr_name, g2->gr_name);
	assert_string_equal(g1->gr_passwd, g2->gr_passwd);
	assert_int_equal(g1->gr_gid, g2->gr_gid);
	assert_false(g1->gr_mem != NULL && g2->gr_mem == NULL);
	assert_false(g1->gr_mem == NULL && g2->gr_mem != NULL);
	if (g1->gr_mem == NULL && g2->gr_mem == NULL) {
		return;
	}
	for (i=0; g1->gr_mem[i] && g2->gr_mem[i]; i++) {
		assert_string_equal(g1->gr_mem[i], g2->gr_mem[i]);
	}
}

static bool copy_passwd(const struct passwd *pwd, struct passwd *p)
{
	p->pw_name	= strdup(pwd->pw_name);
	p->pw_passwd	= strdup(pwd->pw_passwd);
	p->pw_uid	= pwd->pw_uid;
	p->pw_gid	= pwd->pw_gid;
	p->pw_gecos	= strdup(pwd->pw_gecos);
	p->pw_dir	= strdup(pwd->pw_dir);
	p->pw_shell	= strdup(pwd->pw_shell);

	return true;
}

static void free_passwd(const struct passwd *p)
{
	if (p->pw_name != NULL) {
		free(p->pw_name);
	}
	if (p->pw_passwd != NULL) {
		free(p->pw_passwd);
	}
	if (p->pw_gecos != NULL) {
		free(p->pw_gecos);
	}
	if (p->pw_dir != NULL) {
		free(p->pw_dir);
	}
	if (p->pw_shell != NULL) {
		free(p->pw_shell);
	}
}

static void free_passwds(struct passwd *pwds, size_t num_pwds)
{
	size_t i;

	for(i = 0; i < num_pwds; i++) {
		free_passwd(&pwds[i]);
	}
	free(pwds);
}

static void print_passwd(struct passwd *pwd)
{
	(void)pwd;

	DEBUG("%s:%s:%lu:%lu:%s:%s:%s\n",
	       pwd->pw_name,
	       pwd->pw_passwd,
	       (unsigned long)pwd->pw_uid,
	       (unsigned long)pwd->pw_gid,
	       pwd->pw_gecos,
	       pwd->pw_dir,
	       pwd->pw_shell);
}

static bool test_nwrap_getpwnam(const char *name, struct passwd *pwd_p)
{
	struct passwd *pwd = NULL;

	DEBUG("Testing getpwnam: %s\n", name);

	pwd = getpwnam(name);
	if (pwd != NULL) {
		print_passwd(pwd);

		if (pwd_p != NULL) {
			copy_passwd(pwd, pwd_p);
		}
	}

	return pwd != NULL ? true : false;
}

static void test_nwrap_getpwnam_r(const char *name,
				  struct passwd *pwd_p)
{
	struct passwd pwd, *pwdp;
	char buffer[4096];
	int ret;

	DEBUG("Testing getpwnam_r: %s\n", name);

	ret = getpwnam_r(name, &pwd, buffer, sizeof(buffer), &pwdp);
	if (ret != 0) {
		if (ret != ENOENT) {
			DEBUG("got %d return code\n", ret);
		}
		assert_true(ret);
	}

	print_passwd(&pwd);

	if (pwd_p) {
		copy_passwd(&pwd, pwd_p);
	}
}

static bool test_nwrap_getpwuid(uid_t uid,
				struct passwd *pwd_p)
{
	struct passwd *pwd = NULL;

	DEBUG("Testing getpwuid: %lu\n", (unsigned long)uid);

	pwd = getpwuid(uid);
	if (pwd != NULL) {
		print_passwd(pwd);

		if (pwd_p != NULL) {
			copy_passwd(pwd, pwd_p);
		}
	}

	return pwd != NULL ? true : false;
}

static bool test_nwrap_getpwuid_r(uid_t uid,
				  struct passwd *pwd_p)
{
	struct passwd pwd, *pwdp;
	char buffer[4096];
	int ret;

	DEBUG("Testing getpwuid_r: %lu\n", (unsigned long)uid);

	ret = getpwuid_r(uid, &pwd, buffer, sizeof(buffer), &pwdp);
	if (ret != 0) {
		if (ret != ENOENT) {
			DEBUG("got %d return code\n", ret);
		}
		assert_true(ret);
	}

	print_passwd(&pwd);

	if (pwd_p) {
		copy_passwd(&pwd, pwd_p);
	}

	return true;
}

static bool copy_group(const struct group *grp,
		       struct group *g)
{
	int i;

	g->gr_name	= strdup(grp->gr_name);
	g->gr_passwd	= strdup(grp->gr_passwd);
	g->gr_gid	= grp->gr_gid;
	g->gr_mem	= NULL;

	for (i = 0; grp->gr_mem != NULL && grp->gr_mem[i] != NULL; i++) {
		char **mem;

		mem = realloc(g->gr_mem, sizeof(char *) * (i + 2));
		assert_non_null(mem);
		g->gr_mem = mem;

		g->gr_mem[i] = strdup(grp->gr_mem[i]);
		assert_non_null(g->gr_mem[i]);

		g->gr_mem[i + 1] = NULL;
	}

	return true;
}

static void free_group(const struct group *g)
{
	if (g->gr_name != NULL) {
		free(g->gr_name);
	}
	if (g->gr_passwd != NULL) {
		free(g->gr_passwd);
	}
	if (g->gr_mem != NULL) {
		int i;

		for (i = 0; g->gr_mem[i] != NULL; i++) {
			free(g->gr_mem[i]);
		}

		free(g->gr_mem);
	}
}

static void free_groups(struct group *grps, size_t num_grps)
{
	size_t i;

	for(i = 0; i < num_grps; i++) {
		free_group(&grps[i]);
	}
	free(grps);
}

static void print_group(struct group *grp)
{
	int i;
	DEBUG("%s:%s:%lu:",
	       grp->gr_name,
	       grp->gr_passwd,
	       (unsigned long)grp->gr_gid);

	if ((grp->gr_mem == NULL) || !grp->gr_mem[0]) {
		DEBUG("\n");
		return;
	}

	for (i=0; grp->gr_mem[i+1]; i++) {
		DEBUG("%s,", grp->gr_mem[i]);
	}
	DEBUG("%s\n", grp->gr_mem[i]);
}

static bool test_nwrap_getgrnam(const char *name,
				struct group *grp_p)
{
	struct group *grp = NULL;

	DEBUG("Testing getgrnam: %s\n", name);

	grp = getgrnam(name);
	if (grp != NULL) {
		print_group(grp);

		if (grp_p != NULL) {
			copy_group(grp, grp_p);
		}
	}

	return grp != NULL ? true : false;
}

static bool test_nwrap_getgrnam_r(const char *name,
				  struct group *grp_p)
{
	struct group grp, *grpp;
	char buffer[4096];
	int ret;

	DEBUG("Testing getgrnam_r: %s\n", name);

	ret = getgrnam_r(name, &grp, buffer, sizeof(buffer), &grpp);
	if (ret != 0) {
		if (ret != ENOENT) {
			DEBUG("got %d return code\n", ret);
		}
		assert_true(ret);
	}

	print_group(&grp);

	if (grp_p) {
		copy_group(&grp, grp_p);
	}

	return true;
}

static bool test_nwrap_getgrgid(gid_t gid,
				struct group *grp_p)
{
	struct group *grp = NULL;

	DEBUG("Testing getgrgid: %lu\n", (unsigned long)gid);

	grp = getgrgid(gid);
	if (grp != NULL) {
		print_group(grp);

		if (grp_p != NULL) {
			copy_group(grp, grp_p);
		}
	}

	return grp != NULL ? true : false;
}

static bool test_nwrap_getgrgid_r(gid_t gid,
				  struct group *grp_p)
{
	struct group grp, *grpp;
	char buffer[4096];
	int ret;

	DEBUG("Testing getgrgid_r: %lu\n", (unsigned long)gid);

	ret = getgrgid_r(gid, &grp, buffer, sizeof(buffer), &grpp);
	if (ret != 0) {
		if (ret != ENOENT) {
			DEBUG("got %d return code\n", ret);
		}
		assert_true(ret);
	}

	print_group(&grp);

	if (grp_p) {
		copy_group(&grp, grp_p);
	}

	return true;
}

static bool test_nwrap_enum_passwd(struct passwd **pwd_array_p,
				   size_t *num_pwd_p)
{
	struct passwd *pwd;
	struct passwd *pwd_array = NULL;
	size_t num_pwd = 0;

	DEBUG("Testing setpwent\n");

	setpwent();

	while ((pwd = getpwent()) != NULL) {
		DEBUG("Testing getpwent\n");

		print_passwd(pwd);
		if (pwd_array_p && num_pwd_p) {
			pwd_array = realloc(pwd_array, sizeof(struct passwd) * (num_pwd + 1));
			assert_non_null(pwd_array);
			copy_passwd(pwd, &pwd_array[num_pwd]);
			num_pwd++;
		}
	}

	DEBUG("Testing endpwent\n");
	endpwent();

	if (pwd_array_p) {
		*pwd_array_p = pwd_array;
	}
	if (num_pwd_p) {
		*num_pwd_p = num_pwd;
	}

	return true;
}

static bool test_nwrap_enum_r_passwd(struct passwd **pwd_array_p,
				     size_t *num_pwd_p)
{
	struct passwd pwd, *pwdp;
	struct passwd *pwd_array = NULL;
	size_t num_pwd = 0;
	char buffer[4096];
	int ret;

	DEBUG("Testing setpwent\n");
	setpwent();

	while (1) {
		DEBUG("Testing getpwent_r\n");

#ifdef HAVE_SOLARIS_GETPWENT_R
		pwdp = getpwent_r(&pwd, buffer, sizeof(buffer));
		if (pwdp == NULL) {
			break;
		}
#else
		ret = getpwent_r(&pwd, buffer, sizeof(buffer), &pwdp);
		if (ret != 0) {
			if (ret != ENOENT) {
				DEBUG("got %d return code\n", ret);
			}
			break;
		}
#endif
		print_passwd(&pwd);
		if (pwd_array_p && num_pwd_p) {
			pwd_array = realloc(pwd_array, sizeof(struct passwd) * (num_pwd + 1));
			assert_non_null(pwd_array);
			copy_passwd(&pwd, &pwd_array[num_pwd]);
			num_pwd++;
		}
	}

	DEBUG("Testing endpwent\n");
	endpwent();

	if (pwd_array_p) {
		*pwd_array_p = pwd_array;
	}
	if (num_pwd_p) {
		*num_pwd_p = num_pwd;
	}

	return true;
}

static bool test_nwrap_passwd(void)
{
	struct passwd *pwd, pwd1, pwd2;
	size_t i, num_pwd;

	test_nwrap_enum_passwd(&pwd, &num_pwd);

	for (i=0; i < num_pwd; i++) {
		test_nwrap_getpwnam(pwd[i].pw_name, &pwd1);
		assert_passwd_equal(&pwd[i], &pwd1);

		test_nwrap_getpwuid(pwd[i].pw_uid, &pwd2);
		assert_passwd_equal(&pwd[i], &pwd2);
		assert_passwd_equal(&pwd1, &pwd2);

		free_passwd(&pwd1);
		free_passwd(&pwd2);
	}
	free_passwds(pwd, num_pwd);

	return true;
}

static void test_nwrap_passwd_r(void)
{
	struct passwd *pwd, pwd1, pwd2;
	size_t i, num_pwd;

	test_nwrap_enum_r_passwd(&pwd, &num_pwd);

	for (i=0; i < num_pwd; i++) {
		test_nwrap_getpwnam_r(pwd[i].pw_name, &pwd1);
		assert_passwd_equal(&pwd[i], &pwd1);

		test_nwrap_getpwuid_r(pwd[i].pw_uid, &pwd2);
		assert_passwd_equal(&pwd[i], &pwd2);

		assert_passwd_equal(&pwd1, &pwd2);

		free_passwd(&pwd1);
		free_passwd(&pwd2);
	}
	free_passwds(pwd, num_pwd);
}

static bool test_nwrap_passwd_r_cross(void)
{
	struct passwd *pwd, pwd1, pwd2, pwd3, pwd4;
	size_t i, num_pwd;

	test_nwrap_enum_r_passwd(&pwd, &num_pwd);

	for (i=0; i < num_pwd; i++) {
		test_nwrap_getpwnam_r(pwd[i].pw_name, &pwd1);
		assert_passwd_equal(&pwd[i], &pwd1);

		test_nwrap_getpwuid_r(pwd[i].pw_uid, &pwd2);
		assert_passwd_equal(&pwd[i], &pwd2);

		assert_passwd_equal(&pwd1, &pwd2);

		test_nwrap_getpwnam(pwd[i].pw_name, &pwd3);
		assert_passwd_equal(&pwd[i], &pwd3);

		test_nwrap_getpwuid(pwd[i].pw_uid, &pwd4);
		assert_passwd_equal(&pwd[i], &pwd4);

		assert_passwd_equal(&pwd3, &pwd4);

		free_passwd(&pwd1);
		free_passwd(&pwd2);
		free_passwd(&pwd3);
		free_passwd(&pwd4);
	}
	free_passwds(pwd, num_pwd);

	return true;
}

static bool test_nwrap_enum_group(struct group **grp_array_p,
				  size_t *num_grp_p)
{
	struct group *grp;
	struct group *grp_array = NULL;
	size_t num_grp = 0;

	DEBUG("Testing setgrent\n");
	setgrent();

	while ((grp = getgrent()) != NULL) {
		DEBUG("Testing getgrent\n");

		print_group(grp);
		if (grp_array_p && num_grp_p) {
			grp_array = realloc(grp_array, sizeof(struct group) * (num_grp + 1));
			assert_non_null(grp_array);
			copy_group(grp, &grp_array[num_grp]);
			num_grp++;
		}
	}

	DEBUG("Testing endgrent\n");
	endgrent();

	if (grp_array_p) {
		*grp_array_p = grp_array;
	}
	if (num_grp_p) {
		*num_grp_p = num_grp;
	}

	return true;
}

static bool test_nwrap_enum_r_group(struct group **grp_array_p,
				    size_t *num_grp_p)
{
	struct group grp, *grpp;
	struct group *grp_array = NULL;
	size_t num_grp = 0;
	char buffer[4096];
	int ret;

	DEBUG("Testing setgrent\n");
	setgrent();

	while (1) {
		DEBUG("Testing getgrent_r\n");

#ifdef HAVE_SOLARIS_GETGRENT_R
		grpp = getgrent_r(&grp, buffer, sizeof(buffer));
		if (grpp == NULL) {
			break;
		}
#else
		ret = getgrent_r(&grp, buffer, sizeof(buffer), &grpp);
		if (ret != 0) {
			if (ret != ENOENT) {
				DEBUG("got %d return code\n", ret);
			}
			break;
		}
#endif
		print_group(&grp);
		if (grp_array_p && num_grp_p) {
			grp_array = realloc(grp_array, sizeof(struct group) * (num_grp + 1));
			assert_non_null(grp_array);
			copy_group(&grp, &grp_array[num_grp]);
			num_grp++;
		}
	}

	DEBUG("Testing endgrent\n");
	endgrent();

	if (grp_array_p) {
		*grp_array_p = grp_array;
	}
	if (num_grp_p) {
		*num_grp_p = num_grp;
	}

	return true;
}

static bool test_nwrap_group(void)
{
	struct group *grp, grp1, grp2;
	size_t i, num_grp;

	test_nwrap_enum_group(&grp, &num_grp);

	for (i=0; i < num_grp; i++) {
		test_nwrap_getgrnam(grp[i].gr_name, &grp1);
		assert_group_equal(&grp[i], &grp1);

		test_nwrap_getgrgid(grp[i].gr_gid, &grp2);
		assert_group_equal(&grp[i], &grp2);

		assert_group_equal(&grp1, &grp2);

		free_group(&grp1);
		free_group(&grp2);
	}
	free_groups(grp, num_grp);

	return true;
}

static bool test_nwrap_group_r(void)
{
	struct group *grp, grp1, grp2;
	size_t i, num_grp;

	test_nwrap_enum_r_group(&grp, &num_grp);

	for (i=0; i < num_grp; i++) {
		test_nwrap_getgrnam_r(grp[i].gr_name, &grp1);
		assert_group_equal(&grp[i], &grp1);

		test_nwrap_getgrgid_r(grp[i].gr_gid, &grp2);
		assert_group_equal(&grp[i], &grp2);

		assert_group_equal(&grp1, &grp2);

		free_group(&grp1);
		free_group(&grp2);
	}
	free_groups(grp, num_grp);

	return true;
}

static bool test_nwrap_group_r_cross(void)
{
	struct group *grp, grp1, grp2, grp3, grp4;
	size_t i, num_grp;

	test_nwrap_enum_r_group(&grp, &num_grp);

	for (i=0; i < num_grp; i++) {
		test_nwrap_getgrnam_r(grp[i].gr_name, &grp1);
		assert_group_equal(&grp[i], &grp1);

		test_nwrap_getgrgid_r(grp[i].gr_gid, &grp2);
		assert_group_equal(&grp[i], &grp2);

		assert_group_equal(&grp1, &grp2);

		test_nwrap_getgrnam(grp[i].gr_name, &grp3);
		assert_group_equal(&grp[i], &grp3);

		test_nwrap_getgrgid(grp[i].gr_gid, &grp4);
		assert_group_equal(&grp[i], &grp4);

		assert_group_equal(&grp3, &grp4);

		free_group(&grp1);
		free_group(&grp2);
		free_group(&grp3);
		free_group(&grp4);
	}
	free_groups(grp, num_grp);

	return true;
}

#ifdef HAVE_GETGROUPLIST
static bool test_nwrap_getgrouplist(const char *user,
				    gid_t gid,
				    gid_t **gids_p,
				    int *num_gids_p)
{
	int ret;
	int num_groups = 0;
	gid_t *groups = NULL;

	DEBUG("Testing getgrouplist: %s\n", user);

	ret = getgrouplist(user, gid, NULL, &num_groups);
	if (ret == -1 || num_groups != 0) {

		groups = malloc(sizeof(gid_t) * num_groups);
		assert_non_null(groups);

		ret = getgrouplist(user, gid, groups, &num_groups);
	}

	assert_false(ret == -1);

	DEBUG("%s is member in %d groups\n", user, num_groups);

	if (gids_p) {
		*gids_p = groups;
	}
	if (num_gids_p) {
		*num_gids_p = num_groups;
	}

	return true;
}

static bool test_nwrap_user_in_group(const struct passwd *pwd,
				     const struct group *grp)
{
	int i;

	for (i = 0; grp->gr_mem != NULL && grp->gr_mem[i] != NULL; i++) {
		if (strcmp(grp->gr_mem[i], pwd->pw_name) == 0) {
			return true;
		}
	}

	return false;
}

static bool test_nwrap_membership_user(const struct passwd *pwd,
				       struct group *grp_array,
				       size_t num_grp)
{
	int num_user_groups = 0;
	size_t num_user_groups_from_enum = 0;
	gid_t *user_groups = NULL;
	size_t i;
	int g;
	bool primary_group_had_user_member = false;

	test_nwrap_getgrouplist(pwd->pw_name,
				pwd->pw_gid,
				&user_groups,
				&num_user_groups);

	for (g=0; g < num_user_groups; g++) {
		test_nwrap_getgrgid(user_groups[g], NULL);
	}
	free(user_groups);

	for (i=0; i < num_grp; i++) {
		struct group grp = grp_array[i];

		if (test_nwrap_user_in_group(pwd, &grp)) {

			struct group current_grp;
			num_user_groups_from_enum++;

			test_nwrap_getgrnam(grp.gr_name, &current_grp);

			if (current_grp.gr_gid == pwd->pw_gid) {
				DEBUG("primary group %s of user %s lists user as member\n",
				      current_grp.gr_name,
				      pwd->pw_name);
				primary_group_had_user_member = true;
			}
			free_group(&current_grp);

			continue;
		}
	}

	if (!primary_group_had_user_member) {
		num_user_groups_from_enum++;
	}

	assert_int_equal(num_user_groups, num_user_groups_from_enum);

	return true;
}

static void test_nwrap_membership(void **state)
{
	const char *old_pwd = getenv("NSS_WRAPPER_PASSWD");
	const char *old_group = getenv("NSS_WRAPPER_GROUP");
	struct passwd *pwd;
	size_t num_pwd;
	struct group *grp;
	size_t num_grp;
	size_t i;

	(void) state; /* unused */

	if (!old_pwd || !old_group) {
		DEBUG("ENV NSS_WRAPPER_PASSWD or NSS_WRAPPER_GROUP not set\n");
		return;
	}

	test_nwrap_enum_passwd(&pwd, &num_pwd);
	test_nwrap_enum_group(&grp, &num_grp);

	for (i=0; i < num_pwd; i++) {
		test_nwrap_membership_user(&pwd[i], grp, num_grp);
	}
	free_passwds(pwd, num_pwd);
	free_groups(grp, num_grp);
}
#endif /* HAVE_GETGROUPLIST */

static void test_nwrap_enumeration(void **state)
{
	const char *old_pwd = getenv("NSS_WRAPPER_PASSWD");
	const char *old_group = getenv("NSS_WRAPPER_GROUP");

	(void) state; /* unused */

	if (!old_pwd || !old_group) {
		DEBUG("ENV NSS_WRAPPER_PASSWD or NSS_WRAPPER_GROUP not set\n");
		return;
	}

	test_nwrap_passwd();
	test_nwrap_group();
}

static void test_nwrap_reentrant_enumeration(void **state)
{
	const char *old_pwd = getenv("NSS_WRAPPER_PASSWD");
	const char *old_group = getenv("NSS_WRAPPER_GROUP");

	(void) state; /* unused */

	if (!old_pwd || !old_group) {
		DEBUG("ENV NSS_WRAPPER_PASSWD or NSS_WRAPPER_GROUP not set\n");
		return;
	}

	DEBUG("Testing re-entrant calls\n");

	test_nwrap_passwd_r();
	test_nwrap_group_r();
}

static void test_nwrap_reentrant_enumeration_crosschecks(void **state)
{
	const char *old_pwd = getenv("NSS_WRAPPER_PASSWD");
	const char *old_group = getenv("NSS_WRAPPER_GROUP");

	(void) state; /* unused */

	if (!old_pwd || !old_group) {
		DEBUG("ENV NSS_WRAPPER_PASSWD or NSS_WRAPPER_GROUP not set\n");
		return;
	}

	DEBUG("Testing re-entrant calls with cross checks\n");

	test_nwrap_passwd_r_cross();
	test_nwrap_group_r_cross();
}

static bool test_nwrap_passwd_duplicates(void)
{
	struct passwd *pwd;
	size_t d, i, num_pwd;
	int duplicates = 0;

	test_nwrap_enum_passwd(&pwd, &num_pwd);

	for (i=0; i < num_pwd; i++) {
		const char *current_name = pwd[i].pw_name;
		for (d=0; d < num_pwd; d++) {
			const char *dup_name = pwd[d].pw_name;
			if (d == i) {
				continue;
			}
			if (strcmp(current_name, dup_name) != 0) {
				continue;
			}

			DEBUG("found duplicate names:");

			print_passwd(&pwd[d]);
			print_passwd(&pwd[i]);
			duplicates++;
		}
	}
	free_passwds(pwd, num_pwd);

	assert_false(duplicates);

	return true;
}

static bool test_nwrap_group_duplicates(void)
{
	struct group *grp;
	size_t d, i, num_grp;
	int duplicates = 0;

	test_nwrap_enum_group(&grp, &num_grp);

	for (i=0; i < num_grp; i++) {
		const char *current_name = grp[i].gr_name;
		for (d=0; d < num_grp; d++) {
			const char *dup_name = grp[d].gr_name;
			if (d == i) {
				continue;
			}
			if (strcmp(current_name, dup_name) != 0) {
				continue;
			}

			DEBUG("found duplicate names:");

			print_group(&grp[d]);
			print_group(&grp[i]);
			duplicates++;
		}
	}
	free_groups(grp, num_grp);

	assert_false(duplicates);

	return true;
}

static void test_nwrap_duplicates(void **state)
{
	const char *old_pwd = getenv("NSS_WRAPPER_PASSWD");
	const char *old_group = getenv("NSS_WRAPPER_GROUP");

	(void) state; /* unused */

	if (!old_pwd || !old_group) {
		DEBUG("ENV NSS_WRAPPER_PASSWD or NSS_WRAPPER_GROUP not set\n");
		return;
	}

	test_nwrap_passwd_duplicates();
	test_nwrap_group_duplicates();
}

int main(void) {
	int rc;

	const UnitTest tests[] = {
		unit_test(test_nwrap_enumeration),
		unit_test(test_nwrap_reentrant_enumeration),
		unit_test(test_nwrap_reentrant_enumeration_crosschecks),
#ifdef HAVE_GETGROUPLIST
		unit_test(test_nwrap_membership),
#endif
		unit_test(test_nwrap_duplicates),
	};

	rc = run_tests(tests);

	return rc;
}
