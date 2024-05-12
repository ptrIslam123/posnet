#include "include/base_frame.h"

#include <cstring>

namespace posnet {
    
BaseFrame::BaseFrame(const ByteType *const frameStart, const SizeType frameSize):
m_start(frameStart),
m_size(frameSize)
{}

BaseFrame::~BaseFrame() = default;

BaseFrame::SizeType BaseFrame::serialize(const RawFrameViewType outBuffer)
{
    std::memcpy(outBuffer.data(), getStart(), getSize());
    return getSize();
}

BaseFrame::SizeType BaseFrame::serialize(const RawFrameViewType outBuffer) const
{
    std::memcpy(outBuffer.data(), getStart(), getSize());
    return getSize();
}

BaseFrame::ConstRawFrameViewType BaseFrame::getAsRawFrameView() const
{
    return ConstRawFrameViewType{ getStart(), getSize() };
}

BaseFrame::ConstRawFrameViewType BaseFrame::getAsRawFrameView()
{
    return ConstRawFrameViewType{ getStart(), getSize() };
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
