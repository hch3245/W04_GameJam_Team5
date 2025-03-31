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

    float controllWindowWidth = static_cast<float>(width) * 0.25f;
    float controllWindowHeight = static_cast<float>(height) * 0.5f;

    float controllWindowPosX = 200.0f;
    float controllWindowPosY = 80.0f;

    // 창 크기와 위치 설정
    ImGui::SetNextWindowPos(ImVec2(controllWindowPosX, controllWindowPosY));
    ImGui::SetNextWindowSize(ImVec2(controllWindowWidth, controllWindowHeight), ImGuiCond_Always);

    UpdateFPS();

    if (ImGui::Begin("Performance Stats"))
    {
        ImGui::Text("FPS: %.3f", currentFPS);
        ImGui::Text("Frame Time: %.3f ms", frameTime);

        if (GetStat("PickingTime")) {
            ImGui::Text("Picking Count: %d", GetStat("PickingTime")->CallCount);
            ImGui::Text("Picking Time: %.3f ms", GetStat("PickingTime")->LastTime);
            ImGui::Text("Picking Time(Avg): %.3f ms", GetStat("PickingTime")->AverageTime);
            ImGui::Text("Accumulated Time: %.3f ms", GetStat("PickingTime")->AccumulatedTime);
        }
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

void StatPanel::BeginStat(const std::string& Name)
{
    if (StatItems.find(Name) == StatItems.end()) {
        StatItems.emplace(Name, FStatItem());
        StatItems[Name].Name = Name;
    }

    FStatItem& item = StatItems[Name];

    if (item.IsActive) {
        return;
    }

    item.StartCycles = FPlatformTime::Cycles64();
    item.IsActive = true;
}

void StatPanel::EndStat(const std::string& Name, bool bRecordStats)
{
    auto it = StatItems.find(Name);
    if (it == StatItems.end() || !it->second.IsActive) {
        return; // 존재하지 않거나 측정 중이 아니면 무시
    }

    FStatItem& item = it->second;
    uint64 endCycles = FPlatformTime::Cycles64();
    uint64 cycleDiff = endCycles - item.StartCycles;

    double timeMs = FPlatformTime::ToMilliseconds(cycleDiff);
    item.IsActive = false;

    // bRecordStats가 true일 때만 통계에 반영
    if (bRecordStats) {
        item.LastTime = timeMs;
        item.CallCount++;
        item.AccumulatedTime += item.LastTime;
        // 최소/최대 갱신
        item.MinTime = std::min(item.MinTime, timeMs);
        item.MaxTime = std::max(item.MaxTime, timeMs);

        // 최근 샘플 추가
        item.RecentTimes.push_back(timeMs);
        if (item.RecentTimes.size() > item.MAX_RECENT_SAMPLES) {
            item.RecentTimes.erase(item.RecentTimes.begin());
        }

        // 평균 재계산
        item.AverageTime = 0;
        for (double t : item.RecentTimes) {
            item.AverageTime += t;
        }
        item.AverageTime /= item.RecentTimes.size();
    }
}

void StatPanel::CancelStat(const std::string& Name)
{
    auto it = StatItems.find(Name);
    if (it == StatItems.end() || !it->second.IsActive) {
        return; // 존재하지 않거나 측정 중이 아니면 무시
    }

    // 측정을 취소하고 통계에 반영하지 않음
    it->second.IsActive = false;
}

const FStatItem* StatPanel::GetStat(const std::string& Name) const
{
    auto it = StatItems.find(Name);
    if (it != StatItems.end()) {
        return &(it->second);
    }
    return nullptr;
}