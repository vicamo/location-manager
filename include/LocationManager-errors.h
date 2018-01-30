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
 * Copyright (C) 2011 Google, Inc.
 */

#ifndef _LOCATIONMANAGER_ERRORS_H_
#define _LOCATIONMANAGER_ERRORS_H_

#if !defined (__LOCATION_MANAGER_H_INSIDE__)
#error "Only <LocationManager.h> can be included directly."
#endif

#include <LocationManager-names.h>

/**
 * SECTION:lm-errors
 * @short_description: Common errors in the API.
 *
 * This section defines errors that may be reported when using methods from the
 * LocationManager interface.
 **/

#define LM_CORE_ERROR_DBUS_PREFIX             LM_DBUS_ERROR_PREFIX ".Core"

/**
 * LMCoreError:
 * @LM_CORE_ERROR_FAILED: Operation failed.
 * @LM_CORE_ERROR_CANCELLED: Operation was cancelled.
 * @LM_CORE_ERROR_ABORTED: Operation was aborted.
 * @LM_CORE_ERROR_UNSUPPORTED: Operation is not supported.
 * @LM_CORE_ERROR_NO_PLUGINS: Cannot operate without valid plugins.
 * @LM_CORE_ERROR_UNAUTHORIZED: Authorization is required to perform the operation.
 * @LM_CORE_ERROR_INVALID_ARGS: Invalid arguments given.
 * @LM_CORE_ERROR_IN_PROGRESS: Operation is already in progress.
 * @LM_CORE_ERROR_WRONG_STATE: Operation cannot be executed in the current state.
 * @LM_CORE_ERROR_CONNECTED: Operation cannot be executed while being connected.
 * @LM_CORE_ERROR_TOO_MANY: Too many items.
 * @LM_CORE_ERROR_NOT_FOUND: Item not found.
 * @LM_CORE_ERROR_RETRY: Operation cannot yet be performed, retry later.
 * @LM_CORE_ERROR_EXISTS: Item already exists.
 *
 * Common errors that may be reported by LocationManager.
 */
typedef enum { /*< underscore_name=lm_core_error >*/
    LM_CORE_ERROR_FAILED       = 0,  /*< nick=Failed       >*/
    LM_CORE_ERROR_CANCELLED    = 1,  /*< nick=Cancelled    >*/
    LM_CORE_ERROR_ABORTED      = 2,  /*< nick=Aborted      >*/
    LM_CORE_ERROR_UNSUPPORTED  = 3,  /*< nick=Unsupported  >*/
    LM_CORE_ERROR_NO_PLUGINS   = 4,  /*< nick=NoPlugins    >*/
    LM_CORE_ERROR_UNAUTHORIZED = 5,  /*< nick=Unauthorized >*/
    LM_CORE_ERROR_INVALID_ARGS = 6,  /*< nick=InvalidArgs  >*/
    LM_CORE_ERROR_IN_PROGRESS  = 7,  /*< nick=InProgress   >*/
    LM_CORE_ERROR_WRONG_STATE  = 8,  /*< nick=WrongState   >*/
    LM_CORE_ERROR_CONNECTED    = 9,  /*< nick=Connected    >*/
    LM_CORE_ERROR_TOO_MANY     = 10, /*< nick=TooMany      >*/
    LM_CORE_ERROR_NOT_FOUND    = 11, /*< nick=NotFound     >*/
    LM_CORE_ERROR_RETRY        = 12, /*< nick=Retry        >*/
    LM_CORE_ERROR_EXISTS       = 13, /*< nick=Exists       >*/
} LMCoreError;

#endif /*  _LOCATIONMANAGER_ERRORS_H_ */
