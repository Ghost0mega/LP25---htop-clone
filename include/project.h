#ifndef PROJECT_H
#define PROJECT_H

/* Central project header: includes commonly used headers and project headers.
 */

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "machine.h"
#include "manager.h"
#include "network.h"
#include "parameters.h"
#include "process.h"
#include "ui.h"
#include "utils.h"

#endif /* PROJECT_H */
