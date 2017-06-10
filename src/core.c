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
 * NetSurf Logging Core
 */

#include "nslog/nslog.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static bool nslog__corked = true;

static struct nslog_cork_chain {
	struct nslog_cork_chain *next;
	nslog_entry_t *entry;
} *nslog__cork_chain = NULL;

static nslog_callback nslog__cb = NULL;
static void *nslog__cb_ctx = NULL;

static nslog_category_t *nslog__all_categories = NULL;

const char *nslog_level_name(nslog_level level)
{
	switch (level) {
	case NSLOG_LEVEL_DEEPDEBUG:
		return "DEEPDEBUG";
	case NSLOG_LEVEL_DEBUG:
		return "DEBUG";
	case NSLOG_LEVEL_VERBOSE:
		return "VERBOSE";
	case NSLOG_LEVEL_INFO:
		return "INFO";
	case NSLOG_LEVEL_WARNING:
		return "WARNING";
	case NSLOG_LEVEL_ERROR:
		return "ERROR";
	case NSLOG_LEVEL_CRITICAL:
		return "CRITICAL";
	};
	
	return "**UNKNOWN**";
}


static void nslog__normalise_category(nslog_category_t *cat)
{
	if (cat->parent == NULL) {
		cat->name = strdup(cat->cat_name);
	} else {
		nslog__normalise_category(cat->parent);
		cat->name = malloc(strlen(cat->parent->name) + strlen(cat->cat_name) + 2);
		strcpy(cat->name, cat->parent->name);
		strcat(cat->name, "/");
		strcat(cat->name, cat->cat_name);
		cat->next = nslog__all_categories;
		nslog__all_categories = cat;
	}
}

static void nslog__deliver(nslog_entry_t *entry)
{
	/* TODO: Add filtering here */
	if (nslog__cb != NULL) {
		if (entry->category->name == NULL) {
			nslog__normalise_category(entry->category);
		}
		(*nslog__cb)(nslog__cb_ctx, entry);
	}
}

void nslog__log(nslog_category_t *category,
		nslog_level level,
		const char *filename,
		int lineno,
		const char *funcname,
		const char *pattern,
		...)
{
	va_list ap;
	va_start(ap, pattern);
	va_list ap2;
	va_copy(ap2, ap);
	int slen = vsnprintf(NULL, 0, pattern, ap);
	va_end(ap);
	nslog_entry_t *entry = malloc(sizeof(nslog_entry_t) + slen + 1);
	if (entry == NULL) {
		/* We're at ENOMEM! log entry is lost */
		va_end(ap2);
		return;
	}
	entry->category = category;
	entry->level = level;
	entry->filename = filename;
	entry->funcname = funcname;
	entry->lineno = lineno;
	vsprintf(entry->message, pattern, ap2);
	va_end(ap2);
	if (nslog__corked) {
		struct nslog_cork_chain *chained = malloc(sizeof(struct nslog_cork_chain));
		if (chained == NULL) {
			/* ENOMEM during corked operation! wow */
			free(entry);
			return;
		}
		chained->next = nslog__cork_chain;
		chained->entry = entry;
		nslog__cork_chain = chained;
	} else {
		/* Not corked */
		nslog__deliver(entry);
		free(entry);
	}
}

nslog_error nslog_set_render_callback(nslog_callback cb, void *context)
{
	nslog__cb = cb;
	nslog__cb_ctx = context;
	
	return NSLOG_NO_ERROR;
}

nslog_error nslog_uncork()
{
	if (nslog__corked) {
		while (nslog__cork_chain != NULL) {
			struct nslog_cork_chain *ent = nslog__cork_chain;
			nslog__cork_chain = ent->next;
			nslog__deliver(ent->entry);
			free(ent->entry);
			free(ent);
		}
		nslog__corked = false;
	}
	return NSLOG_NO_ERROR;
}

