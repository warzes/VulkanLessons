#pragma once

#if defined(_MSC_VER)
#	pragma warning(push, 3)
#	pragma warning(disable : 5039)
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h> 

#include <cassert>
#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <filesystem>

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

#include "Engine/BaseMacros.h"
#include "Engine/BaseFunc.h"
#include "Engine/Log.h"
#include "Engine/EngineApp.h"
#include "Engine/Camera.h"
#include "Engine/KeyCodes.h"
#include "Engine/Frustum.h"
#include "Engine/ThreadPool.h"
#include "Engine/VulkanFrameBuffer.h"


#if defined(_MSC_VER)
#	pragma warning(pop)
#endif

