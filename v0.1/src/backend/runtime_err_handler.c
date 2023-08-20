#include "runtime_err_handler.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void runtime_error(const char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);

	exit(EXIT_FAILURE);
}

