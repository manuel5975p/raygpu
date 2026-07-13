// Links against the installed raygpu package; referencing window and drawing
// entry points forces the linker to resolve the bundled SDL3/wgvk/glslang objects.
#include <raygpu.h>

int main(int argc, char **argv) {
    (void)argv;
    if (argc > 1) { // never taken in CI: link-time smoke test only
        InitWindow(640, 480, "test_package");
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawFPS(0, 0);
            EndDrawing();
        }
        CloseProgram();
    }
    return 0;
}
