#pragma once

#include <juce_core/juce_core.h>


//==============================================================================
// A small helper class for invoking a lambda periodically
// via juce::Timer
template <typename Fn>
class LambdaTimer : public juce::Timer {
public:
    LambdaTimer(int rate, Fn&& f) : callback(std::move(f)) {
        startTimerHz(rate);
    }

    ~LambdaTimer() {
        stopTimer();
    }

    void timerCallback() override {
        if (callback) {
            callback();
        }
    }

private:
    Fn callback;
};
