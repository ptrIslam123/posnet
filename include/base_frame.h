#ifndef VS_BASE_FRAME_H
#define VS_BASE_FRAME_H

#include "definitions.h"

#include <span>
#include <optional>
#include <ostream>

namespace posnet {

class BaseFrame {
public:
    using ByteType = def::ByteType;
    using SizeType = def::SizeType;
    using RawFrameViewType = def::RawFrameViewType;
    using ConstRawFrameViewType = def::ConstRawFrameViewType;

    explicit BaseFrame(const ByteType* frameStart, SizeType frameSize);
    virtual ~BaseFrame() = 0;

    const ByteType* getStart() const;
    const ByteType* getStart();

    SizeType getSize() const;
    SizeType getSize();

    ConstRawFrameViewType getAsRawFrameView() const;
    ConstRawFrameViewType getAsRawFrameView();

    SizeType serialize(RawFrameViewType outBuffer);
    SizeType serialize(RawFrameViewType outBuffer) const;

protected:
    void setFrameSize(SizeType size);

private:
    const ByteType* m_start;
    SizeType m_size;
};

} // namespace posnet


#endif //! VS_BASE_FRAME_H