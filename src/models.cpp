#include <raygpu.h>

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
    cubeMesh.vertices = (float*)calloc(vertexCount, sizeof(float) * 3);
    cubeMesh.texcoords = (float*)calloc(vertexCount, sizeof(float) * 2);
    cubeMesh.normals = (float*)calloc(vertexCount, sizeof(float) * 3);
    cubeMesh.colors = (float*)calloc(vertexCount, sizeof(float) * 4);
    cubeMesh.indices = (uint32_t*)calloc(36, sizeof(uint32_t));
    
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