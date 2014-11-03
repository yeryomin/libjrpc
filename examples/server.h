#include <libjrpc.h>

#define RPC_METHOD_SYSTEM_DESCRIBE	"system.describe"
#define RPC_METHOD_RETURN_PARAMS	"return.params"
#define RPC_METHOD_RETURN_PARAMS2	"return.params2"
#define RPC_METHOD_NOT_IMPLEMENTED	"not.implemented"

/* implemented handlers */
ssize_t check_status( ipsc_t *ipsc, void *params, void *reqid );
ssize_t rpc_method_system_describe( ipsc_t *ipsc, void *params, void *reqid );
ssize_t rpc_method_return_params( ipsc_t *ipsc, void *params, void *reqid );
ssize_t rpc_method_return_params2( ipsc_t *ipsc, void *params, void *reqid );

/* register all methods and their handlers */
jrpc_method_t rpc_methods[] = {
	{
		.name     = RPC_METHOD_SYSTEM_DESCRIBE,
		.params   = JRPC_CB_NO_PARAMS,
		.handlers = JRPC_CBS {
				&check_status,
				&rpc_method_system_describe,
				NULL
			    }
	},
	{
		.name     = RPC_METHOD_RETURN_PARAMS,
		.params   = JRPC_CB_HAS_PARAMS,
		.handlers = JRPC_CBS{ &rpc_method_return_params, NULL }
	},
	{
		.name     = RPC_METHOD_RETURN_PARAMS2,
		.params   = JRPC_CB_OPT_PARAMS,
		.handlers = JRPC_CBS{ &rpc_method_return_params2, NULL }
	},
	{
		.name     = RPC_METHOD_NOT_IMPLEMENTED,
		.params   = JRPC_CB_OPT_PARAMS,
		.handlers = NULL
	},
	JRPC_METHODS_END
};
