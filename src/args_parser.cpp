#include "include/utils/args_parser.h"

#include "include/utils/algorithms.h"

#include <algorithm>
#include <array>
#include <cstring>

namespace posnet::utils::args {

Parser::Parser(const int argc, char** argv):
m_argc(argc),
m_argv(argv),
m_argsPattern(),
m_parsedArgs()
{}

Parser& Parser::addArgPattern(std::string_view argName) &
{
    m_argsPattern.emplace_back(argName);
    return *this;
}

const std::vector<std::pair<std::string, std::string>>& Parser::parse()
{
    for (auto i = 1; i < m_argc; ++i) {
        const std::string_view options{ m_argv[i] };

        for (const auto& argName : m_argsPattern) {
             if (const auto startPos = options.find(argName); startPos != std::string_view::npos) {
                 if (options.size() > argName.size()) {
                    if (options[argName.size()] == '=') {
                        auto parsedArgValue = options.substr(startPos + argName.size(), options.size() - startPos);
                        parsedArgValue = parsedArgValue.substr(1, parsedArgValue.size() - 1);
                        m_parsedArgs.emplace_back(argName, std::move(parsedArgValue));
                    }
                } else {
                    m_parsedArgs.emplace_back(argName, std::move(options));
                }
                break;
            }
        }
    }

    return m_parsedArgs;
}

bool Parser::hasArg(std::string_view argName)
{
    const auto it = std::find_if(m_parsedArgs.cbegin(), m_parsedArgs.cend(), [argName](const auto& arg) {
        return (arg.first == argName);
    });

    const auto presented = (it != m_parsedArgs.cend());
    return presented;
}

template<> std::optional<std::string> Parser::getArgValue(const std::string_view argName, const char /*delimiter*/)
{
    const auto it = std::find_if(m_parsedArgs.cbegin(), m_parsedArgs.cend(), [argName](const auto& arg) {
        return (arg.first == argName);
    });

    if (it == m_parsedArgs.cend()) {
        return std::nullopt;
    }

    const auto& [_, parsedArgValue] = *it;
    return parsedArgValue;
}

template<> std::optional<int> Parser::getArgValue(const std::string_view argName, const char /*delimiter*/)
{
    const auto it = std::find_if(m_parsedArgs.cbegin(), m_parsedArgs.cend(), [argName](const auto& arg) {
        return (arg.first == argName);
    });

    if (it == m_parsedArgs.cend()) {
        return std::nullopt;
    }

    const auto& [_, parsedArgValue] = *it;
    return std::atoi(parsedArgValue.data());
}

template<> std::optional<double> Parser::getArgValue(std::string_view argName, char delimiter)
{
    const auto it = std::find_if(m_parsedArgs.cbegin(), m_parsedArgs.cend(), [argName](const auto& arg) {
        return (arg.first == argName);
    });

    if (it == m_parsedArgs.cend()) {
        return std::nullopt;
    }

    const auto& [_, parsedArgValue] = *it;
    return std::atof(parsedArgValue.data());
}

template<> std::optional<bool> Parser::getArgValue(const std::string_view argName, const char /*delimiter*/)
{
    const auto it = std::find_if(m_parsedArgs.cbegin(), m_parsedArgs.cend(), [argName](const auto& arg) {
        return (arg.first == argName);
    });

    if (it == m_parsedArgs.cend()) {
        return std::nullopt;
    }

    const auto& [_, parsedArgValue] = *it;
    if (parsedArgValue == "true" || parsedArgValue == "1") {
        return true;
    } else if (parsedArgValue == "false" || parsedArgValue == "0") {
        return false;
    } else {
        return std::nullopt;
    }
}

template<> std::optional<std::vector<std::string>> Parser::getArgValue(const std::string_view argName, const char delimiter)
{
    const auto it = std::find_if(m_parsedArgs.cbegin(), m_parsedArgs.cend(), [argName](const auto& arg) {
        return (arg.first == argName);
    });

    if (it == m_parsedArgs.cend()) {
        return std::nullopt;
    }

    const auto& [_, parsedArgValue] = *it;
    auto values = posnet::utils::SplitString(parsedArgValue, delimiter);
    if (!values.empty()) {
        return values;
    }

    return std::nullopt;
}

template<> std::optional<std::vector<int>> Parser::getArgValue(const std::string_view argName, const char delimiter)
{
    const auto it = std::find_if(m_parsedArgs.cbegin(), m_parsedArgs.cend(), [argName](const auto& arg) {
        return (arg.first == argName);
    });

    if (it == m_parsedArgs.cend()) {
        return std::nullopt;
    }

    const auto& [_, parsedArgValue] = *it;
    auto values = posnet::utils::SplitString(parsedArgValue, delimiter);
    if (!values.empty()) {
        std::vector<int> intValues;
        for (auto v : values) {
            intValues.push_back(std::atoi(v.data()));
        }

        return intValues;
    }

    return std::nullopt;
}

template<> std::optional<std::vector<double>> Parser::getArgValue(std::string_view argName, char delimiter)
{
     const auto it = std::find_if(m_parsedArgs.cbegin(), m_parsedArgs.cend(), [argName](const auto& arg) {
        return (arg.first == argName);
    });

    if (it == m_parsedArgs.cend()) {
        return std::nullopt;
    }

    const auto& [_, parsedArgValue] = *it;
    auto values = posnet::utils::SplitString(parsedArgValue, delimiter);
    return std::nullopt; 
    if (!values.empty()) {
        std::vector<double> doubleValues;
        for (auto v : values) {
            doubleValues.push_back(std::atof(v.data()));
        }

        return doubleValues;
    }

    return std::nullopt;
}

template<> std::optional<std::vector<bool>> Parser::getArgValue(const std::string_view argName, const char delimiter)
{
    const auto it = std::find_if(m_parsedArgs.cbegin(), m_parsedArgs.cend(), [argName](const auto& arg) {
        return (arg.first == argName);
    });

    if (it == m_parsedArgs.cend()) {
        return std::nullopt;
    }

    const auto& [_, parsedArgValue] = *it;
    auto values = posnet::utils::SplitString(parsedArgValue, delimiter);
    if (!values.empty()) {
        std::vector<bool> boolValues;
        for (auto v : values) {
            if (v == "true" || v == "1") {
                boolValues.push_back(true);
            } else if (v == "false" || v == "0") {
                boolValues.push_back(false);
            } else {
                return std::nullopt;
            }
        }

        return boolValues;
    }

    return std::nullopt;
}

template<> std::optional<std::unordered_set<std::string>> Parser::getArgValue(const std::string_view argName, const char delimiter)
{
    const auto args = getArgValue<std::vector<std::string>>(argName, delimiter);
    if (args.has_value()) {
        return std::unordered_set<std::string>{ args->begin(), args->end() };
    } else {
        return std::nullopt;
    }
}

template<> std::optional<std::unordered_set<int>> Parser::getArgValue(const std::string_view argName, const char delimiter)
{
    const auto args = getArgValue<std::vector<int>>(argName, delimiter);
    if (args.has_value()) {
        return std::unordered_set<int>{ args->begin(), args->end() };
    } else {
        return std::nullopt;
    }
}

template<> std::optional<std::unordered_set<double>> Parser::getArgValue(const std::string_view argName, const char delimiter)
{
    const auto args = getArgValue<std::vector<double>>(argName, delimiter);
    if (args.has_value()) {
        return std::unordered_set<double>{ args->begin(), args->end() };
    } else {
        return std::nullopt;
    }
}

template<> std::optional<std::unordered_set<bool>> Parser::getArgValue(const std::string_view argName, const char delimiter)
{
    const auto args = getArgValue<std::vector<bool>>(argName, delimiter);
    if (args.has_value()) {
        return std::unordered_set<bool>{ args->begin(), args->end() };
    } else {
        return std::nullopt;
    }
}


} // namespace posnet::utils::args
