#include "server.h"

namespace Sentinel {  namespace HTTP { namespace Server {

using namespace Sentinel;

int BuildSocket(const char *listen_addr, const char *port)
{
  struct addrinfo hints, *result, *result_ptr;
  int socket_fd, status, yes = 1;

  // Get an addrinfo struct for the interface we are going to bind to
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;      /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = SOCK_STREAM;  /* We want a TCP socket */

  if (listen_addr == NULL) {
    hints.ai_flags = AI_PASSIVE;  /* All interfaces */
  }

  status = getaddrinfo(listen_addr, port, &hints, &result);

  if (status != 0) {
    Logger::Log("ERROR", gai_strerror(status));
    return -1;
  }

  // getaddrinfo can return multiple interfaces, find one to bind to
  for (result_ptr = result; result_ptr != NULL; result_ptr = result_ptr->ai_next) {
    socket_fd = socket(result_ptr->ai_family, result_ptr->ai_socktype, result_ptr->ai_protocol);

    if (socket_fd < 0) {
      continue;
    }

    // Lose the pesky "Address already in use" error message
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
      Logger::Log("ERROR", "Could not set SO_REUSEADDR on socket");
      return -1;
    }

    // Try to bind and if we're successful, jump out of the loop
    if (bind(socket_fd, result_ptr->ai_addr, result_ptr->ai_addrlen) == 0) {
      break;
    }

    close(socket_fd);
  }

  if (result_ptr == NULL) {
    Logger::Log("ERROR", "Could not bind to an interface");
    return -1;
  }

  freeaddrinfo(result);

  return socket_fd;
}

int MakeSocketNonBlocking(int socket_fd)
{
  int flags, status;

  flags = fcntl(socket_fd, F_GETFL, 0);

  if (flags < 0) {
    Logger::Log("ERROR", "fcntl get failed");
    return -1;
  }

  flags |= O_NONBLOCK;

  status = fcntl(socket_fd, F_SETFL, flags);

  if (status < 0) {
    Logger::Log("ERROR", "fcntl set failed");
    return -1;
  }

  return 0;
}

void Start(struct Config *config)
{
  // Other vars
  int listen_fd, status, epoll_fd;
  struct epoll_event event, *events;

  listen_fd = BuildSocket("127.0.0.1", "3000");

  if (listen_fd < 0) {
    Bail();
  }

  if (MakeSocketNonBlocking(listen_fd) < 0) {
    Bail();
  }

  if (listen(listen_fd, SOMAXCONN) < 0) {
    Bail("Unable to listen on socket");
  }

  epoll_fd = epoll_create1(0);

  if (epoll_fd < 0) {
    Bail("Unable to create epoll descriptor");
  }

  event.data.fd = listen_fd;
  event.events = EPOLLIN | EPOLLET;

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) < 0) {
    Bail("epoll_ctl failed");
  }

  // Buffer where events are returned
  events = (epoll_event *) calloc(config->MaxEvents, sizeof(event));

  Logger::Log("INFO", "Starting main event loop");

  int conn_count = 0;

  // Event loop
  while (1) {
    int n, i;

    n = epoll_wait(epoll_fd, events, config->MaxEvents, -1);

    for (i = 0; i < n; i++) {
      if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
        // An error has occured on this fd, or the socket is not ready
        // for reading
        Logger::Log("ERROR", "epoll error, closing file descriptor for event");
        close(events[i].data.fd);
      } else if (listen_fd == events[i].data.fd) {
        // We have a notification on the listening socket, which means
        // incoming connections
        while (1) {
          struct sockaddr client_addr;
          socklen_t client_length;
          int client_fd, status;
          char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

          client_length = sizeof(client_addr);
          client_fd = accept(listen_fd, &client_addr, &client_length);

          if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
              // We have processed all incoming connections
              break;
            } else {
              Logger::Log("ERROR", "Accept() returned an error");
              break;
            }
          }

          status = getnameinfo(
            &client_addr, client_length,
            hbuf, sizeof(hbuf),
            sbuf, sizeof(sbuf),
            NI_NUMERICHOST | NI_NUMERICSERV
          );

          if (status == 0) {
            conn_count++;
            Logger::Log("INFO", "Accepted connection %d on descriptor %d (host=%s, port=%s)", conn_count, client_fd, hbuf, sbuf);
          }

          // Make the incoming socket non-blocking and add it to the
          // list of fds to monitor
          if (MakeSocketNonBlocking(client_fd) < 0) {
            Bail("Could not make socket non blocking for client");
          }

          event.data.fd = client_fd;
          event.events = EPOLLIN | EPOLLET;

          if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
            Bail("epoll_ctl failed for client");
          }
        }

        continue;
      } else {
        // We have data on the fd waiting to be read. Read and display
        // it. We must read whatever data is available completely, as we
        // are running in edge-triggered mode and won't get a
        // notification again for the same data.
        int done = 0;

        while (1) {
          ssize_t count;
          char buffer[512];

          count = read(events[i].data.fd, buffer, sizeof(buffer));

          if (count < 0) {
            // If errno is EAGAIN, we've read all the data so we
            // should go back to the main loop
            if (errno != EAGAIN) {
              Logger::Log("ERROR", "Unable to read from client");
              done = 1;
            }

            break;
          } else if (count == 0) {
            // We've reached the EOF
            done = 1;
            break;
          }
        }

        HTTP::SendErrorResponse(events[i].data.fd, 404);
        Logger::Log("INFO", "Closed connection on descriptor %d", events[i].data.fd);
        close(events[i].data.fd);
      }
    }
  }

  Logger::Log("INFO", "ALL DONE");

  free(events);
  close(listen_fd);
}

}}}
