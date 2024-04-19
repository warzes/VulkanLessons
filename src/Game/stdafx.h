#pragma once

#if defined(_MSC_VER)
#	pragma warning(push, 3)
#	pragma warning(disable : 5039)
#endif

#include <vulkan/vulkan.h>
#include <cassert>
#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <numeric>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif
#include "tinygltf/tiny_gltf.h"

#include "Engine/BaseMacros.h"
#include "Engine/BaseFunc.h"
#include "Engine/Log.h"
#include "Engine/EngineApp.h"
#include "Engine/Camera.h"
#include "Engine/KeyCodes.h"


#if defined(_MSC_VER)
#	pragma warning(pop)
#endif

