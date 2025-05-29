#pragma once
namespace rfct {

    struct InputEvent {
        int action;
        float x, y;
        int pointerID;
    };
    class InputQueue{
    public:
        static std::vector<InputEvent> eventQueue;

    };
}