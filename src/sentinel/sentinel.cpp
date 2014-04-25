#include "sentinel.h"

namespace Sentinel {

void Bail()
{
  exit(1);
}

void Bail(const char *message)
{
  Logger::Log("ERROR", message);
  Bail();
}

}
