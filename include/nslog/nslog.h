/*
 * Copyright 2017 Daniel Silverstone <dsilvers@netsurf-browser.org>
 *
 * This file is part of libnslog.
 *
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 */

/**
 * \file
 * NetSurf Logging
 */

#ifndef NSLOG_NSLOG_H_
#define NSLOG_NSLOG_H_

#include <stdarg.h>

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

#define NSLOG_LEVEL_DD		NSLOG_LEVEL_DEEPDEBUG
#define NSLOG_LEVEL_DBG		NSLOG_LEVEL_DEBUG
#define NSLOG_LEVEL_CHAT	NSLOG_LEVEL_VERBOSE
#define NSLOG_LEVEL_WARN	NSLOG_LEVEL_WARNING
#define NSLOG_LEVEL_ERR		NSLOG_LEVEL_ERROR
#define NSLOG_LEVEL_CRIT	NSLOG_LEVEL_CRITICAL

#ifndef NSLOG_COMPILED_MIN_LEVEL
#define NSLOG_COMPILED_MIN_LEVEL NSLOG_LEVEL_DEBUG
#endif

typedef struct nslog_category_s {
	const char *cat_name;
	const char *description;
	struct nslog_category_s *parent;
	char *name;
	int namelen;
	struct nslog_category_s *next;
} nslog_category_t;

typedef struct nslog_entry_context_s {
	nslog_category_t *category;
	nslog_level level;
	const char *filename;
	int filenamelen;
	const char *funcname;
	int funcnamelen;
	int lineno;
} nslog_entry_context_t;

#define NSLOG_DECLARE_CATEGORY(catname)				\
	extern nslog_category_t __nslog_category_##catname

#define NSLOG_DEFINE_CATEGORY(catname, description)	\
	nslog_category_t __nslog_category_##catname = { \
		#catname,				\
		description,				\
		NULL,					\
		NULL,					\
		0,					\
		NULL,					\
	}

#define NSLOG_DEFINE_SUBCATEGORY(parentcatname, catname, description)	\
	nslog_category_t __nslog_category_##catname = {			\
		#catname,						\
		description,						\
		&__nslog_category_##parentcatname,			\
		NULL,							\
		0,							\
		NULL,							\
	}

#define NSLOG(catname, level, logmsg, args...)				\
	do {								\
		if (NSLOG_LEVEL_##level >= NSLOG_COMPILED_MIN_LEVEL) {	\
			nslog_entry_context_t ctx = {			\
				&__nslog_category_##catname,		\
				NSLOG_LEVEL_##level,			\
				__FILE__,				\
				sizeof(__FILE__) - 1,			\
				__PRETTY_FUNCTION__,			\
				sizeof(__PRETTY_FUNCTION__) - 1,	\
				__LINE__,				\
			};						\
			nslog__log(&ctx, logmsg, ##args);		\
		}							\
	} while(0)

void nslog__log(nslog_entry_context_t *ctx,
		const char *pattern,
		...) __attribute__ ((format (printf, 2, 3)));

typedef enum {
	NSLOG_NO_ERROR = 0,
	NSLOG_NO_MEMORY = 1,
	NSLOG_UNCORKED = 2,
	NSLOG_PARSE_ERROR = 3,
} nslog_error;

typedef void (*nslog_callback)(void *context, nslog_entry_context_t *ctx,
			       const char *fmt, va_list args);

nslog_error nslog_set_render_callback(nslog_callback cb, void *context);

nslog_error nslog_uncork(void);

typedef struct nslog_filter_s nslog_filter_t;

nslog_error nslog_filter_category_new(const char *catname,
				      nslog_filter_t **filter);
nslog_error nslog_filter_level_new(nslog_level level,
				   nslog_filter_t **filter);
nslog_error nslog_filter_filename_new(const char *filename,
				      nslog_filter_t **filter);
nslog_error nslog_filter_dirname_new(const char *dirname,
				     nslog_filter_t **filter);
nslog_error nslog_filter_funcname_new(const char *funcname,
				      nslog_filter_t **filter);

nslog_error nslog_filter_and_new(nslog_filter_t *left,
				 nslog_filter_t *right,
				 nslog_filter_t **filter);
nslog_error nslog_filter_or_new(nslog_filter_t *left,
				nslog_filter_t *right,
				nslog_filter_t **filter);
nslog_error nslog_filter_xor_new(nslog_filter_t *left,
				 nslog_filter_t *right,
				 nslog_filter_t **filter);
nslog_error nslog_filter_not_new(nslog_filter_t *input,
				 nslog_filter_t **filter);

nslog_filter_t *nslog_filter_ref(nslog_filter_t *filter);
nslog_filter_t *nslog_filter_unref(nslog_filter_t *filter);

nslog_error nslog_filter_set_active(nslog_filter_t *filter,
				    nslog_filter_t **prev);

char *nslog_filter_sprintf(nslog_filter_t *filter);

nslog_error nslog_filter_from_text(const char *input,
				   nslog_filter_t **output);

#endif /* NSLOG_NSLOG_H_ */
