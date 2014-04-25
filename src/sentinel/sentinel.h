#ifndef GUARD_SENTINEL
#define GUARD_SENTINEL

#include "logger/logger.h"

namespace Sentinel {
  void Bail();
  void Bail(const char *message);
}

#endif
