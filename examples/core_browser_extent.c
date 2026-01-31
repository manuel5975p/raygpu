#include <raygpu.h>

void setup(void){
}

void render(void){
    BeginDrawing();
    ClearBackground((Color){40, 44, 52, 255});

    const char* title = "Browser Extent Example";
    int fontSize = 40;
    DrawText(title, GetScreenWidth() / 2 - MeasureText(title, fontSize) / 2, 40, fontSize, WHITE);

    const char* sizeText = TextFormat("Canvas: %d x %d", GetScreenWidth(), GetScreenHeight());
    DrawText(sizeText, GetScreenWidth() / 2 - MeasureText(sizeText, 20) / 2, 100, 20, (Color){150, 150, 150, 255});

    DrawText(
        "Resize your browser window - the canvas follows",
        GetScreenWidth() / 2 - MeasureText("Resize your browser window - the canvas follows", 20) / 2,
        140, 20, (Color){100, 200, 100, 255}
    );

    DrawRectangleLinesEx((Rectangle){2, 2, (float)GetScreenWidth() - 4, (float)GetScreenHeight() - 4}, 2.0f, (Color){80, 80, 80, 255});

    DrawCircleSector(GetMousePosition(), 30.0f, 0.0f, 360.0f, 64, (Color){100, 149, 237, 200});
    DrawFPS(10, 10);
    if(IsKeyPressed(KEY_F)){
        ToggleFullscreen();
    }
    EndDrawing();
}

int main(void){
    SetConfigFlags(FLAG_WINDOW_RESIZE_TO_BROWSER_EXTENT | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    ProgramInfo progInfo = {
        .windowTitle = "Browser Extent",
        .windowWidth = 800,
        .windowHeight = 600,
        .setupFunction = setup,
        .renderFunction = render
    };
    InitProgram(progInfo);
}
