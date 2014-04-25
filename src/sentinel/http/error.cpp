#include "http.h"

namespace Sentinel { namespace HTTP {

void SendErrorResponse(int fd, int status)
{
  SendLine(fd, "HTTP/1.0 %d %s", status, HTTP::StatusCodeToString(status));
  SendLine(fd, "Content-Type: text/html; charset=UTF-8");
  SendLine(fd, "Server: sentinel-httpd\n");

  SendLine(fd, "<!DOCTYPE>");
  SendLine(fd, "<html>");
  SendLine(fd, "<head>");
  SendLine(fd, "<meta charset=\"utf-8\">");
  SendLine(fd, "<title>%d %s</title>", status, HTTP::StatusCodeToString(status));
  SendLine(fd, "</head>");
  SendLine(fd, "<body>");
  SendLine(fd, "<h1>Error %d %s</h1>", status, HTTP::StatusCodeToString(status));
  SendLine(fd, "<p>%s</p>", HTTP::StatusCodeToString(status));
  SendLine(fd, "<h3>Guru Meditation:</h3>");
  SendLine(fd, "<p>Socket Descriptor: %d</p>", fd);
  SendLine(fd, "<hr>");
  SendLine(fd, "<p>Sentinel HTTPd Web Server</p>");
  SendLine(fd, "</body>");
  SendLine(fd, "</html>\n");
}

}}
