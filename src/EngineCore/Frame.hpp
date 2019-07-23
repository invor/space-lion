#ifndef Frame_hpp
#define Frame_hpp

#include <mutex>
#include <vector>

#include "types.hpp"
#include "RenderPass.hpp"

namespace EngineCore
{
    namespace Common
    {
        /**
         * A frame is a "snapshot" of the engine state for a specific point in time.
         * It contains all state -but nothing else- that is required for rendering an output image/frame.
         * TODO: Optimize for GPU upload later on...
         */
        struct Frame
        {
            Frame() : m_frameID(0), m_dt(0.0), m_window_width(0), m_window_height(0) {}

            // frame meta-data
            size_t m_frameID;
            double m_dt;

            // info on output window
            int m_window_width;
            int m_window_height;

            // camera data
            Mat4x4 m_view_matrix;
            Mat4x4 m_projection_matrix;
            float  m_fovy;
            float  m_aspect_ratio;
            float  m_exposure;

            // render passes
            std::vector<Graphics::RenderPass> m_render_passes;

            template<typename T1, typename T2>
            void addRenderPass(
                std::string const& pass_description,
                std::function<void(T1 &, T2 &)> setup_callback,
                std::function<void(T1 &, T2 &)> compile_callback,
                std::function<void(T1 const&, T2 const&)> execute_callback)
            {
                m_render_passes.push_back(
                    Graphics::RenderPass(pass_description, setup_callback,compile_callback,execute_callback)
                );
                m_render_passes.back().setupData();
            }
        };


        class FrameManager
        {
        private:
            Frame m_frame_tripleBuffer[3];
            unsigned int m_render_frame;
            unsigned int m_update_frame;
            unsigned int m_unused_frame;

            mutable std::mutex m_swap_frame_mutex;

        public:
            FrameManager();

            void swapRenderFrame();

            void swapUpdateFrame();

            Frame& setUpdateFrame(Frame && new_frame);

            Frame& getRenderFrame();
        };


    }
}

#endif // !Frame_hpp
