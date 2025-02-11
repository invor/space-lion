#include "TurntableCameraController.hpp"

#include "../EngineCore/types.hpp"
#include "../EngineCore/CameraComponent.hpp"
#include "../EngineCore/TransformComponentManager.hpp"

Editor::Controls::TurntableCameraController::TurntableCameraController(EngineCore::WorldState& world_state)
    : world_state_(world_state)
{
    EngineCore::Common::Input::StateDrivenAction gamepad_state_action = {
        {
            {EngineCore::Common::Input::Device::GAMEPAD_AXES,EngineCore::Common::Input::GamepadAxes::GAMEPAD_AXIS_LEFT_X},
            {EngineCore::Common::Input::Device::GAMEPAD_AXES,EngineCore::Common::Input::GamepadAxes::GAMEPAD_AXIS_LEFT_Y},
            {EngineCore::Common::Input::Device::GAMEPAD_AXES,EngineCore::Common::Input::GamepadAxes::GAMEPAD_AXIS_RIGHT_X},
            {EngineCore::Common::Input::Device::GAMEPAD_AXES,EngineCore::Common::Input::GamepadAxes::GAMEPAD_AXIS_RIGHT_Y},
            {EngineCore::Common::Input::Device::GAMEPAD_AXES,EngineCore::Common::Input::GamepadAxes::GAMEPAD_AXIS_LEFT_TRIGGER},
            {EngineCore::Common::Input::Device::GAMEPAD_AXES,EngineCore::Common::Input::GamepadAxes::GAMEPAD_AXIS_RIGHT_TRIGGER}
        },
        std::bind(&TurntableCameraController::controlCameraGamepadAction, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    };

    gamepad_input_action_context_ = { "editor_gamepad_cam_controls", true, {}, {gamepad_state_action} };
}

EngineCore::Common::Input::InputActionContext const& Editor::Controls::TurntableCameraController::getGamepadInputActionContext()
{
    return gamepad_input_action_context_;
}

void Editor::Controls::TurntableCameraController::setRotCenter(Vec3 rot_center)
{
    rot_center_ = rot_center;
}

void Editor::Controls::TurntableCameraController::controlCameraGamepadAction(
    EngineCore::Common::Input::HardwareStateQuery const& input_hardware,
    std::vector<EngineCore::Common::Input::HardwareState> states,
    float dt)
{
    auto& camera_mngr = world_state_.get<EngineCore::Graphics::CameraComponentManager>();
    auto& transform_mngr = world_state_.get<EngineCore::Common::TransformComponentManager>();

    Entity camera_entity = camera_mngr.getActiveCamera();

    if (camera_entity == EntityManager::invalidEntity())
    {
        return;
    }

    size_t camera_transform_idx = transform_mngr.getIndex(camera_entity);

    Vec3 cam_position = transform_mngr.getPosition(camera_transform_idx);
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
    if (std::sqrt(std::pow(std::abs(states[0]), 2.0) + std::pow(std::abs(states[1]), 2.0)) > dead_zone)
    {
    }

    if (std::sqrt(std::pow(std::abs(states[2]), 2.0) + std::pow(std::abs(states[3]), 2.0)) > dead_zone)
    {
        float sign_flip_x = std::signbit(states[2]) ? -1.0 : 1.0;
        float sign_flip_y = std::signbit(states[3]) ? -1.0 : 1.0;

        float remapped_state_x = (states[2] - sign_flip_x * dead_zone) / (1.0 - dead_zone);
        float remapped_state_y = (states[3] - sign_flip_y * dead_zone) / (1.0 - dead_zone);

        float factor = 90.0f;

        // rotate horizontally
        auto rot_lon = glm::angleAxis(factor* remapped_state_x * static_cast<float>(dt) * (3.14159265f / 180.0f), glm::vec3(0.0, 1.0, 0.0));
        cam_right = glm::rotate(rot_lon, cam_right);
        cam_forward = glm::rotate(rot_lon, cam_forward);
        cam_up = glm::rotate(rot_lon, cam_up);

        // rotate vertically
        auto rot_lat = glm::angleAxis(factor * remapped_state_y * static_cast<float>(dt) * (3.14159265f / 180.0f), -cam_right);
        cam_forward = glm::rotate(rot_lat, cam_forward);
        cam_up = glm::rotate(rot_lat, cam_up);

        // transform s.t. rotation center is origin
        auto shifted_pos = cam_position - rot_center_;
        shifted_pos = glm::rotate(rot_lon, shifted_pos);
        shifted_pos = glm::rotate(rot_lat, shifted_pos);

        // transform back
        cam_position = shifted_pos + glm::vec3(rot_center_);

        transform_mngr.setPosition(camera_transform_idx, cam_position);
        transform_mngr.rotate(camera_transform_idx, rot_lon * rot_lat);
    }

    if (std::sqrt(std::pow(std::abs(states[4]), 2.0) + std::pow(std::abs(states[5]), 2.0)) > dead_zone)
    {
        float dz = states[4] - states[5];

        auto v = glm::normalize(rot_center_ - cam_position);

        auto altitude = glm::length(rot_center_ - cam_position);

        cam_position = cam_position - (v * dz * (altitude / 50.0f));
        transform_mngr.setPosition(camera_transform_idx, cam_position);
    }
}
