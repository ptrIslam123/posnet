#include "utils/system_error.h"

#include <string.h>
#include <errno.h>

namespace posnet::utils {

std::string GetLastSysError()
{
    return strerror(errno);
}

} //! namespace posnet::utils