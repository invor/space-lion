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
        struct BaseFrame {
            size_t m_frameID = 0; ///< frame id assigned to each frame upon creation
            double m_simulation_dt = 0.0; ///< time elapsed since last frame was created

            size_t m_render_frameID = 0; ///< frame id assigned by graphics backend to processed frames
            double m_render_dt = 0.0; ///< time elapsed since last frame processed by graphics backend

            // render passes
            std::vector<Graphics::RenderPass> m_render_passes;

            template<typename T1, typename T2>
            void addRenderPass(
                std::string const& pass_description,
                std::function<void(T1&, T2&)> setup_callback,
                std::function<void(T1&, T2&)> compile_callback,
                std::function<void(T1 const&, T2 const&)> execute_callback)
            {
                m_render_passes.push_back(
                    Graphics::RenderPass(pass_description, setup_callback, compile_callback, execute_callback)
                );
                m_render_passes.back().setupData();
            }
        };

        struct Frame : public BaseFrame
        {
            Frame() : m_window_width(0), m_window_height(0) {}

            // info on output window
            int m_window_width;
            int m_window_height;

            // camera data
            Mat4x4 m_view_matrix;
            Mat4x4 m_projection_matrix;
            float  m_fovy;
            float  m_aspect_ratio;
            float  m_exposure;
        };

        template<typename FrameType>
        class FrameManager
        {
        private:
            FrameType m_frame_tripleBuffer[3];
            unsigned int m_render_frame;
            unsigned int m_update_frame;
            unsigned int m_unused_frame;

            mutable std::mutex m_swap_frame_mutex;

        public:
            FrameManager();

            void swapRenderFrame();

            void swapUpdateFrame();

            FrameType& setUpdateFrame(FrameType&& new_frame);

            FrameType& getUpdateFrame();

            FrameType& getRenderFrame();
        };

        template<typename FrameType>
        FrameManager<FrameType>::FrameManager()
            : m_render_frame(0), m_unused_frame(1), m_update_frame(2)
        {

        }

        template<typename FrameType>
        void FrameManager<FrameType>::swapRenderFrame()
        {
            std::unique_lock<std::mutex> lock(m_swap_frame_mutex);

            if (m_frame_tripleBuffer[m_unused_frame].m_frameID > m_frame_tripleBuffer[m_render_frame].m_frameID)
            {
                unsigned int render_frame = m_render_frame;
                m_render_frame = m_unused_frame;
                m_unused_frame = render_frame;
            }
        }

        template<typename FrameType>
        void FrameManager<FrameType>::swapUpdateFrame()
        {
            std::unique_lock<std::mutex> lock(m_swap_frame_mutex);

            unsigned int unused_frame = m_unused_frame;
            m_unused_frame = m_update_frame;
            m_update_frame = unused_frame;
        }

        template<typename FrameType>
        FrameType& FrameManager<FrameType>::setUpdateFrame(FrameType&& new_frame)
        {
            m_frame_tripleBuffer[m_update_frame] = new_frame;

            return m_frame_tripleBuffer[m_update_frame];
        }

        template<typename FrameType>
        inline FrameType& FrameManager<FrameType>::getUpdateFrame()
        {
            return m_frame_tripleBuffer[m_update_frame];
        }

        template<typename FrameType>
        FrameType& FrameManager<FrameType>::getRenderFrame()
        {
            return m_frame_tripleBuffer[m_render_frame];
        }
    }
}

#endif // !Frame_hpp
