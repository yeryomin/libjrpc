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

void *jrpc_server( void *args )
{
	if ( !args )
		return NULL;

	int epfd = -1;
	jrpc_t *jrpc = (jrpc_t *)args;
	ipsc_t *ipsc = ipsc_listen( jrpc->conn.socktype, jrpc->conn.proto,
				    jrpc->conn.host, jrpc->conn.port,
				    jrpc->maxqueue );
	if ( !ipsc ) {
		syslog( LOG_WARNING,"jrpc_server(listen): %m" );
		return NULL;
	}

	if ( jrpc->conn.flags & JRPC_FLAG_TLS ) {
		if ( ipsc_tls_init( ipsc, jrpc->conn.tlscert,
					  jrpc->conn.tlskey,
					  jrpc->conn.tlsca,
					  jrpc->conn.tlsdh ) )
		{
			ipsc_close( ipsc );
			syslog( LOG_WARNING,"jrpc_server(tls_init): %m" );
			return NULL;
		}
	}

	ipsc->cb_args = args;

	/* joinable thread callback helper */
	if ( jrpc->connreg )
		jrpc->connreg( ipsc );

	epfd = ipsc_epoll_init( ipsc );
	if ( epfd < 0 ) {
		ipsc_close(ipsc);
		syslog( LOG_WARNING, "jrpc_server(create): %m (%i)", epfd );
		return NULL;
	}

	while (1) {
		/* do we actually need to check for error here? */
		ipsc_epoll_wait( ipsc, epfd, &jrpc_process );
		usleep( jrpc->epsleep );
	}

	/* should never get here */
	ipsc_close( ipsc );
	return NULL;
}

ssize_t jrpc_request( jrpc_req_t *req )
{
	if ( !req || !req->method )
		return JRPC_ERR_GENERIC;

	ssize_t sb = 0;
	ssize_t rb = 0;
	json_object *p = NULL;
	json_object *tmp = NULL;
	json_object *root = NULL;
	ipsc_t *ipsc = NULL;

	ipsc = ipsc_connect( req->conn.socktype, req->conn.proto,
			     req->conn.host, req->conn.port );
	if ( !ipsc ) {
		sb = JRPC_ERR_GENERIC;
		goto exit;
	}

	if ( req->conn.flags & JRPC_FLAG_TLS ) {
		if ( ipsc_tls_init( ipsc, req->conn.tlscert,
					  req->conn.tlskey,
					  req->conn.tlsca,
					  req->conn.tlsdh ) )
		{
			sb = JRPC_ERR_TLSINIT;
			goto exit;
		}
		if ( ipsc_connect_tls( ipsc ) ) {
			sb = JRPC_ERR_TLSCONN;
			goto exit;
		}
	}

	root = json_object_new_object();

#ifndef JRPC_LITE
	jrpc_add_version( root, req->id );
#endif

	json_object_object_add( root, JRPC_KEY_METHOD,
					json_object_new_string( req->method ) );
	if ( req->params )
		json_object_object_add( root, JRPC_KEY_PARAMS, req->params );

	/* send request */
	ipsc->cb_args = (void *)req;
	sb = jrpc_send_json( ipsc, root );
	if ( sb < 2 ) {
		sb = JRPC_ERR_SEND;
		goto exit;
	}

	/* get reply */
	rb = jrpc_recv_json( ipsc, &p );
	if ( rb < 2 ) {
		syslog( LOG_WARNING, "jrpc_process(recv): %m (%li)", rb );
		sb = JRPC_ERR_RECV;
		goto exit;
	}

	sb = JRPC_SUCCESS;
	tmp = json_object_object_get( p, JRPC_KEY_RESULT );
	if ( !tmp ) {
		sb = JRPC_ERR_NORESULT;
		tmp = json_object_object_get( p, JRPC_KEY_ERROR );
		if ( tmp )
			sb = JRPC_ERR_USER;
	}

	if ( tmp ) {
		json_object_deep_copy( tmp, req->res, NULL );
		json_object_put( tmp );
	}

exit:
	ipsc_close( ipsc );
	json_object_put( p );
	json_object_put( root );

	return sb;
}

ssize_t jrpc_send( ipsc_t *ipsc, json_object *obj, json_object *id, int type )
{
	if ( !ipsc || !obj )
		return JRPC_ERR_GENERIC;

	ssize_t sb = 0;
	char msg_type[8]; /* either "error" or "result" */
	json_object *root = NULL;

	switch (type) {
	case JRPC_REPLY_TYPE_ERROR:
		snprintf( msg_type, sizeof msg_type, "%s", JRPC_KEY_ERROR );
		break;
	case JRPC_REPLY_TYPE_RESULT:
		snprintf( msg_type, sizeof msg_type, "%s", JRPC_KEY_RESULT );
		break;
	default:
		sb = JRPC_ERR_UNKNOWN_REPLY_TYPE;
		goto exit;
	}

	root = json_object_new_object();

#ifndef JRPC_LITE
	jrpc_add_version( root, id );
#endif

	if ( json_object_object_add( root, msg_type, obj ) ) {
		sb = JRPC_ERR_GENERIC;
		goto exit;
	}

	sb = jrpc_send_json( ipsc, root );

exit:
	json_object_put( root );

	return sb;
}

ssize_t jrpc_error( ipsc_t *ipsc, json_object *id, int code, const char *message )
{
	json_object *err = json_object_new_object();
	json_object_object_add( err, JRPC_KEY_ERROR_CODE,
					json_object_new_int( code ) );
	json_object_object_add( err, JRPC_KEY_ERROR_TEXT,
					json_object_new_string( message ) );

	return jrpc_send( ipsc, err, id, JRPC_REPLY_TYPE_ERROR );
}

ssize_t jrpc_invalid_params( ipsc_t *ipsc, json_object *id )
{
	return jrpc_error( ipsc, id,
			   JRPC_CODE_INVALID_PARAMS,
			   JRPC_ERR_INVALID_PARAMS );
}

ssize_t jrpc_internal_error( ipsc_t *ipsc, json_object *id )
{
	return jrpc_error( ipsc, id,
			   JRPC_CODE_INTERNAL_ERROR,
			   JRPC_ERR_INTERNAL_ERROR );
}

ssize_t jrpc_not_implemented( ipsc_t *ipsc, json_object *id )
{
	return jrpc_error( ipsc, id,
			   JRPC_CODE_NOT_IMPLEMENTED,
			   JRPC_ERR_NOT_IMPLEMENTED );
}
