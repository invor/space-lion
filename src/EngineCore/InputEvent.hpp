#ifndef InputEvent_hpp
#define InputEvent_hpp

#include <functional>

namespace EngineCore
{
    namespace Common
    {
        namespace Input
        {
            // Enumeration of all hardware parts that can be used for input
            enum HardwarePart
            {

                // Mouse buttons
                MOUSE_BUTTON_1,
                MOUSE_BUTTON_2,
                MOUSE_BUTTON_3,
                MOUSE_BUTTON_4,
                MOUSE_BUTTON_5,
                MOUSE_BUTTON_6,
                MOUSE_BUTTON_7,
                MOUSE_BUTTON_8,
                MOUSE_BUTTON_LAST = MOUSE_BUTTON_8,
                MOUSE_BUTTON_LEFT = MOUSE_BUTTON_1,
                MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_2,
                MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_3,

                // Joystick
                JOYSTICK_1,
                JOYSTICK_2,
                JOYSTICK_3,
                JOYSTICK_4,
                JOYSTICK_5,
                JOYSTICK_6,
                JOYSTICK_7,
                JOYSTICK_8,
                JOYSTICK_9,
                JOYSTICK_10,
                JOYSTICK_11,
                JOYSTICK_12,
                JOYSTICK_13,
                JOYSTICK_14,
                JOYSTICK_15,
                JOYSTICK_16,
                JOYSTICK_LAST = JOYSTICK_16,

                // Gamepad buttons
                GAMEPAD_BUTTON_A               ,
                GAMEPAD_BUTTON_B               ,
                GAMEPAD_BUTTON_X               ,
                GAMEPAD_BUTTON_Y               ,
                GAMEPAD_BUTTON_LEFT_BUMPER     ,
                GAMEPAD_BUTTON_RIGHT_BUMPER    ,
                GAMEPAD_BUTTON_BACK            ,
                GAMEPAD_BUTTON_START           ,
                GAMEPAD_BUTTON_GUIDE           ,
                GAMEPAD_BUTTON_LEFT_THUMB      ,
                GAMEPAD_BUTTON_RIGHT_THUMB     ,
                GAMEPAD_BUTTON_DPAD_UP         ,
                GAMEPAD_BUTTON_DPAD_RIGHT      ,
                GAMEPAD_BUTTON_DPAD_DOWN       ,
                GAMEPAD_BUTTON_DPAD_LEFT       ,
                GAMEPAD_BUTTON_LAST            = GAMEPAD_BUTTON_DPAD_LEFT,
                GAMEPAD_BUTTON_CROSS           = GAMEPAD_BUTTON_A,
                GAMEPAD_BUTTON_CIRCLE          = GAMEPAD_BUTTON_B,
                GAMEPAD_BUTTON_SQUARE          = GAMEPAD_BUTTON_X,
                GAMEPAD_BUTTON_TRIANGLE        = GAMEPAD_BUTTON_Y,
                // Gamepad axes
                GAMEPAD_AXIS_LEFT_X,
                GAMEPAD_AXIS_LEFT_Y,
                GAMEPAD_AXIS_RIGHT_X,
                GAMEPAD_AXIS_RIGHT_Y,
                GAMEPAD_AXIS_LEFT_TRIGGER,
                GAMEPAD_AXIS_RIGHT_TRIGGER,
                GAMEPAD_AXIS_LAST = GAMEPAD_AXIS_RIGHT_TRIGGER

            };
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

            typedef float ElementState;

            InputEvent();
            ~InputEvent();

            std::tuple<Input::HardwarePart, EventTrigger, ElementState> m_event_conditions;

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
