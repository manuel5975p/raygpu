#include <vector>
#include <mutex>
#include <unordered_map>
#include <map>
#include <deque>
#include <array>
struct PenInputState{
    float axes[16];
    Vector2 position;
};

struct window_input_state{
    
    Rectangle windowPosition; // Recovery after fullscreen
    std::vector<uint8_t> keydownPrevious = std::vector<uint8_t>(512, 0);
    std::vector<uint8_t> keydown = std::vector<uint8_t>(512, 0);
    Vector2 scrollThisFrame, scrollPreviousFrame;
    float gestureZoomThisFrame = 1.0f;
    float gestureAngleThisFrame;
    Vector2 mousePosPrevious;
    Vector2 mousePos;
    int cursorInWindow;
    std::array<uint8_t, 16> mouseButtonDownPrevious;
    std::array<uint8_t, 16> mouseButtonDown;
    
    std::vector<std::pair<int64_t, Vector2>> touchPoints; //Map from SDL_FingerID to Vector2
    std::deque<int> charQueue;

    std::unordered_map<unsigned int, PenInputState> penStates;
};
struct wgpustate{
    wgpu::Instance instance{};
    wgpu::Adapter adapter{};
    wgpu::Device device{};
    wgpu::Queue queue{};
};

#include "renderstate.inc"