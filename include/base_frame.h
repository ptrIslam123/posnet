#ifndef VS_BASE_FRAME_H
#define VS_BASE_FRAME_H

#include "definitions.h"

#include <span>
#include <optional>

namespace posnet {

class BaseFrame {
public:
    using ByteType = def::ByteType;
    using SizeType = def::SizeType;
    using RawFrameViewType = def::RawFrameViewType;
    using ConstRawFrameViewType = def::ConstRawFrameViewType;

    struct FrameInfo {};

    explicit BaseFrame(const ByteType* frameStart, SizeType frameSize, std::optional<FrameInfo> info = std::nullopt);
    virtual ~BaseFrame() = 0;

    const ByteType* getStart() const;
    const ByteType* getStart();

    SizeType getSize() const;
    SizeType getSize();

    ConstRawFrameViewType getAsRawFrameView() const;
    ConstRawFrameViewType getAsRawFrameView();

    std::optional<FrameInfo> getFrameInfo() const;
    std::optional<FrameInfo> getFrameInfo();
protected:
    void setFrameSize(SizeType size);

private:
    const ByteType* m_start;
    SizeType m_size;
    const std::optional<FrameInfo> m_info;
};
    
} // namespace posnet


#endif //! VS_BASE_FRAME_H