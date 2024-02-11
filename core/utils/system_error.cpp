#include "system_error.h"

#include <string.h>
#include <errno.h>

namespace posix::error {

std::string GetLastSysError()
{
    return strerror(errno);
}

} //! namespace posix::error