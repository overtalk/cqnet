#pragma once

#if defined _MSC_VER || defined __MINGW32__
#define CQNET_PLATFORM_WINDOWS
#elif defined __APPLE_CC__ || defined __APPLE__
#define CQNET_PLATFORM_DARWIN
#else
#define CQNET_PLATFORM_LINUX
#endif

#include <atomic>
#include <unordered_map>
#include <atomic>
#include <chrono>
#include <mutex>
#include <memory>
#include <vector>
#include <iostream>
#include <functional>
#include <condition_variable>
#include <cstring>

// macos plarform
#ifdef CQNET_PLATFORM_DARWIN
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef CQNET_PLATFORM_WINDOWS
#define CQNET_SOCKET_ERROR SOCKET_ERROR
#define CQNET_INVALID_SOCKET INVALID_SOCKET

#elif defined CQNET_PLATFORM_LINUX || defined CQNET_PLATFORM_DARWIN
#define CQNET_SOCKET_ERROR (-1)
#define CQNET_INVALID_SOCKET (-1)
#endif
