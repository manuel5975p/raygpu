// begin file src/wgsl_parse_lite.c
#include <config.h>
#include <raygpu.h>
#include <internals.h>


InOutAttributeInfo                                      getAttributesWGSL_Simple(ShaderSources sources);
StringToUniformMap*                                     getBindingsWGSL_Simple  (ShaderSources sources);
EntryPointSet                                           getEntryPointsWGSL_Simple(const char* shaderSourceWGSL);

InOutAttributeInfo                                      getAttributesWGSL_Tint  (ShaderSources sources);
StringToUniformMap*                                     getBindingsWGSL_Tint    (ShaderSources sources);
EntryPointSet                                           getEntryPointsWGSL_Tint (const char* shaderSourceWGSL);

InOutAttributeInfo                                      getAttributesWGSL       (ShaderSources sources);
StringToUniformMap*                                     getBindingsWGSL         (ShaderSources sources);
EntryPointSet                                           getEntryPointsWGSL      (const char* shaderSourceWGSL);

// =================================================================================================
// SIMPLE WGSL PARSER BACKEND
// Enabled when SUPPORT_TINT_WGSL_PARSER != 1
// =================================================================================================
#if !defined(SUPPORT_TINT_WGSL_PARSER) || (SUPPORT_TINT_WGSL_PARSER != 1)


#include "simple_wgsl/wgsl_parser.c"
#include "simple_wgsl/wgsl_resolve.c"

static inline bool sw_starts_with(const char* s, const char* prefix) {
    if (!s || !prefix) return false;
    const size_t n = strlen(prefix);
    return strncmp(s, prefix, n) == 0;
}

static inline RGShaderStageEnum sw_to_stage_enum(WgslStage st) {
    switch (st) {
        case WGSL_STAGE_VERTEX:   return RGShaderStageEnum_Vertex;
        case WGSL_STAGE_FRAGMENT: return RGShaderStageEnum_Fragment;
        case WGSL_STAGE_COMPUTE:  return RGShaderStageEnum_Compute;
        default:                  return RGShaderStageEnum_Vertex;
    }
}

static inline RGVertexFormat sw_vf_from_numeric(int comps, WgslNumericType nt) {
    switch (nt) {
        case WGSL_NUM_F32:
            switch (comps) {
                case 1: return RGVertexFormat_Float32;
                case 2: return RGVertexFormat_Float32x2;
                case 3: return RGVertexFormat_Float32x3;
                case 4: return RGVertexFormat_Float32x4;
                default: return (RGVertexFormat)0;
            } break;
        case WGSL_NUM_I32:
            switch (comps) {
                case 1: return RGVertexFormat_Sint32;
                case 2: return RGVertexFormat_Sint32x2;
                case 3: return RGVertexFormat_Sint32x3;
                case 4: return RGVertexFormat_Sint32x4;
                default: return (RGVertexFormat)0;
            } break;
        case WGSL_NUM_U32:
        case WGSL_NUM_BOOL:
            switch (comps) {
                case 1: return RGVertexFormat_Uint32;
                case 2: return RGVertexFormat_Uint32x2;
                case 3: return RGVertexFormat_Uint32x3;
                case 4: return RGVertexFormat_Uint32x4;
                default: return (RGVertexFormat)0;
            } break;
        case WGSL_NUM_F16:
            switch (comps) {
                case 1: return RGVertexFormat_Float16;
                case 2: return RGVertexFormat_Float16x2;
                // vec3<f16> is not valid for vertex formats
                case 4: return RGVertexFormat_Float16x4;
                default: return (RGVertexFormat)0;
            } break;
        default: break;
    }
#ifdef WGPUVertexFormat_Undefined
    return WGPUVertexFormat_Undefined;
#else
    return RGVertexFormat_Float32;
#endif
}

static inline format_or_sample_type sw_parse_format_token(const char* tkn) {
    if (!tkn) return we_dont_know;
    if (strcmp(tkn, "r32float") == 0)    return format_r32float;
    if (strcmp(tkn, "r32uint") == 0)     return format_r32uint;
    if (strcmp(tkn, "rgba8unorm") == 0)  return format_rgba8unorm;
    if (strcmp(tkn, "rgba32float") == 0) return format_rgba32float;
    if (strcmp(tkn, "f32") == 0)         return sample_f32;
    if (strcmp(tkn, "u32") == 0)         return sample_u32;
    return we_dont_know;
}

static inline access_type sw_parse_access_token(const char* tkn) {
    if (!tkn) return readwrite;
    if (strcmp(tkn, "read") == 0)        return readonly;
    if (strcmp(tkn, "write") == 0)       return writeonly;
    if (strcmp(tkn, "read_write") == 0)  return readwrite;
    return readwrite;
}

typedef struct SW_ParsedTextureMeta {
    bool is_storage;
    bool is_array;
    bool is_3d;
    format_or_sample_type fmt_or_sample;
    access_type access;
}SW_ParsedTextureMeta;

static inline SW_ParsedTextureMeta sw_parse_texture_typenode(const WgslAstNode* T) {
    SW_ParsedTextureMeta m = {};
    if (!T || T->type != WGSL_NODE_TYPE) return m;
    const char* name = T->type_node.name ? T->type_node.name : "";

    m.is_storage = sw_starts_with(name, "texture_storage_");
    m.is_array   = sw_starts_with(name, "texture_2d_array") || sw_starts_with(name, "texture_storage_2d_array");
    m.is_3d      = sw_starts_with(name, "texture_3d") || sw_starts_with(name, "texture_storage_3d");

    for (int i = 0; i < T->type_node.type_arg_count; ++i) {
        const WgslAstNode* a = T->type_node.type_args[i];
        if (a && a->type == WGSL_NODE_TYPE && a->type_node.name) {
            format_or_sample_type f = sw_parse_format_token(a->type_node.name);
            if (f != we_dont_know) {
                m.fmt_or_sample = f;
                continue;
            }
            m.access = sw_parse_access_token(a->type_node.name);
        }
    }
    return m;
}

EntryPointSet getEntryPointsWGSL_Simple(const char* shaderSourceWGSL) {
    EntryPointSet eps = {0};
    if (!shaderSourceWGSL) return eps;

    WgslAstNode* ast = wgsl_parse(shaderSourceWGSL);
    if (!ast) return eps;

    WgslResolver* R = wgsl_resolver_build(ast);
    if (!R) { wgsl_free_ast(ast); return eps; }

    int n = 0;
    const WgslResolverEntrypoint* arr = wgsl_resolver_entrypoints(R, &n);
    uint32_t insertIndex = 0;
    for (int i = 0; i < n; ++i) {
        char* insert = eps.names[sw_to_stage_enum(arr[i].stage)];
        assert(strlen(arr[i].name) < MAX_SHADER_ENTRYPOINT_NAME_LENGTH);
        if(arr[i].name && strlen(arr[i].name) < MAX_SHADER_ENTRYPOINT_NAME_LENGTH){
            memcpy(insert, arr[i].name, strlen(arr[i].name));
        }
    }

    wgsl_resolve_free((void*)arr);
    wgsl_resolver_free(R);
    wgsl_free_ast(ast);
    return eps;
}

InOutAttributeInfo getAttributesWGSL_Simple(ShaderSources sources) {
    InOutAttributeInfo info = {0};
    if (sources.sourceCount == 0 || sources.sources[0].data == NULL) return info;

    const char* src = (const char*)sources.sources[0].data;

    WgslAstNode* ast = wgsl_parse(src);
    if (!ast) return info;

    WgslResolver* R = wgsl_resolver_build(ast);
    if (!R) { wgsl_free_ast(ast); return info; }

    int ep_count = 0;
    const WgslResolverEntrypoint* eps = wgsl_resolver_entrypoints(R, &ep_count);

    const char* vertex_name = NULL;
    for (int i = 0; i < ep_count; ++i) {
        if (eps[i].stage == WGSL_STAGE_VERTEX) { vertex_name = eps[i].name; break; }
    }

    if (vertex_name) {
        WgslVertexSlot* slots = NULL;
        int slot_count = wgsl_resolver_vertex_inputs(R, vertex_name, &slots);
        info.vertexAttributeCount = (uint32_t)slot_count;
        for (int i = 0; i < slot_count && i < (int)MAX_VERTEX_ATTRIBUTES; ++i) {
            ReflectionVertexAttribute* out = &info.vertexAttributes[i];
            out->location = (uint32_t)slots[i].location;
            out->format   = sw_vf_from_numeric(slots[i].component_count, slots[i].numeric_type);
            out->name[0]  = '\0';
        }
        wgsl_resolve_free(slots);
    }

    wgsl_resolve_free((void*)eps);
    wgsl_resolver_free(R);
    wgsl_free_ast(ast);
    return info;
}

StringToUniformMap* getBindingsWGSL_Simple(ShaderSources sources) {
    StringToUniformMap* out = NULL;
    if (sources.sourceCount == 0 || sources.sources[0].data == NULL){
        return out;
    }
    out = callocnew(StringToUniformMap);
    StringToUniformMap_init(out);
    const char* src = (const char*)sources.sources[0].data;

    WgslAstNode* ast = wgsl_parse(src);
    if (!ast){
        return out;
    }

    WgslResolver* R = wgsl_resolver_build(ast);
    if (!R) {
        wgsl_free_ast(ast);
        return out;
    }

    int n = 0;
    const WgslSymbolInfo* syms = wgsl_resolver_binding_vars(R, &n);

    for (int i = 0; i < n; ++i) {
        const WgslSymbolInfo* s = syms + i;
        if (!s->name || !s->decl_node || s->decl_node->type != WGSL_NODE_GLOBAL_VAR) continue;

        const GlobalVar* gv = &s->decl_node->global_var;

        ResourceTypeDescriptor desc = {0};
        desc.location = (uint32_t)(s->has_binding ? s->binding_index : 0);
        if (s->has_min_binding_size) desc.minBindingSize = (uint32_t)s->min_binding_size;

        const WgslAstNode* T = gv->type;
        const char* tname = (T && T->type == WGSL_NODE_TYPE) ? T->type_node.name : NULL;

        bool handled = false;
        if (tname) {
            if (strncmp(tname, "texture_", 8) == 0) {
                SW_ParsedTextureMeta meta = sw_parse_texture_typenode(T);
                if (meta.is_storage) {
                    if (meta.is_array)      desc.type = storage_texture2d_array;
                    else if (meta.is_3d)    desc.type = storage_texture3d;
                    else                    desc.type = storage_texture2d;
                    desc.fstype = meta.fmt_or_sample;
                    desc.access = meta.access;
                } else {
                    if (meta.is_array)      desc.type = texture2d_array;
                    else if (meta.is_3d)    desc.type = texture3d;
                    else                    desc.type = texture2d;
                    desc.fstype = meta.fmt_or_sample; // sample type
                }
                handled = true;
            } else if (strncmp(tname, "sampler", 7) == 0) {
                desc.type = texture_sampler;
                handled = true;
            }
        }

        if (!handled) {
            
            if (gv->address_space && strcmp(gv->address_space, "uniform") == 0) {
                desc.type = uniform_buffer;
            } else if (gv->address_space && strcmp(gv->address_space, "storage") == 0) {
                desc.type = storage_buffer;
                if(gv->access_modifier && strcmp(gv->access_modifier, "read") == 0){
                    desc.access = readonly;
                }
                else if(gv->access_modifier && strcmp(gv->access_modifier, "write") == 0){
                    desc.access = writeonly;
                }
                else if(gv->access_modifier && strcmp(gv->access_modifier, "read_write") == 0){
                    desc.access = readwrite;
                }
                else{
                    desc.access = readonly;
                }
            } else {
                continue; // non-bindable
            }
        }
        BindingIdentifier identifier = {
            .length = (uint32_t)strlen(s->name),
        };
        assert(identifier.length <= MAX_BINDING_NAME_LENGTH);
        if(identifier.length <= MAX_BINDING_NAME_LENGTH){
            memcpy(identifier.name, s->name, identifier.length);
            identifier.name[identifier.length] = '\0';
        }
        StringToUniformMap_put(out, identifier, desc);
    }

    wgsl_resolve_free((void*)syms);
    wgsl_resolver_free(R);
    wgsl_free_ast(ast);
    return out;
}

#endif // simple backend

// end file src/wgsl_parse_lite.c