#ifndef VS_ASSERT_H
#define VS_ASSERT_H

#include <sstream>
#include <exception>
#include <stdexcept>

#define ASSERTION(expr, except_type, msg)    \
    {                                           \
        if (!static_cast<bool>(expr)) {                    \
            std::stringstream ss;   \
            ss << __FILE__ ":" << __LINE__ << ": " msg;  \
            throw except_type(ss.str());                \
        }                               \
    }

#endif //! VS_ASSERT_H
