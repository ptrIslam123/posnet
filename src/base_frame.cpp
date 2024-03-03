#include "include/base_frame.h"

namespace posnet {
    
BaseFrame::BaseFrame(const ByteType *const frameStart, const SizeType frameSize, const std::optional<FrameInfo> info):
m_start(frameStart),
m_size(frameSize),
m_info(info)
{}

BaseFrame::~BaseFrame() = default;

BaseFrame::ConstRawFrameViewType BaseFrame::getAsRawFrameView() const
{
    return ConstRawFrameViewType{ getStart(), getSize() };
}

BaseFrame::ConstRawFrameViewType BaseFrame::getAsRawFrameView()
{
    return ConstRawFrameViewType{ getStart(), getSize() };
}

std::optional<BaseFrame::FrameInfo> BaseFrame::getFrameInfo() const
{
    return m_info;
}

std::optional<BaseFrame::FrameInfo> BaseFrame::getFrameInfo()
{
    return m_info;
}

void BaseFrame::setFrameSize(const SizeType size)
{
    m_size = size;
}

const BaseFrame::ByteType* BaseFrame::getStart() const
{
    return m_start;
}

const BaseFrame::ByteType* BaseFrame::getStart()
{
    return m_start;
}

BaseFrame::SizeType BaseFrame::getSize() const
{
    return m_size;
}

BaseFrame::SizeType BaseFrame::getSize()
{
    return m_size;
}

} // namespace posnet
