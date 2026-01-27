#include <raygpu.h>

const int screenWidth = 800;
const int screenHeight = 450;

Vector2 ballPosition = { 400, 225 };
Color ballColor = {0, 82, 172, 255};

void setup(void){
    SetTargetFPS(60);
}

void render(void) {
    if (IsGamepadAvailable(0)) {
        
        
        ballPosition.x += GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) * 5.0f;
        ballPosition.y += GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) * 5.0f;        
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_SOUTH))
            ballColor = GREEN;
        else if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_EAST)){
            ballColor = RED;
        }
        else if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_WEST)){
            ballColor = BLUE;
        }
        else if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_NORTH)){
            ballColor = YELLOW;
        }
        else ballColor = DARKBLUE;
    }

    BeginDrawing();
        ClearBackground(RAYWHITE);

        if (IsGamepadAvailable(0)) {
            DrawText(GetGamepadName(0), 10, 10, 20, GRAY);
            DrawText("Left stick to move, face buttons to change color", 10, 40, 20, GRAY);

            DrawText(TextFormat("Left X: %.2f", GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X)), 10, 80, 20, BLACK);
            DrawText(TextFormat("Left Y: %.2f", GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)), 10, 110, 20, BLACK);
            DrawText(TextFormat("Right X: %.2f", GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X)), 10, 140, 20, BLACK);
            DrawText(TextFormat("Right Y: %.2f", GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y)), 10, 170, 20, BLACK);
            DrawText(TextFormat("L Trigger: %.2f", GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_TRIGGER)), 10, 200, 20, BLACK);
            DrawText(TextFormat("R Trigger: %.2f", GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER)), 10, 230, 20, BLACK);

            DrawCircleV(ballPosition, 50, ballColor);
        } else {
            DrawText("No gamepad detected", screenWidth/2 - 100, screenHeight/2, 20, GRAY);
            DrawText("Connect a controller and it will be detected automatically", screenWidth/2 - 220, screenHeight/2 + 30, 20, LIGHTGRAY);
        }

    EndDrawing();
}

int main(void) {
    ProgramInfo programInfo = {
        .windowWidth = screenWidth,
        .windowHeight = screenHeight,
        .windowTitle = "Gamepad Input Example",
        .setupFunction = setup,
        .renderFunction = render
    };

    InitProgram(programInfo);
    
    while (!WindowShouldClose()) {
        render();
    }

    return 0;
}
