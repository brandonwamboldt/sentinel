#ifndef GUARD_SENTINEL_HTTP_SERVER
#define GUARD_SENTINEL_HTTP_SERVER

#include "error.h"
#include "http.h"
#include "../config.h"

namespace Sentinel { namespace HTTP {

int BuildSocket(const char *listen_addr, const char *port);
int MakeSocketNonBlocking(int socket_fd);
void Start(struct ::Sentinel::Config *config);

}}

#endif
