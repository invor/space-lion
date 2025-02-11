#ifndef TurntableCameraController_hpp
#define TurntableCameraController_hpp

#include "../EngineCore/Frame.hpp"
#include "../EngineCore/InputEvent.hpp"
#include "../EngineCore/WorldState.hpp"

namespace Editor {
    namespace Controls {

        class TurntableCameraController
        {
        public:
            TurntableCameraController(EngineCore::WorldState& world_state);
            ~TurntableCameraController() = default;

            //EngineCore::Common::Input::InputActionContext const& getKeyboardInputActionContext();

            EngineCore::Common::Input::InputActionContext const& getGamepadInputActionContext();

            void setRotCenter(Vec3 rot_center);

        private:

            /** Keep a reference (i.e. non-owning) to the world state */
            EngineCore::WorldState& world_state_;

            Vec3 rot_center_;

            /** Input context used by this controller */
            EngineCore::Common::Input::InputActionContext gamepad_input_action_context_;

            /** Callback function for gamepad controls, i.e. action to take based on a queried input state */
            void controlCameraGamepadAction(
                EngineCore::Common::Input::HardwareStateQuery const& input_hardware,
                std::vector<EngineCore::Common::Input::HardwareState> states,
                float dt);

        };

    }
}

#endif // !TurntableCameraController
