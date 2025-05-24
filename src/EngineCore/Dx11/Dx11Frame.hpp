#ifndef Dx11Frame_hpp
#define Dx11Frame_hpp

#include <d3d11_4.h>
#include <winrt/base.h> // winrt::check_hresult
#include <wrl.h>

#include "Frame.hpp"

namespace EngineCore {
namespace Graphics {
namespace Dx11 {
    struct Frame : public Common::BaseFrame
    {
        Frame() : m_window_width(0), m_window_height(0), m_render_target_view(nullptr), m_depth_stencil_view(nullptr) {}

        // info on output window and swapchain
        int m_window_width;
        int m_window_height;
        ID3D11RenderTargetView* m_render_target_view;
        ID3D11DepthStencilView* m_depth_stencil_view;

        // camera pose and intrinsics in matrix form
        struct CameraMatrices {
            Mat4x4 view_matrix;
            Mat4x4 projection_matrix;
        };
        std::array<CameraMatrices, 1> m_view_projections;

        inline void createViewProjectionConstantBuffer(
            ID3D11Device4* d3d11_device,
            Microsoft::WRL::ComPtr<ID3D11Buffer>& view_proj_buffer) const
        {
            struct ViewProjectionConstantBuffer {
                Mat4x4 view_inverse;
                Mat4x4 view_projection;
                Mat4x4 view;
                Mat4x4 proj;
            };
            ViewProjectionConstantBuffer view_proj_buffer_data;


            view_proj_buffer_data.view_inverse = glm::transpose(glm::inverse(m_view_projections[0].view_matrix));
            view_proj_buffer_data.view_projection = glm::transpose(m_view_projections[0].projection_matrix * m_view_projections[0].view_matrix);
            view_proj_buffer_data.view = glm::transpose(m_view_projections[0].view_matrix);
            view_proj_buffer_data.proj = glm::transpose(m_view_projections[0].projection_matrix);

            // create constant buffers
            {
                D3D11_SUBRESOURCE_DATA constantBufferData = { 0 };
                constantBufferData.pSysMem = &(view_proj_buffer_data);
                constantBufferData.SysMemPitch = 0;
                constantBufferData.SysMemSlicePitch = 0;
                const CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
                winrt::check_hresult(
                    d3d11_device->CreateBuffer(
                        &constantBufferDesc,
                        &constantBufferData,
                        &view_proj_buffer
                    ));
            }
        }
    };
}
}
}

#endif // !Dx11Frame_hpp
