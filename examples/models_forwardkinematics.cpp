#include "raygpu.h" // Include your provided math header
#include <vector>      // For storing points dynamically (optional but clean)
#include <cmath>       // For sinf

// --- FK Function adapted for MathUtils.h ---

// Helper to compute the transformation matrix for a single joint (MDH)
// Uses COLUMN-MAJOR matrices as defined/used in MathUtils.h
void compute_mdh_transform_mu(float a_i_minus_1, float alpha_i_minus_1, float d_i, float theta_i, Matrix *T) {
    float ct = cosf(theta_i);
    float st = sinf(theta_i);
    float ca = cosf(alpha_i_minus_1);
    float sa = sinf(alpha_i_minus_1);

    // Column 0
    T->data[0] = ct;        // m0
    T->data[1] = st * ca;   // m1
    T->data[2] = st * sa;   // m2
    T->data[3] = 0.0f;      // m3

    // Column 1
    T->data[4] = -st;       // m4
    T->data[5] = ct * ca;   // m5
    T->data[6] = ct * sa;   // m6
    T->data[7] = 0.0f;      // m7

    // Column 2
    T->data[8] = 0.0f;      // m8
    T->data[9] = -sa;       // m9
    T->data[10] = ca;       // m10
    T->data[11] = 0.0f;     // m11

    // Column 3 (Translation)
    T->data[12] = a_i_minus_1; // m12 (x)
    T->data[13] = -sa * d_i;   // m13 (y)
    T->data[14] = ca * d_i;    // m14 (z)
    T->data[15] = 1.0f;        // m15
}

// Main FK Calculation using MathUtils types
void calculate_tx2_90_fk_mu(const float q[6], Matrix T_out[7]) {
    // Stäubli TX2-90 Modified Denavit-Hartenberg Parameters (in meters and radians)
    const float a[]     = {0.0f, 0.124f, 0.425f, 0.0f, 0.0f, 0.0f};
    const float alpha[] = {0.0f, M_PI_2, 0.0f,   M_PI_2, -M_PI_2, M_PI_2};
    const float d[]     = {0.478f, 0.0f, 0.0f, 0.425f, 0.0f, 0.100f};
    // theta_i uses q[i]

    Matrix T_prev_curr zeroinit; // Transform {i-1} to {i}

    // T_out[0] is Base Frame relative to Base (Identity)
    T_out[0] = MatrixIdentity();

    // Calculate cumulative transformations: T_out[i+1] = ^0_T_{i+1}
    for (int i = 0; i < 6; ++i) {
        // Compute T_{i}_{i+1} (transform from frame i to frame i+1)
        compute_mdh_transform_mu(a[i], alpha[i], d[i], q[i], &T_prev_curr);

        // Calculate ^0_T_{i+1} = ^0_T_i * ^{i}_T_{i+1}
        // T_out[i+1] = MatrixMultiply(T_out[i], T_prev_curr)
        // Important: MathUtils.h MatrixMultiply(A, B) calculates A * B
        T_out[i + 1] = MatrixMultiply(T_out[i], T_prev_curr);
    }
}

// Function to extract position from a Matrix
Vector3 GetMatrixPosition(const Matrix& m) {
    // For column-major matrix: translation is in m12, m13, m14
    return { m.m12, m.m13, m.m14 };
    // Or equivalently: return { m.data[12], m.data[13], m.data[14] };
}


// --- Main Program ---
int main(void) {
    // Initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Stäubli TX2-90 FK Visualization");

    // Define the camera to look at the robot
    Camera3D camera = { 0 };
    camera.position = { 2.0f, 2.0f, 1.5f }; // Camera position (Adjust as needed)
    camera.target = { 0.0f, 0.0f, 0.6f };   // Camera looking point (center of base-ish)
    camera.up = { 0.0f, 0.0f, 1.0f };       // Camera up vector (Z-up, matching typical robot base)
    camera.fovy = 45.0f;                    // Camera field-of-view Y

    // Robot state
    float joint_angles[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; // Initial joint angles (radians)
    Matrix joint_transforms[7]; // Store T_0_0 to T_0_6

    // Joint colors (optional)
    Color joint_colors[7] = {
        GRAY, RED, GREEN, BLUE, YELLOW, PURPLE, ORANGE
    };

    float time = 0.0f;

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    // Main game loop
    while (!WindowShouldClose()){
        // Update

        time += GetFrameTime();
        // Simple animation: Oscillate joint 2 (shoulder) and joint 4 (wrist)
        joint_angles[1] = sinf(time * 0.8f) * 0.8f - M_PI_2/2.0f; // Make J2 move around -pi/4
        joint_angles[3] = cosf(time * 1.1f) * 1.5f; // Make J4 move

        // Calculate Forward Kinematics
        calculate_tx2_90_fk_mu(joint_angles, joint_transforms);

        // Extract joint positions
        std::vector<Vector3> joint_positions(7);
        for(int i = 0; i < 7; ++i) {
            joint_positions[i] = GetMatrixPosition(joint_transforms[i]);
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        {
            DrawGrid(10, 0.5f); // Draw a grid

            // Draw Bones (Lines connecting joints)
            // Start from joint 1, connect back to joint 0 (base origin)
            // Connect joint i to joint i-1
            for (int i = 1; i < 7; ++i) {
                DrawLine3D(joint_positions[i-1], joint_positions[i], DARKGRAY);
                // Draw thicker "bones" using cylinders (optional, more visually appealing)
                // DrawCylinderEx(joint_positions[i-1], joint_positions[i], 0.02f, 0.02f, 10, DARKGRAY);
            }

             // Draw Joints (Spheres at joint centers)
            float joint_radius = 0.04f;
            //DrawSphere(joint_positions[0], joint_radius, joint_colors[0]); // Base Origin
            //for (int i = 1; i < 7; ++i) {
            //    DrawSphere(joint_positions[i], joint_radius, joint_colors[i]);
            //}

            // Optional: Draw coordinate frame for the end-effector
             DrawLine3D(joint_positions[6], Vector3Add(joint_positions[6], Vector3Transform({0.1f, 0, 0}, joint_transforms[6])), RED); // X-axis
             DrawLine3D(joint_positions[6], Vector3Add(joint_positions[6], Vector3Transform({0, 0.1f, 0}, joint_transforms[6])), GREEN); // Y-axis
             DrawLine3D(joint_positions[6], Vector3Add(joint_positions[6], Vector3Transform({0, 0, 0.1f}, joint_transforms[6])), BLUE); // Z-axis

        }
        EndMode3D();

        // Draw 2D info text
        DrawText("Use mouse (scroll/drag) to control camera", 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("J1: %.2f rad", joint_angles[0]), 10, 40, 10, BLACK);
        DrawText(TextFormat("J2: %.2f rad", joint_angles[1]), 10, 55, 10, BLACK);
        DrawText(TextFormat("J3: %.2f rad", joint_angles[2]), 10, 70, 10, BLACK);
        DrawText(TextFormat("J4: %.2f rad", joint_angles[3]), 10, 85, 10, BLACK);
        DrawText(TextFormat("J5: %.2f rad", joint_angles[4]), 10, 100, 10, BLACK);
        DrawText(TextFormat("J6: %.2f rad", joint_angles[5]), 10, 115, 10, BLACK);
        DrawFPS(screenWidth - 90, 10);

        EndDrawing();
    }

    // De-Initialization
    //CloseWindow(); // Close window and OpenGL context

    return 0;
}