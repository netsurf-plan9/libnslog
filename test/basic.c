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

static void test_render_function(void *ctx, nslog_entry_t *log)
{
	(void)ctx;
        fprintf(stderr, "%s %s:%d [%s] %s() %s\n",
                nslog_level_name(log->level),
                log->filename, log->lineno,
                log->category->name,
		log->funcname,
                log->message);
}

int main(int argc, char **argv)
{
	nslog_set_render_callback(test_render_function, NULL);
	nslog_uncork();
	NSLOG(test, WARN, "argc=%d", argc);
	return 0;
}
