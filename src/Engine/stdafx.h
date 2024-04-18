#pragma once

#if defined(_MSC_VER)
#	pragma warning(push, 3)
#	pragma warning(disable : 5039)
#endif

#include "WindowsInclude.h"
#include <ShellScalingAPI.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <ktx/ktx.h>
#include <ktx/ktxvulkan.h>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif
#include "tinygltf/tiny_gltf.h"

#include "imgui/imgui.h"

#include <cassert>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <fstream>

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif