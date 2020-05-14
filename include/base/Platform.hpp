#pragma once

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
#include <fcntl.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>