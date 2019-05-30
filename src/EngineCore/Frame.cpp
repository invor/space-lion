#include "Frame.hpp"

EngineCore::Common::FrameManager::FrameManager()
    : m_render_frame(0), m_unused_frame(1), m_update_frame(2)
{

}

void EngineCore::Common::FrameManager::swapRenderFrame()
{
    std::unique_lock<std::mutex> lock(m_swap_frame_mutex);

    if (m_frame_tripleBuffer[m_unused_frame].m_frameID > m_frame_tripleBuffer[m_render_frame].m_frameID)
    {
        uint render_frame = m_render_frame;
        m_render_frame = m_unused_frame;
        m_unused_frame = render_frame;
    }
}

void EngineCore::Common::FrameManager::swapUpdateFrame()
{
    std::unique_lock<std::mutex> lock(m_swap_frame_mutex);

    uint unused_frame = m_unused_frame;
    m_unused_frame = m_update_frame;
    m_update_frame = unused_frame;
}

EngineCore::Common::Frame & EngineCore::Common::FrameManager::setUpdateFrame(Frame && new_frame)
{
    m_frame_tripleBuffer[m_update_frame] = new_frame;

    return m_frame_tripleBuffer[m_update_frame];
}

EngineCore::Common::Frame& EngineCore::Common::FrameManager::getRenderFrame()
{
    return m_frame_tripleBuffer[m_render_frame];
}