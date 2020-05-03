#ifndef InputEvent_hpp
#define InputEvent_hpp

#include <functional>

namespace EngineCore
{
    namespace Common
    {
        namespace Input
        {

        }

        /**
         * Self-contained defintion for all sorts of hardware input events that acts as intermediate layer between specific definitions
         * of window/context management and the (game) logic part.
         */
        struct InputEvent
        {
            /**
             * Specifies type of event, e.g. key is pressed/released to held down
             */
            enum EventTrigger { PRESS = 1, RELEASE = 2, HOLD = 3, MOVE = 4};

            /**
             * Specifies hardware part that triggered the event, e.g. key, mouse button, gamepad button etc.
             */
            enum EventElement
            {

            };

            typedef float ElementState;

            InputEvent();
            ~InputEvent();

            std::tuple<EventElement, EventTrigger, ElementState> m_event_conditions;

        };


        /**
         * Matches input events to actions (callbacks) that are to be peformed in response to the event.
         * The (game) logic part should register these as part of an input context into some sort of map datastructure.
         * Use for actions triggered by a single, specific event such as a single key press
         */
        struct InputEventAction
        {
            InputEventAction();
            ~InputEventAction();

            InputEvent                             m_event;
            std::function<void(InputEvent const&)> m_action;
        };


        /**
         * Self-contained defintion for input state of one or several devices and keys/buttons/axes.
         */
        struct InputState
        {
            enum EventDevice { KEYBOARD, MOUSE_BUTTON, MOUSE_AXIS, GAMEPAD_BUTTON, GAMEPAD_AXIS };

            enum EventElement
            {

            };

        };

        /**
         * Matches (more complex) input states to actions (callbacks) that are to be performed if the given state persists.
         */
        struct InputStateAction
        {
            InputState                             m_event;
            std::function<void(InputState const&)> m_action;
        };


        /**
         * A combination of several event-driven and state-driven input actions.
         * Allows to define and switch between different controls, e.g. camera movement, player movement, etc.
         * Input events should check with all active input contexts for matching event actions,
         * while once a frame the input system should iterate all state actions for every active context.
         */
        struct InputActionContext
        {
            std::vector<InputEventAction> m_event_actions;
            std::vector<InputStateAction> m_state_actions;
        };

    }
}

#endif // !InputEvent_hpp
