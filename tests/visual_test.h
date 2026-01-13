// visual_test.h
#include <gtest/gtest.h>
#include <raygpu.h>
#include <filesystem>
#include <string>
#include <iostream>
#include <cmath>

namespace fs = std::filesystem;

// Global flag determined in main()
extern bool g_GenerateGoldenImages;

class VisualTest : public ::testing::Test {
protected:
    // Helper to get paths
    fs::path GetGoldenPath(const std::string& imageName) {
        return fs::path("tests/golden") / (imageName + ".png");
    }

    fs::path GetArtifactPath(const std::string& imageName) {
        return fs::path("tests/artifacts") / (imageName + ".png");
    }

    // The Core Verification Logic
    void VerifyBuffer(RenderTexture target, const std::string& imageName) {
        Image resultImage = LoadImageFromTexture(target.texture);
        fs::path goldenPath = GetGoldenPath(imageName);
        
        if (g_GenerateGoldenImages) {
            fs::create_directories(goldenPath.parent_path());
            ExportImage(resultImage, goldenPath.string().c_str());
            std::cout << "[GEN] Generated golden image: " << goldenPath << std::endl;
            UnloadImage(resultImage);
            return; // Test passes
        }

        if (!fs::exists(goldenPath)) {
            UnloadImage(resultImage);
            FAIL() << "Golden image missing: " << goldenPath << ". Run with --generate first.";
        }

        Image goldenImage = LoadImage(goldenPath.string().c_str());

        if (resultImage.width != goldenImage.width || resultImage.height != goldenImage.height) {
            UnloadImage(resultImage);
            UnloadImage(goldenImage);
            FAIL() << "Image dimensions do not match for " << imageName;
        }

        Color* resultPixels = LoadImageColors(resultImage);
        Color* goldenPixels = LoadImageColors(goldenImage);
        
        uint32_t pixelCount = resultImage.width * resultImage.height;
        bool match = true;

        ///TODO: don't use ==
        for (int i = 0; i < pixelCount; i++) {
            int tolerance = 5;
            if (std::abs((int)resultPixels[i].r - (int)goldenPixels[i].r) > tolerance ||
                std::abs((int)resultPixels[i].g - (int)goldenPixels[i].g) > tolerance ||
                std::abs((int)resultPixels[i].b - (int)goldenPixels[i].b) > tolerance ||
                std::abs((int)resultPixels[i].a - (int)goldenPixels[i].a) > tolerance) {
                match = false;
                break;
            }
        }

        if (!match) {
            fs::create_directories(GetArtifactPath(imageName).parent_path());
            ExportImage(resultImage, GetArtifactPath(imageName).string().c_str());
            
            UnloadImageColors(resultPixels);
            UnloadImageColors(goldenPixels);
            UnloadImage(resultImage);
            UnloadImage(goldenImage);
            
            FAIL() << "Pixel mismatch detected! Artifact saved to " << GetArtifactPath(imageName);
        }

        UnloadImageColors(resultPixels);
        UnloadImageColors(goldenPixels);
        UnloadImage(resultImage);
        UnloadImage(goldenImage);
    }
};
