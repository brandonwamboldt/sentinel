#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sentinel/config.h"
#include "sentinel/sentinel.cpp"
#include "sentinel/logger/logger.cpp"
#include "sentinel/http/server.cpp"
#include "sentinel/http/http.cpp"
#include "sentinel/http/error.cpp"

void signal_handler_terminate(int signal)
{
    Sentinel::Logger::Log("INFO", "Caught signal %d", signal);
    Sentinel::Bail();
}

void signal_handler_noop(int signal)
{
    // No-op
}

int main(int argc, const char* argv[])
{
    // Setup a signal handler so we can cleanup after ourselves
    struct sigaction signal_handler;
    struct sigaction signal_handler2;

    signal_handler.sa_handler = signal_handler_terminate;
    sigemptyset(&signal_handler.sa_mask);
    signal_handler.sa_flags = 0;

    signal_handler2.sa_handler = signal_handler_noop;
    sigemptyset(&signal_handler2.sa_mask);
    signal_handler2.sa_flags = 0;

    sigaction(SIGQUIT, &signal_handler, NULL);
    sigaction(SIGHUP, &signal_handler, NULL);
    sigaction(SIGINT, &signal_handler, NULL);
    sigaction(SIGTERM, &signal_handler, NULL);
    sigaction(SIGABRT, &signal_handler, NULL);
    sigaction(SIGPIPE, &signal_handler2, NULL);

    // Setup config
    Sentinel::Config config;
    config.MaxEvents = 64;

    Sentinel::HTTP::Server::Start(&config);

    return EXIT_SUCCESS;
}
