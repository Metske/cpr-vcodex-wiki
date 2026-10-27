#pragma once
#include <string>
#include <memory>
#include "GfxRenderer.h"
#include "MappedInputManager.h"
#include "RenderLock.h"
#include "activities/ActivityResult.h"
#include "activities/ActivityManager.h"

class Activity {
protected:
    std::string name;
    GfxRenderer& renderer;
    MappedInputManager& mappedInput;
public:
    Activity(std::string name, GfxRenderer& renderer, MappedInputManager& mappedInput) : name(name), renderer(renderer), mappedInput(mappedInput) {}
    virtual ~Activity() = default;
    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void loop() {}
    virtual void render(RenderLock&&) {}
    virtual void requestUpdate() {}
    virtual bool isReaderActivity() const { return false; }
    void setResult(ActivityResult&& res) {}
    void finish() {}
};
