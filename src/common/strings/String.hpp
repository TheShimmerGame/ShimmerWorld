
#pragma once

#include <string>

namespace ed::str
{
    std::string ConvertWide( const wchar_t * wide_string );
    std::wstring ConvertToWide( const char * narrow_string );
}
