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
 * NetSurf Logging Filters
 */

#include "nslog_internal.h"

typedef enum {
	/* Fundamentals */
	NSLFK_CATEGORY = 0,
	NSLFK_LEVEL,
	NSLFK_FILENAME,
	NSLFK_DIRNAME,
	NSLFK_FUNCNAME,
	/* logical operations */
	NSLFK_AND,
	NSLFK_OR,
	NSLFK_XOR,
	NSLFK_NOT,
} nslog_filter_kind;

struct nslog_filter_s {
	nslog_filter_kind kind;
	int refcount;
	union {
		struct {
			char *ptr;
			int len;
		} str;
		nslog_level level;
		nslog_filter_t *unary_input;
		struct {
			nslog_filter_t *input1;
			nslog_filter_t *input2;
		} binary;
	} params;
};

static nslog_filter_t *nslog__active_filter = NULL;

nslog_error nslog_filter_category_new(const char *catname,
				      nslog_filter_t **filter)
{
	nslog_filter_t *ret = calloc(sizeof(*ret), 1);
	if (ret == NULL)
		return NSLOG_NO_MEMORY;
	ret->kind = NSLFK_CATEGORY;
	ret->refcount = 1;
	ret->params.str.ptr = strdup(catname);
	ret->params.str.len = strlen(catname);
	if (ret->params.str.ptr == NULL) {
		free(ret);
		return NSLOG_NO_MEMORY;
	}
	*filter = ret;
	return NSLOG_NO_ERROR;
}

nslog_error nslog_filter_level_new(nslog_level level,
				   nslog_filter_t **filter)
{
	nslog_filter_t *ret = calloc(sizeof(*ret), 1);
	if (ret == NULL)
		return NSLOG_NO_MEMORY;
	ret->kind = NSLFK_LEVEL;
	ret->refcount = 1;
	ret->params.level = level;
	*filter = ret;
	return NSLOG_NO_ERROR;
}

nslog_error nslog_filter_filename_new(const char *filename,
				      nslog_filter_t **filter)
{
	nslog_filter_t *ret = calloc(sizeof(*ret), 1);
	if (ret == NULL)
		return NSLOG_NO_MEMORY;
	ret->kind = NSLFK_FILENAME;
	ret->refcount = 1;
	ret->params.str.ptr = strdup(filename);
	ret->params.str.len = strlen(filename);
	if (ret->params.str.ptr == NULL) {
		free(ret);
		return NSLOG_NO_MEMORY;
	}
	*filter = ret;
	return NSLOG_NO_ERROR;
}

nslog_error nslog_filter_dirname_new(const char *dirname,
				     nslog_filter_t **filter)
{
	nslog_filter_t *ret = calloc(sizeof(*ret), 1);
	if (ret == NULL)
		return NSLOG_NO_MEMORY;
	ret->kind = NSLFK_DIRNAME;
	ret->refcount = 1;
	ret->params.str.ptr = strdup(dirname);
	ret->params.str.len = strlen(dirname);
	if (ret->params.str.ptr == NULL) {
		free(ret);
		return NSLOG_NO_MEMORY;
	}
	*filter = ret;
	return NSLOG_NO_ERROR;
}

nslog_error nslog_filter_funcname_new(const char *funcname,
				      nslog_filter_t **filter)
{
	nslog_filter_t *ret = calloc(sizeof(*ret), 1);
	if (ret == NULL)
		return NSLOG_NO_MEMORY;
	ret->kind = NSLFK_FUNCNAME;
	ret->refcount = 1;
	ret->params.str.ptr = strdup(funcname);
	ret->params.str.len = strlen(funcname);
	if (ret->params.str.ptr == NULL) {
		free(ret);
		return NSLOG_NO_MEMORY;
	}
	*filter = ret;
	return NSLOG_NO_ERROR;
}


nslog_error nslog_filter_and_new(nslog_filter_t *left,
				 nslog_filter_t *right,
				 nslog_filter_t **filter)
{
	nslog_filter_t *ret = calloc(sizeof(*ret), 1);
	if (ret == NULL)
		return NSLOG_NO_MEMORY;
	ret->kind = NSLFK_AND;
	ret->refcount = 1;
	ret->params.binary.input1 = nslog_filter_ref(left);
	ret->params.binary.input2 = nslog_filter_ref(right);
	*filter = ret;
	return NSLOG_NO_ERROR;
}

nslog_error nslog_filter_or_new(nslog_filter_t *left,
				nslog_filter_t *right,
				nslog_filter_t **filter)
{
	nslog_filter_t *ret = calloc(sizeof(*ret), 1);
	if (ret == NULL)
		return NSLOG_NO_MEMORY;
	ret->kind = NSLFK_OR;
	ret->refcount = 1;
	ret->params.binary.input1 = nslog_filter_ref(left);
	ret->params.binary.input2 = nslog_filter_ref(right);
	*filter = ret;
	return NSLOG_NO_ERROR;
}

nslog_error nslog_filter_xor_new(nslog_filter_t *left,
				 nslog_filter_t *right,
				 nslog_filter_t **filter)
{
	nslog_filter_t *ret = calloc(sizeof(*ret), 1);
	if (ret == NULL)
		return NSLOG_NO_MEMORY;
	ret->kind = NSLFK_XOR;
	ret->refcount = 1;
	ret->params.binary.input1 = nslog_filter_ref(left);
	ret->params.binary.input2 = nslog_filter_ref(right);
	*filter = ret;
	return NSLOG_NO_ERROR;
}

nslog_error nslog_filter_not_new(nslog_filter_t *input,
				 nslog_filter_t **filter)
{
	nslog_filter_t *ret = calloc(sizeof(*ret), 1);
	if (ret == NULL)
		return NSLOG_NO_MEMORY;
	ret->kind = NSLFK_NOT;
	ret->refcount = 1;
	ret->params.unary_input = nslog_filter_ref(input);
	*filter = ret;
	return NSLOG_NO_ERROR;
}


nslog_filter_t *nslog_filter_ref(nslog_filter_t *filter)
{
	if (filter != NULL)
		filter->refcount++;

	return filter;
}

nslog_filter_t *nslog_filter_unref(nslog_filter_t *filter)
{
	if (filter != NULL && filter->refcount-- == 1) {
		switch(filter->kind) {
		case NSLFK_CATEGORY:
		case NSLFK_FILENAME:
		case NSLFK_DIRNAME:
		case NSLFK_FUNCNAME:
			free(filter->params.str.ptr);
			break;
		case NSLFK_AND:
		case NSLFK_OR:
		case NSLFK_XOR:
			nslog_filter_unref(filter->params.binary.input1);
			nslog_filter_unref(filter->params.binary.input2);
			break;
		case NSLFK_NOT:
			nslog_filter_unref(filter->params.unary_input);
			break;
		default:
			/* Nothing to do for the other kind(s) */
			break;
		}
		free(filter);
	}

	return NULL;
}

nslog_error nslog_filter_set_active(nslog_filter_t *filter,
				    nslog_filter_t **prev)
{
	if (prev != NULL)
		*prev = nslog__active_filter;
	else
		nslog_filter_unref(nslog__active_filter);

	nslog__active_filter = nslog_filter_ref(filter);

	return NSLOG_NO_ERROR;
}

static bool _nslog__filter_matches(nslog_entry_context_t *ctx,
				   nslog_filter_t *filter)
{
	switch (filter->kind) {
	case NSLFK_CATEGORY:
		if (filter->params.str.len > ctx->category->namelen)
			return false;
		if (ctx->category->name[filter->params.str.len] != '\0' &&
		    ctx->category->name[filter->params.str.len] != '/')
			return false;
		return (strncmp(filter->params.str.ptr,
				ctx->category->name,
				filter->params.str.len) == 0);

	case NSLFK_LEVEL:
		return (ctx->level >= filter->params.level);
	case NSLFK_FILENAME:
		if (filter->params.str.len > ctx->filenamelen)
			return false;
		if ((filter->params.str.len == ctx->filenamelen) &&
		    (strcmp(filter->params.str.ptr, ctx->filename) == 0))
			return true;
		if ((ctx->filename[ctx->filenamelen - filter->params.str.len - 1] == '/')
		    && (strcmp(filter->params.str.ptr,
			       ctx->filename + ctx->filenamelen - filter->params.str.len) == 0))
			return true;
		return false;
	case NSLFK_DIRNAME:
		if (filter->params.str.len >= ctx->filenamelen)
			return false;
		if ((ctx->filename[filter->params.str.len] == '/')
		    && (strncmp(filter->params.str.ptr,
				ctx->filename,
				filter->params.str.len) == 0))
			return true;
		return false;
	case NSLFK_FUNCNAME:
		return (filter->params.str.len == ctx->funcnamelen &&
			strcmp(ctx->funcname, filter->params.str.ptr) == 0);
	case NSLFK_AND:
		return (_nslog__filter_matches(ctx, filter->params.binary.input1)
			&&
			_nslog__filter_matches(ctx, filter->params.binary.input2));
	case NSLFK_OR:
		return (_nslog__filter_matches(ctx, filter->params.binary.input1)
			||
			_nslog__filter_matches(ctx, filter->params.binary.input2));
	case NSLFK_XOR:
		return (_nslog__filter_matches(ctx, filter->params.binary.input1)
			^
			_nslog__filter_matches(ctx, filter->params.binary.input2));
	case NSLFK_NOT:
		return !_nslog__filter_matches(ctx, filter->params.unary_input);
	default:
		/* unknown */
		assert("Unknown filter kind" == NULL);
		return false;
	}
}

bool nslog__filter_matches(nslog_entry_context_t *ctx)
{
	if (nslog__active_filter == NULL)
		return true;
	return _nslog__filter_matches(ctx, nslog__active_filter);
}
