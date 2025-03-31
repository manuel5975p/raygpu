/*
 * MIT License
 * 
 * Copyright (c) 2025 @manuel5975p
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */




#ifndef INTERNALS_HPP_INCLUDED
#define INTERNALS_HPP_INCLUDED
#include "pipeline.h"
#include <memory>
#include <raygpu.h>
#include <algorithm>
#include <vector>

template<typename T>
using small_vector = std::vector<T>;
template<typename T>
struct rl_free_deleter{
    void operator()(T* t)const noexcept{
        RL_FREE(t);
    }
};

template<typename T, typename... Args>
std::unique_ptr<T, rl_free_deleter<T>> rl_make_unique(Args&&... args) {
    void* ptr = RL_MALLOC(sizeof(T));
    if (!ptr) return std::unique_ptr<T, rl_free_deleter<T>>(nullptr);
    new (ptr) T(std::forward<Args>(args)...);
    return std::unique_ptr<T, rl_free_deleter<T>>(reinterpret_cast<T*>(ptr));
}


static inline uint32_t bitcount32(uint32_t x){
    #ifdef _MSC_VER
    return __popcnt(x);
    #elif defined(__GNUC__) 
    return __builtin_popcount(x);
    #else
    return std::popcount(x);
    #endif
}

static inline uint32_t bitcount64(uint64_t x){
    #ifdef _MSC_VER
    return __popcnt64(x);
    #elif defined(__GNUC__) 
    return __builtin_popcountll(x);
    #else
    return std::popcount(x);
    #endif
}

extern "C" void PrepareFrameGlobals();
extern "C" DescribedBuffer* UpdateVulkanRenderbatch();
extern "C" void PushUsedBuffer(void* nativeBuffer);
struct VertexBufferLayout{
    uint64_t arrayStride;
    VertexStepMode stepMode;
    size_t attributeCount;
    VertexAttribute* attributes; //NOT owned, points into data owned by VertexBufferLayoutSet::attributePool with an offset
};

typedef struct VertexBufferLayoutSet{
    uint32_t number_of_buffers;
    std::vector<VertexBufferLayout> layouts;    
    std::vector<VertexAttribute> attributePool;
}VertexBufferLayoutSet;

static inline uint64_t hash_bytes(const void* bytes, size_t count){
    if(count < 32){
        uint64_t data[32 / sizeof(uint64_t)] zeroinit;
        std::memcpy(data, bytes, count);
        xorshiftstate xsstate{0x324234fff1f1};
        for(uint32_t i = 0;i < 4;i++){
            xsstate.update(data[i]);
        }
        return xsstate.x64;
    }
    else{
        std::vector<uint64_t> vec(((count + sizeof(uint64_t) - 1) / sizeof(uint64_t)) * 8, 0);
        std::memcpy(vec.data(), bytes, count);
        xorshiftstate xsstate{0x324234fff1f1};
        for(uint32_t i = 0;i < vec.size();i++){
            xsstate.update(vec[i]);
        }
        return xsstate.x64;
    }
}
namespace std{
    template<>
    struct hash<VertexBufferLayoutSet>{
        size_t operator()(const VertexBufferLayoutSet& set)const noexcept{
            xorshiftstate xsstate{uint64_t(0x1919846573) * uint64_t(set.number_of_buffers << 14)};
            for(uint32_t i = 0;i < set.number_of_buffers;i++){
                for(uint32_t j = 0;j < set.layouts[i].attributeCount;j++){
                    xsstate.update(set.layouts[i].attributes[j].offset, set.layouts[i].attributes[j].shaderLocation);
                }
            }
            return xsstate.x64;
        }
    };
}
struct attributeVectorCompare{
    inline bool operator()(const std::vector<AttributeAndResidence>& a, const std::vector<AttributeAndResidence>& b)const noexcept{
        #ifndef NDEBUG
        int prevloc = -1;

        for(size_t i = 0;i < a.size();i++){
            assert((int)a[i].attr.shaderLocation > prevloc && "std::vector<AttributeAndResidence> not sorted");
            prevloc = a[i].attr.shaderLocation;
        }
        prevloc = -1;
        for(size_t i = 0;i < b.size();i++){
            assert((int)b[i].attr.shaderLocation > prevloc && "std::vector<AttributeAndResidence> not sorted");
            prevloc = b[i].attr.shaderLocation;
        }
        #endif
        if(a.size() != b.size()){
            return false;
        }
        for(size_t i = 0;i < a.size();i++){
            if(
                   a[i].bufferSlot != b[i].bufferSlot 
                || a[i].enabled != b[i].enabled
                || a[i].stepMode != b[i].stepMode 
                || a[i].attr.format != b[i].attr.format 
                || a[i].attr.offset != b[i].attr.offset 
                || a[i].attr.shaderLocation != b[i].attr.shaderLocation
            ){
                return false;
            }

        }
        return true;
    }
};
struct vblayoutVectorCompare{
    inline bool operator()(const VertexBufferLayoutSet& a, const VertexBufferLayoutSet& b)const noexcept{
        
        if(a.number_of_buffers != b.number_of_buffers)return false;
        for(uint32_t i = 0;i < a.number_of_buffers;i++){
            if(a.layouts[i].attributeCount != b.layouts[i].attributeCount)return false;
            for(uint32_t j = 0;j < a.layouts[i].attributeCount;j++){
                if(
                    a.layouts[i].attributes[j].format != b.layouts[i].attributes[j].format
                 || a.layouts[i].attributes[j].shaderLocation != b.layouts[i].attributes[j].shaderLocation
                 || a.layouts[i].attributes[j].offset != b.layouts[i].attributes[j].offset
                )return false;
            }
        }
        return true;
    }
};


typedef struct VertexStateToPipelineMap{
    std::unordered_map<VertexBufferLayoutSet, RenderPipelineQuartet, std::hash<VertexBufferLayoutSet>, vblayoutVectorCompare> pipelines;
}VertexStateToPipelineMap;

typedef struct ModifiablePipelineState{
    std::vector<AttributeAndResidence> vertexAttributes;
    WGVKBlendState blendState;
    uint32_t sampleCount;
    PrimitiveType primitive;
    Bool32 depthTest;
    Bool32 faceCull;
    FrontFace frontFace;
    CompareFunction depthCompare;
    bool operator==(const ModifiablePipelineState& mfps)const noexcept{
        if(attributeVectorCompare{}(vertexAttributes, mfps.vertexAttributes)){
            return true;
        }
        return false;
    }
}ModifiablePipelineState;

//typedef struct PipelineState{
//    ModifiablePipelineState modifiablePart;
//}PipelineState;

namespace std{
    template<> struct hash<ModifiablePipelineState>{
        size_t operator()(const ModifiablePipelineState& mfps)const noexcept{
            return hash<vector<AttributeAndResidence>>{}(mfps.vertexAttributes) ^ hash_bytes(&mfps.blendState, sizeof(WGVKBlendState));
        }
    };
}
extern "C" WGVKRenderPipeline createSingleRenderPipe(const ModifiablePipelineState& mst, const DescribedShaderModule& shaderModule, const DescribedBindGroupLayout& bglayout, const DescribedPipelineLayout& pllayout);
typedef struct HighLevelPipelineCache{
    std::unordered_map<ModifiablePipelineState, WGVKRenderPipeline> cacheMap; 
    WGVKRenderPipeline getOrCreate(const ModifiablePipelineState& mst, const DescribedShaderModule& shaderModule, const DescribedBindGroupLayout& bglayout, const DescribedPipelineLayout& pllayout){
        auto it = cacheMap.find(mst);
        if(it != cacheMap.end()){
            return it->second;
        }
        else{
            WGVKRenderPipeline toEmplace = createSingleRenderPipe(mst, shaderModule, bglayout, pllayout);
            cacheMap.emplace(mst, toEmplace);
            return toEmplace;
        }
    }
}HighLevelPipelineCache;

typedef struct DescribedPipeline{
    ModifiablePipelineState state;
    
    DescribedShaderModule shaderModule;
    DescribedBindGroup bindGroup;
    DescribedPipelineLayout layout;
    DescribedBindGroupLayout bglayout;

    HighLevelPipelineCache pipelineCache;
    WGVKRenderPipeline activePipeline;
}DescribedPipeline;

/**
 * @brief Get the Buffer Layout representation compatible with WebGPU or Vulkan
 * 
 * @return VertexBufferLayoutSet 
 */
inline VertexBufferLayoutSet getBufferLayoutRepresentation(const AttributeAndResidence* attributes, const uint32_t number_of_attribs, uint32_t number_of_buffers){
    std::vector<std::vector<VertexAttribute>> buffer_to_attributes(number_of_buffers);
    
    std::vector<VertexAttribute> attributePool(number_of_attribs);
    std::vector<VertexBufferLayout> vbLayouts(number_of_buffers);

    std::vector<uint32_t> strides  (number_of_buffers, 0);
    std::vector<VertexStepMode> stepmodes(number_of_buffers, VertexStepMode_None);
    std::vector<uint32_t> attrIndex(number_of_buffers, 0);
    for(size_t i = 0;i < number_of_attribs;i++){
        buffer_to_attributes[attributes[i].bufferSlot].push_back(attributes[i].attr);
        strides[attributes[i].bufferSlot] += attributeSize(attributes[i].attr.format);
        if(stepmodes[attributes[i].bufferSlot] != VertexStepMode_None && stepmodes[attributes[i].bufferSlot] != attributes[i].stepMode){
            TRACELOG(LOG_ERROR, "Conflicting stepmodes");
        }
        stepmodes[attributes[i].bufferSlot] = attributes[i].stepMode;
    }
    uint32_t poolOffset = 0;
    
    std::vector<uint32_t> attributeOffsets(number_of_buffers, 0);

    for(size_t i = 0;i < number_of_buffers;i++){
        attributeOffsets[i] = poolOffset;
        std::memcpy(attributePool.data() + poolOffset, buffer_to_attributes[i].data(), buffer_to_attributes[i].size() * sizeof(VertexAttribute));
        poolOffset += buffer_to_attributes[i].size();
    }

    for(size_t i = 0;i < number_of_buffers;i++){
        vbLayouts[i].attributes = attributePool.data() + attributeOffsets[i];
        vbLayouts[i].attributeCount = buffer_to_attributes[i].size();
        vbLayouts[i].arrayStride = strides[i];
        vbLayouts[i].stepMode = stepmodes[i];
    }
    // return value
    VertexBufferLayoutSet ret{
        .number_of_buffers = number_of_buffers,
        .layouts = std::move(vbLayouts),
        .attributePool = std::move(attributePool)
    };
    return ret;
}
extern "C" const char* copyString(const char* str);
//static inline void UnloadBufferLayoutSet(VertexBufferLayoutSet set){
//    std::free(set.layouts);
//    std::free(set.attributePool);
//}

static inline ShaderSources singleStage(const char* code, ShaderSourceType language, ShaderStage stage){
    ShaderSources sources zeroinit;
    sources.language = language;
    sources.sourceCount = 1;
    sources.sources[0].data = code;
    sources.sources[0].sizeInBytes = std::strlen(code);
    sources.sources[0].stageMask = ShaderStageMask(1u << uint32_t(+stage));
    return sources;
}

static inline ShaderSources dualStage(const char* code, ShaderSourceType language, ShaderStage stage1, ShaderStage stage2){
    ShaderSources sources zeroinit;
    sources.language = language;
    sources.sourceCount = 1;
    sources.sources[0].data = code;
    sources.sources[0].sizeInBytes = std::strlen(code);
    sources.sources[0].stageMask = ShaderStageMask((1u << uint32_t(stage1)) | (1u << uint32_t(stage2)));
    return sources;
}
static inline ShaderSources dualStage(const char* code1, const char* code2, ShaderSourceType language, ShaderStage stage1, ShaderStage stage2){
    ShaderSources sources zeroinit;
    sources.language = language;
    sources.sourceCount = 2;
    sources.sources[0].data = code1;
    sources.sources[0].sizeInBytes = std::strlen(code1);
    sources.sources[0].stageMask = ShaderStageMask(1u << uint32_t(+stage1));

    sources.sources[1].data = code2;
    sources.sources[1].sizeInBytes = std::strlen(code2);
    sources.sources[1].stageMask = ShaderStageMask(1u << uint32_t(+stage2));
 
    return sources;
}

void detectShaderLanguage(ShaderSources* sources);
std::unordered_map<std::string, ResourceTypeDescriptor> getBindingsGLSL(ShaderSources source);
std::vector<std::pair<ShaderStage, std::string>> getEntryPointsWGSL(const char* shaderSourceWGSL);
DescribedShaderModule LoadShaderModule(ShaderSources source);

extern "C" RenderPipelineQuartet GetPipelinesForLayout(DescribedPipeline* pl, const std::vector<AttributeAndResidence>& attribs);
inline VertexBufferLayoutSet getBufferLayoutRepresentation(const AttributeAndResidence* attributes, const uint32_t number_of_attribs){
    uint32_t maxslot = 0;
    for(size_t i = 0;i < number_of_attribs;i++){
        maxslot = std::max(maxslot, attributes[i].bufferSlot);
    }
    const uint32_t number_of_buffers = maxslot + 1;
    return getBufferLayoutRepresentation(attributes, number_of_attribs, number_of_buffers);
}
typedef struct VertexArray{
    std::vector<AttributeAndResidence> attributes;
    std::vector<std::pair<DescribedBuffer*, VertexStepMode>> buffers;
    void add(DescribedBuffer* buffer, uint32_t shaderLocation, 
                              VertexFormat fmt, uint32_t offset, 
                              VertexStepMode stepmode) {
        // Search for existing attribute by shaderLocation
        auto it = std::find_if(attributes.begin(), attributes.end(),
                               [shaderLocation](const AttributeAndResidence& ar) {
                                   return ar.attr.shaderLocation == shaderLocation;
                               });

        if (it != attributes.end()) {
            // Attribute exists, update it
            size_t existingBufferSlot = it->bufferSlot;
            auto& existingBufferPair = buffers[existingBufferSlot];

            // Check if the buffer is the same
            if (existingBufferPair.first->buffer != buffer->buffer) {
                // Attempting to update to a new buffer
                // Check if the new buffer is already in buffers
                auto bufferIt = std::find_if(buffers.begin(), buffers.end(),
                                            [buffer, stepmode](const std::pair<DescribedBuffer*, VertexStepMode>& b) {
                                                return b.first->buffer == buffer->buffer && b.second == stepmode;
                                            });

                if (bufferIt != buffers.end()) {
                    // Reuse existing buffer slot
                    it->bufferSlot = std::distance(buffers.begin(), bufferIt);
                } else {
                    uint32_t pufferIndex = bufferIt - buffers.begin();
                    auto attribIt = std::find_if(attributes.begin(), attributes.end(), [&](const AttributeAndResidence& attr){
                        return attr.bufferSlot == it->bufferSlot && attr.attr.shaderLocation != it->attr.shaderLocation;
                    });
                    //This buffer slot is unused otherwise, so reuse
                    if(attribIt == attributes.end()){
                        buffers[it->bufferSlot] = {buffer, stepmode};
                    }
                    // Add new buffer
                    else{
                        it->bufferSlot = buffers.size();
                        buffers.emplace_back(buffer, stepmode);
                    }
                }
            }

            // Update the rest of the attribute properties
            it->stepMode = stepmode;
            it->attr.format = fmt;
            it->attr.offset = offset;
            it->enabled = true;

            TRACELOG(LOG_DEBUG, "Attribute at shader location %u updated.", shaderLocation);
        } else {
            // Attribute does not exist, add as new
            AttributeAndResidence insert{};
            insert.enabled = true;
            bool bufferFound = false;

            for (size_t i = 0; i < buffers.size(); ++i) {
                auto& existingBufferPair = buffers[i];
                if (existingBufferPair.first->buffer == buffer->buffer) {
                    if (existingBufferPair.second == stepmode) {
                        // Reuse existing buffer slot
                        insert.bufferSlot = i;
                        bufferFound = true;
                        break;
                    } else {
                        TRACELOG(LOG_FATAL, "Mixed step modes for the same buffer are not implemented");
                        // Handle mixed step modes as per your application's requirements
                        // For now, we'll exit to indicate the limitation
                        exit(EXIT_FAILURE);
                    }
                }
            }

            if (!bufferFound) {
                // Add new buffer
                insert.bufferSlot = buffers.size();
                buffers.emplace_back(buffer, stepmode);
            }

            // Set the attribute properties
            insert.stepMode = stepmode;
            insert.attr.format = fmt;
            insert.attr.offset = offset;
            insert.attr.shaderLocation = shaderLocation;
            attributes.emplace_back(insert);

            TRACELOG(LOG_DEBUG,  "New attribute added at shader location %u", shaderLocation);
        }
        std::sort(attributes.begin(), attributes.end(), [](const AttributeAndResidence& a, const AttributeAndResidence& b){
            return a.attr.shaderLocation < b.attr.shaderLocation;
        });
    }
    void add_old(DescribedBuffer* buffer, uint32_t shaderLocation, VertexFormat fmt, uint32_t offset, VertexStepMode stepmode){
        AttributeAndResidence insert{};
        for(size_t i = 0;i < buffers.size();i++){
            auto& _buffer = buffers[i];
            if(_buffer.first->buffer == buffer->buffer){
                if(_buffer.second == stepmode){
                    insert.bufferSlot = i;
                    insert.stepMode = stepmode;
                    insert.attr.format = fmt;
                    insert.attr.offset = offset;
                    insert.attr.shaderLocation = shaderLocation;
                    attributes.push_back(insert);
                    return;
                }
                else{
                    std::cerr << "Mixed stepmodes not implemented yet\n";
                    exit(1);
                }
            }
        }
        insert.bufferSlot = buffers.size();
        buffers.push_back({buffer, stepmode});
        insert.stepMode = stepmode;
        insert.attr.format = fmt;
        insert.attr.offset = offset;
        insert.attr.shaderLocation = shaderLocation;
        attributes.push_back(insert);
        std::sort(attributes.begin(), attributes.end(), [](const AttributeAndResidence& a, const AttributeAndResidence& b){
            return a.attr.shaderLocation < b.attr.shaderLocation;
        });
    }

    /**
     * Enables an attribute based on shaderLocation.
     *
     * @param shaderLocation The shader location identifier to enable.
     * @return true if the attribute was found and enabled, false otherwise.
     */
    bool enableAttribute(uint32_t shaderLocation) {
        auto it = std::find_if(attributes.begin(), attributes.end(),
                               [shaderLocation](const AttributeAndResidence& ar) {
                                   return ar.attr.shaderLocation == shaderLocation;
                               });

        if (it != attributes.end()) {
            if (!it->enabled) {
                it->enabled = true;
            }
            return true;
        }

        TRACELOG(LOG_WARNING, "Attribute with shader location %u not found.", shaderLocation);
        return false;
    }

    /**
     * Disables an attribute based on shaderLocation.
     *
     * @param shaderLocation The shader location identifier to disable.
     * @return true if the attribute was found and disabled, false otherwise.
     */
    bool disableAttribute(uint32_t shaderLocation) {
        auto it = std::find_if(attributes.begin(), attributes.end(),
                               [shaderLocation](const AttributeAndResidence& ar) {
                                   return ar.attr.shaderLocation == shaderLocation;
                               });

        if (it != attributes.end()) {
            if (it->enabled) {
                it->enabled = false;
                TRACELOG(LOG_DEBUG, "Attribute at shader location %u disabled.", shaderLocation);
            } else {
                TRACELOG(LOG_DEBUG, "Attribute at shader location %u is already disabled", shaderLocation);
            }
            return true;
        }
        TRACELOG(LOG_WARNING, "Attribute at shader location %u does not exist", shaderLocation);
        return false;
    }
    
}VertexArray;


typedef struct StringToUniformMap{
    std::unordered_map<std::string, ResourceTypeDescriptor> uniforms;
    ResourceTypeDescriptor operator[](const std::string& v)const noexcept{
        return uniforms.find(v)->second;
    }
    uint32_t GetLocation(const std::string& v)const noexcept{
        auto it = uniforms.find(v);
        if(it == uniforms.end())
            return LOCATION_NOT_FOUND;
        return it->second.location;
    }
    ResourceTypeDescriptor operator[](const char* v)const noexcept{
        return uniforms.find(v)->second;
    }
    uint32_t GetLocation(const char* v)const noexcept{
        auto it = uniforms.find(v);
        if(it == uniforms.end())
            return LOCATION_NOT_FOUND;
        return it->second.location;
    }
}StringToUniformMap;

typedef struct StringToAttributeMap{
    std::unordered_map<std::string, std::pair<VertexFormat, uint32_t>> attributes;
    std::pair<VertexFormat, uint32_t> operator[](const std::string& v)const noexcept{
        return attributes.find(v)->second;
    }
    uint32_t GetLocation(const std::string& v)const noexcept{
        auto it = attributes.find(v);
        if(it == attributes.end())
            return LOCATION_NOT_FOUND;
        return it->second.second;
    }
    std::pair<VertexFormat, uint32_t> operator[](const char* v)const noexcept{
        return attributes.find(v)->second;
    }
    uint32_t GetLocation(const char* v)const noexcept{
        auto it = attributes.find(v);
        if(it == attributes.end())
            return LOCATION_NOT_FOUND;
        return it->second.second;
    }
}StringToAttributeMap;


namespace std{
    template<>
    struct hash<VertexArray>{
        inline size_t operator()(const VertexArray& va)const noexcept{
            size_t hashValue = 0;

            // Hash the attributes
            for (const auto& attrRes : va.attributes) {
                // Hash bufferSlot
                size_t bufferSlotHash = std::hash<size_t>()(attrRes.bufferSlot);
                hashValue ^= bufferSlotHash;
                hashValue = ROT_BYTES(hashValue, 5);

                // Hash stepMode
                size_t stepModeHash = std::hash<int>()(static_cast<int>(attrRes.stepMode));
                hashValue ^= stepModeHash;
                hashValue = ROT_BYTES(hashValue, 5);

                // Hash shaderLocation
                size_t shaderLocationHash = std::hash<uint32_t>()(attrRes.attr.shaderLocation);
                hashValue ^= shaderLocationHash;
                hashValue = ROT_BYTES(hashValue, 5);

                // Hash format
                size_t formatHash = std::hash<int>()(static_cast<int>(attrRes.attr.format));
                hashValue ^= formatHash;
                hashValue = ROT_BYTES(hashValue, 5);

                // Hash offset
                size_t offsetHash = std::hash<uint32_t>()(attrRes.attr.offset);
                hashValue ^= offsetHash;
                hashValue = ROT_BYTES(hashValue, 5);

                // Hash enabled flag
                size_t enabledHash = std::hash<bool>()(attrRes.enabled);
                hashValue ^= enabledHash;
                hashValue = ROT_BYTES(hashValue, 5);
            }

            // Hash the buffers (excluding DescribedBuffer* pointers)
            for (const auto& bufferPair : va.buffers) {
                // Only hash the WGPUVertexStepMode, not the buffer pointer
                size_t stepModeHash = std::hash<uint64_t>()(static_cast<uint64_t>(bufferPair.second));
                hashValue ^= stepModeHash;
                hashValue = ROT_BYTES(hashValue, 5);
            }

            return hashValue;
        }
    };
}



// Shader compilation utilities

/**
 * WGSL compilation is single code only because an arbitrary set of entrypoints can be placed there anyway.
 */

/**
 * @brief Compiles Vertex and Fragment shaders sources from GLSL (Vulkan rules) to SPIRV
 * Returns _two_ spirv blobs containing the respective modules.
 */
std::pair<std::vector<uint32_t>, std::vector<uint32_t>> glsl_to_spirv(const char* vs, const char* fs);
std::vector<uint32_t> wgsl_to_spirv(const char* anything);
std::vector<uint32_t> glsl_to_spirv(const char* cs);
ShaderSources wgsl_to_spirv(ShaderSources sources);
ShaderSources glsl_to_spirv(ShaderSources sources);
extern "C" void UpdatePipeline(DescribedPipeline* pl);
extern "C" void negotiateSurfaceFormatAndPresentMode(const void* SurfaceHandle);
extern "C" void ResetSyncState(cwoid);
extern "C" void CharCallback(void* window, unsigned int codePoint);

extern "C" void* CreateSurfaceForWindow(SubWindow window);
extern "C" void* CreateSurfaceForWindow_SDL2(void* windowHandle);
extern "C" void* CreateSurfaceForWindow_SDL3(void* windowHandle);
extern "C" void* CreateSurfaceForWindow_GLFW(void* windowHandle);
extern "C" void* CreateSurfaceForWindow_RGFW(void* windowHandle);
#endif
