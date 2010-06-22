#ifndef libJSON_DEBUG_H
#define libJSON_DEBUG_H

#ifdef DEBUG
    #define libJSON_ASSERT(condition, message)\
	    if ((!(condition)) && (Error)) Error(message);
    #define libJSON_FAIL(message)\
        if (Error) Error(message);

    #include "JSON_Strings.h"

	extern "C"{
		typedef void (*ErrorCallback)(const json_char *);
	}
#else
    #define libJSON_ASSERT(condition, message)
    #define libJSON_FAIL(message)
#endif

#endif
