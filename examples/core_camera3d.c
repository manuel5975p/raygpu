#include <raygpu.h>

int main(void){
    RequestBackend(WGPUBackendType_D3D12);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1000, 1000, "Camera3D Example");
    Camera3D cam = (Camera3D){
        .position = (Vector3){0,0,10},
        .target = (Vector3){0,0,0},
        .up = (Vector3){0,1,0},
        .fovy = 50.0f
    };
    float angle = 0.0f;
    while(!WindowShouldClose()){        
        BeginDrawing();
        
        angle += GetFrameTime();
        cam.position = (Vector3){sinf(angle) * 10.f, 0.0f, cosf(angle) * 10.f};

        ClearBackground(BLACK);
        BeginMode3D(cam);
        UseNoTexture();
        rlBegin(RL_QUADS);
        rlColor4ub(220, 220, 220, 255);
        rlVertex3f(-1, -1, 0);
        rlVertex3f( 1, -1, 0);
        rlVertex3f( 1,  1, 0);
        rlVertex3f(-1,  1, 0);
        rlColor4ub(220, 70, 70, 255);
        rlVertex3f( 1, -1, 0);
        rlVertex3f( 1, -1, 2);
        rlVertex3f( 1,  1, 2);
        rlVertex3f( 1,  1, 0);
        rlColor4ub(70, 220, 70, 255);
        rlVertex3f(-1, -1, 0);
        rlVertex3f(-1, -1, 2);
        rlVertex3f(-1,  1, 2);
        rlVertex3f(-1,  1, 0);
        rlEnd();
        
        EndMode3D();
        DrawText("This is in screen space", 0, 100, 30, RED);
        DrawFPS(0,0);
        EndDrawing();
    }
    return 0;
}

