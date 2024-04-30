#pragma once

#if defined(_MSC_VER)
#	pragma warning(disable : 4514)
#	pragma warning(disable : 5045)
#	pragma warning(push, 3)
#	pragma warning(disable : 5039)
#endif

#define _SILENCE_CXX20_CISO646_REMOVED_WARNING

#include "WindowsInclude.h"
#include <ShellScalingAPI.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h> 

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <ktx/ktx.h>
#include <ktx/ktxvulkan.h>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

#include "imgui/imgui.h"

#include <cassert>
#include <string>
#include <vector>
#include <set>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif