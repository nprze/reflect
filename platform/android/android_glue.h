#pragma once
namespace rfct {

    struct InputEvent {
        int action;
        float x, y;
        long timestamp;
    };
    class InputQueue{
    public:
        static std::vector<InputEvent> eventQueue;

    };
}