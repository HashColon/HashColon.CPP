#ifndef _HG_HASHCOLON_STRINGHELPER
#define _HG_HASHCOLON_STRINGHELPER

#include <HashColon/header>

#include <string>
#include <vector>

// boost libraries should be considered to be removed
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

namespace HashColon
{

    /// @brief Trim the whitespaces in the front of the string
    /// @details This function modifies the given string
    /// @param s String to trim
    /// @param t Characters to remove, white spaces are in default
    /// @return Front-trimmed string
    std::string &TrimStart(std::string &s, const char *t = " \t\n\r\f\v");

    /// @brief Trim the whitespaces in the end of the string
    /// @details This function modifies the given string
    /// @param s String to trim
    /// @param t Characters to remove, white spaces are in default
    /// @return End-trimmed string
    std::string &TrimEnd(std::string &s, const char *t = " \t\n\r\f\v");

    /// @brief Trim the whitespaces in both ends of the string
    /// @details This function modifies the given string
    /// @param s String to trim
    /// @param t Characters to remove, white spaces are in default
    /// @return Trimmed string
    std::string &Trim(std::string &s, const char *t = " \t\n\r\f\v");

    /// @brief Copy version of TrimStart
    /// @details This function does not modify the given string
    /// @param s String to trim
    /// @param t Characters to remove, white spaces are in default
    /// @return Front-trimmed string
    std::string TrimStartCopy(std::string s, const char *t = " \t\n\r\f\v");

    /// @brief Copy version of TrimEnd
    /// @details This function does not modify the given string
    /// @param s String to trim
    /// @param t Characters to remove, white spaces are in default
    /// @return End-trimmed string
    std::string TrimEndCopy(std::string s, const char *t = " \t\n\r\f\v");

    /// @brief Copy version of Trim
    /// @details This function does not modify the given string
    /// @param s String to trim
    /// @param t Characters to remove, white spaces are in default
    /// @return Trimmed string
    std::string TrimCopy(std::string s, const char *t = " \t\n\r\f\v");

    /// @brief Split string with given tokens
    /// @param s String to be splitted
    /// @param spliter Splitter token characters
    /// @return Splitted string in vector of strings
    std::vector<std::string> Split(std::string s, const char *splitter);

    /// @brief Make the string to lowercase-only
    /// @details This function modifies the given string
    /// @param s String to be lowercased
    /// @return Lower-cased string
    std::string &ToLower(std::string &s);

    /// @brief Make the string to uppercase-only
    /// @details This function modifies the given string
    /// @param s String to be uppercased
    /// @return Upper-cased string
    std::string &ToUpper(std::string &s);

    /// @brief Make the string to lowercase-only
    /// @details This function does not modify the given string
    /// @param s String to be lowercased
    /// @return Lower-cased string
    std::string ToLowerCopy(std::string s);

    /// @brief Make the string to uppercase-only
    /// @details This function does not modify the given string
    /// @param s String to be uppercased
    /// @return Upper-cased string
    std::string ToUpperCopy(std::string s);
}

/* Inline function implementation */

namespace HashColon
{
    // trim from left
    inline std::string &TrimStart(std::string &s, const char *t)
    {
        s.erase(0, s.find_first_not_of(t));
        return s;
    }

    // trim from right
    inline std::string &TrimEnd(std::string &s, const char *t)
    {
        s.erase(s.find_last_not_of(t) + 1);
        return s;
    }

    // trim from left & right
    inline std::string &Trim(std::string &s, const char *t)
    {
        return TrimStart(TrimEnd(s, t), t);
    }

    // copying versions
    inline std::string TrimStartCopy(std::string s, const char *t)
    {
        return TrimStart(s, t);
    }

    inline std::string TrimEndCopy(std::string s, const char *t)
    {
        return TrimEnd(s, t);
    }

    inline std::string TrimCopy(std::string s, const char *t)
    {
        return Trim(s, t);
    }

    /* functions using boost libraries are excluded */

    // split string
    inline std::vector<std::string> Split(std::string s, const char *splitter)
    {
        std::vector<std::string> re;
        boost::split(re, s, boost::is_any_of(splitter));
        return re;
    }

    // case convolution
    inline std::string &ToLower(std::string &s)
    {
        boost::to_lower(s);
        return s;
    }

    inline std::string &ToUpper(std::string &s)
    {
        boost::to_upper(s);
        return s;
    }

    inline std::string ToLowerCopy(std::string s)
    {
        return ToLower(s);
    }

    inline std::string ToUpperCopy(std::string s)
    {
        return ToUpper(s);
    }
}

#endif
