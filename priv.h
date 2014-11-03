/**
 * This file is part of libjrpc library code.
 *
 * Copyright (C) 2014 Roman Yeryomin <roman@advem.lv>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENCE.txt file for more details.
 */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#include "libjrpc.h"

extern void jrpc_add_version( fmt_t *root, fmt_t *id );
extern ssize_t jrpc_send_json( ipsc_t *ipsc, fmt_t *root );
extern ssize_t jrpc_recv_json( ipsc_t *ipsc, fmt_t *p );
extern ssize_t jrpc_process( ipsc_t *ipsc );
