#include <raygpu.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <tinyobj_loader_c.h>
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
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
    for(int i = 0; i < 36; i += 6){
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
    memcpy(cubeMesh.normals, normals, vertexCount * sizeof(float) * 4);
    cubeMesh.vertexCount = vertexCount;
    std::fill(cubeMesh.colors, cubeMesh.colors + vertexCount * 4, 1.0f);
    UploadMesh(&cubeMesh, false);
    return cubeMesh;
}
void UploadMesh(Mesh *mesh, bool dynamic){
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
    SetStorageBuffer(3, &trfBuffer);
    BindVertexArray(GetActivePipeline(), mesh.vao);
    if(mesh.ibo.buffer){
        DrawArraysIndexed(mesh.ibo, mesh.triangleCount * 3);
    }else{
        DrawArraysInstanced(mesh.vertexCount, instances);
    }
    //wgpuBufferRelease(trfBuffer.buffer);
}
extern "C" void DrawMesh(Mesh mesh, Material material, Matrix transform){
    SetStorageBufferData(3, &transform, sizeof(Matrix));
    BindVertexArray(GetActivePipeline(), mesh.vao);
    if(mesh.ibo.buffer){
        DrawArraysIndexed(mesh.ibo, mesh.triangleCount * 3);
    }else{
        DrawArrays(mesh.vertexCount);
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
Model LoadModel(const char *fileName){
    
    Model model zeroinit;

    if (IsFileExtension(fileName, ".obj")) model = LoadOBJ(fileName);
    return model;
}