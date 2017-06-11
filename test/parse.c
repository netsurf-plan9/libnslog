#include "nslog/nslog.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: parse 'filtertext'\n");
		return 1;
	}
	nslog_filter_t *filt;
	nslog_error err;

	err = nslog_filter_from_text(argv[1], &filt);
	if (err != NSLOG_NO_ERROR) {
		fprintf(stderr, "Unable to parse.\n");
		return 2;
	}
	char *ct = nslog_filter_sprintf(filt);
	filt = nslog_filter_unref(filt);
	printf("%s\n", ct);
	free(ct);
	return 0;
}
