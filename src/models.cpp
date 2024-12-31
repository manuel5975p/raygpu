#include <raygpu.h>
#ifndef RL_CALLOC
#define RL_CALLOC calloc
#endif
#ifndef RL_MALLOC
#define RL_MALLOC malloc
#endif
#ifndef RL_REALLOC
#define RL_REALLOC realloc
#endif
#ifndef RL_FREE
#define RL_FREE free
#endif
#ifndef RL_REALLOC
#define RL_REALLOC realloc
#endif
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <tinyobj_loader_c.h>
#include <cgltf.h>
cgltf_result LoadFileGLTFCallback(const struct cgltf_memory_options *,
                     const struct cgltf_file_options *, const char *,
                     cgltf_size *, void **){
                        return cgltf_result{};
                     }
void ReleaseFileGLTFCallback(const struct cgltf_memory_options *, const struct cgltf_file_options *, void *){
    
}
Mesh GenMeshCube(float width, float height, float length){
    constexpr size_t vertexCount = 6 * 4;    //6 sides of 4 vertices
    const float vertices[] = {//3 components per vertex
        -width/2, -height/2, length/2,
        width/2, -height/2, length/2,
        width/2, height/2, length/2,
        -width/2, height/2, length/2,
        -width/2, -height/2, -length/2,
        -width/2, height/2, -length/2,
        width/2, height/2, -length/2,
        width/2, -height/2, -length/2,
        -width/2, height/2, -length/2,
        -width/2, height/2, length/2,
        width/2, height/2, length/2,
        width/2, height/2, -length/2,
        -width/2, -height/2, -length/2,
        width/2, -height/2, -length/2,
        width/2, -height/2, length/2,
        -width/2, -height/2, length/2,
        width/2, -height/2, -length/2,
        width/2, height/2, -length/2,
        width/2, height/2, length/2,
        width/2, -height/2, length/2,
        -width/2, -height/2, -length/2,
        -width/2, -height/2, length/2,
        -width/2, height/2, length/2,
        -width/2, height/2, -length/2
    };

    constexpr float texcoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    constexpr float normals[] = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f
    };

    Mesh cubeMesh zeroinit;
    cubeMesh.vertices = (float*)  calloc(vertexCount, sizeof(float) * 3);
    cubeMesh.texcoords = (float*) calloc(vertexCount, sizeof(float) * 2);
    cubeMesh.normals = (float*)   calloc(vertexCount, sizeof(float) * 3);
    cubeMesh.colors = (float*)    calloc(vertexCount, sizeof(float) * 4);
    cubeMesh.indices = (uint32_t*)calloc(36, sizeof(uint32_t));
    uint32_t k = 0;
    for(uint32_t i = 0; i < 36; i += 6){
        cubeMesh.indices[i] = 4*k;
        cubeMesh.indices[i + 1] = 4*k + 1;
        cubeMesh.indices[i + 2] = 4*k + 2;
        cubeMesh.indices[i + 3] = 4*k;
        cubeMesh.indices[i + 4] = 4*k + 2;
        cubeMesh.indices[i + 5] = 4*k + 3;
        k++;
    }
    cubeMesh.triangleCount = 12;
    memcpy(cubeMesh.vertices, vertices, vertexCount * sizeof(float) * 3);
    memcpy(cubeMesh.texcoords, texcoords, vertexCount * sizeof(float) * 2);
    memcpy(cubeMesh.normals, normals, vertexCount * sizeof(float) * 3);
    cubeMesh.vertexCount = vertexCount;
    std::fill(cubeMesh.colors, cubeMesh.colors + vertexCount * 4, 1.0f);
    UploadMesh(&cubeMesh, false);
    return cubeMesh;
}
void UploadMesh(Mesh *mesh, bool dynamic){
    if(mesh->colors == nullptr){
        mesh->colors = (float*)RL_CALLOC(mesh->vertexCount, sizeof(Vector4));
        for(size_t i = 0;i < mesh->vertexCount * 4;i++){
            mesh->colors[i] = 1;
        }
    }
    if(mesh->vbos == nullptr){
        mesh->vbos = (DescribedBuffer*)calloc(4, sizeof(DescribedBuffer));
        mesh->vbos[0] = GenBuffer(mesh->vertices , mesh->vertexCount * sizeof(float  ) * 3);
        mesh->vbos[1] = GenBuffer(mesh->texcoords, mesh->vertexCount * sizeof(float  ) * 2);
        mesh->vbos[2] = GenBuffer(mesh->normals  , mesh->vertexCount * sizeof(float  ) * 3);
        mesh->vbos[3] = GenBuffer(mesh->colors   , mesh->vertexCount * sizeof(float  ) * 4);
        if(mesh->indices){
            mesh->ibo = GenIndexBuffer(mesh->indices, mesh->triangleCount * 3 * sizeof(uint32_t));
        }
        mesh->vao = LoadVertexArray();
        VertexAttribPointer(mesh->vao, &mesh->vbos[0], 0, WGPUVertexFormat_Float32x3, sizeof(float) * 0, WGPUVertexStepMode_Vertex);
        EnableVertexAttribArray(mesh->vao, 0);
        VertexAttribPointer(mesh->vao, &mesh->vbos[1], 1, WGPUVertexFormat_Float32x2, sizeof(float) * 0, WGPUVertexStepMode_Vertex);
        EnableVertexAttribArray(mesh->vao, 1);
        VertexAttribPointer(mesh->vao, &mesh->vbos[2], 2, WGPUVertexFormat_Float32x3, sizeof(float) * 0, WGPUVertexStepMode_Vertex);
        EnableVertexAttribArray(mesh->vao, 2);
        VertexAttribPointer(mesh->vao, &mesh->vbos[3], 3, WGPUVertexFormat_Float32x4, sizeof(float) * 0, WGPUVertexStepMode_Vertex);
        EnableVertexAttribArray(mesh->vao, 3);
    }
    else{
        BufferData(&mesh->vbos[0], mesh->vertices , mesh->vertexCount * sizeof(float  ) * 3);
        BufferData(&mesh->vbos[1], mesh->texcoords, mesh->vertexCount * sizeof(float  ) * 2);
        BufferData(&mesh->vbos[2], mesh->normals  , mesh->vertexCount * sizeof(float  ) * 3);
        BufferData(&mesh->vbos[3], mesh->colors   , mesh->vertexCount * sizeof(float  ) * 4);
        if(mesh->indices){
            BufferData(&mesh->ibo, mesh->indices, mesh->triangleCount * 3 * sizeof(uint32_t));
        }
    }
}
DescribedBuffer trfBuffer{};
extern "C" void DrawMeshInstanced(Mesh mesh, Material material, const Matrix* transforms, int instances){
    if(trfBuffer.buffer){
        BufferData(&trfBuffer, transforms, instances * sizeof(Matrix));
    }else{
        trfBuffer = GenStorageBuffer(transforms, instances * sizeof(Matrix));
    }
    SetPipelineStorageBuffer(GetActivePipeline(), 3, &trfBuffer);
    BindVertexArray(GetActivePipeline(), mesh.vao);
    if(mesh.ibo.buffer){
        DrawArraysIndexedInstanced(WGPUPrimitiveTopology_TriangleList, mesh.ibo, mesh.triangleCount * 3, instances);
    }else{
        DrawArraysInstanced(WGPUPrimitiveTopology_TriangleList, mesh.vertexCount, instances);
    }
    //wgpuBufferRelease(trfBuffer.buffer);
}
extern "C" void DrawMesh(Mesh mesh, Material material, Matrix transform){
    SetStorageBufferData(3, &transform, sizeof(Matrix));
    BindVertexArray(GetActivePipeline(), mesh.vao);
    if(mesh.ibo.buffer){
        DrawArraysIndexed(WGPUPrimitiveTopology_TriangleList, mesh.ibo, mesh.triangleCount * 3);
    }else{
        DrawArrays(WGPUPrimitiveTopology_TriangleList, mesh.vertexCount);
    }
}
Model LoadOBJ(const char *fileName)
{
    tinyobj_attrib_t objAttributes zeroinit;
    tinyobj_shape_t *objShapes = NULL;
    unsigned int objShapeCount = 0;

    tinyobj_material_t *objMaterials = NULL;
    unsigned int objMaterialCount = 0;

    Model model zeroinit;
    model.transform = MatrixIdentity();

    char *fileText = LoadFileText(fileName);

    if (fileText == NULL){
        TRACELOG(LOG_ERROR, "MODEL: [%s] Unable to read obj file", fileName);
        return model;
    }

    char currentDir[1024] zeroinit;


    //strcpy(currentDir, GetWorkingDirectory()); // Save current working directory
    //const char *workingDir = GetDirectoryPath(fileName); // Switch to OBJ directory for material path correctness
    //if (CHDIR(workingDir) != 0) TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to change working directory", workingDir);

    unsigned int dataSize = (unsigned int)strlen(fileText);

    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
    int ret = tinyobj_parse_obj(&objAttributes, &objShapes, &objShapeCount, &objMaterials, &objMaterialCount, fileText, dataSize, flags);

    if (ret != TINYOBJ_SUCCESS)
    {
        TRACELOG(LOG_ERROR, "MODEL: Unable to read obj data %s", fileName);
        return model;
    }

    UnloadFileText(fileText);

    unsigned int faceVertIndex = 0;
    unsigned int nextShape = 1;
    int lastMaterial = -1;
    unsigned int meshIndex = 0;

    // Count meshes
    unsigned int nextShapeEnd = objAttributes.num_face_num_verts;

    // See how many verts till the next shape
    if (objShapeCount > 1) nextShapeEnd = objShapes[nextShape].face_offset;

    // Walk all the faces
    for (unsigned int faceId = 0; faceId < objAttributes.num_faces; faceId++)
    {
        if (faceId >= nextShapeEnd)
        {
            // Try to find the last vert in the next shape
            nextShape++;
            if (nextShape < objShapeCount) nextShapeEnd = objShapes[nextShape].face_offset;
            else nextShapeEnd = objAttributes.num_face_num_verts; // This is actually the total number of face verts in the file, not faces
            meshIndex++;
        }
        else if ((lastMaterial != -1) && (objAttributes.material_ids[faceId] != lastMaterial))
        {
            meshIndex++; // If this is a new material, we need to allocate a new mesh
        }

        lastMaterial = objAttributes.material_ids[faceId];
        faceVertIndex += objAttributes.face_num_verts[faceId];
    }

    // Allocate the base meshes and materials
    model.meshCount = meshIndex + 1;
    model.meshes = (Mesh *)calloc(model.meshCount, sizeof(Mesh));

    if (objMaterialCount > 0)
    {
        model.materialCount = objMaterialCount;
        model.materials = (Material *)calloc(objMaterialCount, sizeof(Material));
    }
    else // We must allocate at least one material
    {
        model.materialCount = 1;
        model.materials = (Material *)calloc(1, sizeof(Material));
    }

    model.meshMaterial = (int *)calloc(model.meshCount, sizeof(int));

    // See how many verts are in each mesh
    unsigned int *localMeshVertexCounts = (unsigned int *)calloc(model.meshCount, sizeof(unsigned int));

    faceVertIndex = 0;
    nextShapeEnd = objAttributes.num_face_num_verts;
    lastMaterial = -1;
    meshIndex = 0;
    unsigned int localMeshVertexCount = 0;

    nextShape = 1;
    if (objShapeCount > 1) nextShapeEnd = objShapes[nextShape].face_offset;

    // Walk all the faces
    for (unsigned int faceId = 0; faceId < objAttributes.num_faces; faceId++)
    {
        bool newMesh = false; // Do we need a new mesh?
        if (faceId >= nextShapeEnd)
        {
            // Try to find the last vert in the next shape
            nextShape++;
            if (nextShape < objShapeCount) nextShapeEnd = objShapes[nextShape].face_offset;
            else nextShapeEnd = objAttributes.num_face_num_verts; // this is actually the total number of face verts in the file, not faces

            newMesh = true;
        }
        else if ((lastMaterial != -1) && (objAttributes.material_ids[faceId] != lastMaterial))
        {
            newMesh = true;
        }

        lastMaterial = objAttributes.material_ids[faceId];

        if (newMesh)
        {
            localMeshVertexCounts[meshIndex] = localMeshVertexCount;

            localMeshVertexCount = 0;
            meshIndex++;
        }

        faceVertIndex += objAttributes.face_num_verts[faceId];
        localMeshVertexCount += objAttributes.face_num_verts[faceId];
    }

    localMeshVertexCounts[meshIndex] = localMeshVertexCount;

    for (int i = 0; i < model.meshCount; i++)
    {
        // Allocate the buffers for each mesh
        unsigned int vertexCount = localMeshVertexCounts[i];

            model.meshes[i].vertexCount = vertexCount;
            model.meshes[i].triangleCount = vertexCount/3;

            model.meshes[i].vertices = (float *) calloc(vertexCount * 3, sizeof(float));
            model.meshes[i].normals = (float *)  calloc(vertexCount * 3, sizeof(float));
            model.meshes[i].texcoords = (float *)calloc(vertexCount * 2, sizeof(float));
            model.meshes[i].colors = (float *)   calloc(vertexCount * 4, sizeof(float));
        }

    free(localMeshVertexCounts);
    localMeshVertexCounts = NULL;

    // Fill meshes
    faceVertIndex = 0;

    nextShapeEnd = objAttributes.num_face_num_verts;

    // See how many verts till the next shape
    nextShape = 1;
    if (objShapeCount > 1) nextShapeEnd = objShapes[nextShape].face_offset;
    lastMaterial = -1;
    meshIndex = 0;
    localMeshVertexCount = 0;

    // Walk all the faces
    for (unsigned int faceId = 0; faceId < objAttributes.num_faces; faceId++)
    {
        bool newMesh = false; // Do we need a new mesh?
        if (faceId >= nextShapeEnd)
        {
            // Try to find the last vert in the next shape
            nextShape++;
            if (nextShape < objShapeCount) nextShapeEnd = objShapes[nextShape].face_offset;
            else nextShapeEnd = objAttributes.num_face_num_verts; // This is actually the total number of face verts in the file, not faces
            newMesh = true;
        }

        // If this is a new material, we need to allocate a new mesh
        if (lastMaterial != -1 && objAttributes.material_ids[faceId] != lastMaterial) newMesh = true;
        lastMaterial = objAttributes.material_ids[faceId];

        if (newMesh)
        {
            localMeshVertexCount = 0;
            meshIndex++;
        }

        int matId = 0;
        if ((lastMaterial >= 0) && (lastMaterial < (int)objMaterialCount)) matId = lastMaterial;

        model.meshMaterial[meshIndex] = matId;

        for (int f = 0; f < objAttributes.face_num_verts[faceId]; f++)
        {
            int vertIndex = objAttributes.faces[faceVertIndex].v_idx;
            int normalIndex = objAttributes.faces[faceVertIndex].vn_idx;
            int texcordIndex = objAttributes.faces[faceVertIndex].vt_idx;

            for (int i = 0; i < 3; i++) model.meshes[meshIndex].vertices[localMeshVertexCount*3 + i] = objAttributes.vertices[vertIndex*3 + i];

            for (int i = 0; i < 3; i++) model.meshes[meshIndex].normals[localMeshVertexCount*3 + i] = objAttributes.normals[normalIndex*3 + i];

            for (int i = 0; i < 2; i++) model.meshes[meshIndex].texcoords[localMeshVertexCount*2 + i] = objAttributes.texcoords[texcordIndex*2 + i];

            model.meshes[meshIndex].texcoords[localMeshVertexCount*2 + 1] = 1.0f - model.meshes[meshIndex].texcoords[localMeshVertexCount*2 + 1];

            for (int i = 0; i < 4; i++) model.meshes[meshIndex].colors[localMeshVertexCount * 4 + i] = 1.0f;

            faceVertIndex++;
            localMeshVertexCount++;
        }
    }

    //if (objMaterialCount > 0) ProcessMaterialsOBJ(model.materials, objMaterials, objMaterialCount);
    //else model.materials[0] = LoadMaterialDefault(); // Set default material for the mesh

    tinyobj_attrib_free(&objAttributes);
    tinyobj_shapes_free(objShapes, objShapeCount);
    tinyobj_materials_free(objMaterials, objMaterialCount);

    for (int i = 0; i < model.meshCount; i++) UploadMesh(model.meshes + i, true);

    // Restore current working directory
    //if (CHDIR(currentDir) != 0)
    //{
    //    TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to change working directory", currentDir);
    //}

    return model;
}
static Image LoadImageFromCgltfImage(cgltf_image *cgltfImage, const char *texPath)
{
    Image image zeroinit;

    if (cgltfImage == NULL) return image;

    if (cgltfImage->uri != NULL)     // Check if image data is provided as an uri (base64 or path)
    {
        if ((strlen(cgltfImage->uri) > 5) &&
            (cgltfImage->uri[0] == 'd') &&
            (cgltfImage->uri[1] == 'a') &&
            (cgltfImage->uri[2] == 't') &&
            (cgltfImage->uri[3] == 'a') &&
            (cgltfImage->uri[4] == ':'))     // Check if image is provided as base64 text data
        {
            // Data URI Format: data:<mediatype>;base64,<data>

            // Find the comma
            int i = 0;
            while ((cgltfImage->uri[i] != ',') && (cgltfImage->uri[i] != 0)) i++;

            if (cgltfImage->uri[i] == 0) TRACELOG(LOG_WARNING, "IMAGE: glTF data URI is not a valid image");
            else
            {
                int base64Size = (int)strlen(cgltfImage->uri + i + 1);
                while (cgltfImage->uri[i + base64Size] == '=') base64Size--;    // Ignore optional paddings
                int numberOfEncodedBits = base64Size*6 - (base64Size*6) % 8 ;   // Encoded bits minus extra bits, so it becomes a multiple of 8 bits
                int outSize = numberOfEncodedBits/8 ;                           // Actual encoded bytes
                void *data = NULL;

                cgltf_options options = zeroinit;
                options.file.read = LoadFileGLTFCallback;
                options.file.release = ReleaseFileGLTFCallback;
                cgltf_result result = cgltf_load_buffer_base64(&options, outSize, cgltfImage->uri + i + 1, &data);

                if (result == cgltf_result_success)
                {
                    image = LoadImageFromMemory(".png", (unsigned char *)data, outSize);
                    RL_FREE(data);
                }
            }
        }
        else     // Check if image is provided as image path
        {
            image = LoadImage(TextFormat("%s/%s", texPath, cgltfImage->uri));
        }
    }
    else if (cgltfImage->buffer_view != NULL && cgltfImage->buffer_view->buffer->data != NULL)    // Check if image is provided as data buffer
    {
        unsigned char *data = (unsigned char*) RL_MALLOC(cgltfImage->buffer_view->size);
        int offset = (int)cgltfImage->buffer_view->offset;
        int stride = (int)cgltfImage->buffer_view->stride? (int)cgltfImage->buffer_view->stride : 1;

        // Copy buffer data to memory for loading
        for (unsigned int i = 0; i < cgltfImage->buffer_view->size; i++)
        {
            data[i] = ((unsigned char *)cgltfImage->buffer_view->buffer->data)[offset];
            offset += stride;
        }

        // Check mime_type for image: (cgltfImage->mime_type == "image/png")
        // NOTE: Detected that some models define mime_type as "image\\/png"
        if ((strcmp(cgltfImage->mime_type, "image\\/png") == 0) ||
            (strcmp(cgltfImage->mime_type, "image/png") == 0)) image = LoadImageFromMemory(".png", data, (int)cgltfImage->buffer_view->size);
        else if ((strcmp(cgltfImage->mime_type, "image\\/jpeg") == 0) ||
                 (strcmp(cgltfImage->mime_type, "image/jpeg") == 0)) image = LoadImageFromMemory(".jpg", data, (int)cgltfImage->buffer_view->size);
        else TRACELOG(LOG_WARNING, "MODEL: glTF image data MIME type not recognized", TextFormat("%s/%s", texPath, cgltfImage->uri));

        RL_FREE(data);
    }

    return image;
}

Model LoadGLTFFromMemory(const void* fileData, size_t size){
    Model ret zeroinit;
    cgltf_options options zeroinit;
    cgltf_data* data = nullptr;
    cgltf_result result = cgltf_parse(&options, fileData, size, &data);
    if (result == cgltf_result_success){
    	/* TODO make awesome stuff */
        
    	cgltf_free(data);
    }
    else{
        TRACELOG(LOG_WARNING, "GLTF could not be parsed");
    }
    return ret;
}
static BoneInfo *LoadBoneInfoGLTF(cgltf_skin skin, int *boneCount)
{
    *boneCount = (int)skin.joints_count;
    BoneInfo *bones = (BoneInfo*)RL_MALLOC(skin.joints_count*sizeof(BoneInfo));

    for (unsigned int i = 0; i < skin.joints_count; i++)
    {
        cgltf_node node = *skin.joints[i];
        if (node.name != NULL)
        {
            strncpy(bones[i].name, node.name, sizeof(bones[i].name));
            bones[i].name[sizeof(bones[i].name) - 1] = '\0';
        }

        // Find parent bone index
        int parentIndex = -1;

        for (unsigned int j = 0; j < skin.joints_count; j++)
        {
            if (skin.joints[j] == node.parent)
            {
                parentIndex = (int)j;
                break;
            }
        }

        bones[i].parent = parentIndex;
    }

    return bones;
}

Material LoadMaterialDefault(void)
{
    Material material zeroinit;
    material.maps = (MaterialMap *)RL_CALLOC(MAX_MATERIAL_MAPS, sizeof(MaterialMap));

    // Using rlgl default shader
    material.pipeline = DefaultPipeline();

    // Using rlgl default texture (1x1 pixel, UNCOMPRESSED_R8G8B8A8, 1 mipmap)
    material.maps[MATERIAL_MAP_DIFFUSE].texture = GetDefaultTexture();
    //material.maps[MATERIAL_MAP_NORMAL].texture;         // NOTE: By default, not set
    //material.maps[MATERIAL_MAP_SPECULAR].texture;       // NOTE: By default, not set

    material.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;    // Diffuse color
    material.maps[MATERIAL_MAP_SPECULAR].color = WHITE;   // Specular color

    return material;
}


static Model LoadGLTF(const char *fileName)
{
    /*********************************************************************************************

        Function implemented by Wilhem Barbier(@wbrbr), with modifications by Tyler Bezera(@gamerfiend)
        Transform handling implemented by Paul Melis (@paulmelis).
        Reviewed by Ramon Santamaria (@raysan5)

        FEATURES:
          - Supports .gltf and .glb files
          - Supports embedded (base64) or external textures
          - Supports PBR metallic/roughness flow, loads material textures, values and colors
                     PBR specular/glossiness flow and extended texture flows not supported
          - Supports multiple meshes per model (every primitives is loaded as a separate mesh)
          - Supports basic animations
          - Transforms, including parent-child relations, are applied on the mesh data, but the
            hierarchy is not kept (as it can't be represented).
          - Mesh instances in the glTF file (i.e. same mesh linked from multiple nodes)
            are turned into separate raylib Meshes.

        RESTRICTIONS:
          - Only triangle meshes supported
          - Vertex attribute types and formats supported:
              > Vertices (position): vec3: float
              > Normals: vec3: float
              > Texcoords: vec2: float
              > Colors: vec4: u8, u16, f32 (normalized)
              > Indices: u16, u32 (truncated to u16)
          - Scenes defined in the glTF file are ignored. All nodes in the file
            are used.

    ***********************************************************************************************/

    // Macro to simplify attributes loading code
    #define LOAD_ATTRIBUTE(accesor, numComp, srcType, dstPtr) LOAD_ATTRIBUTE_CAST(accesor, numComp, srcType, dstPtr, srcType)

    #define LOAD_ATTRIBUTE_CAST(accesor, numComp, srcType, dstPtr, dstType) \
    { \
        int n = 0; \
        srcType *buffer = (srcType *)accesor->buffer_view->buffer->data + accesor->buffer_view->offset/sizeof(srcType) + accesor->offset/sizeof(srcType); \
        for (unsigned int k = 0; k < accesor->count; k++) \
        {\
            for (int l = 0; l < numComp; l++) \
            {\
                dstPtr[numComp*k + l] = (dstType)buffer[n + l];\
            }\
            n += (int)(accesor->stride/sizeof(srcType));\
        }\
    }

    Model model zeroinit;

    // glTF file loading
    size_t dataSize = 0;
    unsigned char *fileData = (unsigned char*)LoadFileData(fileName, &dataSize);

    if (fileData == NULL) return model;

    // glTF data loading
    cgltf_options options zeroinit;
    options.file.read =    nullptr;//LoadFileGLTFCallback;
    options.file.release = nullptr;//ReleaseFileGLTFCallback;
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, fileData, dataSize, &data);

    if (result == cgltf_result_success)
    {
        if (data->file_type == cgltf_file_type_glb) TRACELOG(LOG_INFO, "MODEL: [%s] Model basic data (glb) loaded successfully", fileName);
        else if (data->file_type == cgltf_file_type_gltf) TRACELOG(LOG_INFO, "MODEL: [%s] Model basic data (glTF) loaded successfully", fileName);
        else TRACELOG(LOG_WARNING, "MODEL: [%s] Model format not recognized", fileName);

        TRACELOG(LOG_INFO, "    > Meshes count: %i", data->meshes_count);
        TRACELOG(LOG_INFO, "    > Materials count: %i (+1 default)", data->materials_count);
        TRACELOG(LOG_DEBUG, "    > Buffers count: %i", data->buffers_count);
        TRACELOG(LOG_DEBUG, "    > Images count: %i", data->images_count);
        TRACELOG(LOG_DEBUG, "    > Textures count: %i", data->textures_count);

        // Force reading data buffers (fills buffer_view->buffer->data)
        // NOTE: If an uri is defined to base64 data or external path, it's automatically loaded
        result = cgltf_load_buffers(&options, data, fileName);
        if (result != cgltf_result_success) TRACELOG(LOG_INFO, "MODEL: [%s] Failed to load mesh/material buffers", fileName);

        int primitivesCount = 0;
        // NOTE: We will load every primitive in the glTF as a separate raylib Mesh.
        // Determine total number of meshes needed from the node hierarchy.
        for (unsigned int i = 0; i < data->nodes_count; i++)
        {
            cgltf_node *node = &(data->nodes[i]);
            cgltf_mesh *mesh = node->mesh;
            if (!mesh)
                continue;

            for (unsigned int p = 0; p < mesh->primitives_count; p++)
            {
                if (mesh->primitives[p].type == cgltf_primitive_type_triangles)
                    primitivesCount++;
            }
        }
        TRACELOG(LOG_DEBUG, "    > Primitives (triangles only) count based on hierarchy : %i", primitivesCount);

        // Load our model data: meshes and materials
        model.meshCount = primitivesCount;
        model.meshes = (Mesh*)RL_CALLOC(model.meshCount, sizeof(Mesh));

        // NOTE: We keep an extra slot for default material, in case some mesh requires it
        model.materialCount = (int)data->materials_count + 1;
        model.materials = (Material*)RL_CALLOC(model.materialCount, sizeof(Material));
        model.materials[0] = LoadMaterialDefault();     // Load default material (index: 0)

        // Load mesh-material indices, by default all meshes are mapped to material index: 0
        model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));

        // Load materials data
        //----------------------------------------------------------------------------------------------------
        for (unsigned int i = 0, j = 1; i < data->materials_count; i++, j++)
        {
            model.materials[j] = LoadMaterialDefault();
            const char *texPath = GetDirectoryPath(fileName);

            // Check glTF material flow: PBR metallic/roughness flow
            // NOTE: Alternatively, materials can follow PBR specular/glossiness flow
            if (data->materials[i].has_pbr_metallic_roughness)
            {
                // Load base color texture (albedo)
                if (data->materials[i].pbr_metallic_roughness.base_color_texture.texture)
                {
                    Image imAlbedo = LoadImageFromCgltfImage(data->materials[i].pbr_metallic_roughness.base_color_texture.texture->image, texPath);
                    if (imAlbedo.data != NULL)
                    {
                        model.materials[j].maps[MATERIAL_MAP_ALBEDO].texture = LoadTextureFromImage(imAlbedo);
                        UnloadImage(imAlbedo);
                    }
                }
                // Load base color factor (tint)
                model.materials[j].maps[MATERIAL_MAP_ALBEDO].color.r = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[0]*255);
                model.materials[j].maps[MATERIAL_MAP_ALBEDO].color.g = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[1]*255);
                model.materials[j].maps[MATERIAL_MAP_ALBEDO].color.b = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[2]*255);
                model.materials[j].maps[MATERIAL_MAP_ALBEDO].color.a = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[3]*255);

                // Load metallic/roughness texture
                if (data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture)
                {
                    Image imMetallicRoughness = LoadImageFromCgltfImage(data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture->image, texPath);
                    if (imMetallicRoughness.data != NULL)
                    {
                        model.materials[j].maps[MATERIAL_MAP_ROUGHNESS].texture = LoadTextureFromImage(imMetallicRoughness);
                        UnloadImage(imMetallicRoughness);
                    }

                    // Load metallic/roughness material properties
                    float roughness = data->materials[i].pbr_metallic_roughness.roughness_factor;
                    model.materials[j].maps[MATERIAL_MAP_ROUGHNESS].value = roughness;

                    float metallic = data->materials[i].pbr_metallic_roughness.metallic_factor;
                    model.materials[j].maps[MATERIAL_MAP_METALNESS].value = metallic;
                }

                // Load normal texture
                if (data->materials[i].normal_texture.texture)
                {
                    Image imNormal = LoadImageFromCgltfImage(data->materials[i].normal_texture.texture->image, texPath);
                    if (imNormal.data != NULL)
                    {
                        model.materials[j].maps[MATERIAL_MAP_NORMAL].texture = LoadTextureFromImage(imNormal);
                        UnloadImage(imNormal);
                    }
                }

                // Load ambient occlusion texture
                if (data->materials[i].occlusion_texture.texture)
                {
                    Image imOcclusion = LoadImageFromCgltfImage(data->materials[i].occlusion_texture.texture->image, texPath);
                    if (imOcclusion.data != NULL)
                    {
                        model.materials[j].maps[MATERIAL_MAP_OCCLUSION].texture = LoadTextureFromImage(imOcclusion);
                        UnloadImage(imOcclusion);
                    }
                }

                // Load emissive texture
                if (data->materials[i].emissive_texture.texture)
                {
                    Image imEmissive = LoadImageFromCgltfImage(data->materials[i].emissive_texture.texture->image, texPath);
                    if (imEmissive.data != NULL)
                    {
                        model.materials[j].maps[MATERIAL_MAP_EMISSION].texture = LoadTextureFromImage(imEmissive);
                        UnloadImage(imEmissive);
                    }

                    // Load emissive color factor
                    model.materials[j].maps[MATERIAL_MAP_EMISSION].color.r = (unsigned char)(data->materials[i].emissive_factor[0]*255);
                    model.materials[j].maps[MATERIAL_MAP_EMISSION].color.g = (unsigned char)(data->materials[i].emissive_factor[1]*255);
                    model.materials[j].maps[MATERIAL_MAP_EMISSION].color.b = (unsigned char)(data->materials[i].emissive_factor[2]*255);
                    model.materials[j].maps[MATERIAL_MAP_EMISSION].color.a = 255;
                }
            }

            // Other possible materials not supported by raylib pipeline:
            // has_clearcoat, has_transmission, has_volume, has_ior, has specular, has_sheen
        }

        // Visit each node in the hierarchy and process any mesh linked from it.
        // Each primitive within a glTF node becomes a Raylib Mesh.
        // The local-to-world transform of each node is used to transform the
        // points/normals/tangents of the created Mesh(es).
        // Any glTF mesh linked from more than one Node (i.e. instancing)
        // is turned into multiple Mesh's, as each Node will have its own
        // transform applied.
        // Note: the code below disregards the scenes defined in the file, all nodes are used.
        //----------------------------------------------------------------------------------------------------
        int meshIndex = 0;
        for (unsigned int i = 0; i < data->nodes_count; i++)
        {
            cgltf_node *node = &(data->nodes[i]);

            cgltf_mesh *mesh = node->mesh;
            if (!mesh)
                continue;

            cgltf_float worldTransform[16];
            cgltf_node_transform_world(node, worldTransform);

            Matrix worldMatrix = {
                worldTransform[0], worldTransform[4], worldTransform[8], worldTransform[12],
                worldTransform[1], worldTransform[5], worldTransform[9], worldTransform[13],
                worldTransform[2], worldTransform[6], worldTransform[10], worldTransform[14],
                worldTransform[3], worldTransform[7], worldTransform[11], worldTransform[15]
            };

            Matrix worldMatrixNormals = MatrixTranspose(MatrixInvert(worldMatrix));

            for (unsigned int p = 0; p < mesh->primitives_count; p++)
            {
                // NOTE: We only support primitives defined by triangles
                // Other alternatives: points, lines, line_strip, triangle_strip
                if (mesh->primitives[p].type != cgltf_primitive_type_triangles) continue;

                // NOTE: Attributes data could be provided in several data formats (8, 8u, 16u, 32...),
                // Only some formats for each attribute type are supported, read info at the top of this function!

                for (unsigned int j = 0; j < mesh->primitives[p].attributes_count; j++)
                {
                    // Check the different attributes for every primitive
                    if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_position)      // POSITION, vec3, float
                    {
                        cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;

                        // WARNING: SPECS: POSITION accessor MUST have its min and max properties defined

                        if ((attribute->type == cgltf_type_vec3) && (attribute->component_type == cgltf_component_type_r_32f))
                        {
                            // Init raylib mesh vertices to copy glTF attribute data
                            model.meshes[meshIndex].vertexCount = (int)attribute->count;
                            model.meshes[meshIndex].vertices = (float*)RL_MALLOC(attribute->count*3*sizeof(float));

                            // Load 3 components of float data type into mesh.vertices
                            LOAD_ATTRIBUTE(attribute, 3, float, model.meshes[meshIndex].vertices)

                            // Transform the vertices
                            float *vertices = model.meshes[meshIndex].vertices;
                            for (unsigned int k = 0; k < attribute->count; k++)
                            {
                                Vector3 vt = worldMatrix * Vector3{ vertices[3*k], vertices[3*k+1], vertices[3*k+2] };
                                //Vector3 vt = Vector3Transform((Vector3){ vertices[3*k], vertices[3*k+1], vertices[3*k+2] }, worldMatrix);
                                vertices[3*k] = vt.x;
                                vertices[3*k+1] = vt.y;
                                vertices[3*k+2] = vt.z;
                            }
                        }
                        else TRACELOG(LOG_WARNING, "MODEL: [%s] Vertices attribute data format not supported, use vec3 float", fileName);
                    }
                    else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_normal)   // NORMAL, vec3, float
                    {
                        cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;

                        if ((attribute->type == cgltf_type_vec3) && (attribute->component_type == cgltf_component_type_r_32f))
                        {
                            // Init raylib mesh normals to copy glTF attribute data
                            model.meshes[meshIndex].normals = (float*)RL_MALLOC(attribute->count*3*sizeof(float));

                            // Load 3 components of float data type into mesh.normals
                            LOAD_ATTRIBUTE(attribute, 3, float, model.meshes[meshIndex].normals)

                            // Transform the normals
                            float *normals = model.meshes[meshIndex].normals;
                            for (unsigned int k = 0; k < attribute->count; k++)
                            {
                                Vector3 nt = worldMatrixNormals * Vector3{ normals[3*k], normals[3*k+1], normals[3*k+2] };
                                normals[3*k] = nt.x;
                                normals[3*k+1] = nt.y;
                                normals[3*k+2] = nt.z;
                            }
                        }
                        else TRACELOG(LOG_WARNING, "MODEL: [%s] Normal attribute data format not supported, use vec3 float", fileName);
                    }
                    else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_tangent)   // TANGENT, vec3, float
                    {
                        cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;

                        if ((attribute->type == cgltf_type_vec4) && (attribute->component_type == cgltf_component_type_r_32f))
                        {
                            // Init raylib mesh tangent to copy glTF attribute data
                            model.meshes[meshIndex].tangents = (float*)RL_MALLOC(attribute->count*4*sizeof(float));

                            // Load 4 components of float data type into mesh.tangents
                            LOAD_ATTRIBUTE(attribute, 4, float, model.meshes[meshIndex].tangents)

                            // Transform the tangents
                            float *tangents = model.meshes[meshIndex].tangents;
                            for (unsigned int k = 0; k < attribute->count; k++)
                            {
                                Vector3 tt = worldMatrix * Vector3{tangents[3*k], tangents[3*k+1], tangents[3*k+2]};
                                tangents[3*k] = tt.x;
                                tangents[3*k+1] = tt.y;
                                tangents[3*k+2] = tt.z;
                            }
                        }
                        else TRACELOG(LOG_WARNING, "MODEL: [%s] Tangent attribute data format not supported, use vec4 float", fileName);
                    }
                    else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_texcoord) // TEXCOORD_n, vec2, float/u8n/u16n
                    {
                        // Support up to 2 texture coordinates attributes
                        float *texcoordPtr = NULL;

                        cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;

                        if (attribute->type == cgltf_type_vec2)
                        {
                            if (attribute->component_type == cgltf_component_type_r_32f)  // vec2, float
                            {
                                // Init raylib mesh texcoords to copy glTF attribute data
                                texcoordPtr = (float *)RL_MALLOC(attribute->count*2*sizeof(float));

                                // Load 3 components of float data type into mesh.texcoords
                                LOAD_ATTRIBUTE(attribute, 2, float, texcoordPtr)
                            }
                            else if (attribute->component_type == cgltf_component_type_r_8u) // vec2, u8n
                            {
                                // Init raylib mesh texcoords to copy glTF attribute data
                                texcoordPtr = (float *)RL_MALLOC(attribute->count*2*sizeof(float));

                                // Load data into a temp buffer to be converted to raylib data type
                                unsigned char *temp = (unsigned char *)RL_MALLOC(attribute->count*2*sizeof(unsigned char));
                                LOAD_ATTRIBUTE(attribute, 2, unsigned char, temp)

                                // Convert data to raylib texcoord data type (float)
                                for (unsigned int t = 0; t < attribute->count*2; t++) texcoordPtr[t] = (float)temp[t]/255.0f;

                                RL_FREE(temp);
                            }
                            else if (attribute->component_type == cgltf_component_type_r_16u) // vec2, u16n
                            {
                                // Init raylib mesh texcoords to copy glTF attribute data
                                texcoordPtr = (float *)RL_MALLOC(attribute->count*2*sizeof(float));

                                // Load data into a temp buffer to be converted to raylib data type
                                unsigned short *temp = (unsigned short *)RL_MALLOC(attribute->count*2*sizeof(unsigned short));
                                LOAD_ATTRIBUTE(attribute, 2, unsigned short, temp)

                                // Convert data to raylib texcoord data type (float)
                                for (unsigned int t = 0; t < attribute->count*2; t++) texcoordPtr[t] = (float)temp[t]/65535.0f;

                                RL_FREE(temp);
                            }
                            else TRACELOG(LOG_WARNING, "MODEL: [%s] Texcoords attribute data format not supported", fileName);
                        }
                        else TRACELOG(LOG_WARNING, "MODEL: [%s] Texcoords attribute data format not supported, use vec2 float", fileName);

                        int index = mesh->primitives[p].attributes[j].index;
                        if (index == 0) model.meshes[meshIndex].texcoords = texcoordPtr;
                        else if (index == 1) model.meshes[meshIndex].texcoords2 = texcoordPtr;
                        else
                        {
                            TRACELOG(LOG_WARNING, "MODEL: [%s] No more than 2 texture coordinates attributes supported", fileName);
                            if (texcoordPtr != NULL) RL_FREE(texcoordPtr);
                        }
                    }
                    else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_color)    // COLOR_n, vec3/vec4, float/u8n/u16n
                    {
                        cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;

                        // WARNING: SPECS: All components of each COLOR_n accessor element MUST be clamped to [0.0, 1.0] range

                        if (attribute->type == cgltf_type_vec3)  // RGB
                        {
                            if (attribute->component_type == cgltf_component_type_r_8u)
                            {
                                // Init raylib mesh color to copy glTF attribute data
                                model.meshes[meshIndex].colors = (float*)RL_MALLOC(attribute->count*4*sizeof(float));

                                // Load data into a temp buffer to be converted to raylib data type
                                unsigned char *temp = (unsigned char*)RL_MALLOC(attribute->count*3*sizeof(unsigned char));

                                LOAD_ATTRIBUTE(attribute, 3, unsigned char, temp)

                                // Convert data to raylib color data type (4 bytes)
                                for (unsigned int c = 0, k = 0; c < (attribute->count*4 - 3); c += 4, k += 3)
                                {
                                    model.meshes[meshIndex].colors[c] = temp[k] / 255.0f;
                                    model.meshes[meshIndex].colors[c + 1] = temp[k + 1] / 255.0f;
                                    model.meshes[meshIndex].colors[c + 2] = temp[k + 2] / 255.0f;
                                    model.meshes[meshIndex].colors[c + 3] = 1.0f;
                                }

                                RL_FREE(temp);
                            }
                            else if (attribute->component_type == cgltf_component_type_r_16u)
                            {
                                // Init raylib mesh color to copy glTF attribute data
                                model.meshes[meshIndex].colors = (float*)RL_MALLOC(attribute->count * 4 * sizeof(float));

                                // Load data into a temp buffer to be converted to raylib data type
                                unsigned short *temp = (unsigned short *)RL_MALLOC(attribute->count*3*sizeof(unsigned short));
                                LOAD_ATTRIBUTE(attribute, 3, unsigned short, temp)

                                // Convert data to raylib color data type (4 bytes)
                                for (unsigned int c = 0, k = 0; c < (attribute->count*4 - 3); c += 4, k += 3)
                                {
                                    model.meshes[meshIndex].colors[c] = (unsigned char)(((float)temp[k]/65535.0f));
                                    model.meshes[meshIndex].colors[c + 1] = (unsigned char)(((float)temp[k + 1]/65535.0f));
                                    model.meshes[meshIndex].colors[c + 2] = (unsigned char)(((float)temp[k + 2]/65535.0f));
                                    model.meshes[meshIndex].colors[c + 3] = 1.0f;
                                }

                                RL_FREE(temp);
                            }
                            else if (attribute->component_type == cgltf_component_type_r_32f)
                            {
                                // Init raylib mesh color to copy glTF attribute data
                                model.meshes[meshIndex].colors = (float*)RL_MALLOC(attribute->count*4*sizeof(float));

                                // Load data into a temp buffer to be converted to raylib data type
                                float *temp = (float *)RL_MALLOC(attribute->count*3*sizeof(float));
                                LOAD_ATTRIBUTE(attribute, 3, float, temp)

                                // Convert data to raylib color data type (4 bytes)
                                for (unsigned int c = 0, k = 0; c < (attribute->count*4 - 3); c += 4, k += 3)
                                {
                                    model.meshes[meshIndex].colors[c]     = (unsigned char)(temp[k]);
                                    model.meshes[meshIndex].colors[c + 1] = (unsigned char)(temp[k + 1]);
                                    model.meshes[meshIndex].colors[c + 2] = (unsigned char)(temp[k + 2]);
                                    model.meshes[meshIndex].colors[c + 3] = 1.0f;
                                }

                                RL_FREE(temp);
                            }
                            else TRACELOG(LOG_WARNING, "MODEL: [%s] Color attribute data format not supported", fileName);
                        }
                        else if (attribute->type == cgltf_type_vec4) // RGBA
                        {
                            if (attribute->component_type == cgltf_component_type_r_8u)
                            {
                                // Init raylib mesh color to copy glTF attribute data
                                model.meshes[meshIndex].colors = (float*)RL_MALLOC(attribute->count*4*sizeof(float));

                                unsigned char *temp = (unsigned char*)RL_MALLOC(attribute->count*4*sizeof(unsigned char));
                                LOAD_ATTRIBUTE(attribute, 4, unsigned char, temp)
                                for (unsigned int c = 0; c < attribute->count*4; c++) model.meshes[meshIndex].colors[c] = (((float)temp[c]/255.0f));

                                RL_FREE(temp);
                            }
                            else if (attribute->component_type == cgltf_component_type_r_16u)
                            {
                                // Init raylib mesh color to copy glTF attribute data
                                model.meshes[meshIndex].colors = (float*)RL_MALLOC(attribute->count*4*sizeof(float));

                                // Load data into a temp buffer to be converted to raylib data type
                                unsigned short *temp = (unsigned short*)RL_MALLOC(attribute->count*4*sizeof(unsigned short));
                                LOAD_ATTRIBUTE(attribute, 4, unsigned short, temp)

                                // Convert data to raylib color data type (4 bytes)
                                for (unsigned int c = 0; c < attribute->count*4; c++) model.meshes[meshIndex].colors[c] = (((float)temp[c]/65535.0f));

                                RL_FREE(temp);
                            }
                            else if (attribute->component_type == cgltf_component_type_r_32f)
                            {
                                // Init raylib mesh color to copy glTF attribute data
                                model.meshes[meshIndex].colors = (float*)RL_MALLOC(attribute->count*4*sizeof(float));

                                // Load data into a temp buffer to be converted to raylib data type
                                float *temp = (float*)RL_MALLOC(attribute->count*4*sizeof(float));
                                LOAD_ATTRIBUTE(attribute, 4, float, temp)

                                // Convert data to raylib color data type (4 bytes), we expect the color data normalized
                                for (unsigned int c = 0; c < attribute->count*4; c++) model.meshes[meshIndex].colors[c] = (temp[c]);

                                RL_FREE(temp);
                            }
                            else TRACELOG(LOG_WARNING, "MODEL: [%s] Color attribute data format not supported", fileName);
                        }
                        else TRACELOG(LOG_WARNING, "MODEL: [%s] Color attribute data format not supported", fileName);
                    }

                    // NOTE: Attributes related to animations are processed separately
                }

                // Load primitive indices data (if provided)
                if ((mesh->primitives[p].indices != NULL) && (mesh->primitives[p].indices->buffer_view != NULL))
                {
                    cgltf_accessor *attribute = mesh->primitives[p].indices;

                    model.meshes[meshIndex].triangleCount = (int)attribute->count/3;

                    if (attribute->component_type == cgltf_component_type_r_16u)
                    {
                        // Init raylib mesh indices to copy glTF attribute data
                        model.meshes[meshIndex].indices = (uint32_t*)RL_MALLOC(attribute->count*sizeof(uint32_t));

                        // Load unsigned short data type into mesh.indices
                        LOAD_ATTRIBUTE(attribute, 1, unsigned short, model.meshes[meshIndex].indices)
                    }
                    else if (attribute->component_type == cgltf_component_type_r_8u)
                    {
                        // Init raylib mesh indices to copy glTF attribute data
                        model.meshes[meshIndex].indices = (uint32_t*)RL_MALLOC(attribute->count*sizeof(uint32_t));
                        LOAD_ATTRIBUTE_CAST(attribute, 1, unsigned char, model.meshes[meshIndex].indices, uint32_t)

                    }
                    else if (attribute->component_type == cgltf_component_type_r_32u)
                    {
                        // Init raylib mesh indices to copy glTF attribute data
                        model.meshes[meshIndex].indices = (uint32_t*)RL_MALLOC(attribute->count*sizeof(uint32_t));
                        LOAD_ATTRIBUTE_CAST(attribute, 1, unsigned int, model.meshes[meshIndex].indices, uint32_t)

                        TRACELOG(LOG_WARNING, "MODEL: [%s] Indices data converted from u32 to u16, possible loss of data", fileName);
                    }
                    else
                    {
                        TRACELOG(LOG_WARNING, "MODEL: [%s] Indices data format not supported, use u16", fileName);
                    }
                }
                else model.meshes[meshIndex].triangleCount = model.meshes[meshIndex].vertexCount/3;    // Unindexed mesh

                // Assign to the primitive mesh the corresponding material index
                // NOTE: If no material defined, mesh uses the already assigned default material (index: 0)
                for (unsigned int m = 0; m < data->materials_count; m++)
                {
                    // The primitive actually keeps the pointer to the corresponding material,
                    // raylib instead assigns to the mesh the by its index, as loaded in model.materials array
                    // To get the index, we check if material pointers match, and we assign the corresponding index,
                    // skipping index 0, the default material
                    if (&data->materials[m] == mesh->primitives[p].material)
                    {
                        model.meshMaterial[meshIndex] = m + 1;
                        break;
                    }
                }

                meshIndex++;       // Move to next mesh
            }
        }

        // Load glTF meshes animation data
        // REF: https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#skins
        // REF: https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#skinned-mesh-attributes
        //
        // LIMITATIONS:
        //  - Only supports 1 armature per file, and skips loading it if there are multiple armatures
        //  - Only supports linear interpolation (default method in Blender when checked "Always Sample Animations" when exporting a GLTF file)
        //  - Only supports translation/rotation/scale animation channel.path, weights not considered (i.e. morph targets)
        //----------------------------------------------------------------------------------------------------
        if (data->skins_count > 0)
        {
            cgltf_skin skin = data->skins[0];
            model.bones = LoadBoneInfoGLTF(skin, &model.boneCount);
            model.bindPose = (Transform*)RL_MALLOC(model.boneCount*sizeof(Transform));

            for (int i = 0; i < model.boneCount; i++)
            {
                cgltf_node *node = skin.joints[i];
                cgltf_float worldTransform[16];
                cgltf_node_transform_world(node, worldTransform);
                Matrix worldMatrix = {
                    worldTransform[0], worldTransform[4], worldTransform[8], worldTransform[12],
                    worldTransform[1], worldTransform[5], worldTransform[9], worldTransform[13],
                    worldTransform[2], worldTransform[6], worldTransform[10], worldTransform[14],
                    worldTransform[3], worldTransform[7], worldTransform[11], worldTransform[15]
                };
                MatrixDecompose(worldMatrix, &(model.bindPose[i].translation), &(model.bindPose[i].rotation), &(model.bindPose[i].scale));
            }
        }
        if (data->skins_count > 1)
        {
            TRACELOG(LOG_WARNING, "MODEL: [%s] can only load one skin (armature) per model, but gltf skins_count == %i", fileName, data->skins_count);
        }

        meshIndex = 0;
        for (unsigned int i = 0; i < data->nodes_count; i++)
        {
            cgltf_node *node = &(data->nodes[i]);

            cgltf_mesh *mesh = node->mesh;
            if (!mesh)
                continue;

            for (unsigned int p = 0; p < mesh->primitives_count; p++)
            {
                // NOTE: We only support primitives defined by triangles
                if (mesh->primitives[p].type != cgltf_primitive_type_triangles) continue;

                for (unsigned int j = 0; j < mesh->primitives[p].attributes_count; j++)
                {
                    // NOTE: JOINTS_1 + WEIGHT_1 will be used for +4 joints influencing a vertex -> Not supported by raylib

                    if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_joints) // JOINTS_n (vec4: 4 bones max per vertex / u8, u16)
                    {
                        cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;

                        // NOTE: JOINTS_n can only be vec4 and u8/u16
                        // SPECS: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#meshes-overview

                        // WARNING: raylib only supports model.meshes[].boneIds as u8 (unsigned char),
                        // if data is provided in any other format, it is converted to supported format but
                        // it could imply data loss (a warning message is issued in that case)

                        if (attribute->type == cgltf_type_vec4)
                        {
                            if (attribute->component_type == cgltf_component_type_r_8u)
                            {
                                // Init raylib mesh boneIds to copy glTF attribute data
                                model.meshes[meshIndex].boneIds = (unsigned char*)RL_CALLOC(model.meshes[meshIndex].vertexCount*4, sizeof(unsigned char));

                                // Load attribute: vec4, u8 (unsigned char)
                                LOAD_ATTRIBUTE(attribute, 4, unsigned char, model.meshes[meshIndex].boneIds)
                            }
                            else if (attribute->component_type == cgltf_component_type_r_16u)
                            {
                                // Init raylib mesh boneIds to copy glTF attribute data
                                model.meshes[meshIndex].boneIds = (unsigned char*)RL_CALLOC(model.meshes[meshIndex].vertexCount*4, sizeof(unsigned char));

                                // Load data into a temp buffer to be converted to raylib data type
                                unsigned short *temp = (unsigned short*)RL_CALLOC(model.meshes[meshIndex].vertexCount*4, sizeof(unsigned short));
                                LOAD_ATTRIBUTE(attribute, 4, unsigned short, temp)

                                // Convert data to raylib color data type (4 bytes)
                                bool boneIdOverflowWarning = false;
                                for (int b = 0; b < model.meshes[meshIndex].vertexCount*4; b++)
                                {
                                    if ((temp[b] > 255) && !boneIdOverflowWarning)
                                    {
                                        TRACELOG(LOG_WARNING, "MODEL: [%s] Joint attribute data format (u16) overflow", fileName);
                                        boneIdOverflowWarning = true;
                                    }

                                    // Despite the possible overflow, we convert data to unsigned char
                                    model.meshes[meshIndex].boneIds[b] = (unsigned char)temp[b];
                                }

                                RL_FREE(temp);
                            }
                            else TRACELOG(LOG_WARNING, "MODEL: [%s] Joint attribute data format not supported", fileName);
                        }
                        else TRACELOG(LOG_WARNING, "MODEL: [%s] Joint attribute data format not supported", fileName);
                    }
                    else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_weights)  // WEIGHTS_n (vec4, u8n/u16n/f32)
                    {
                        cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;

                        if (attribute->type == cgltf_type_vec4)
                        {
                            // TODO: Support component types: u8, u16?
                            if (attribute->component_type == cgltf_component_type_r_8u)
                            {
                                // Init raylib mesh bone weight to copy glTF attribute data
                                model.meshes[meshIndex].boneWeights = (float*)RL_CALLOC(model.meshes[meshIndex].vertexCount*4, sizeof(float));

                                // Load data into a temp buffer to be converted to raylib data type
                                unsigned char *temp = (unsigned char*)RL_MALLOC(attribute->count*4*sizeof(unsigned char));
                                LOAD_ATTRIBUTE(attribute, 4, unsigned char, temp)

                                // Convert data to raylib bone weight data type (4 bytes)
                                for (unsigned int b = 0; b < attribute->count*4; b++) model.meshes[meshIndex].boneWeights[b] = (float)temp[b]/255.0f;

                                RL_FREE(temp);
                            }
                            else if (attribute->component_type == cgltf_component_type_r_16u)
                            {
                                // Init raylib mesh bone weight to copy glTF attribute data
                                model.meshes[meshIndex].boneWeights = (float*)RL_CALLOC(model.meshes[meshIndex].vertexCount*4, sizeof(float));

                                // Load data into a temp buffer to be converted to raylib data type
                                unsigned short *temp = (unsigned short*)RL_MALLOC(attribute->count*4*sizeof(unsigned short));
                                LOAD_ATTRIBUTE(attribute, 4, unsigned short, temp)

                                // Convert data to raylib bone weight data type
                                for (unsigned int b = 0; b < attribute->count*4; b++) model.meshes[meshIndex].boneWeights[b] = (float)temp[b]/65535.0f;

                                RL_FREE(temp);
                            }
                            else if (attribute->component_type == cgltf_component_type_r_32f)
                            {
                                // Init raylib mesh bone weight to copy glTF attribute data
                                model.meshes[meshIndex].boneWeights = (float*)RL_CALLOC(model.meshes[meshIndex].vertexCount*4, sizeof(float));

                                // Load 4 components of float data type into mesh.boneWeights
                                // for cgltf_attribute_type_weights we have:
                                //   - data.meshes[0] (256 vertices)
                                //   - 256 values, provided as cgltf_type_vec4 of float (4 byte per joint, stride 16)
                                LOAD_ATTRIBUTE(attribute, 4, float, model.meshes[meshIndex].boneWeights)
                            }
                            else TRACELOG(LOG_WARNING, "MODEL: [%s] Joint weight attribute data format not supported, use vec4 float", fileName);
                        }
                        else TRACELOG(LOG_WARNING, "MODEL: [%s] Joint weight attribute data format not supported, use vec4 float", fileName);
                    }
                }

                // Animated vertex data
                model.meshes[meshIndex].animVertices = (float*)RL_CALLOC(model.meshes[meshIndex].vertexCount*3, sizeof(float));
                memcpy(model.meshes[meshIndex].animVertices, model.meshes[meshIndex].vertices, model.meshes[meshIndex].vertexCount*3*sizeof(float));
                model.meshes[meshIndex].animNormals = (float*)RL_CALLOC(model.meshes[meshIndex].vertexCount*3, sizeof(float));
                if (model.meshes[meshIndex].normals != NULL)
                {
                    memcpy(model.meshes[meshIndex].animNormals, model.meshes[meshIndex].normals, model.meshes[meshIndex].vertexCount*3*sizeof(float));
                }

                // Bone Transform Matrices
                model.meshes[meshIndex].boneCount = model.boneCount;
                model.meshes[meshIndex].boneMatrices = (Matrix*)RL_CALLOC(model.meshes[meshIndex].boneCount, sizeof(Matrix));

                for (int j = 0; j < model.meshes[meshIndex].boneCount; j++)
                {
                    model.meshes[meshIndex].boneMatrices[j] = MatrixIdentity();
                }

                meshIndex++;       // Move to next mesh
            }

        }

        // Free all cgltf loaded data
        cgltf_free(data);
    }
    else TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load glTF data", fileName);

    // WARNING: cgltf requires the file pointer available while reading data
    UnloadFileData(fileData);

    return model;
}

Model LoadModel(const char *fileName){
    
    Model model zeroinit;

    if (IsFileExtension(fileName, ".obj")) model = LoadOBJ(fileName);
    else if (IsFileExtension(fileName, ".glb")) model = LoadGLTF(fileName);
    return model;
}