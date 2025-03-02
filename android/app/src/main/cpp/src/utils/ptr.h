#pragma once
#include <memory>
template<typename T>
using shared = std::shared_ptr<T>;
template<typename T>
using unique = std::unique_ptr<T>;