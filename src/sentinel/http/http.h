#ifndef GUARD_SENTINEL_HTTP_HTTP
#define GUARD_SENTINEL_HTTP_HTTP

namespace Sentinel { namespace HTTP {

int send_line(int fd, const char *buff, ...);
const char *StatusCodeToString(int status);

}}

#endif
