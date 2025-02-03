#pragma once
#include <cstdlib>
#include <cassert>
#include <exception>
#define RFCT_ASSERT(x) if (!x) { RFCT_ERROR("assert ({}) failed.", #x); std::abort(); };