/**
 * @file logger.c
 *
 *  Created on: 2009-04-21
 *      Author: Jean-Lou Dupont
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "logger.h"

static const char *defaultId = "ECHO";
char *_LOGGER_IDENTITY = NULL;

void loggerSetIdentity(const char *ident) {
	_LOGGER_IDENTITY = (char *) ident;
}

/**
 * Crude logging function
 */
void doLog(int priority, const char *message, ...) {

	if (NULL==_LOGGER_IDENTITY)
		openlog(defaultId, LOG_PID, LOG_LOCAL1);
	else
		openlog(_LOGGER_IDENTITY, LOG_PID, LOG_LOCAL1);

	char buffer[2048];
	va_list ap;

	va_start(ap, message);
		vsnprintf (buffer, sizeof(buffer), message, ap);
	va_end(ap);

	syslog(priority, buffer, NULL);

	closelog();

}

void doLogEx(int priority, const char *message, ...) {

	va_list ap;

	if (NULL==_LOGGER_IDENTITY)
		openlog(defaultId, LOG_PID, LOG_LOCAL1);
	else
		openlog(_LOGGER_IDENTITY, LOG_PID, LOG_LOCAL1);

	char buffer[2048];

	va_start(ap, message);
		vfprintf (stderr, message, ap);
		vsnprintf (buffer, sizeof(buffer), message, ap);
	va_end(ap);

	syslog(priority, buffer, NULL);

	closelog();

}//
