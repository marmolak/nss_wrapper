/* Name of package */
#cmakedefine PACKAGE "${APPLICATION_NAME}"

/* Version number of package */
#cmakedefine VERSION "${APPLICATION_VERSION}"

#cmakedefine LOCALEDIR "${LOCALE_INSTALL_DIR}"
#cmakedefine DATADIR "${DATADIR}"
#cmakedefine LIBDIR "${LIBDIR}"
#cmakedefine PLUGINDIR "${PLUGINDIR}"
#cmakedefine SYSCONFDIR "${SYSCONFDIR}"
#cmakedefine BINARYDIR "${BINARYDIR}"
#cmakedefine SOURCEDIR "${SOURCEDIR}"

/************************** HEADER FILES *************************/

#cmakedefine HAVE_SYS_TYPES_H 1
#cmakedefine HAVE_PWD_H 1
#cmakedefine HAVE_GRP_H 1
#cmakedefine HAVE_NSS_H 1

/*************************** FUNCTIONS ***************************/

/* Define to 1 if you have the `getpwnam_r' function. */
#cmakedefine HAVE_GETPWNAM_R 1

/* Define to 1 if you have the `getpwuid_r' function. */
#cmakedefine HAVE_GETPWUID_R 1

/* Define to 1 if you have the `getpwent_r' function. */
#cmakedefine HAVE_GETPWENT_R 1

/* Define to 1 if you have the `getgrnam_r' function. */
#cmakedefine HAVE_GETGRNAM_R 1

/* Define to 1 if you have the `getgrgid_r' function. */
#cmakedefine HAVE_GETGRGID_R 1

/* Define to 1 if you have the `getgrent_r' function. */
#cmakedefine HAVE_GETGRENT_R 1

/* Define to 1 if you have the `getgrouplist' function. */
#cmakedefine HAVE_GETGROUPLIST 1

/*************************** LIBRARIES ***************************/


/**************************** OPTIONS ****************************/

#cmakedefine SOLARIS_GETPWENT_R 1
#cmakedefine SOLARIS_GETGRENT_R 1

#cmakedefine HAVE_STRUCT_SOCKADDR_SA_LEN 1
#cmakedefine HAVE_IPV6 1

/*************************** ENDIAN *****************************/

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#cmakedefine WORDS_BIGENDIAN 1
