
#include "String.hpp"

//#include <boost/nowide/convert.hpp>

std::string ed::str::ConvertWide( const wchar_t * wide_string )
{
    return {};
    //return boost::nowide::narrow( wide_string );
}

std::wstring ed::str::ConvertToWide( const char * narrow_string )
{
    //return boost::nowide::widen( narrow_string );
    return {};
}

