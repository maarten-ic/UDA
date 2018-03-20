#include "stringUtils.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef __GNUC__
#  include <strings.h>
#elif defined(_WIN32)
#  include <string.h>
#  define strncasecmp _strnicmp
#endif

#ifndef strcasestr
char *strcasestr(const char *haystack, const char *needle)
{
    char c, sc;
    size_t len;

    if ((c = *needle++) != 0) {
        c = (char)tolower(c);
        len = strlen(needle);
        do {
            do {
                if ((sc = *haystack++) == 0)
                    return (NULL);
            } while ((char)tolower((unsigned char)sc) != c);
        } while (strncasecmp(haystack, needle, len) != 0);
        haystack--;
    }
    return ((char *)haystack);
}
#endif

// Reverse a String

void reverseString(const char* in, char* out)
{
    int i, lstr = (int) strlen(in);
    out[lstr] = '\0';
    for (i = 0; i < lstr; i++) out[i] = in[lstr - 1 - i];
}

// Copy a String subject to a Maximum length constraint

void copyString(const char* in, char* out, int maxlength)
{
    int lstr = (int) strlen(in);
    if (lstr < maxlength)
        strcpy(out, in);
    else {
        strncpy(out, in, maxlength - 1);
        out[maxlength - 1] = '\0';
    }
}

char* FormatString(const char* fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);

    size_t len = vsnprintf(NULL, 0, fmt, vargs) + 1;

    va_end(vargs);
    va_start(vargs, fmt);

    char* strp = malloc(len * sizeof(char));
    vsnprintf(strp, len, fmt, vargs);

    va_end(vargs);

    return strp;
}

// Trim Trailing Space Characters from a String

char* TrimString(char* szSource)
{
    char* pszEOS;

    /*Set pointer to end of string to point to the character just
     *before the 0 at the end of the string.
     */
    pszEOS = szSource + strlen(szSource) - 1;

    while (pszEOS >= szSource && *pszEOS == ' ')
        *pszEOS-- = '\0';

    return szSource;
}

// Trim Leading Space Characters from a String

char* LeftTrimString(char* str)
{
    int i = 0, trim = 0, lstr;

    lstr = (int) strlen(str);

    while (str[trim] == ' ' && i++ <= lstr) trim++;

    if (trim > 0) {
        lstr = lstr - trim;
        for (i = 0; i < lstr; i++) str[i] = str[i + trim];
        str[lstr] = '\0';
    }
    return str;
}

#ifdef __GNUC__
void StringCopy(char* dest, const char* src, size_t len)
{
    strncpy(dest, src, len);
    dest[len-1] = '\0';
}

// Convert all LowerCase Characters to Upper Case

char* strupr(char* a)
{
    char* ret = a;

    while (*a != '\0') {
        if (islower (*a))
            *a = (char)toupper(*a);
        ++a;
    }

    return ret;
}


// Convert all UpperCase Characters to Lower Case

char* strlwr(char* a)
{
    char* ret = a;

    while (*a != '\0') {
        if (isupper (*a))
            *a = (char)tolower(*a);
        ++a;
    }

    return ret;
}

#endif

// Trim Internal Space Characters from a String

char* MidTrimString(char* str)
{
    int i = 0, j = 0, lstr;
    lstr = (int) strlen(str);
    for (i = 0; i < lstr; i++) {
        if (str[i] != ' ') str[j++] = str[i];
    }
    str[j] = '\0';
    return (str);
}

// Is the String an Integer Number? (Simple but not exhaustive Check)

int IsNumber(char* a)
{
    char* wrk = a;
    while (*wrk != '\0') {
        if (!isdigit (*wrk) && *wrk != '-' && *wrk != '+') return 0;
        ++wrk;
    }
    return 1;
}

// Is the String a Simple Float Number?

int IsFloat(char* a)
{
    char* wrk = a;
    while (*wrk != '\0') {
        if (!isdigit (*wrk) && *wrk != '-' && *wrk != '+' && *wrk != '.') return 0;
        ++wrk;
    }
    return 1;
}


// Is the String a Number List (#,#,#,#;#;#;#)?

int IsNumberList(char* a)
{
    char* wrk = a;
    while (*wrk != '\0') {
        if (!isdigit (*wrk) || *wrk != ',' || *wrk != ';') return 0;
        ++wrk;
    }
    if (a[0] == ',' || a[strlen(a)] == ',' ||
        a[0] == ';' || a[strlen(a)] == ';')
        return 0;

    return 1;
}

char* convertNonPrintable(char* str)
{

// Remove CR & LF Characters from a Number List

    char* ret = str;
    while (*str != '\0') {
        if (!isalpha (*str) && !isdigit(*str)
            && *str != ' ' && *str != '.' && *str != '+' && *str != '-')
            *str = ' ';
        ++str;
    }
    return ret;
}

char* convertNonPrintable2(char* str)
{

// Remove NonPrintable Characters from a String

    char* ret = str;
    while (*str != '\0') {
        if (*str < ' ' || *str > '~') *str = ' ';
        ++str;
    }
    return ret;
}

int IsLegalFilePath(const char* str)
{

// Basic check that the filename complies with good naming practice - some protection against malign embedded code!
// Test against the Portable Filename Character Set A-Z, a-z, 0-9, <period>, <underscore> and <hyphen> and <plus>
// Include <space> and back-slash for windows filenames only, forward-slash for the path seperator and $ for environment variables

// The API source argument can also be a server based source containing a ':' character
// The delimiter characters separating the device or format name from the source should have been split off of the path
//

    const char* tst = str;
    while (*tst != '\0') {
        if ((*tst >= '0' && *tst <= '9') || (*tst >= 'A' && *tst <= 'Z') || (*tst >= 'a' && *tst <= 'z')) {
            tst++;
            continue;
        }

        if (strchr("_-+./$:", *tst) != NULL) {
            tst++;
            continue;
        }

#ifdef _WIN32
	if (*tst == ' ' || *tst == '\\') {
            tst++;
            continue;
        }
#endif
        return 0;    // Error - not compliant!
    }
    return 1;
}

#if !defined(asprintf)
/*
 * Allocating sprintf
 */
int asprintf(char** strp, const char* fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);

    size_t len = vsnprintf(NULL, 0, fmt, vargs) + 1;

    va_end(vargs);
    va_start(vargs, fmt);

    *strp = malloc(len * sizeof(char));
    len = vsnprintf(*strp, len, fmt, vargs);

    va_end(vargs);
    return (int)len;
}
#endif

/**
 * Split a string using the given deliminator and return a list of the resultant tokens, with NULL indicating the end
 * of the list.
 *
 * The returned list should be freed after use with FreeSplitStringTokens().
 * @param string
 * @param delims
 * @return
 */
#ifdef _WIN32
#  define strtok_r strtok_s
#  define strdup _strdup
#endif 
 
char** SplitString(const char* string, const char* delims)
{
    char** names = NULL;
    char* tokptr = NULL;
    size_t num_names = 0;

    char* temp = strdup(string);
    char* tok = strtok_r(temp, delims, &tokptr);
    while (tok != NULL) {
        num_names++;
        names = (char**)realloc((void*)names, num_names * sizeof(char*));
        names[num_names - 1] = strdup(tok);
        tok = strtok_r(NULL, delims, &tokptr);
    }

    num_names++;
    names = (char**)realloc((void*)names, num_names * sizeof(char*));
    names[num_names - 1] = NULL;

    free(temp);
    return names;
}

char* StringReplaceAll(const char* string, const char* find, const char* replace)
{
    char* prev_string = NULL;
    char* new_string = strdup(string);

    do {
        free(prev_string);
        prev_string = new_string;
        new_string = StringReplace(prev_string, find, replace);
    } while (!StringEquals(prev_string, new_string));

    free(prev_string);
    return new_string;
}

char* StringReplace(const char* string, const char* find, const char* replace)
{
    if (find == NULL || find[0] == '\0') {
        return strdup(string);
    }

    char* idx = strstr(string, find);

    if (idx != NULL) {
        int diff = strlen(replace) - strlen(find);
        size_t len = strlen(string) + diff + 1;
        char* result = malloc(len);
        size_t offset = idx - string;
        strncpy(result, string, offset);
        strcpy(result + offset, replace);
        strcpy(result + offset + strlen(replace), idx + strlen(find));
        result[len - 1] = '\0';
        return result;
    } else {
        return strdup(string);
    }
}

/**
 * Free the token list generated by SplitString().
 * @param tokens
 */
void FreeSplitStringTokens(char*** tokens)
{
    size_t i = 0;
    while ((*tokens)[i] != NULL) {
        free((*tokens)[i]);
        ++i;
    }
    free(*tokens);
    *tokens = NULL;
}

bool StringEquals(const char* a, const char* b)
{
    if (a == NULL || b == NULL) return false;

    while (*a != '\0') {
        if (*b == '\0' || *a != *b)
            return false;
        ++a;
        ++b;
    }

    return *a == *b;
}

bool StringIEquals(const char* a, const char* b)
{
    if (a == NULL || b == NULL) return false;

    while (*a != '\0') {
        if (*b == '\0' || toupper(*a) != toupper(*b))
            return false;
        ++a;
        ++b;
    }

    return *a == *b;
}

bool StringEndsWith(const char* str, const char* find)
{
    if (str == NULL || find == NULL) return false;

    size_t len = strlen(str);
    size_t find_len = strlen(find);

    const char* a = str + len;
    const char* b = find + find_len;

    if (find_len > len) return false;

    size_t count = 0;
    while (count <= find_len) {
        if (*a != *b) return false;
        --a;
        --b;
        ++count;
    }

    return true;
}
