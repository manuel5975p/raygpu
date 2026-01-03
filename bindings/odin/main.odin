package main

import rg "raygpu"

main :: proc() {
    rg.InitWindow(600, 600, "Hello!")

    tex := rg.LoadRenderTexture(100, 100)
    //`defer rg.UnloadRenderTexture(tex)

    rg.BeginTextureMode(tex)
    rg.ClearBackground(rg.RED)
    rg.EndTextureMode()

    for !rg.WindowShouldClose() {
        rg.BeginDrawing()
        rg.ClearBackground(rg.WHITE)
        // rg.DrawTextureV(tex.texture, {10, 10}, rg.WHITE)
        rg.DrawCircleV(rg.GetMousePosition(),   100, rg.BLACK)
        rg.EndDrawing()
    }
}
