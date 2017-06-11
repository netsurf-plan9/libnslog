/*
 * Copyright 2017 Daniel Silverstone <dsilvers@netsurf-browser.org>
 *
 * This file is part of libnslog.
 *
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 */

#include "nslog/nslog.h"

#include <stdio.h>
#include <assert.h>

NSLOG_DEFINE_CATEGORY(test, "Test category");

static void test_render_function(
	void *_ctx, nslog_entry_context_t *ctx,
	const char *fmt, va_list args)
{
	(void)ctx;
	fprintf(stderr, "%s %s:%d [%s] %s() ",
		nslog_level_name(ctx->level),
		ctx->filename, ctx->lineno,
		ctx->category->name,
		ctx->funcname);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
	nslog_set_render_callback(test_render_function, NULL);
	nslog_uncork();

	nslog_filter_t *cat_test, *cat_another;

	assert(nslog_filter_category_new("test", &cat_test) == NSLOG_NO_ERROR);
	assert(nslog_filter_category_new("another", &cat_another) == NSLOG_NO_ERROR);

	NSLOG(test, INFO, "Hurrah, a message!");
	assert(nslog_filter_set_active(cat_test, NULL) == NSLOG_NO_ERROR);
	NSLOG(test, INFO, "You should see me.");
	assert(nslog_filter_set_active(cat_another, NULL) == NSLOG_NO_ERROR);
	NSLOG(test, INFO, "You should not see me!");
	assert(nslog_filter_set_active(NULL, NULL) == NSLOG_NO_ERROR);
	NSLOG(test, INFO, "You should see this one though.");

	return 0;
}
