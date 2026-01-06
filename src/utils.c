#include "../include/project.h"
#include <stdarg.h>

/*=========
* METHODS: *
==========*/

void log_msg(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
}

void strupper(char *buffer) {
  if (buffer) {
    for (; *buffer; ++buffer) {
      *buffer = (char)toupper((unsigned char)*buffer);
    }
  }
}
