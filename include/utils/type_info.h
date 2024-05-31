#ifndef VS_TYPE_INFO_H
#define VS_TYPE_INFO_H

#include <typeinfo>
#include <string>
#include <cxxabi.h>
#include <string>
#include <string_view>

namespace posnet::utils::type::info {

template<typename T>
std::string GetTypeName();

std::string DemangleTypeName(std::string_view typeName);

template<typename T>
std::string GetDemangleTypeName();

template<typename T>
inline std::string GetTypeName()
{
    return std::string{ typeid(T).name() };
}

inline std::string DemangleTypeName(std::string_view typeName)
{
    int status;
    char* realName = abi::__cxa_demangle(typeName.data(), 0, 0, &status);
    if (status == 0) {
        std::string name(realName);
        free(realName);
        return name;
    } else {
        return {};
    }
}

template<typename T>
std::string GetDemangleTypeName()
{
    return DemangleTypeName(GetTypeName<T>());
}

} //! namespace posnet::utils::type::info

#endif //! VS_TYPE_INFO_H