#pragma once
#include "Define.h"
#include "IWindowToggleable.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Runtime/WindowsPlatformTime.h"

class UWorld;

struct FStatItem {
    std::string Name;
    double LastTime;       // 마지막 측정 시간 (ms)
    double AverageTime;    // 평균 시간 (ms)
    double MinTime;        // 최소 시간 (ms)
    double MaxTime;        // 최대 시간 (ms)
    double AccumulatedTime; // 누적 시간 (ms)
    uint64 CallCount;      // 호출 횟수
    uint64 StartCycles;    // 시작 사이클 (현재 측정 중인 경우)
    bool IsActive;         // 현재 측정 중인지 여부

    std::vector<double> RecentTimes;
    const size_t MAX_RECENT_SAMPLES = 100;

    FStatItem() : LastTime(0), AverageTime(0), MinTime(DBL_MAX), MaxTime(0),
        CallCount(0), StartCycles(0), AccumulatedTime(0), IsActive(false) {
    }

    void AddSample(double time) {
        LastTime = time;
        CallCount++;
        AccumulatedTime += time;

        // 최소/최대 갱신
        MinTime = std::min(MinTime, time);
        MaxTime = std::max(MaxTime, time);

        // 최근 샘플 추가
        RecentTimes.push_back(time);
        if (RecentTimes.size() > MAX_RECENT_SAMPLES) {
            RecentTimes.erase(RecentTimes.begin());
        }

        // 평균 재계산
        AverageTime = 0;
        for (double t : RecentTimes) {
            AverageTime += t;
        }
        AverageTime /= RecentTimes.size();
    }
};

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
 
    void BeginStat(const std::string& Name);
    void EndStat(const std::string& Name, bool bRecordStats = true);
    void CancelStat(const std::string& Name);
    const FStatItem* GetStat(const std::string& Name) const;

private:
    std::unordered_map<std::string, FStatItem> StatItems;
    bool bWasOpen = true;
    UINT width;
    UINT height;
    float currentFPS = 0;
    uint64 lastTime = FPlatformTime::Cycles64();
    int32 frameCount = 0;
    double frameTime = 0.0;
    double lastFrameTime = 0;
};
