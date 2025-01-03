#define MSF_GIF_IMPL
#include "msf_gif.h"
#include <raygpu.h>
#include <stdint.h>
#include <stdio.h>
typedef struct GIFRecordState{
    uint64_t delayInCentiseconds;
    uint64_t lastFrameTimestamp;
    MsfGifState msf_state;
    bool recording;
}GIFRecordState;

void startRecording(GIFRecordState* grst, uint64_t delay){
    if(grst->recording){
        TRACELOG(LOG_WARNING, "Already recording");
        return;
    }
    grst->delayInCentiseconds = delay;
    msf_gif_begin(&grst->msf_state, GetScreenWidth(), GetScreenHeight());
    grst->recording = true;
}

void addScreenshot(GIFRecordState* grst){
    grst->lastFrameTimestamp = NanoTime();
    Image fb = LoadImageFromTextureEx(GetActiveColorTarget());
    msf_gif_frame(&grst->msf_state, (uint8_t*)fb.data, grst->delayInCentiseconds, GetScreenWidth(), GetScreenHeight());
    UnloadImage(fb);
}
void stopRecording(GIFRecordState* grst, const char* filename){
    MsfGifResult result = msf_gif_end(&grst->msf_state);
    if (result.data) {
        FILE * fp = fopen(filename, "wb");
        fwrite(result.data, result.dataSize, 1, fp);
        fclose(fp);
    }
    msf_gif_free(result);
    grst->recording = false;
    memset(grst, 0, sizeof(GIFRecordState));
}