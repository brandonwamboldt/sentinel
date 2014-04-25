#include "http.h"

namespace Sentinel { namespace HTTP {

int SendLine(int fd, const char *buff, ...)
{
  va_list arglist;

  // This will do nothing except return the length of the formatted string
  // which we add 2 to for the newline character and null character
  va_start(arglist, buff);
  int buffer_size = vsnprintf(NULL, 0, buff, arglist) + 2;
  va_end(arglist);

  // Allocate memory needed for the formatted string
  char *line = (char *) malloc((buffer_size) * sizeof(char));

  if (line == NULL) {
    ::Sentinel::Bail("malloc failed in SendLine");
  }

  // Actually format the string now
  va_start(arglist, buff);
  vsnprintf(line, buffer_size, buff, arglist);
  va_end(arglist);

  // Add a trailing newline and null terminator
  line[buffer_size - 2] = '\n';
  line[buffer_size - 1] = '\0';

  int s = write(fd, line, buffer_size);

  free(line);

  return s;
}

const char *StatusCodeToString(int status)
{
  switch (status) {
    case 200: return "OK";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "403 Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
  }
}

}}
