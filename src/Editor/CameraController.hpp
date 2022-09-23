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
        CameraController(EngineCore::WorldState& world_state, EngineCore::Common::FrameManager<EngineCore::Common::Frame>& frame_mngr);
        ~CameraController();

        EngineCore::Common::Input::InputActionContext const& getInputActionContext();

    private:
        
        /** Keep a reference (i.e. non-owning) to the world state */
        EngineCore::WorldState& m_world_state;

        /** Keep a reference (i.e. non-owning) to the frame manager */
        EngineCore::Common::FrameManager<EngineCore::Common::Frame>& m_frame_manager;

        /** Input context used by this controller */
        EngineCore::Common::Input::InputActionContext m_input_action_context;

        /** Keep track of mouse cursor x position to compute movement delta over frames */
        double m_cursor_x;
        /** Keep track of mouse cursor y position to compute movement delta over frames */
        double m_cursor_y;
        /** Keep track on whether mouse right was pressed over frames to differentiate press from hold */
        bool m_mouse_right_pressed;

        /** Callback function, i.e. action to take based on a queried input state */
        void controlCameraAction(EngineCore::Common::Input::HardwareStateQuery const& input_hardware, std::vector<EngineCore::Common::Input::HardwareState> states);

    };

}
}

#endif // !CameraController
