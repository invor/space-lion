#include "CameraController.hpp"

#include "../EngineCore/types.hpp"
#include "../EngineCore/CameraComponent.hpp"
#include "../EngineCore/TransformComponentManager.hpp"

Editor::Controls::CameraController::CameraController(EngineCore::WorldState & world_state)
    : m_world_state(world_state), m_cursor_x(0.0), m_cursor_y(0.0), m_mouse_right_pressed(false)
{

    EngineCore::Common::Input::StateDrivenAction mouseKeyboard_state_action = {
        {
            {EngineCore::Common::Input::Device::KEYBOARD,EngineCore::Common::Input::KeyboardKeys::KEY_W},
            {EngineCore::Common::Input::Device::KEYBOARD,EngineCore::Common::Input::KeyboardKeys::KEY_A},
            {EngineCore::Common::Input::Device::KEYBOARD,EngineCore::Common::Input::KeyboardKeys::KEY_S},
            {EngineCore::Common::Input::Device::KEYBOARD,EngineCore::Common::Input::KeyboardKeys::KEY_D},

            {EngineCore::Common::Input::Device::MOUSE_AXES,EngineCore::Common::Input::MouseAxes::MOUSE_CURSOR_X},
            {EngineCore::Common::Input::Device::MOUSE_AXES,EngineCore::Common::Input::MouseAxes::MOUSE_CURSOR_Y},
            {EngineCore::Common::Input::Device::MOUSE_BUTTON,EngineCore::Common::Input::MouseButtons::MOUSE_BUTTON_RIGHT}
        },
        std::bind(&CameraController::controlCameraKeyboardAction, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    };

    m_keyboard_input_action_context = { "editor_mk_cam_controls", true, {}, {mouseKeyboard_state_action} };


    EngineCore::Common::Input::StateDrivenAction gamepad_state_action = {
        {
            {EngineCore::Common::Input::Device::GAMEPAD_AXES,EngineCore::Common::Input::GamepadAxes::GAMEPAD_AXIS_LEFT_X},
            {EngineCore::Common::Input::Device::GAMEPAD_AXES,EngineCore::Common::Input::GamepadAxes::GAMEPAD_AXIS_LEFT_Y},
            {EngineCore::Common::Input::Device::GAMEPAD_AXES,EngineCore::Common::Input::GamepadAxes::GAMEPAD_AXIS_RIGHT_X},
            {EngineCore::Common::Input::Device::GAMEPAD_AXES,EngineCore::Common::Input::GamepadAxes::GAMEPAD_AXIS_RIGHT_Y}
        },
        std::bind(&CameraController::controlCameraGamepadAction, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    };

    m_gamepad_input_action_context = { "editor_gamepad_cam_controls", true, {}, {gamepad_state_action} };
}

Editor::Controls::CameraController::~CameraController()
{
}

EngineCore::Common::Input::InputActionContext const & Editor::Controls::CameraController::getKeyboardInputActionContext()
{
    return m_keyboard_input_action_context;
}

EngineCore::Common::Input::InputActionContext const& Editor::Controls::CameraController::getGamepadInputActionContext()
{
    return m_gamepad_input_action_context;
}

void Editor::Controls::CameraController::controlCameraKeyboardAction(
    EngineCore::Common::Input::HardwareStateQuery const & input_hardware,
    std::vector<EngineCore::Common::Input::HardwareState> states,
    float dt)
{
    auto& camera_mngr = m_world_state.get<EngineCore::Graphics::CameraComponentManager>();
    //auto& transform_mngr = m_world_state.accessTransformManager();
    auto& transform_mngr = m_world_state.get<EngineCore::Common::TransformComponentManager>();

    Entity camera_entity = camera_mngr.getActiveCamera();

    if (camera_entity == EntityManager::invalidEntity())
    {
        return;
    }

    size_t camera_transform_idx = transform_mngr.getIndex(camera_entity);

    auto view_to_world = transform_mngr.getWorldTransformation(camera_transform_idx);
    auto view_to_world_vec = glm::mat3x3(view_to_world);
    auto world_to_view_vec = glm::inverse(view_to_world_vec);
    view_to_world_vec = glm::inverse(view_to_world_vec);
    view_to_world_vec = glm::transpose(view_to_world_vec);
    Vec3 cam_forward = view_to_world_vec * Vec3(0.0, 0.0, -1.0);
    Vec3 cam_right = view_to_world_vec * Vec3(1.0, 0.0, 0.0);
    Vec3 cam_up = view_to_world_vec * Vec3(0.0, 1.0, 0.0);

    Vec3 world_up_vs = world_to_view_vec * Vec3(0.0, 1.0, 0.0);

    Vec3 movement = Vec3(0.0, 0.0, 0.0);

    // first hardware part is w key, state value greater 0 shows that key is currently pressed
    if (states[0] > 0.0f)
    {
        movement += static_cast<float>(dt) * cam_forward;
    }
    if (states[1] > 0.0f)
    {
        movement += -static_cast<float>(dt) * cam_right;
    }
    if (states[2] > 0.0f)
    {
        movement += -static_cast<float>(dt) * cam_forward;
    }
    if (states[3] > 0.0f)
    {
        movement += static_cast<float>(dt) * cam_right;
    }

    transform_mngr.translate(camera_transform_idx, movement);


    if (states[6] > 0.0f)
    {
        auto current_cursor_x = states[4];
        auto current_cursor_y = states[5];

        if (!m_mouse_right_pressed) {
            m_cursor_x = current_cursor_x;
            m_cursor_y = current_cursor_y;
        }

        float dx = static_cast<float>(current_cursor_x - m_cursor_x);
        float dy = static_cast<float>(current_cursor_y - m_cursor_y);

        auto rotation = glm::angleAxis(-dx * 0.001f, world_up_vs);
        rotation *= glm::angleAxis(-dy * 0.001f, Vec3(1.0, 0.0, 0.0));

        m_cursor_x = current_cursor_x;
        m_cursor_y = current_cursor_y;

        transform_mngr.rotateLocal(camera_transform_idx, rotation);

        m_mouse_right_pressed = true;
    }
    else
    {
        m_mouse_right_pressed = false;
    }

    // TODO reduce camera latency by updating current render frame, camera view matrix is no longer accessible in frame (or used from frame anyway)

    //m_frame_manager.getRenderFrame().m_view_matrix = glm::inverse(transform_mngr.getWorldTransformation(camera_transform_idx));
}

void Editor::Controls::CameraController::controlCameraGamepadAction(
    EngineCore::Common::Input::HardwareStateQuery const& input_hardware,
    std::vector<EngineCore::Common::Input::HardwareState> states,
    float dt)
{
    auto& camera_mngr = m_world_state.get<EngineCore::Graphics::CameraComponentManager>();
    //auto& transform_mngr = m_world_state.accessTransformManager();
    auto& transform_mngr = m_world_state.get<EngineCore::Common::TransformComponentManager>();

    Entity camera_entity = camera_mngr.getActiveCamera();

    if (camera_entity == EntityManager::invalidEntity())
    {
        return;
    }

    size_t camera_transform_idx = transform_mngr.getIndex(camera_entity);

    auto view_to_world = transform_mngr.getWorldTransformation(camera_transform_idx);
    auto view_to_world_vec = glm::mat3x3(view_to_world);
    auto world_to_view_vec = glm::inverse(view_to_world_vec);
    view_to_world_vec = glm::inverse(view_to_world_vec);
    view_to_world_vec = glm::transpose(view_to_world_vec);
    Vec3 cam_forward = view_to_world_vec * Vec3(0.0, 0.0, -1.0);
    Vec3 cam_right = view_to_world_vec * Vec3(1.0, 0.0, 0.0);
    Vec3 cam_up = view_to_world_vec * Vec3(0.0, 1.0, 0.0);

    Vec3 world_up_vs = world_to_view_vec * Vec3(0.0, 1.0, 0.0);

    Vec3 movement = Vec3(0.0, 0.0, 0.0);

    float dead_zone = 0.05;

    // first hardware part is left x axis
    if ( std::sqrt(std::pow(std::abs(states[0]),2.0) + std::pow(std::abs(states[1]),2.0)) > dead_zone)
    {
        float sign_flip_x = std::signbit(states[0]) ? -1.0 : 1.0;
        float sign_flip_y = std::signbit(states[1]) ? -1.0 : 1.0;

        float remapped_state_x = (states[0] - sign_flip_x*dead_zone) / (1.0 - dead_zone);
        float remapped_state_y = (states[1] - sign_flip_y*dead_zone) / (1.0 - dead_zone);

        movement += static_cast<float>(dt) * remapped_state_x * cam_right;
        movement += static_cast<float>(dt) * remapped_state_y * cam_forward;

        transform_mngr.translate(camera_transform_idx, movement);
    }

    if (std::sqrt(std::pow(std::abs(states[2]), 2.0) + std::pow(std::abs(states[3]), 2.0)) > dead_zone)
    {
        float sign_flip_x = std::signbit(states[2]) ? -1.0 : 1.0;
        float sign_flip_y = std::signbit(states[3]) ? -1.0 : 1.0;

        float remapped_state_x = (states[2] - sign_flip_x*dead_zone) / (1.0 - dead_zone);
        float remapped_state_y = (states[3] - sign_flip_y*dead_zone) / (1.0 - dead_zone);
        
        auto rotation = glm::angleAxis(-remapped_state_x * static_cast<float>(dt), world_up_vs);
        rotation *= glm::angleAxis(remapped_state_y * static_cast<float>(dt), Vec3(1.0, 0.0, 0.0));

        transform_mngr.rotateLocal(camera_transform_idx, rotation);
    }
}
