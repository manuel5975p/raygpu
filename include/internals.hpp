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
#include <raygpu.h>
#include <algorithm>
#include <vector>

template<typename T>
using small_vector = std::vector<T>;

/**
 * @brief Get the Buffer Layout representation compatible with WebGPU or Vulkan
 * 
 * @return VertexBufferLayoutSet 
 */
inline VertexBufferLayoutSet getBufferLayoutRepresentation(const AttributeAndResidence* attributes, const uint32_t number_of_attribs, uint32_t number_of_buffers){
    std::vector<std::vector<VertexAttribute>> buffer_to_attributes(number_of_buffers);
    VertexAttribute* attributePool = (VertexAttribute*   )std::calloc(number_of_attribs, sizeof(VertexAttribute));
    VertexBufferLayout* vbLayouts  = (VertexBufferLayout*)std::calloc(number_of_buffers, sizeof(VertexBufferLayout));

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
        std::memcpy(attributePool + poolOffset, buffer_to_attributes[i].data(), buffer_to_attributes[i].size() * sizeof(VertexAttribute));
        poolOffset += buffer_to_attributes[i].size();
    }

    for(size_t i = 0;i < number_of_buffers;i++){
        vbLayouts[i].attributes = attributePool + attributeOffsets[i];
        vbLayouts[i].attributeCount = buffer_to_attributes[i].size();
        vbLayouts[i].arrayStride = strides[i];
        vbLayouts[i].stepMode = stepmodes[i];
    }
    // return value
    VertexBufferLayoutSet ret{
        .number_of_buffers = number_of_buffers,
        .layouts = vbLayouts,
        .attributePool = attributePool
    };
    return ret;
}
inline void UnloadBufferLayoutSet(VertexBufferLayoutSet set){
    std::free(set.layouts);
    std::free(set.attributePool);
}

ShaderSourceType detectShaderLanguage(std::string_view source);
ShaderSourceType detectShaderLanguage(ShaderSources sources);
std::unordered_map<std::string, ResourceTypeDescriptor> getBindingsGLSL(ShaderSources source);


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
std::pair<std::vector<uint32_t>, std::vector<uint32_t>> glsl_to_spirv(const char *vs, const char *fs);
std::vector<uint32_t> glsl_to_spirv(const char *cs);
extern "C" void negotiateSurfaceFormatAndPresentMode(const void* SurfaceHandle);
extern "C" void ResetSyncState(cwoid);
extern "C" void CharCallback(void* window, unsigned int codePoint);

extern "C" void* CreateSurfaceForWindow(SubWindow window);
extern "C" void* CreateSurfaceForWindow_SDL2(void* windowHandle);
extern "C" void* CreateSurfaceForWindow_SDL3(void* windowHandle);
extern "C" void* CreateSurfaceForWindow_GLFW(void* windowHandle);
#endif
