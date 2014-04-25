#include "logger.h"

namespace Sentinel { namespace Logger {

int Log(const char *level, const char *buff, ...)
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
    return -1;
  }

  // Actually format the string now
  va_start(arglist, buff);
  vsnprintf(line, buffer_size, buff, arglist);
  va_end(arglist);

  // Add a trailing newline and null terminator
  line[buffer_size - 2] = '\n';
  line[buffer_size - 1] = '\0';

  std::cout << "[" << level << "] " << line;
  fflush(stdout);

  free(line);

  return 0;
}

}}
