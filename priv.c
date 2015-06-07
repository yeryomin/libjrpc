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
#include "priv.h"

/* TODO */
/*
static ssize_t jrpc_send_signed( ipsc_t *sock, void *ctx, char *buf, int size )
{
	return -1;
}
static ssize_t jrpc_recv_signed( ipsc_t *sock, void *ctx,
				 char *buf, int size, int timeout )
{
	return -1;
}
static ssize_t jrpc_send_encr( ipsc_t *sock, void *ctx, char *buf, int size )
{
	return -1;
}
static ssize_t jrpc_recv_encr( ipsc_t *sock, void *ctx,
				char *buf, int size, int timeout )
{
	return -1;
}
*/
static ssize_t jrpc_send_bin( ipsc_t *sock, void *ctx, char *buf, int size )
{
	return -1;
}
static ssize_t jrpc_recv_bin( ipsc_t *sock, void *ctx,
				char *buf, int size, int timeout )
{
	return -1;
}

static ssize_t jrpc_parse_error( ipsc_t *ipsc, fmt_t *id )
{
	return jrpc_error( ipsc, id,
			   JRPC_CODE_PARSE_ERROR,
			   JRPC_ERR_PARSE_ERROR );
}

static ssize_t jrpc_invalid_request( ipsc_t *ipsc, fmt_t *id )
{
	return jrpc_error( ipsc, id,
			   JRPC_CODE_INVALID_REQUEST,
			   JRPC_ERR_INVALID_REQUEST );
}

static ssize_t jrpc_method_not_found( ipsc_t *ipsc, fmt_t *id )
{
	return jrpc_error( ipsc, id,
			   JRPC_CODE_METHOD_NOT_FOUND,
			   JRPC_ERR_METHOD_NOT_FOUND );
}

static int jrpc_check_version( fmt_t *root )
{
	char *version = NULL;

	if ( fmt_get_string( root, NULL, JRPC_KEY_JSONRPC, &version ) )
		return -1;

	if ( strncmp( version, JRPC_KEY_VERSION,
		      strlen(JRPC_KEY_VERSION) + 1 ) )
	{
		free( version );
		return -1;
	}

	free( version );
	return 0;
}

void jrpc_add_version( fmt_t *root, fmt_t *id )
{
	fmt_set_new( root, JRPC_KEY_JSONRPC, fmt_string( JRPC_KEY_VERSION ) );
	if ( id )
		fmt_set_new( root, JRPC_KEY_ID, id );
	else
		fmt_set_new( root, JRPC_KEY_ID, fmt_null() );
}

ssize_t jrpc_send_json( ipsc_t *ipsc, fmt_t *root )
{
	char *buf = NULL;
	size_t buflen = fmt_dump_len( root );
	ssize_t sb = 0;
	int flags;
	jrpc_runtime_t rt;

	if ( ipsc->flags & IPSC_FLAG_SERVER ) {
		flags = ((jrpc_t *)ipsc->cb_args)->conn.flags;
		rt = ((jrpc_t *)ipsc->cb_args)->rt;
	} else {
		flags = ((jrpc_req_t *)ipsc->cb_args)->conn.flags;
		rt = ((jrpc_req_t *)ipsc->cb_args)->rt;
	}

	if ( fmt_dump_string( root, NULL, &buf ) ) {
		sb = JRPC_ERR_GENERIC;
		goto exit;
	}

	if ( flags & JRPC_FLAG_BINARIZE )
		sb = jrpc_send_bin( ipsc, rt.bin_ctx, buf, buflen );
	else
		sb = ipsc_send( ipsc, buf, buflen );

exit:
	free( buf );
	return sb;
}

ssize_t jrpc_recv_json( ipsc_t *ipsc, fmt_t *p )
{
	char *buf = NULL;
	size_t buflen = 0;
	ssize_t rb = 0;
	ssize_t trb = 0;
	int flags;
	int socktype;
	int timeout;
	jrpc_runtime_t rt;

	if ( ipsc->flags & IPSC_FLAG_SERVER ) {
		flags = ((jrpc_t *)ipsc->cb_args)->conn.flags;
		socktype = ((jrpc_t *)ipsc->cb_args)->conn.socktype;
		timeout = ((jrpc_t *)ipsc->cb_args)->conn.timeout;
		rt = ((jrpc_t *)ipsc->cb_args)->rt;
	} else {
		flags = ((jrpc_req_t *)ipsc->cb_args)->conn.flags;
		socktype = ((jrpc_req_t *)ipsc->cb_args)->conn.socktype;
		timeout = ((jrpc_req_t *)ipsc->cb_args)->conn.timeout;
		rt = ((jrpc_req_t *)ipsc->cb_args)->rt;
	}

	switch (socktype) {
		case SOCK_DGRAM:
			buflen = JRPC_DEFAULT_RCVBUF_DGRAM;
			break;
		case SOCK_STREAM:
		default:
			buflen = JRPC_DEFAULT_RCVBUF_STREAM;
			break;
	}

	/* TODO: add hard memory limit */
	buf = (char *)malloc( buflen );
	if ( flags & JRPC_FLAG_BINARIZE )
		rb = jrpc_recv_bin( ipsc, rt.bin_ctx,
				    buf, buflen - 1, timeout );
	else
		while ( (trb = ipsc_recv( ipsc, buf + rb,
					  buflen - rb - 1, timeout )) > 0 )
		{
			/* lower the timeout after first data received */
			if ( timeout > 0 )
				timeout = 10;
			rb += trb;
			if ( rb > buflen - 2 ) {
				buflen += buflen;
				buf = (char *)realloc( buf, buflen );
			}
		}

	if ( rb < 2 )
		rb = 0;

	if ( fmt_load_string( buf, (size_t)rb, p ) )
		rb = -1;

	free( buf );
	return rb;
}

ssize_t jrpc_process( ipsc_t *ipsc )
{
	int i, idx;
	size_t rb;
	size_t sb = 0;
	fmt_t p = FMT_NULL;
	fmt_t tmp = FMT_NULL;
	fmt_t tmpid = FMT_NULL;
	fmt_t *params = NULL;
	fmt_t *id = NULL;
	jrpc_cb_t cb;
	jrpc_t *jrpc = (jrpc_t *)ipsc->cb_args;
	char *method = NULL;

	ipsc->flags |= IPSC_FLAG_SERVER;

	rb = jrpc_recv_json( ipsc, &p );
	if ( rb < 2 ) {
		syslog( LOG_WARNING, "jrpc_process(recv): %m (%li)", rb );
		sb = jrpc_parse_error( ipsc, NULL );
		goto exit;
	}

	if ( !fmt_get( &p, NULL, JRPC_KEY_ID, &tmpid ) )
		id = &tmpid;

#ifndef JRPC_LITE
	/* check version string if we use standart fields */
	if ( jrpc_check_version( &p ) ) {
		sb = jrpc_invalid_request( ipsc, id );
		goto exit;
	}
#endif

	/* send error back if 'method' key is not found */
	if ( fmt_get_string( &p, NULL, JRPC_KEY_METHOD, &method ) ) {
		sb = jrpc_invalid_request( ipsc, id );
		goto exit;
	}

	for ( i = 0; jrpc->methods[i].name; i++ ) {
		if ( strncmp( method, jrpc->methods[i].name,
			      strlen(jrpc->methods[i].name) + 1 ) )
			continue;

		switch ( jrpc->methods[i].params ) {
		case JRPC_CB_HAS_PARAMS:
			if ( fmt_get( &p, NULL, JRPC_KEY_PARAMS, &tmp ) ) {
				sb = jrpc_invalid_params( ipsc, id );
				goto exit;
			}
			params = &tmp;
			break;
		case JRPC_CB_OPT_PARAMS:
			if ( !fmt_get( &p, NULL, JRPC_KEY_PARAMS, &tmp ) )
				params = &tmp;
			break;
		case JRPC_CB_NO_PARAMS:
		default:
			break;
		}

		if ( !jrpc->methods[i].handlers ) {
			sb = jrpc_not_implemented( ipsc, id );
			goto exit;
		}

		if ( !jrpc->methods[i].handlers[0] ) {
			sb = jrpc_not_implemented( ipsc, id );
			goto exit;
		}

		for( idx = 0; jrpc->methods[i].handlers[idx]; idx++ ) {
			cb = jrpc->methods[i].handlers[idx];
			sb = cb( ipsc, params, id );
			if ( sb == 0 )
				break;
			if ( sb < 0 ) {
				sb = jrpc_internal_error( ipsc, id );
				break;
			}
		}

		goto exit;
	}

	/* no method defined, send standard error */
	sb = jrpc_method_not_found( ipsc, id );

exit:
	if ( sb < 0 )
		syslog( LOG_WARNING, "jrpc_process(recv|send): %m (%li)", sb );

	free( method );
	fmt_free( &p );
	fmt_free( id );
	fmt_free( params );

	return sb;
}
