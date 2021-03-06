#ifndef LIBJSON_STRINGS_H
#define LIBJSON_STRINGS_H

#ifdef JSON_UNICODE
    #define json_char wchar_t
    #define json_string std::wstring
    #define json_stringstream std::wstringstream
    #define json_strlen wcslen
    #define json_strcmp wcscmp
    #define TEXT(s) L ## s
#else
    #define json_char char
    #define json_string std::string
    #define json_stringstream std::stringstream
    #define json_strlen strlen
    #define json_strcmp strcmp
    #define TEXT(s) s
#endif

#endif
