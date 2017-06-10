/*
 * Copyright 2017 Daniel Silverstone <dsilvers@netsurf-browser.org>
 *
 * This file is part of libnslog.
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include "nslog/nslog.h"

#include <stdio.h>

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
	NSLOG(test, INFO, "Pre-uncorking");
	fprintf(stderr, "About to nslog_uncork()\n");
	nslog_uncork();
	fprintf(stderr, "Uncorked now\n");
	NSLOG(test, WARN, "argc=%d", argc);
	for (int i = 0; i < argc; ++i)
		NSLOG(test, WARN, "argv[%d] = %s", i, argv[i]);
	return 0;
}
