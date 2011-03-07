#include <sstream>
#include "string.h"

std::string util::replace_all(std::string src, std::string const& origin_text, std::string const& replacement)
{
    std::string::size_type origin_length = origin_text.size();
    std::string::size_type replace_length = replacement.size();
    for (std::string::size_type occurrence = src.find(origin_text);
         std::string::npos != occurrence;
         occurrence = src.find(origin_text, occurrence))
    {
        src.replace(occurrence, origin_length, replacement);
        occurrence += replace_length;
    }
    return src;
}

template <typename _T>
static std::string str_from_something(_T const& t)
{
    std::stringstream oss;
    oss << t;
    return oss.str();
}

std::string util::str(int i)
{
    return str_from_something(i);
}

std::string util::str(double d)
{
    return str_from_something(d);
}