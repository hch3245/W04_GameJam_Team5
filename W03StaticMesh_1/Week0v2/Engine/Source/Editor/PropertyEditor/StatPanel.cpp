#pragma once
#include "StatPanel.h"

StatPanel::StatPanel()
{
}

StatPanel::~StatPanel()
{
}

StatPanel& StatPanel::GetInstance()
{
    static StatPanel instance;
    return instance;
}

void StatPanel::Draw()
{
    if (!bWasOpen) return;

    float controllWindowWidth = static_cast<float>(width) * 0.12f;
    float controllWindowHeight = static_cast<float>(height) * 0.05f;

    float controllWindowPosX = 50.0f;
    float controllWindowPosY = 50.0f;

    // 창 크기와 위치 설정
    ImGui::SetNextWindowPos(ImVec2(controllWindowPosX, controllWindowPosY));
    ImGui::SetNextWindowSize(ImVec2(controllWindowWidth, controllWindowHeight), ImGuiCond_Always);

    UpdateFPS();

    if (ImGui::Begin("FPS"))
    {
        ImGui::Text("FPS: %.3f", currentFPS);
        ImGui::Text("Frame Time: %.3f ms", frameTime);
    }
    ImGui::End();
}

void StatPanel::UpdateFPS() {
    const uint64 currentTime = FPlatformTime::Cycles64();

    currentFPS = 1000.0f / frameTime;

    if (lastFrameTime != 0) {
        frameTime = FPlatformTime::ToMilliseconds(currentTime - lastFrameTime);
    }
    lastFrameTime = currentTime;
}

void StatPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    width = clientRect.right - clientRect.left;
    height = clientRect.bottom - clientRect.top;
}