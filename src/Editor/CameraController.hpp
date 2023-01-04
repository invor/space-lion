#ifndef CameraController_hpp
#define CameraController_hpp

#include "../EngineCore/Frame.hpp"
#include "../EngineCore/InputEvent.hpp"
#include "../EngineCore/WorldState.hpp"

namespace Editor{
namespace Controls{

    class CameraController
    {
    public:
        CameraController(EngineCore::WorldState& world_state);
        ~CameraController();

        EngineCore::Common::Input::InputActionContext const& getKeyboardInputActionContext();

        EngineCore::Common::Input::InputActionContext const& getGamepadInputActionContext();

    private:
        
        /** Keep a reference (i.e. non-owning) to the world state */
        EngineCore::WorldState& m_world_state;

        /** Input context used by this controller */
        EngineCore::Common::Input::InputActionContext m_keyboard_input_action_context;

        /** Second input context used by this controller */
        EngineCore::Common::Input::InputActionContext m_gamepad_input_action_context;

        /** Keep track of mouse cursor x position to compute movement delta over frames */
        double m_cursor_x;
        /** Keep track of mouse cursor y position to compute movement delta over frames */
        double m_cursor_y;
        /** Keep track on whether mouse right was pressed over frames to differentiate press from hold */
        bool m_mouse_right_pressed;

        /** Callback function for mouse-keyboard controls, i.e. action to take based on a queried input state */
        void controlCameraKeyboardAction(
            EngineCore::Common::Input::HardwareStateQuery const& input_hardware,
            std::vector<EngineCore::Common::Input::HardwareState> states,
            float dt);

        /** Callback function for gamepad controls, i.e. action to take based on a queried input state */
        void controlCameraGamepadAction(
            EngineCore::Common::Input::HardwareStateQuery const& input_hardware,
            std::vector<EngineCore::Common::Input::HardwareState> states,
            float dt);

    };

}
}

#endif // !CameraController
