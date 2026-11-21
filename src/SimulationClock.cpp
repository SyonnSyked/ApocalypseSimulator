#include "SimulationClock.h"

void SimulationClock::StartClock()
{
    tickCount = 0;
    clockStart = std::chrono::steady_clock::now();
    clockEnd = clockStart;
    isClockRunning = true;
}

void SimulationClock::EndClock()
{
    if (isClockRunning) {
        clockEnd = std::chrono::steady_clock::now();
        isClockRunning = false;
    }
}

void SimulationClock::AdvanceTick()
{
    if (isClockRunning) ++tickCount;
}

std::uint64_t SimulationClock::GetTickCount() const { return tickCount; }

std::chrono::hours SimulationClock::GetSimulatedTime() const
{
    return std::chrono::hours(tickCount);
}

std::chrono::steady_clock::duration SimulationClock::GetRealElapsedTime() const
{
    if (isClockRunning) return std::chrono::steady_clock::now() - clockStart;
    return clockEnd - clockStart;
}
