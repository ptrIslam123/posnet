#ifndef VS_ARGS_PARSER_H
#define VS_ARGS_PARSER_H

#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <unordered_set>
#include <optional>
#include <initializer_list>
#include <span>

namespace posnet::utils::args {

class Parser final {
public:
    explicit Parser(int argc, char** argv);

    Parser& addArgPattern(std::string_view argName) &;

    bool hasArg(std::string_view argName);

    template<typename T>
    std::optional<T> getArgValue(std::string_view argName, char delimiter);

    const std::vector<std::pair<std::string, std::string>>& parse();

private:
    int m_argc;
    const char* const* m_argv;
    std::vector<std::string /*argName*/> m_argsPattern;
    std::vector<std::pair<std::string /*argName*/, std::string /*parsedArgValue*/>> m_parsedArgs;
};

template<> std::optional<std::string> Parser::getArgValue(std::string_view argName, char delimiter);
template<> std::optional<int> Parser::getArgValue(std::string_view argName, char delimiter);
template<> std::optional<double> Parser::getArgValue(std::string_view argName, char delimiter);
template<> std::optional<bool> Parser::getArgValue(std::string_view argName, char delimiter);

template<> std::optional<std::vector<std::string>> Parser::getArgValue(std::string_view argName, char delimiter);
template<> std::optional<std::vector<int>> Parser::getArgValue(std::string_view argName, char delimiter);
template<> std::optional<std::vector<double>> Parser::getArgValue(std::string_view argName, char delimiter);
template<> std::optional<std::vector<bool>> Parser::getArgValue(std::string_view argName, char delimiter);

template<> std::optional<std::unordered_set<std::string>> Parser::getArgValue(std::string_view argName, char delimiter);
template<> std::optional<std::unordered_set<int>> Parser::getArgValue(std::string_view argName, char delimiter);
template<> std::optional<std::unordered_set<double>> Parser::getArgValue(std::string_view argName, char delimiter);
template<> std::optional<std::unordered_set<bool>> Parser::getArgValue(std::string_view argName, char delimiter);

} // namespace posnet::utils


#endif //! VS_ARGS_PARSER_H
