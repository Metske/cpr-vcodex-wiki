#pragma once
struct ActivityResult {
    bool isCancelled;
};
using ActivityResultHandler = void(*)(ActivityResult);
