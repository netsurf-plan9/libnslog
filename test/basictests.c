/* test/basictests.c
 *
 * Basic tests for the test suite for libnslog
 *
 * Copyright 2009,2017 The NetSurf Browser Project
 *                Daniel Silverstone <dsilvers@netsurf-browser.org>
 */

#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "tests.h"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#ifndef NDEBUG

/* Tests go here which need assert() to be checked */

#endif

NSLOG_DEFINE_CATEGORY(test, "Top level test category");

static void *captured_render_context = NULL;
static nslog_entry_context_t captured_context = { 0 };
static char captured_rendered_message[4096] = { 0 };
static int captured_rendered_message_length = 0;
static int captured_message_count = 0;

static const char* anchor_context_1 = "1";

static void
nslog__test__render_function(void *_ctx, nslog_entry_context_t *ctx,
			     const char *fmt, va_list args)
{
	captured_context = *ctx;
	captured_render_context = _ctx;
	captured_rendered_message_length =
		vsnprintf(captured_rendered_message,
			  sizeof(captured_rendered_message),
			  fmt, args);
	captured_message_count++;
}

/**** The next set of tests need a fixture set ****/

static void
with_simple_context_setup(void)
{
	captured_render_context = NULL;
	memset(&captured_context, 0, sizeof(captured_context));
	memset(captured_rendered_message, 0, sizeof(captured_rendered_message));
	captured_rendered_message_length = 0;
	captured_message_count = 0;
	fail_unless(nslog_set_render_callback(
			    nslog__test__render_function,
			    (void *)anchor_context_1) == NSLOG_NO_ERROR,
		    "Unable to set up render callback");
}

static void
with_simple_context_teardown(void)
{
        /* Nothing to do to tear down */
}

START_TEST (test_nslog_trivial_corked_message)
{
	NSLOG(test, INFO, "Hello %s", "world");
	fail_unless(nslog_uncork() == NSLOG_NO_ERROR,
		    "Unable to uncork");
	fail_unless(captured_message_count == 1,
		    "Captured message count was wrong");
	fail_unless(captured_render_context == anchor_context_1,
		    "Captured context wasn't passed through");
	fail_unless(strcmp(captured_context.category->name, "test") == 0,
		    "Captured context category wasn't normalised");
	fail_unless(captured_context.category == &__nslog_category_test,
		    "Captured context category wasn't the one we wanted");
	fail_unless(captured_rendered_message_length == 11,
		    "Captured message wasn't correct length");
	fail_unless(strcmp(captured_rendered_message, "Hello world") == 0,
		    "Captured message wasn't correct");
	fail_unless(strcmp(captured_context.filename, "test/basictests.c") == 0,
		    "Captured message wasn't correct filename");
	fail_unless(strcmp(captured_context.funcname, "test_nslog_trivial_corked_message") == 0,
		    "Captured message wasn't correct function name");
}
END_TEST

START_TEST (test_nslog_trivial_uncorked_message)
{
	fail_unless(nslog_uncork() == NSLOG_NO_ERROR,
		    "Unable to uncork");
	fail_unless(captured_message_count == 0,
		    "Unusual, we had messages from before uncorking");
	NSLOG(test, INFO, "Hello %s", "world");
	fail_unless(captured_message_count == 1,
		    "Captured message count was wrong");
	fail_unless(captured_render_context == anchor_context_1,
		    "Captured context wasn't passed through");
	fail_unless(strcmp(captured_context.category->name, "test") == 0,
		    "Captured context category wasn't normalised");
	fail_unless(captured_context.category == &__nslog_category_test,
		    "Captured context category wasn't the one we wanted");
	fail_unless(captured_rendered_message_length == 11,
		    "Captured message wasn't correct length");
	fail_unless(strcmp(captured_rendered_message, "Hello world") == 0,
		    "Captured message wasn't correct");
	fail_unless(strcmp(captured_context.filename, "test/basictests.c") == 0,
		    "Captured message wasn't correct filename");
	fail_unless(strcmp(captured_context.funcname, "test_nslog_trivial_uncorked_message") == 0,
		    "Captured message wasn't correct function name");
}
END_TEST

/**** The next set of tests need a fixture set for filters ****/

static nslog_filter_t *cat_test = NULL;
static nslog_filter_t *cat_another = NULL;

static const char *anchor_context_2 = "2";

static void
with_simple_filter_context_setup(void)
{
	captured_render_context = NULL;
	memset(&captured_context, 0, sizeof(captured_context));
	memset(captured_rendered_message, 0, sizeof(captured_rendered_message));
	captured_rendered_message_length = 0;
	captured_message_count = 0;
	fail_unless(nslog_set_render_callback(
			    nslog__test__render_function,
			    (void *)anchor_context_2) == NSLOG_NO_ERROR,
		    "Unable to set up render callback");
	fail_unless(nslog_filter_category_new("test", &cat_test) == NSLOG_NO_ERROR,
		    "Unable to create a category filter for 'test'");
	fail_unless(nslog_filter_category_new("another", &cat_another) == NSLOG_NO_ERROR,
		    "Unable to create a category filter for 'another'");
}

static void
with_simple_filter_context_teardown(void)
{
        /* Nothing to do to tear down */
	fail_unless(nslog_filter_set_active(NULL, NULL) == NSLOG_NO_ERROR,
		    "Unable to clear active filter");
	cat_test = nslog_filter_unref(cat_test);
	cat_another = nslog_filter_unref(cat_another);
}

START_TEST (test_nslog_simple_filter_corked_message)
{
	NSLOG(test, INFO, "Hello world");
	fail_unless(nslog_filter_set_active(cat_test, NULL) == NSLOG_NO_ERROR,
		    "Unable to set active filter to cat:test");
	fail_unless(nslog_uncork() == NSLOG_NO_ERROR,
		    "Unable to uncork");
	fail_unless(captured_message_count == 1,
		    "Captured message count was wrong");
	fail_unless(captured_render_context == anchor_context_2,
		    "Captured context wasn't passed through");
	fail_unless(strcmp(captured_context.category->name, "test") == 0,
		    "Captured context category wasn't normalised");
	fail_unless(captured_context.category == &__nslog_category_test,
		    "Captured context category wasn't the one we wanted");
	fail_unless(captured_rendered_message_length == 11,
		    "Captured message wasn't correct length");
	fail_unless(strcmp(captured_rendered_message, "Hello world") == 0,
		    "Captured message wasn't correct");
	fail_unless(strcmp(captured_context.filename, "test/basictests.c") == 0,
		    "Captured message wasn't correct filename");
	fail_unless(strcmp(captured_context.funcname, "test_nslog_simple_filter_corked_message") == 0,
		    "Captured message wasn't correct function name");
	
}
END_TEST

/**** And the suites are set up here ****/

void
nslog_basic_suite(SRunner *sr)
{
        Suite *s = suite_create("libnslog: Basic tests");
        TCase *tc_basic = tcase_create("Abort checking");
        
#ifndef NDEBUG
	/*
        tcase_add_test_raise_signal(tc_basic,
                                    test_lwc_string_hash_value_aborts,
                                    SIGABRT);
	*/
#endif
        
        suite_add_tcase(s, tc_basic);
        
        tc_basic = tcase_create("Simple log checks, no filters");
        
        tcase_add_checked_fixture(tc_basic, with_simple_context_setup,
                                  with_simple_context_teardown);
        tcase_add_test(tc_basic, test_nslog_trivial_corked_message);
        tcase_add_test(tc_basic, test_nslog_trivial_uncorked_message);
        suite_add_tcase(s, tc_basic);
        
        tc_basic = tcase_create("Simple filter checks");
        
        tcase_add_checked_fixture(tc_basic, with_simple_filter_context_setup,
                                  with_simple_filter_context_teardown);
        tcase_add_test(tc_basic, test_nslog_simple_filter_corked_message);
        suite_add_tcase(s, tc_basic);
        
        srunner_add_suite(sr, s);
}
