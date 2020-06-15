#include "CameraController.hpp"

#include "..\EngineCore\types.hpp"

Editor::Controls::CameraController::CameraController(EngineCore::WorldState & world_state, EngineCore::Common::FrameManager& frame_mngr)
    : m_world_state(world_state), m_frame_manager(frame_mngr), m_cursor_x(0.0), m_cursor_y(0.0), m_mouse_right_pressed(false)
{

    EngineCore::Common::InputStateAction state_action = {
        {
            {EngineCore::Common::Input::Device::KEYBOARD,EngineCore::Common::Input::KeyboardKeys::KEY_W, 1.0f},
            {EngineCore::Common::Input::Device::KEYBOARD,EngineCore::Common::Input::KeyboardKeys::KEY_A, 1.0f},
            {EngineCore::Common::Input::Device::KEYBOARD,EngineCore::Common::Input::KeyboardKeys::KEY_S, 1.0f},
            {EngineCore::Common::Input::Device::KEYBOARD,EngineCore::Common::Input::KeyboardKeys::KEY_D, 1.0f},

            {EngineCore::Common::Input::Device::MOUSE_AXES,EngineCore::Common::Input::MouseAxes::MOUSE_CURSOR_X, 1.0f},
            {EngineCore::Common::Input::Device::MOUSE_AXES,EngineCore::Common::Input::MouseAxes::MOUSE_CURSOR_Y, 1.0f},
            {EngineCore::Common::Input::Device::MOUSE_BUTTON,EngineCore::Common::Input::MouseButtons::MOUSE_BUTTON_RIGHT, 0.0f}
        },
        std::bind(&CameraController::controlCameraAction, this, std::placeholders::_1)
    };

    m_input_action_context = { "editor_cam_controls", true, {}, {state_action} };
}

Editor::Controls::CameraController::~CameraController()
{
}

EngineCore::Common::InputActionContext const & Editor::Controls::CameraController::getInputActionContext()
{
    return m_input_action_context;
}

void Editor::Controls::CameraController::controlCameraAction(EngineCore::Common::InputState const & input_state)
{
    auto& camera_mngr = m_world_state.accessCameraComponentManager();
    auto& transform_mngr = m_world_state.accessTransformManager();

    uint camera_idx = camera_mngr.getActiveCameraIndex();
    Entity camera_entity = camera_mngr.getEntity(camera_idx);
    size_t camera_transform_idx = transform_mngr.getIndex(camera_entity).front();

    auto view_to_world = transform_mngr.getWorldTransformation(camera_transform_idx);
    auto view_to_world_vec = glm::mat3x3(view_to_world);
    auto world_to_view_vec = glm::inverse(view_to_world_vec);
    view_to_world_vec = glm::inverse(view_to_world_vec);
    view_to_world_vec = glm::transpose(view_to_world_vec);
    Vec3 cam_forward = view_to_world_vec * Vec3(0.0, 0.0, -1.0);
    Vec3 cam_right = view_to_world_vec * Vec3(1.0, 0.0, 0.0);
    Vec3 cam_up = view_to_world_vec * Vec3(0.0, 1.0, 0.0);

    Vec3 world_up_vs = world_to_view_vec * Vec3(0.0, 1.0, 0.0);

    auto dt = static_cast<float>(m_frame_manager.getRenderFrame().m_dt);

    Vec3 movement = Vec3(0.0, 0.0, 0.0);

    // first hardware part is w key, state value greater 0 shows that key is currently pressed
    if (std::get<2>(input_state[0]) > 0.0f)
    {
        movement += static_cast<float>(dt) * cam_forward;
    }
    if (std::get<2>(input_state[1]) > 0.0f)
    {
        movement += -static_cast<float>(dt) * cam_right;
    }
    if (std::get<2>(input_state[2]) > 0.0f)
    {
        movement += -static_cast<float>(dt) * cam_forward;
    }
    if (std::get<2>(input_state[3]) > 0.0f)
    {
        movement += static_cast<float>(dt) * cam_right;
    }

    transform_mngr.translate(camera_transform_idx, movement);


    if (std::get<2>(input_state[6]) > 0.0f)
    {
        auto current_cursor_x = std::get<2>(input_state[4]);
        auto current_cursor_y = std::get<2>(input_state[5]);

        if (!m_mouse_right_pressed) {
            m_cursor_x = current_cursor_x;
            m_cursor_y = current_cursor_y;
        }

        float dx = current_cursor_x - m_cursor_x;
        float dy = current_cursor_y - m_cursor_y;

        auto rotation = glm::angleAxis(-dx * dt, world_up_vs);
        rotation *= glm::angleAxis(-dy * dt, Vec3(1.0, 0.0, 0.0));

        m_cursor_x = current_cursor_x;
        m_cursor_y = current_cursor_y;

        transform_mngr.rotateLocal(camera_transform_idx, rotation);

        m_mouse_right_pressed = true;
    }
    else
    {
        m_mouse_right_pressed = false;
    }
}
