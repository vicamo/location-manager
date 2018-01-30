/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details:
 *
 * Copyright (C) 2011 Red Hat, Inc.
 */

#include <config.h>

#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#if defined WITH_SYSTEMD_JOURNAL
#define SD_JOURNAL_SUPPRESS_LOCATION
#include <systemd/sd-journal.h>
#endif

#include "LocationManager.h"
#include "lm-errors-types.h"

#include "lm-log.h"

enum {
    TS_FLAG_NONE = 0,
    TS_FLAG_WALL,
    TS_FLAG_REL
};

static gboolean ts_flags = TS_FLAG_NONE;
static guint32 log_level = LM_LOG_LEVEL_INFO | LM_LOG_LEVEL_WARN | LM_LOG_LEVEL_ERR;
static GTimeVal rel_start = { 0, 0 };
static int logfd = -1;
static gboolean append_log_level_text = TRUE;

static void (*log_backend) (const char *loc,
                            const char *func,
                            int syslog_level,
                            const char *message,
                            size_t length);

typedef struct {
    guint32 num;
    const char *name;
} LogDesc;

static const LogDesc level_descs[] = {
    { LM_LOG_LEVEL_ERR, "ERR" },
    { LM_LOG_LEVEL_WARN | LM_LOG_LEVEL_ERR, "WARN" },
    { LM_LOG_LEVEL_INFO | LM_LOG_LEVEL_WARN | LM_LOG_LEVEL_ERR, "INFO" },
    { LM_LOG_LEVEL_DEBUG | LM_LOG_LEVEL_INFO | LM_LOG_LEVEL_WARN | LM_LOG_LEVEL_ERR, "DEBUG" },
    { 0, NULL }
};

static GString *msgbuf = NULL;
static volatile gsize msgbuf_once = 0;

static int
lm_to_syslog_priority (LMLogLevel level)
{
    switch (level) {
    case LM_LOG_LEVEL_DEBUG:
        return LOG_DEBUG;
    case LM_LOG_LEVEL_WARN:
        return LOG_WARNING;
    case LM_LOG_LEVEL_INFO:
        return LOG_INFO;
    case LM_LOG_LEVEL_ERR:
        return LOG_ERR;
    }
    g_assert_not_reached ();
    return 0;
}

static int
glib_to_syslog_priority (GLogLevelFlags level)
{
    switch (level) {
    case G_LOG_LEVEL_ERROR:
        return LOG_CRIT;
    case G_LOG_LEVEL_CRITICAL:
        return LOG_ERR;
    case G_LOG_LEVEL_WARNING:
        return LOG_WARNING;
    case G_LOG_LEVEL_MESSAGE:
        return LOG_NOTICE;
    case G_LOG_LEVEL_DEBUG:
        return LOG_DEBUG;
    default:
        return LOG_INFO;
    }
}

static const char *
log_level_description (LMLogLevel level)
{
    switch (level) {
    case LM_LOG_LEVEL_DEBUG:
        return "<debug>";
    case LM_LOG_LEVEL_WARN:
        return "<warn> ";
    case LM_LOG_LEVEL_INFO:
        return "<info> ";
    case LM_LOG_LEVEL_ERR:
        return "<error>";
    }
    g_assert_not_reached ();
    return NULL;
}

static void
log_backend_file (const char *loc,
                  const char *func,
                  int syslog_level,
                  const char *message,
                  size_t length)
{
    ssize_t ign;
    ign = write (logfd, message, length);
    if (ign) {} /* whatever; really shut up about unused result */

    fsync (logfd);  /* Make sure output is dumped to disk immediately  */
}

static void
log_backend_syslog (const char *loc,
                    const char *func,
                    int syslog_level,
                    const char *message,
                    size_t length)
{
    syslog (syslog_level, "%s", message);
}

#if defined WITH_SYSTEMD_JOURNAL
static void
log_backend_systemd_journal (const char *loc,
                             const char *func,
                             int syslog_level,
                             const char *message,
                             size_t length)
{
    const char *line;
    size_t file_length;

    if (loc == NULL) {
        sd_journal_send ("MESSAGE=%s", message,
                         "PRIORITY=%d", syslog_level,
                         NULL);
        return;
    }

    line = strstr (loc, ":");
    if (line) {
        file_length = line - loc;
        line++;
    } else {
        /* This is not supposed to happen but we must be prepared for this */
        line = loc;
        file_length = 0;
    }

    sd_journal_send ("MESSAGE=%s", message,
                     "PRIORITY=%d", syslog_level,
                     "CODE_FUNC=%s", func,
                     "CODE_FILE=%.*s", file_length, loc,
                     "CODE_LINE=%s", line,
                     NULL);
}
#endif

void
_lm_log (const char *loc,
         const char *func,
         LMLogLevel level,
         const char *fmt,
         ...)
{
    va_list args;
    GTimeVal tv;

    if (!(log_level & level))
        return;

    if (g_once_init_enter (&msgbuf_once)) {
        msgbuf = g_string_sized_new (512);
        g_once_init_leave (&msgbuf_once, 1);
    } else
        g_string_truncate (msgbuf, 0);

    if (append_log_level_text)
        g_string_append_printf (msgbuf, "%s ", log_level_description (level));

    if (ts_flags == TS_FLAG_WALL) {
        g_get_current_time (&tv);
        g_string_append_printf (msgbuf, "[%09ld.%06ld] ", tv.tv_sec, tv.tv_usec);
    } else if (ts_flags == TS_FLAG_REL) {
        glong secs;
        glong usecs;

        g_get_current_time (&tv);
        secs = tv.tv_sec - rel_start.tv_sec;
        usecs = tv.tv_usec - rel_start.tv_usec;
        if (usecs < 0) {
            secs--;
            usecs += 1000000;
        }

        g_string_append_printf (msgbuf, "[%06ld.%06ld] ", secs, usecs);
    }

#if defined LM_LOG_FUNC_LOC
    g_string_append_printf (msgbuf, "[%s] %s(): ", loc, func);
#endif

    va_start (args, fmt);
    g_string_append_vprintf (msgbuf, fmt, args);
    va_end (args);

    g_string_append_c (msgbuf, '\n');

    log_backend (loc, func, lm_to_syslog_priority (level), msgbuf->str, msgbuf->len);
}

static void
log_handler (const gchar *log_domain,
             GLogLevelFlags level,
             const gchar *message,
             gpointer ignored)
{
    log_backend (NULL, NULL, glib_to_syslog_priority (level), message, strlen (message));
}

gboolean
lm_log_set_level (const char *level, GError **error)
{
    gboolean found = FALSE;
    const LogDesc *diter;

    for (diter = &level_descs[0]; diter->name; diter++) {
        if (!strcasecmp (diter->name, level)) {
            log_level = diter->num;
            found = TRUE;
            break;
        }
    }

    if (!found)
        g_set_error (error, LM_CORE_ERROR, LM_CORE_ERROR_INVALID_ARGS,
                     "Unknown log level '%s'", level);

    return found;
}

gboolean
lm_log_setup (const char *level,
              const char *log_file,
              gboolean log_journal,
              gboolean show_timestamps,
              gboolean rel_timestamps,
              GError **error)
{
    /* levels */
    if (level && strlen (level) && !lm_log_set_level (level, error))
        return FALSE;

    if (show_timestamps)
        ts_flags = TS_FLAG_WALL;
    else if (rel_timestamps)
        ts_flags = TS_FLAG_REL;

    /* Grab start time for relative timestamps */
    g_get_current_time (&rel_start);

#if defined WITH_SYSTEMD_JOURNAL
    if (log_journal) {
        log_backend = log_backend_systemd_journal;
        append_log_level_text = FALSE;
    } else
#endif
    if (log_file == NULL) {
        openlog (G_LOG_DOMAIN, LOG_CONS | LOG_PID | LOG_PERROR, LOG_DAEMON);
        log_backend = log_backend_syslog;
    } else {
        logfd = open (log_file,
                      O_CREAT | O_APPEND | O_WRONLY,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (logfd < 0) {
            g_set_error (error, LM_CORE_ERROR, LM_CORE_ERROR_FAILED,
                         "Couldn't open log file: (%d) %s",
                         errno, strerror (errno));
            return FALSE;
        }
        log_backend = log_backend_file;
    }

    g_log_set_handler (G_LOG_DOMAIN,
                       G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                       log_handler,
                       NULL);

    return TRUE;
}

void
lm_log_shutdown (void)
{
    if (logfd < 0)
        closelog ();
    else
        close (logfd);
}