#ifndef Dx11Frame_hpp
#define Dx11Frame_hpp

#include <d3d11_4.h>

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
    };
}
}
}

#endif // !Dx11Frame_hpp
