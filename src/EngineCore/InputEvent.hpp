#ifndef InputEvent_hpp
#define InputEvent_hpp

#include <functional>

namespace EngineCore
{
    namespace Common
    {

        /**
         * Self-contained defintion for all sorts of hardware input events that acts as intermediate layer between specific definitions
         * of window/context management and the (game) logic part.
         * Should be created by window system (e.g. glfw) and put into a queue.
         */
        struct InputEvent
        {
            /**
             * Specifies type of event, e.g. key is pressed/released to held down
             */
            enum EventType { PRESS = 1, RELEASE = 2, HOLD = 4 };

            /**
             * Specifies hardware part that triggered the event, e.g. key, mouse button, joystick axis, etc.
             */
            enum EventTrigger 
            {


            };

            InputEvent();
            ~InputEvent();

        };

        /**
         * Matches input events to actions (callbacks) that are to be peformed in response to the event.
         * The (game) logic part should register these into some sort of map datastructure.
         */
        class InputAction
        {
        public:
            InputAction();
            ~InputAction();

        private:
            InputEvent            m_event;
            std::function<void()> m_action;
        };

    }
}

#endif // !InputEvent_hpp
