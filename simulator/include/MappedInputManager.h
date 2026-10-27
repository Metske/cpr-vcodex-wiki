#pragma once
class MappedInputManager {
public:
    enum class Button { Up, Down, Left, Right, Confirm, Back };
    Button simulatedButton = Button::Back;
    bool hasSimulated = false;

    void simulateKey(Button b) {
        simulatedButton = b;
        hasSimulated = true;
    }

    bool wasScreenTapped(int& x, int& y) { return false; }
    
    bool wasReleased(Button b) { 
        if (hasSimulated && simulatedButton == b) {
            hasSimulated = false; // consume
            return true;
        }
        return false; 
    }
    bool wasPressed(Button b) { return wasReleased(b); }
    bool wasBackGesture() { return false; }
    bool wasHomeGesture() { return false; }
};
