#ifndef STRINGBUILDER_H_
#define STRINGBUILDER_H_

// single header string library to use define
// #define STRINGBUILDER_IMPLEMENTATION
// in exactly one file (preferably the main file)

// tips:
// StringBuilder is null terminated and as such getting null terminated strings from it is trivial simply
// get the .data pointer
// use "%*.s" format to print lengthed strings example:
// printf("%*.s\n", string.count, string.data);
// you can get unicode character literals by prefixing the character with U example
// U'漢'
// you can trivially convert string literals to lengthed strings using string_literal or the shortened
// version sl you could also use the string_from_cstring function but this requires calling strlen
// whereas string_literal uses sizeof to get the length of the string

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <stdarg.h>
#include <stdio.h>
#include <uchar.h>

#include "memory.h"

// dynamically allocated string, automatically null terminated (doesn't prevent you from adding null characters into middle of string)
// the len doesn't include null terminator and is the number of bytes not characters
// owns its allocation and is mutable
typedef struct StringBuilder {
    char*     data;             // pointer to buffer
    ptrdiff_t count;            // length of string (in bytes)
    ptrdiff_t capacity;         // capacity of buffer
} StringBuilder;

// basic lengthed string, may not own its contents, may be a slice or a full string (not nessisarily null terminated)
// equivelent to rust's &str type or std::string_view in c++, you can use printf("%.*s", string.count, string.data); to print a string
typedef struct String {
    const char* data;           // pointer to buffer
    ptrdiff_t   count;          // length of string (in bytes)
} String;

typedef char32_t Codepoint; // represents one unicode codepoint

// CONVENIENT MACROS

#define string_literal(str) ((String){.data = u8 ## str, .count = sizeof(u8 ## str) - 1}) // creates a String from a string literal
#define sl string_literal

#define string_char_literal(c) (Codepoint){U ## c}
#define cl(c) string_char_literal(c)

// STRING FUNCTIONS

String string_build_slice(StringBuilder sb, ptrdiff_t start, ptrdiff_t stop);
String string_build(StringBuilder sb);
String string_slice(String string, ptrdiff_t start, ptrdiff_t stop);

String string_from_cstring(const char* str);
char* string_to_cstring(String string);
char* string_to_cstringn(String string, char* buff, ptrdiff_t n);

// STRINGBUILDER FUNCTIONS

StringBuilder string_make(ptrdiff_t initial_capacity);
void string_free(StringBuilder* sb);
void string_clear(StringBuilder* sb);

void string_setcount(StringBuilder* sb, ptrdiff_t new_count);
void string_setcapacity(StringBuilder* sb, ptrdiff_t new_capacity);
void string_expand(StringBuilder* sb, ptrdiff_t n);
void string_expand_maybe(StringBuilder* sb, ptrdiff_t n);
void string_shrink_to_fit(StringBuilder* sb);

void string_append_byte(StringBuilder* sb, char c);
void string_append(StringBuilder* sb, Codepoint c);
void string_append_string(StringBuilder* sb, String string);

Codepoint string_pop(StringBuilder* sb);
void string_popn(StringBuilder* sb, ptrdiff_t n_codepoints);
void string_popn_bytes(StringBuilder* sb, ptrdiff_t n_bytes);

void string_insert_byte(StringBuilder* sb, ptrdiff_t index, char c);
void string_insert(StringBuilder* sb, ptrdiff_t index, Codepoint c);
void string_insert_string(StringBuilder* sb, ptrdiff_t index, String string);

Codepoint string_remove(StringBuilder* sb, ptrdiff_t index);
void string_removen(StringBuilder* sb, ptrdiff_t index, ptrdiff_t n_codepoints);
void string_removen_bytes(StringBuilder* sb, ptrdiff_t index, ptrdiff_t n_bytes);

void string_replace(StringBuilder* sb, ptrdiff_t index, Codepoint c);
void string_replacen(StringBuilder* sb, ptrdiff_t index, String string, ptrdiff_t n_codepoints);
void string_replacen_bytes(StringBuilder* sb, ptrdiff_t index, String string, ptrdiff_t n_bytes);

StringBuilder string_clone(String string);

// ALGORITHMS

uint64_t string_hash(String string);
bool string_compare(String lhs, String rhs);

ptrdiff_t string_findc(String haystack, Codepoint needle, ptrdiff_t start);
ptrdiff_t string_find(String haystack, String needle, ptrdiff_t start);

String string_tokenc(String haystack, Codepoint delimiter, ptrdiff_t* index);
String string_token(String haystack, String delimiter, ptrdiff_t* index);

// UTF8

#define string_uchar(c) ((uint8_t)(c)) // converts char to unsigned char for unsigned promotions and comparison

#define STRING_REPLACEMENT_CODEPOINT (Codepoint){0xFFFD}
#define STRING_REPLACEMENT_CHAR (Utf8Char) {.data = u8"�", .count = 3}

#define STRING_MIN_ONE_BYTE (Codepoint){0x0}
#define STRING_MIN_TWO_BYTE (Codepoint){0x80}
#define STRING_MIN_THREE_BYTE (Codepoint){0x800}
#define STRING_MIN_FOUR_BYTE (Codepoint){0x10000}

#define STRING_MAX_ONE_BYTE (Codepoint){0x7F}
#define STRING_MAX_TWO_BYTE (Codepoint){0x7FF}
#define STRING_MAX_THREE_BYTE (Codepoint){0xFFFF}
#define STRING_MAX_FOUR_BYTE (Codepoint)0x10FFFF

#define STRING_MIN_CODEPOINT (Codepoint){0x0}
#define STRING_MAX_CODEPOINT (Codepoint){0x10FFFF}

// represents the utf8 representation of a character
typedef struct Utf8Char {
    char data[4];
    uint32_t count;
} Utf8Char;

ptrdiff_t string_utf8_len(char header_byte);
bool string_get_valid(String string, ptrdiff_t index, ptrdiff_t* len_out);
Utf8Char string_utf8(Codepoint codepoint);

Codepoint string_get(String string, ptrdiff_t index);
bool string_try_get(String string, ptrdiff_t index, Codepoint* out);

ptrdiff_t string_iterate(String string, ptrdiff_t index);
ptrdiff_t string_iterate_back(String string, ptrdiff_t index);

Codepoint string_next_codepoint(String string, ptrdiff_t* index);
Codepoint string_prev_codepoint(String string, ptrdiff_t* index);

Codepoint string_peek_next(String string, ptrdiff_t index);
Codepoint string_peek_prev(String string, ptrdiff_t index);

ptrdiff_t string_get_codepoint_count(String string);
ptrdiff_t string_validate(String string);

// File Handling and IO

void string_print(String string);
void string_println(String string);
void string_scanln(StringBuilder* string);

ptrdiff_t string_get_file_length(FILE* file);
void string_write_file(FILE* file, String string);
void string_read_file(FILE* file, StringBuilder* string);

void string_write_entire_file(const char* filename, String string);
void string_read_entire_file(const char* filename, StringBuilder* string);

// ASCII PARSING

bool string_is_digit(char byte, int32_t* out);
bool string_is_hexdigit(char byte, int32_t* out);

bool string_is_ascii(String string);
bool string_is_ascii_alpha(char byte);
bool string_is_ascii_whitespace(char byte);
bool string_is_ascii_printable(char byte);

void string_to_uppercase_ascii(StringBuilder* string);
void string_to_lowercase_ascii(StringBuilder* string);

String string_trim_whitespace(String string);
String string_trim_leading_whitespace(String string);
String string_trim_trailing_whitespace(String string);

// STRING PARSING

int64_t string_to_int(String string);
uint64_t string_to_int_hex(String string);
double string_to_float(String string);

void string_format(StringBuilder* string, const char* fmt, ...);

#ifdef STRINGBUILDER_IMPLEMENTATION

#include <stdlib.h>
#include <assert.h>
#include <string.h>

/////////////////////////////////////
// STRING FUNCTION IMPLEMENTATIONS //
/////////////////////////////////////

// creates a string from [start, stop)
String string_build_slice(StringBuilder sb, ptrdiff_t start, ptrdiff_t stop) {
    if (start > stop || stop > sb.count) return (String) {0};

    return (String)
    {
        .count = stop - start,
        .data = sb.data + start
    };
}
// creates a string from [start, stop)
String string_slice(String string, ptrdiff_t start, ptrdiff_t stop) {
    if (start > stop || stop > string.count) return (String) {0};

    return (String) {
        .data = string.data + start,
        .count = stop - start,
    };
}
// takes the stringbuilder and converts it to a string
String string_build(StringBuilder sb) {
    return (String) {.data = sb.data, .count = sb.count};
}
// creates a view from a null terminated string
String string_from_cstring(const char* str) {
    if (!str) return (String) {0};
    return (String) {
        .data = str,
        .count = strlen(str)
    };
}
// creates a null terminated string from a view in the buffer provided copies min(n - 1, string.count) bytes into buffer
// returns buff
char* string_to_cstringn(String string, char* buff, ptrdiff_t n) {
    if (string.count + 1 < n) n = string.count + 1;
    memcpy(buff, string.data, n - 1);
    buff[n - 1] = '\0';
    return buff;
}

// creates a null terminated string from a view using the provided allocator (uses global if NULL is provided)
// returns pointer to the null string
char* string_to_cstring(String string) {
    char* ptr = malloc(string.count + 1);
    assert(ptr);
    memcpy(ptr, string.data, string.count);
    ptr[string.count] = '\0';
    return ptr;
}

////////////////////////////////////////////
// STRINGBUILDER FUNCTION IMPLEMENTATIONS //
////////////////////////////////////////////

// creates a string with an initial capacity and an allocator
StringBuilder string_make(ptrdiff_t initial_capacity) {
    StringBuilder sb = (StringBuilder) {.data = malloc(initial_capacity), .capacity = initial_capacity};
    assert(sb.data);
    memset(sb.data, 0, initial_capacity);
    return sb;
}
// frees associated memory and nulls pointers
void string_free(StringBuilder* sb) {
    free(sb->data);
    *sb = (StringBuilder){0};
}
// clears the string and zeros the buffer
void string_clear(StringBuilder* sb) {
    memset(sb->data, 0, sb->count);
    sb->count = 0;
}
// sets capacity to be new_capacity does nothing if new_capacity == sb.capacity
void string_setcapacity(StringBuilder* sb, ptrdiff_t new_capacity) {
    if (new_capacity == sb->capacity)
        return;

    sb->data = realloc(sb->data, new_capacity);
    assert(sb->data && "realloc failed loser!");

    sb->capacity = new_capacity;
    if (sb->count >= new_capacity) {
        sb->count = new_capacity - 1;
        sb->data[sb->count] = '\0';
    } else {
        memset(sb->data + sb->count, 0, new_capacity - sb->count);
    }
}
// sets count to new_count expands capacity as needed
void string_setcount(StringBuilder* sb, ptrdiff_t new_count) {
    if (new_count >= sb->capacity) {
        string_setcapacity(sb, new_count + 1);
    }
    sb->count = new_count;
    sb->data[sb->count] = '\0';
}
// expands the allocation by a factor of 2 or len + n whichever is larger
void string_expand(StringBuilder* sb, ptrdiff_t n) {
    ptrdiff_t double_cap = sb->capacity * 2;
    ptrdiff_t len_plus_n = sb->count + n;

    sb->capacity = double_cap > len_plus_n ? double_cap : len_plus_n;
    
    if (sb->count + n + 1 > sb->capacity * 2) {
        sb->capacity = sb->count + n + 1;
    } else {
        sb->capacity *= 2;
    }

    sb->data = realloc(sb->data, sb->capacity);
    assert(sb->data && "realloc failed loser!");
    memset(sb->data + sb->count, 0, sb->capacity - sb->count);
}
// expands to be able to store at least n extra items, does nothing if there is already enough capacity
// expands by a factor of 2 or up to len + n whichever is larger
void string_expand_maybe(StringBuilder* sb, ptrdiff_t n) {
    if (sb->count + n + 1 <= sb->capacity) return;
    string_expand(sb, n);
}
// shrinks the buffer to fit the current string plus a null-terminator
void string_shrink_to_fit(StringBuilder* sb) {
    if (sb->count + 1 == sb->capacity) return;
    string_setcapacity(sb, sb->count + 1);
}
// appends a byte does not check if it is valid utf8
void string_append_byte(StringBuilder* sb, char c) {
    string_expand_maybe(sb, 1);
    sb->data[sb->count++] = c;
    sb->data[sb->count] = '\0';
}
// appends a unicode character to the end of the string
void string_append(StringBuilder* sb, Codepoint c) {
    Utf8Char c4 = string_utf8(c);
    string_expand_maybe(sb, c4.count);

    memcpy(&sb->data[sb->count], c4.data, c4.count);
    sb->count += c4.count;
    sb->data[sb->count] = '\0';
}
// appends a string to the end of the string
void string_append_string(StringBuilder* sb, String string) {
    string_expand_maybe(sb, string.count);
    memcpy(&sb->data[sb->count], string.data, string.count);
    sb->count += string.count;
    sb->data[sb->count] = '\0';
}
// pops the last character of the string and returns it, returns '\0' if string is empty
Codepoint string_pop(StringBuilder* sb) {
    if (sb->count == 0) return '\0';
    ptrdiff_t index;
    if (!(sb->data[sb->count - 1] & 0x80)) {
        index = sb->count - 1;
    } else {
        index = string_iterate_back(string_build(*sb), sb->count - 1);
    }
    Codepoint c = string_get(string_build(*sb), index);

    sb->count = index;
    sb->data[sb->count] = '\0';
    return c;
}
// pops the last n characters of the string
void string_popn(StringBuilder* sb, ptrdiff_t n) {
    if (sb->count == 0) return;
    for (ptrdiff_t i = 0; i < n && sb->count != 0; i++) {
        string_pop(sb);
    }
}
// pops the last n bytes of the string
void string_popn_bytes(StringBuilder* sb, ptrdiff_t n) {
    if (sb->count == 0) return;

    if (sb->count < n) {
        sb->count = 0;
    } else {
        sb->count -= n;
    }
    sb->data[sb->count] = '\0';
}
// inserts unicode character such that the first byte of its utf8 representation is at index i moves everything in its way to the right
void string_insert(StringBuilder* sb, ptrdiff_t index, Codepoint c) {
    assert(index <= sb->count && "out of valid range to insert");
    Utf8Char c4 = string_utf8(c);
    string_expand_maybe(sb, c4.count);
    
    sb->count += c4.count;
    memmove(&sb->data[index+c4.count], &sb->data[index], sb->count - index);
    memcpy(&sb->data[index], c4.data, c4.count);
}
// inserts a byte to the specified index
void string_insert_byte(StringBuilder* sb, ptrdiff_t index, char c) {
    assert(index <= sb->count && "out of valid range to insert");
    string_expand_maybe(sb, 1);
    sb->count++;
    memmove(&sb->data[index+1], &sb->data[index], sb->count - index);
    sb->data[index] = c;
    sb->data[sb->count] = '\0';
}
// inserts string such that the first byte is at index i, moves everything in the way to the right
void string_insert_string(StringBuilder* sb, ptrdiff_t index, String string) {
    assert(index <= sb->count && "out of valid range to insert");
    string_expand_maybe(sb, string.count);

    sb->count += string.count;
    memmove(&sb->data[index+string.count], &sb->data[index], string.count);
    memcpy(&sb->data[index], string.data, string.count);
    sb->data[sb->count] = '\0';
}
// removes the char which starts at index i (if an index which isn't the start of a valid utf8 character)
// is given it will remove 1 byte
// is an ordered remove
Codepoint string_remove(StringBuilder* sb, ptrdiff_t index) {
    assert(index <= sb->count && "out of valid range to remove");
    ptrdiff_t end = string_iterate(string_build(*sb), index);
    ptrdiff_t size = end - index;

    Codepoint c = string_get(string_build(*sb), index);
    memmove(&sb->data[index], &sb->data[index+size], sb->count - index);
    sb->count-=size;

    sb->data[sb->count] = '\0';
    return c;
}
// removes n unicode codepoints starting at index, it will stop removing once it reaches sb.count
void string_removen(StringBuilder* sb, ptrdiff_t index, ptrdiff_t n) {
    assert(index <= sb->count && "out of valid range to remove");
    ptrdiff_t end = index;

    String string = string_build(*sb);

    for (ptrdiff_t i = 0; i < n || end > sb->count; i++) {
        end = string_iterate(string, end);
    }

    ptrdiff_t size = end - index;
    memmove(&sb->data[index], &sb->data[index+size], sb->count - index);
    sb->count -= size;
    sb->data[sb->count] = '\0';
}
// removes n bytes starting at index, it will stop removing once it reaches sb.count
void string_removen_bytes(StringBuilder* sb, ptrdiff_t index, ptrdiff_t n) {
    assert(index <= sb->count && "out of valid range to remove");

    if (index + n > sb->count) n = sb->count - index;

    memmove(&sb->data[index], &sb->data[index + n], sb->count - index);
    sb->count -= n;
    sb->data[sb->count] = '\0';
}
// replaces the Codepoint at index i with the codepoint provided
// optimised version of string_remove(sb, i); string_insert(sb, i, c)
void string_replace(StringBuilder* sb, ptrdiff_t index, Codepoint c) {
    assert(index < sb->count && "attempting to replace out of bounds");
    Utf8Char c4 = string_utf8(c);
    int32_t len;
    if (!(sb->data[index] & 0x80)) {
        len = 1;
    } else {
        len = string_utf8_len(sb->data[index]);
        if (len > 4) len = 1;
    }
    int32_t move_amount = (int32_t)c4.count - len;
    if (move_amount > 0)
        string_expand_maybe(sb, move_amount);

    memmove(&sb->data[index + c4.count], &sb->data[index + len], sb->count - index - len);
    memcpy(&sb->data[index], c4.data, c4.count);
    sb->count += move_amount;
    sb->data[sb->count] = '\0';
}

// replaces the Codepoint at index i with the string provided
// optimised version of string_removen(sb, i, n); string_insert_string(sb, i, string)
void string_replacen(StringBuilder* sb, ptrdiff_t index, String string, ptrdiff_t n_codepoints) {
    assert(index < sb->count && "attempting to replace out of bounds");
    ptrdiff_t end = index;
    for (ptrdiff_t i = 0; i < n_codepoints; i++) {
        end = string_iterate(string_build(*sb), end);
    }
    int64_t move_amount = string.count - (end - index);
    if (move_amount > 0)
        string_expand_maybe(sb, move_amount);

    memmove(&sb->data[index + string.count], &sb->data[end], sb->count - end);
    memcpy(&sb->data[index], string.data, string.count);
    sb->count += move_amount;
    sb->data[sb->count] = U'\0';
}

// replaces the Codepoint at index i with the string provided
// optimised version of string_removen(sb, i, n); string_insert_string(sb, i, string)
void string_replacen_bytes(StringBuilder* sb, ptrdiff_t index, String string, ptrdiff_t n_bytes) {
    assert(index < sb->count && "attempting to replace out of bounds");
    int64_t move_amount = string.count - n_bytes;
    if (move_amount > 0)
        string_expand_maybe(sb, move_amount);

    memmove(&sb->data[index + string.count], &sb->data[index + n_bytes], sb->count - index - n_bytes);
    memcpy(&sb->data[index], string.data, string.count);
    sb->count += move_amount;
    sb->data[sb->count] = U'\0';
}

// copies the contents of a string into a new string
StringBuilder string_clone(String string) {
    StringBuilder new_sb = string_make(string.count + 1);
    memcpy(new_sb.data, string.data, string.count);
    new_sb.count = string.count;
    return new_sb;
}

////////////////////////////////////////
// ALGORITHM FUNCTION IMPLEMENTATIONS //
////////////////////////////////////////

// implementation of fnv-1a hashing algorithm
// values for basis and prime taken from: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
// NOT CRYPTOGRAPHICALLY SECURE
uint64_t string_hash(String string) {
    uint64_t hash = 0xcbf29ce484222325;
    uint64_t fnv_prime = 0x100000001b3;

    for (ptrdiff_t i = 0; i < string.count; i++) {
        hash = hash * fnv_prime;
        hash = hash ^ string_uchar(string.data[i]); // UCHAR is to ensure unsigned promotion
    }
    return hash;
}

// returns true if the two strings are the same
// returns false if the two strings are different
bool string_compare(String lhs, String rhs) {
    if (lhs.count != rhs.count) return false;
    return memcmp(lhs.data, rhs.data, lhs.count) == 0;
}

// returns the index of first instance of a unicode character in the string
// return haystack.count if none are found
ptrdiff_t string_findc(String haystack, Codepoint needle, ptrdiff_t start) {
    Utf8Char needle_c4 = string_utf8(needle);
    return string_find(haystack, (String){.data = needle_c4.data, .count = needle_c4.count}, start);
}
// returns the index of first instance of the substring needle in the string
// return haystack.count if none are found
ptrdiff_t string_find(String haystack, String needle, ptrdiff_t start) {
    for (ptrdiff_t i = start; i < haystack.count; i++) {
        for (ptrdiff_t j = 0; j < needle.count; j++) {
            if (haystack.data[i + j] == needle.data[j]) {
                if (j == needle.count - 1) {
                    return i;
                }
            } else {
                break;
            }
        }
    }
    return haystack.count;
}

// tokenises the string haystack by the delimiter string, each time the function is called it will
// update index to be just after the last instance of the delimiter, end the loop when
// index == haystack.count returns slices of haystack
String string_token(String haystack, String delimiter, ptrdiff_t* index) {
    if (*index >= haystack.count) return (String) {0};

    ptrdiff_t i = string_find(haystack, delimiter, *index);
    String ret = {.count = i - *index, .data = haystack.data + *index};
    *index = i + delimiter.count;
    return ret;
}
// tokenises the string haystack by the delimiter character, each time the function is called it will
// update index to be just after the last instance of the delimiter, end the loop when
// index == haystack.count returns slices of haystack
String string_tokenc(String haystack, Codepoint delimiter, ptrdiff_t* index) {
    if (*index >= haystack.count) return (String) {0};
    Utf8Char delimiter_c4 = string_utf8(delimiter);

    return string_token(haystack, (String) {.data = delimiter_c4.data, .count = delimiter_c4.count}, index);
}
////////////////////////////////////////////
// UTD8 HANDLING FUNCTION IMPLEMENTATIONS //
////////////////////////////////////////////

// gets the length of the utf8 char from its header byte (returns 1 on invalid header)
ptrdiff_t string_utf8_len(char header_byte) {
    if ((header_byte & 0x80) == 0) return 1; // single byte case
    else if ((header_byte & 0xE0) == 0xC0) return 2;
    else if ((header_byte & 0xF0) == 0xE0) return 3;
    else if ((header_byte & 0xF8) == 0xF0) return 4;
    else return 1;
}

// gets the raw utf8 representation of the character c
// returns (Utf8Char){0} on error
Utf8Char string_utf8(Codepoint codepoint) {
    if (codepoint <= STRING_MAX_ONE_BYTE) return (Utf8Char){.data = {codepoint, 0, 0, 0}, .count = 1}; // single byte case

    ptrdiff_t length;
    if      (codepoint <= STRING_MAX_TWO_BYTE) length = 2;
    else if (codepoint <= STRING_MAX_THREE_BYTE) length = 3;
    else if (codepoint <= STRING_MAX_FOUR_BYTE) length = 4;
    else return STRING_REPLACEMENT_CHAR; // out of unicode range

    const uint8_t masks[]      = {0xc0, 0xe0, 0xf0, 0xf8};
    const uint8_t extensions[] = {0x80, 0xc0, 0xe0, 0xf0};

    Utf8Char ret = {.data = {0}, .count = length};

    switch (length)
    {
        case 4:
            ret.data[0] = ((codepoint >> 18) & ~masks[3]) | extensions[3];
            ret.data[1] = ((codepoint >> 12) & ~masks[0]) | extensions[0];
            ret.data[2] = ((codepoint >> 6)  & ~masks[0]) | extensions[0];
            ret.data[3] = ((codepoint)       & ~masks[0]) | extensions[0];
            break;
        case 3:
            ret.data[0] = ((codepoint >> 12) & ~masks[2]) | extensions[2];
            ret.data[1] = ((codepoint >> 6)  & ~masks[0]) | extensions[0];
            ret.data[2] = ((codepoint)       & ~masks[0]) | extensions[0];
            break;
        case 2:
            ret.data[0] = ((codepoint >> 6)  & ~masks[1]) | extensions[1];
            ret.data[1] = ((codepoint)       & ~masks[0]) | extensions[0];
            break;
        default:
            break;
    }
    return ret;
}
// gets unicode codepoint at the index specified
// returns STRING_REPLACEMENT_CODEPOINT (0xFFFD) for invalid utf8
// returns '\0' for out of bounds
Codepoint string_get(String string, ptrdiff_t index) {
    if (index >= string.count) return U'\0';

    if (!(string.data[index] & 0x80)) return (Codepoint)string.data[index]; // single byte case

    uint8_t mask       = 0xc0; // 0b11000000
    uint8_t extension  = 0x80; // 0b10000000

    const uint8_t masks[] = {0xc0, 0xe0, 0xf0, 0xf8};

    // gets the extension of the character
    ptrdiff_t length = string_utf8_len(string.data[index]);
    if (index + length > string.count) {
        // invalid header (recieved extension byte)
        // or character is longer then the remainder of the string
        return STRING_REPLACEMENT_CODEPOINT;
    }
    switch (length) {
        case 2: {
            Codepoint ret = ((uint8_t)string.data[index] & ~masks[1]) << 6 | ((uint8_t)string.data[index + 1] & ~masks[0]);
            if ((string.data[index + 1] & mask) != extension) return STRING_REPLACEMENT_CODEPOINT;
            if (ret < STRING_MIN_TWO_BYTE) return STRING_REPLACEMENT_CODEPOINT;
            return ret;
        } break;
        case 3: {
            Codepoint ret = ((uint8_t)string.data[index] & ~masks[2]) << 12 | ((uint8_t)string.data[index + 1] & ~masks[0]) << 6 | ((uint8_t)string.data[index + 2] & ~masks[0]);
            if ((string.data[index + 1] & mask) != extension) return STRING_REPLACEMENT_CODEPOINT;
            if ((string.data[index + 2] & mask) != extension) return STRING_REPLACEMENT_CODEPOINT;
            if (ret < STRING_MIN_THREE_BYTE) return STRING_REPLACEMENT_CODEPOINT;
            return ret;
        } break;
        case 4: {
            Codepoint ret = ((uint8_t)string.data[index] & ~masks[3]) << 18 | ((uint8_t)string.data[index + 1] & ~masks[0]) << 12 | ((uint8_t)string.data[index + 2] & ~masks[0]) << 6 | ((uint8_t)string.data[index + 3] & ~masks[0]);
            if ((string.data[index + 1] & mask) != extension) return STRING_REPLACEMENT_CODEPOINT;
            if ((string.data[index + 2] & mask) != extension) return STRING_REPLACEMENT_CODEPOINT;
            if ((string.data[index + 3] & mask) != extension) return STRING_REPLACEMENT_CODEPOINT;
            if (ret < STRING_MIN_FOUR_BYTE || ret > STRING_MAX_CODEPOINT) return STRING_REPLACEMENT_CODEPOINT;
            return ret;
        } break;
        default: return STRING_REPLACEMENT_CODEPOINT;
    }
}

// checks if the codepoint starting at index is valid and returns the length (1 if invalid)
// checks for invalid headers, overlong encodings, incorrect extension bytes, out of unicode range 
// and out of bounds accesses
bool string_get_valid(String string, ptrdiff_t index, ptrdiff_t* len_out) {
    if (index >= string.count) return false;
    *len_out = 1;
    ptrdiff_t length = 1;
    if ((string.data[index] & 0x80) == 0) return true; // single byte case
    else if ((string.data[index] & 0xE0) == 0xC0) length = 2;
    else if ((string.data[index] & 0xF0) == 0xE0) length = 3;
    else if ((string.data[index] & 0xF8) == 0xF0) length = 4;
    else return false; // invalid header

    if (string_uchar(string.data[index]) == 0xC0 || string_uchar(string.data[index]) == 0xC1) return false; // overlong encoding

    if (index + length > string.count) return false; // out of bounds
    if (string_uchar(string.data[index]) == 0xE0 && string_uchar(string.data[index + 1]) <= string_uchar(0xA0)) return false; // overlong encoding
    if (string_uchar(string.data[index]) == 0xF0 && string_uchar(string.data[index + 1]) <= string_uchar(0x90)) return false; // overlong encoding
    if (string_uchar(string.data[index]) == 0xF4 && string_uchar(string.data[index + 1]) >= string_uchar(0x90)) return false; // out of unicode range

    for (ptrdiff_t i = 1; i < length; i++) {
        if ((string.data[index + i] & 0xC0) != 0x80) return false; // incorrect number of extension bytes
    }
    *len_out = length;
    return true;
}

// returns the next index of a utf8 character in the string after index
// returns index + 1 on invalid utf8
// assets if index is out of bounds and returns string.count if the utf8 char extends out of bounds
// returns string.count on attempted out of bounds
ptrdiff_t string_iterate(String string, ptrdiff_t index) {
    assert(index < string.count && "attempting to index out of bounds");
    
    ptrdiff_t len = 0;
    if (!string_get_valid(string, index, &len)) {
        return index + 1;
    }
    return index + len;
}
// returns the prev index of a utf8 character in the string before index
// will iterate until first valid extension / one byte
// if done in middle of char it will return the beggining of that character
// assets if index is 0 or out of bounds returns 0 if it cannot find a non-extension byte
ptrdiff_t string_iterate_back(String string, ptrdiff_t index) {
    assert(index != 0 && index <= string.count && "attempting to iterate out of bounds");

    if (!(string.data[index - 1] & 0x80)) {
        return index - 1;
    }
    uint8_t mask       = 0xc0; // 0b11000000
    uint8_t extension  = 0x80; // 0b10000000
    
    ptrdiff_t len = 1;
    while (index - len != 0 && (string.data[index - len] & mask) == extension) {
        len++;
    }
    return index - len;
}
// gets the next character in the string
// increments index as it is called
// check when index >= string.count to end the loop
Codepoint string_next_codepoint(String string, ptrdiff_t* index) {
    if (*index >= string.count) return STRING_REPLACEMENT_CODEPOINT;
    Codepoint codepoint = string_get(string, *index);
    *index = string_iterate(string, *index);
    return codepoint;
}
// gets the prev character in the string
// decrements index as it is called
// check when index == 0 to end the loop
Codepoint string_prev_codepoint(String string, ptrdiff_t* index) {
    if (*index == 0) return STRING_REPLACEMENT_CODEPOINT;
    
    *index = string_iterate_back(string, *index);
    Codepoint codepoint = string_get(string, *index);
    return codepoint;
}

// gets the result from string_next_codepoint without updating the index
Codepoint string_peek_next(String string, ptrdiff_t index) {
    if (index >= string.count) return STRING_REPLACEMENT_CODEPOINT;
    Codepoint codepoint = string_get(string, index);
    return codepoint;
}
// gets the result from string_prev_codepoint without updating the index
Codepoint string_peek_prev(String string, ptrdiff_t index) {
    if (index == 0) return STRING_REPLACEMENT_CODEPOINT;
    Codepoint codepoint = string_get(string, string_iterate_back(string, index));
    return codepoint;
}
// gets the number of codepoint counts in the string
ptrdiff_t string_get_codepoint_count(String string) {
    ptrdiff_t codepoint_count = 0;
    for (ptrdiff_t i = 0; i < string.count; i = string_iterate(string, i)) {
        codepoint_count++;
    }
    return codepoint_count;
}

// todo add checks for overlong representations
// checks if a string is valid utf8
// only looks at the extension bits doesn't check if the values are in range
// returns the index of the first invalid byte (-1 if it succeeds) (string.count for incomplete character at end of string)
ptrdiff_t string_validate(String string) {
    uint8_t mask       = 0xc0; // 0b11000000
    uint8_t extension  = 0x80; // 0b10000000
    Codepoint codepoint;
    
    for (ptrdiff_t i = 0; i < string.count; i++) {
        if (!(string.data[i] & 0x80)) continue; // one byte case
        if ((string.data[i] & mask) == extension) return i; // character starts with the extension header
        
        ptrdiff_t len = string_utf8_len(string.data[i]);
        if (len == 0 || len > 4) return i; // invalid header
        if (!string_try_get(string, i, &codepoint)) {
            return i;
        }
        for (; len > 1 ; len--) {
            i++;
            if ((i == string.count && len != 2) || (string.data[i] & mask) != extension) return i;
        }
    }
    return -1;
}

// gets unicode codepoint at the index specified
// returns false for invalid utf8 or out of bounds
bool string_try_get(String string, ptrdiff_t index, Codepoint* out) {
    if (index >= string.count) return false;

    // single byte case
    if (!(string.data[index] & 0x80)) {
        *out = (Codepoint)string.data[index];
        return true;
    }

    uint8_t mask       = 0xc0; // 0b11000000
    uint8_t extension  = 0x80; // 0b10000000

    const uint8_t masks[] = {0xc0, 0xe0, 0xf0, 0xf8};

    // gets the extension of the character
    ptrdiff_t length = string_utf8_len(string.data[index]);
    if (index + length > string.count) {
        // invalid header (recieved extension byte) or character is longer then the remainder of the string
        return false;
    }
    switch (length) {
        case 2: {
            *out = ((uint8_t)string.data[index] & ~masks[1]) << 6 | ((uint8_t)string.data[index + 1] & ~masks[0]);
            if ((string.data[index + 1] & mask) != extension) return false;
            if (*out < STRING_MIN_TWO_BYTE) return false;
            
            return true;
        } break;
        case 3: {
            *out = ((uint8_t)string.data[index] & ~masks[2]) << 12 | ((uint8_t)string.data[index + 1] & ~masks[0]) << 6 | ((uint8_t)string.data[index + 2] & ~masks[0]);
            if ((string.data[index + 1] & mask) != extension) return false;
            if ((string.data[index + 2] & mask) != extension) return false;
            if (*out < STRING_MIN_THREE_BYTE) return false;
            return true;
        } break;
        case 4: {
            *out = ((uint8_t)string.data[index] & ~masks[3]) << 18 | ((uint8_t)string.data[index + 1] & ~masks[0]) << 12 | ((uint8_t)string.data[index + 2] & ~masks[0]) << 6 | ((uint8_t)string.data[index + 3] & ~masks[0]);
            if ((string.data[index + 1] & mask) != extension) return false;
            if ((string.data[index + 2] & mask) != extension) return false;
            if ((string.data[index + 3] & mask) != extension) return false;
            if (*out < STRING_MIN_FOUR_BYTE || *out > STRING_MAX_CODEPOINT) return false;
            return true;
        } break;
        default: return false;
    }
}

////////////////////////////////////////////
// FILE HANDLING FUNCTION IMPLEMENTATIONS //
////////////////////////////////////////////

// prints a string to stdout
void string_print(String string) {
    string_write_file(stdout, string);
}
// prints a string to stdout and appends a newline
void string_println(String string) {
    string_write_file(stdout, string);
    putc('\n', stdout);
}
// appends the line to the end of the stringbuilder
// use for reading from terminal, for reading from files
// use read entire file and then tokenise with string_token
void string_scanln(StringBuilder* string) {
    char c = '\0';
    while (!feof(stdin) && (c = fgetc(stdin)) != '\n') {
        string_append(string, c);
    }
}
// writes the string to the file
void string_write_file(FILE* file, String string) {
    fwrite(string.data, 1, string.count, file);
}
// appends the contents of the file to the stringbuilder provided
void string_read_file(FILE* file, StringBuilder* string) {
    ptrdiff_t n = string_get_file_length(file);
    string_expand_maybe(string, n);

    string->count += fread(string->data + string->count, 1, n, file);
    string->data[string->count] = '\0';
}

// appends the contents of the file to the stringbuilder provided
void string_read_entire_file(const char* filename, StringBuilder* string) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("failed to open file");
        return;
    }
    string_read_file(f, string);
    fclose(f);
}
// overwrites the file and writes the string to it
void string_write_entire_file(const char* filename, String string) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("failed to open file");
        return;
    }
    string_write_file(f, string);
    fclose(f);
}
// gets the remaining length of the file (i.e. from the current position in the file how much is left)
ptrdiff_t string_get_file_length(FILE* file) {
    ptrdiff_t curr = ftell(file);
    fseek(file, 0, SEEK_END);
    ptrdiff_t n = ftell(file);
    fseek(file, curr, SEEK_SET);
    return n - curr;
}

// ASCII FUNCTIONS

bool string_is_digit(char byte, int32_t* out) {
    if (byte >= '0' && byte <= '9') {
        if (out != NULL) *out = byte - '0';
        return true;
    }
    return false;
}
bool string_is_hexdigit(char byte, int32_t* out) {
    switch (byte) {
        case '0': *out = 0x0; break;
        case '1': *out = 0x1; break;
        case '2': *out = 0x2; break;
        case '3': *out = 0x3; break;
        case '4': *out = 0x4; break;
        case '5': *out = 0x5; break;
        case '6': *out = 0x6; break;
        case '7': *out = 0x7; break;
        case '8': *out = 0x8; break;
        case '9': *out = 0x9; break;
        case 'A': case 'a': *out = 0xa; break;
        case 'B': case 'b': *out = 0xb; break;
        case 'C': case 'c': *out = 0xc; break;
        case 'D': case 'd': *out = 0xd; break;
        case 'E': case 'e': *out = 0xe; break;
        case 'F': case 'f': *out = 0xf; break;
        default: return false;
    }
    return true;
}

bool string_is_ascii(String string) {
    for (ptrdiff_t i = 0; i < string.count; i++) {
        if (string.data[i] & 0x80) {
            return false;
        }
    }
    return true;
}
bool string_is_ascii_alpha(char byte) {
    return (byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z');
}
bool string_is_ascii_whitespace(char byte) {
    return byte == ' ' || byte == '\r' || byte == '\t' || byte == '\n' || byte == '\v' || byte == '\f';
}
bool string_is_ascii_printable(char byte) {
    return byte > 32 && byte < 127;
}

void string_to_uppercase_ascii(StringBuilder* string) {
    const char offset = 'A' - 'a';
    for (ptrdiff_t i = 0; i < string->count; i++) {
        if (string->data[i] >= 'a' && string->data[i] <= 'z') {
            string->data[i] += offset;
        }
    }
}
void string_to_lowercase_ascii(StringBuilder* string) {
    const char offset = 'a' - 'A';
    for (ptrdiff_t i = 0; i < string->count; i++) {
        if (string->data[i] & 0x80) {
            if (string->data[i] >= 'A' && string->data[i] <= 'Z') {
                string->data[i] += offset;
            }
        }
    }
}

String string_trim_whitespace(String string) {
    ptrdiff_t start = 0;
    ptrdiff_t end = string.count - 1;
    for (; start < string.count; start++) {
        if (!string_is_ascii_whitespace(string.data[start])) {
            break;
        }
    }
    for (; end >= 0; end--) {
        if (!string_is_ascii_whitespace(string.data[end])) {
            break;
        }
    }
    return string_slice(string, start, end);
}
String string_trim_leading_whitespace(String string) {
    for (ptrdiff_t i = 0; i < string.count; i++) {
        if (!string_is_ascii_whitespace(string.data[i])) {
            return string_slice(string, i, string.count);
        }
    }
    return (String) {.data = NULL, .count = 0};
}
String string_trim_trailing_whitespace(String string) {
    for (ptrdiff_t i = string.count - 1; i >= 0; i--) {
        if (!string_is_ascii_whitespace(string.data[i])) {
            return string_slice(string, 0, i);
        }
    }
    return (String) {.data = NULL, .count = 0};
}

// ignores whitespace and leading zeroes
// use +, or - for positive or negative (+) is optional for positives
// returns INT64_MAX * sign if the number is too big
// returns zero if no numbers are found
// never returns INT64_MIN
int64_t string_to_int(String string) {
    ptrdiff_t start = 0;

    int64_t sign = 1;
    for (; start < string.count; start++) {
        if (string.data[start] == ' ' || string.data[start] == '0') continue;
        if (string.data[start] == '-') {
            sign = -1;
            start++;
            break;
        }
        if (string.data[start] == '+') {
            start++;
            break;
        }
        if (string.data[start] < '0' || string.data[start] > '9') return 0;
        break;
    }
    int64_t res = 0;
    for (ptrdiff_t i = start; i < string.count; i++) {
        int32_t val = 0;
        if (string_is_digit(string.data[i], &val)) {
            if (res >= INT64_MAX / 10) {
                return INT64_MAX * sign;
            }
            res *= 10;
            res += val;
        } else {
            return res * sign;
        }
    }
    return res * sign;
}
// ignores leading 0x, returns UINT64_MAX if the number is too big to represent
// ignores leading white space and zeros, will return zero if it doesn't find any numbers
uint64_t string_to_int_hex(String string) {
    ptrdiff_t start = 0;

    for (; start < string.count; start++) {
        if (start + 1 < string.count && string.data[start] == '0' && string.data[start + 1] == 'x') {
            start += 2;
            break;
        }
        if (string.data[start] == ' ' || string.data[start] == '0') continue;
        if ((string.data[start] >= '0' && string.data[start] <= '9') || (string.data[start] >= 'A' && string.data[start] <= 'F') || (string.data[start] >= 'a' && string.data[start] <= 'f')) break;
        else return 0;
        break;
    }
    uint64_t res = 0;
    for (ptrdiff_t i = start; i < string.count; i++) {
        int32_t val = 0;
        if (string_is_hexdigit(string.data[i], &val)) {
            if (res >= 0x1000000000000000) {
                return UINT64_MAX;
            }
            res <<= 4;
            res += val;
        } else {
            return res;
        }
    }
    return res;
}

double double_int_pow(double base, int64_t exp) {
    double acc = 1.0;

    if (exp == 0) {return 1.0;}

    double x_pow = base;
    if (exp < 0) {
        x_pow = 1.0/base; 
        exp = -exp;
    }

    while (exp > 0) {
        if ((exp & 1) == 1) {
            acc *= x_pow;
        }

        x_pow *= x_pow;
        exp >>= 1;
    }

    return acc;
}

// converts the float to a string ignores leading whitespace
// works with scientific notation (eg. 12e4 or 12E4)
// works with either . or , as the radix point
// defaults to 0.0 if it is recieves no digits
double string_to_float(String string) {
    ptrdiff_t start = 0;

    double sign = 1.0;
    for (; start < string.count; start++) {
        if (string.data[start] == ' ') continue;
        if (string.data[start] == '-') {
            sign = -1.0;
            start++;
            break;
        }
        if (string.data[start] == '+') {
            start++;
            break;
        }
        if (string.data[start] < '0' || string.data[start] > '9') return 0;
        break;
    }

    double res = 0.0;
    double acc = 0.0;

    for (ptrdiff_t i = start; i < string.count; i++) {
        int32_t val = 0;
        if (string_is_digit(string.data[i], &val)) {
            if (acc == 0.0) {
                res *= 10.0;
                res += (double)val;
            } else {
                res += (double)val * acc;
                acc *= 0.1;
            }
        } else if ((string.data[i] == '.' || string.data[i] == ',') && acc == 0.0) {
            acc = 0.1;
        } else if (string.data[i] == 'e' || string.data[i] == 'E') {
            int64_t exponent = string_to_int(string_slice(string, i + 1, string.count));
            return sign * res * double_int_pow(10.0, exponent);
        } else {
            return res * sign;
        }
    }

    return res * sign;
}

void string_format(StringBuilder* string, const char* fmt, ...) {
    va_list args;
    va_start (args, fmt);
    ptrdiff_t len = vsnprintf("", 0, fmt, args);
    string_expand_maybe(string, len);
    vsnprintf(&string->data[string->count], len + 1, fmt, args);
    va_end (args);
    string->count += len;
}

#endif // STRINGBUILDER_IMPLEMENTATION
#endif // STRINGBUILDER_H_