#include <vector>
#include <mutex>
#include <unordered_map>
#include <map>
#include <deque>
#include <array>



struct wgpustate{
    wgpu::Instance instance{};
    wgpu::Adapter adapter{};
    wgpu::Device device{};
    wgpu::Queue queue{};
};

#include "renderstate.hpp"