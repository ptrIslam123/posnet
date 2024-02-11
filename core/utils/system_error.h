#ifndef VS_SYS_ERROR_H
#define VS_SYS_ERROR_H

#include <string>

namespace posix::error {

std::string GetLastSysError();

} //! namespace posix::error

#endif //! VS_SYS_ERROR_H