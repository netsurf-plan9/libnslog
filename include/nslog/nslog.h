/*
 * Copyright 2017 Daniel Silverstone <dsilvers@netsurf-browser.org>
 *
 * This file is part of libnslog.
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

/**
 * \file
 * NetSurf Logging
 */

#ifndef NSLOG_NSLOG_H_
#define NSLOG_NSLOG_H_

typedef enum {
        NSLOG_LEVEL_DEEPDEBUG = 0,
        NSLOG_LEVEL_DEBUG = 1,
        NSLOG_LEVEL_VERBOSE = 2,
        NSLOG_LEVEL_INFO = 3,
        NSLOG_LEVEL_WARNING = 4,
        NSLOG_LEVEL_ERROR = 5,
        NSLOG_LEVEL_CRITICAL = 6,
} nslog_level;

const char *nslog_level_name(nslog_level level);

#define NSLOG_LEVEL_DD          NSLOG_LEVEL_DEEPDEBUG
#define NSLOG_LEVEL_CHAT        NSLOG_LEVEL_VERBOSE
#define NSLOG_LEVEL_WARN        NSLOG_LEVEL_WARNING
#define NSLOG_LEVEL_ERR         NSLOG_LEVEL_ERROR
#define NSLOG_LEVEL_CRIT        NSLOG_LEVEL_CRITICAL

#ifndef NSLOG_COMPILED_MIN_LEVEL
#define NSLOG_COMPILED_MIN_LEVEL NSLOG_LEVEL_DEBUG
#endif

typedef struct nslog_category_s {
        const char *cat_name;
        const char *description;
        struct nslog_category_s *parent;
        char *name;
        struct nslog_category_s *next;
} nslog_category_t;

typedef struct nslog_entry_s {
        nslog_category_t *category;
        nslog_level level;
        const char *filename;
	const char *funcname;
        int lineno;
        char message[0]; /* NUL terminated */
} nslog_entry_t;

#define NSLOG_DECLARE_CATEGORY(catname)		\
	extern nslog_category_t __nslog_category_##catname

#define NSLOG_DEFINE_CATEGORY(catname, description)	\
	nslog_category_t __nslog_category_##catname = { \
		#catname,				\
		description,				\
		NULL,					\
		NULL,					\
		NULL,					\
	}

#define NSLOG_DEFINE_SUBCATEGORY(parentcatname, catname, description)	\
	nslog_category_t __nslog_category_##catname = {			\
		#catname,						\
		description,						\
		&__nslog_category_##parentcatname,			\
		NULL,							\
		NULL,							\
	}

#define NSLOG(catname, level, logmsg, args...)		\
	if (NSLOG_LEVEL_##level >= NSLOG_COMPILED_MIN_LEVEL) {	\
		nslog__log(&__nslog_category_##catname,	\
			   NSLOG_LEVEL_##level,		\
			   __FILE__,			\
			   __LINE__,			\
			   __PRETTY_FUNCTION__,		\
			   logmsg,			\
			   ##args);			\
	}

void nslog__log(nslog_category_t *category,
		nslog_level level,
		const char *filename,
		int lineno,
		const char *funcname,
		const char *pattern,
		...) __attribute__ ((format (printf, 6, 7)));

typedef enum {
	NSLOG_NO_ERROR = 0,
	NSLOG_NO_MEMORY = 1,
} nslog_error;

typedef void (*nslog_callback)(void *context, nslog_entry_t *msg);

nslog_error nslog_set_render_callback(nslog_callback cb, void *context);

nslog_error nslog_uncork(void);

#endif /* NSLOG_NSLOG_H_ */
