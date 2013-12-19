#include <nss.h>
#include <pwd.h>
#include <grp.h>

typedef enum nss_status NSS_STATUS;

NSS_STATUS _nss_nwrap_setpwent(void);
NSS_STATUS _nss_nwrap_endpwent(void);
NSS_STATUS _nss_nwrap_getpwent_r(struct passwd *result, char *buffer,
				 size_t buflen, int *errnop);
NSS_STATUS _nss_nwrap_getpwuid_r(uid_t uid, struct passwd *result,
				 char *buffer, size_t buflen, int *errnop);
NSS_STATUS _nss_nwrap_getpwnam_r(const char *name, struct passwd *result,
				   char *buffer, size_t buflen, int *errnop);
NSS_STATUS _nss_nwrap_setgrent(void);
NSS_STATUS _nss_nwrap_endgrent(void);
NSS_STATUS _nss_nwrap_getgrent_r(struct group *result, char *buffer,
				 size_t buflen, int *errnop);
NSS_STATUS _nss_nwrap_getgrnam_r(const char *name, struct group *result,
				 char *buffer, size_t buflen, int *errnop);
NSS_STATUS _nss_nwrap_getgrgid_r(gid_t gid, struct group *result, char *buffer,
				 size_t buflen, int *errnop);
NSS_STATUS _nss_nwrap_initgroups_dyn(char *user, gid_t group, long int *start,
				     long int *size, gid_t **groups,
				     long int limit, int *errnop);

NSS_STATUS _nss_nwrap_setpwent(void)
{
	return NSS_STATUS_UNAVAIL;
}

NSS_STATUS _nss_nwrap_endpwent(void)
{
	return NSS_STATUS_UNAVAIL;
}

NSS_STATUS _nss_nwrap_getpwent_r(struct passwd *result, char *buffer,
				 size_t buflen, int *errnop)
{
	(void) result;
	(void) buffer;
	(void) buflen;
	(void) errnop;

	return NSS_STATUS_UNAVAIL;
}

NSS_STATUS _nss_nwrap_getpwuid_r(uid_t uid, struct passwd *result,
				 char *buffer, size_t buflen, int *errnop)
{
	(void) uid;
	(void) result;
	(void) buffer;
	(void) buflen;
	(void) errnop;

	return NSS_STATUS_UNAVAIL;
}

NSS_STATUS _nss_nwrap_getpwnam_r(const char *name, struct passwd *result,
				 char *buffer, size_t buflen, int *errnop)
{
	(void) name;
	(void) result;
	(void) buffer;
	(void) buflen;
	(void) errnop;

	return NSS_STATUS_UNAVAIL;
}

NSS_STATUS _nss_nwrap_setgrent(void)
{
	return NSS_STATUS_UNAVAIL;
}

NSS_STATUS _nss_nwrap_endgrent(void)
{
	return NSS_STATUS_UNAVAIL;
}

NSS_STATUS _nss_nwrap_getgrent_r(struct group *result, char *buffer,
				 size_t buflen, int *errnop)
{
	(void) result;
	(void) buffer;
	(void) buflen;
	(void) errnop;

	return NSS_STATUS_UNAVAIL;
}

NSS_STATUS _nss_nwrap_getgrnam_r(const char *name, struct group *result,
				 char *buffer, size_t buflen, int *errnop)
{
	(void) name;
	(void) result;
	(void) buffer;
	(void) buflen;
	(void) errnop;

	return NSS_STATUS_UNAVAIL;
}

NSS_STATUS _nss_nwrap_getgrgid_r(gid_t gid, struct group *result, char *buffer,
				 size_t buflen, int *errnop)
{
	(void) gid;
	(void) result;
	(void) buffer;
	(void) buflen;
	(void) errnop;

	return NSS_STATUS_UNAVAIL;
}

NSS_STATUS _nss_nwrap_initgroups_dyn(char *user, gid_t group, long int *start,
				     long int *size, gid_t **groups,
				     long int limit, int *errnop)
{
	(void) user;
	(void) group;
	(void) start;
	(void) size;
	(void) groups;
	(void) limit;
	(void) errnop;

	return NSS_STATUS_UNAVAIL;
}

