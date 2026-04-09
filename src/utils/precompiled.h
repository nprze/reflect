#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "profiling_label.h"
#include "log.h"
#include "ptr.h"
#include "other.h"
#include "sizes.h"
#include "world_p/ecs.h"
#include "context.h"
#include "vulkan/vulkan.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>

using entity = entt::entity;
constexpr float fixedDeltaTime = 1.f / 60.f;