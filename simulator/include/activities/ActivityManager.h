#pragma once
#include <memory>
class Activity;
class ActivityManager {
public:
    void pushActivity(std::unique_ptr<Activity> a) {}
};
extern ActivityManager activityManager;
