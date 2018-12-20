#pragma once
#ifndef ARCH_WIN
#define ARCH_WIN 1
#endif

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#if ARCH_WIN
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#endif
#include <time.h>
#ifdef WIN32
#   include <windows.h>
#else
#   include <sys/time.h>
#endif

#ifdef WIN32
extern int gettimeofday(struct timeval *tp, void *tzp);

#endif