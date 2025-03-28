#pragma once
#include <cstdlib>
#include <cassert>
#include <exception>
#define RFCT_ASSERT(x) if (!((bool)(x))) { RFCT_ERROR("assert ({}) failed.", #x); std::abort(); };
#define RFCT_VULKAN_CHECK(x) {vk::Result result = x; if (result!=vk::Result::eSuccess) { RFCT_ERROR("{0} failed with error code {1}.", #x, static_cast<int>(result)); };}