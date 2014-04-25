#ifndef GUARD_SENTINEL_HTTP_ERROR
#define GUARD_SENTINEL_HTTP_ERROR

namespace Sentinel { namespace HTTP {

void SendErrorResponse(int fd, int status);

}}

#endif
