#pragma once
#include "Define.h"
#include "IWindowToggleable.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Runtime/WindowsPlatformTime.h"

class UWorld;
class StatPanel : public IWindowToggleable
{
private:
    StatPanel();

public:
    ~StatPanel();

    static StatPanel& GetInstance();

    void Draw();
    void OnResize(HWND hWnd);
    void Toggle() override {
        if (bWasOpen) {
            bWasOpen = false;
        }
        else {
            bWasOpen = true;
        }
    }

    void UpdateFPS();

private:
    bool bWasOpen = true;
    UINT width;
    UINT height;
    float currentFPS = 0;
    uint64 lastTime = FPlatformTime::Cycles64();
    int32 frameCount = 0;
    double frameTime = 0.0;
    double lastFrameTime = 0;
};
