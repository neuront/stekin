#include <sstream>
#include "string.h"

std::string util::replace_all(std::string src
                            , std::string const& origin_text
                            , std::string const& replacement)
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
    std::stringstream ss;
    ss << t;
    return ss.str();
}

std::string util::str(int i)
{
    return str_from_something(i);
}

std::string util::str(long i)
{
    return str_from_something(i);
}

std::string util::str(long long i)
{
    return str_from_something(i);
}

std::string util::str(double d)
{
    return str_from_something(d);
}

std::string util::str(bool b)
{
    return b ? "true" : "false";
}

std::string util::str(void const* p)
{
    return str_from_something(p);
}

std::string util::str(mpz_class const& z)
{
    return z.get_str();
}

std::string util::str(mpf_class const& f)
{
    return str_from_something(f);
}
