#include <gtest/gtest.h>
#include <raygpu.h>
#include <cstring>

bool g_GenerateGoldenImages = false;

// Global Environment to handle InitWindow/CloseWindow once
class RaygpuEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        
        SetTraceLogLevel(LOG_WARNING);
        SetConfigFlags(FLAG_HEADLESS);
        InitWindow(800, 600, "Raygpu Test Runner"); 
    }

    void TearDown() override {
        //CloseWindow();
    }
};

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--generate") == 0) {
            g_GenerateGoldenImages = true;
        }
    }

    testing::AddGlobalTestEnvironment(new RaygpuEnvironment);

    return RUN_ALL_TESTS();
}