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

#ifndef LM_LOG_H
#define LM_LOG_H

#include <glib.h>

/* Log levels */
typedef enum {
    LM_LOG_LEVEL_ERR   = 0x00000001,
    LM_LOG_LEVEL_WARN  = 0x00000002,
    LM_LOG_LEVEL_INFO  = 0x00000004,
    LM_LOG_LEVEL_DEBUG = 0x00000008
} LMLogLevel;

#define lm_err(...) \
    _lm_log (G_STRLOC, G_STRFUNC, LM_LOG_LEVEL_ERR, ## __VA_ARGS__ )

#define lm_warn(...) \
    _lm_log (G_STRLOC, G_STRFUNC, LM_LOG_LEVEL_WARN, ## __VA_ARGS__ )

#define lm_info(...) \
    _lm_log (G_STRLOC, G_STRFUNC, LM_LOG_LEVEL_INFO, ## __VA_ARGS__ )

#define lm_dbg(...) \
    _lm_log (G_STRLOC, G_STRFUNC, LM_LOG_LEVEL_DEBUG, ## __VA_ARGS__ )

#define lm_log(level, ...) \
    _lm_log (G_STRLOC, G_STRFUNC, level, ## __VA_ARGS__ )

void _lm_log (const char *loc,
              const char *func,
              LMLogLevel level,
              const char *fmt,
              ...)  __attribute__((__format__ (__printf__, 4, 5)));

gboolean lm_log_set_level (const char *level, GError **error);

gboolean lm_log_setup (const char *level,
                       const char *log_file,
                       gboolean log_journal,
                       gboolean show_ts,
                       gboolean rel_ts,
                       GError **error);

void lm_log_shutdown (void);

#endif  /* LM_LOG_H */
