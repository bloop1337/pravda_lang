/**
 * @file utf8.h
 * @brief simple library for working with UTF-8 encoded strings
 *
 * @code
 * #include "utf8.h"
 * #include <stdio.h>
 *
 * int main() {
 *     const char* str = "Hello, こんにちは, Здравствуйте";
 *     utf8_string ustr = make_utf8_string(str);
 *     utf8_string_slice slice = make_utf8_string_slice(ustr, 2, 11);
 *     utf8_char_iter iter = make_utf8_char_iter(ustr);
 *
 *     printf("string: %s\n", ustr.str);
 *     printf("slice: %.*s\n", (int)slice.byte_len, slice.str);
 *
 *     utf8_char ch;
 *     while ((ch = next_utf8_char(&iter)).byte_len > 0) {
 *         printf("character: %.*s\t", (int)ch.byte_len, ch.str);
 *         printf("unicode code point: U+%04X\n", unicode_code_point(ch));
 *     }
 *
 *     return 0;
 * }
 * @endcode
 */

#ifndef ZAHASH_UTF8_H
#define ZAHASH_UTF8_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Represents the validity of a UTF-8 encoded string.
 *
 * @details The `utf8_validity` struct indicates whether a given UTF-8 encoded string is valid or not,
 * along with the position up to which it is valid.
 *
 * - Invalid case: "hello\xC0\xC0" => { .valid = false, .valid_upto = 5  }
 * - Valid case:   "hello world"   => { .valid = true,  .valid_upto = 11 }
 */
typedef struct {
    bool valid;          ///< Flag indicating the validity of the UTF-8 string.
    size_t valid_upto;   ///< The position up to which the string is valid.
} utf8_validity;

typedef struct {
    bool valid;
    size_t next_offset;
} utf8_char_validity;

/**
 * @brief Represents a non-owning UTF-8 encoded string. (just a wrapper type).
 *
 * @details The `utf8_string` struct holds a pointer to a UTF-8 encoded string along with its byte length,
 */
typedef struct {
    const char* str;     ///< Pointer to the UTF-8 encoded string.
    size_t byte_len;     ///< Byte length of the UTF-8 string ('\0' not counted).
} utf8_string;

/**
 * @brief Represents a UTF-8 encoded string that fully owns its data.
 *
 * @details The `owned_utf8_string` struct holds a pointer to a UTF-8 encoded string that is dynamically allocated
 *          and therefore is owned by the struct, which means the caller is responsible for freeing the memory when
 *          it is no longer needed using the `free_owned_utf8_string` function.
 */
typedef struct {
    char* str;          ///< Pointer to the UTF-8 encoded string (owned). This memory is dynamically allocated.
    size_t byte_len;    ///< Byte length of the UTF-8 string ('\0' not counted).
	size_t capacity;	///< Number of free bytes 
} owned_utf8_string;

/**
 * @brief Represents an iterator for traversing UTF-8 characters in a string.
 *
 * @details The `utf8_char_iter` struct serves as an iterator for traversing UTF-8 characters
 * within a UTF-8 encoded string.
 */
typedef struct {
    const char* str;     ///< Pointer to the current position of the iterator.
} utf8_char_iter;

/**
 * @brief Represents a UTF-8 character.
 *
 * @details The `utf8_char` struct encapsulates a UTF-8 character, including its pointer and byte length.
 * The byte length represents the number of bytes occupied by the UTF-8 character.
 */
typedef struct {
    const char* str;     ///< Pointer to the UTF-8 character.
    uint8_t byte_len;    ///< Byte length of the UTF-8 character.
} utf8_char;

/**
 * @brief Validates whether a given string is UTF-8 compliant in O(n) time.
 *
 * @param str The input string to validate.
 * @return The validity of the UTF-8 string along with the position up to which it is valid.
 */
utf8_validity validate_utf8(const char* str);

/**
 * @brief Wraps a C-style string in a UTF-8 string structure after verifying its UTF-8 compliance.
 *
 * @param str The input C-style string to wrap.
 * @return A UTF-8 string structure containing the wrapped string if valid; otherwise, a structure with NULL string pointer.
 *
 * @code
 * // Example usage:
 * const char *str = "definitely utf8 string こんにちは नमस्ते Здравствуйте";
 * utf8_string ustr = make_utf8_string(str);
 * assert( ustr.str != NULL );
 *
 * const char *s = "non-utf8 sequence \xC0\xC0";
 * utf8_string ustr = make_utf8_string(str);
 * assert( ustr.str == NULL );
 * @endcode
 */
utf8_string make_utf8_string(const char* str);

/**
 * @brief Converts a C-style string to a UTF-8 string, replacing invalid sequences with U+FFFD REPLACEMENT CHARACTER (�).
 *
 * @details It takes a C-style string as input and converts it to a UTF-8 encoded string.
 *          Any invalid UTF-8 sequences in the input string are replaced with the U+FFFD REPLACEMENT CHARACTER (�) to ensure
 *          that the resulting string is valid UTF-8. The resulting string is dynamically allocated and the caller
 *          is responsible for freeing the memory when no longer needed using `free_owned_utf8_string`.
 *
 * @param str The input C-style string to convert. The string can contain invalid UTF-8 sequences.
 * @return An `owned_utf8_string` structure containing the resulting UTF-8 string. If memory allocation fails, the structure
 *         will contain a `NULL` pointer and a `byte_len` of 0.
 *
 * @code
 * // Example usage:
 * const char* str = "hello\xC0\xC0 world!";
 * owned_utf8_string owned_ustr = make_utf8_string_lossy(str);
 * @endcode
 */
owned_utf8_string make_utf8_string_lossy(const char* str);

/**
 * @brief  Creates an empty UTF-8 string with the given capacity.
 *
 * @details
 * Allocates @p capacity bytes for the string buffer and leaves the string empty
 * (byte_len == 0). The capacity is not validated: if @p capacity == 0, the function
 * calls malloc(0).
 *
 * @param capacity  Number of bytes to allocate for the string buffer.
 * @return          An owned UTF-8 string with the requested capacity and size 0.
 *
 * @code
 * owned_utf8_string str = make_owned_with_capacity(100); // 100 bytes allocated
 * // ... use the string ...
 * // When done, don't forget to release it:
 * free_owned_utf8_string(&str);
 * @endcode
 */
owned_utf8_string owned_string_with_capacity(size_t capacity);

/**
 * @brieif Appends a UTF-8 character to the end of a UTF-8 string.
 *
 * @details
 * If @c ustr->str is NULL, the function calls malloc(ch.byte_len) to allocate
 * the initial buffer.
 *
 * @param ustr  Pointer to the UTF-8 string to append to. Must not be NULL.
 * @param ch    UTF-8 character to append
 *
 * @code
 * owned_utf8_string str = {0};
 * utf8_char ch;
 * utf8_char_iter iter = make_utf8_char_iter(&another_str);
 * while ((ch = next_utf8_char(&iter)).byte_len > 0)
 *     owned_push_back(&str, ch);
 * @endcode
 */
void owned_string_push_back(owned_utf8_string* ustr, utf8_char ch);

owned_utf8_string empty_owned_string();
/**
 * @brief Creates the non-owning UTF-8 encoded string `utf8_string` from an `owned_utf8_string`.
 *
 * @details The resulting `utf8_string` will point to the same underlying string without taking ownership.
 *          The caller must ensure the original `owned_utf8_string` remains valid as long as the reference is used.
 *
 * @param owned_str The owned UTF-8 string from which to create a non-owning reference.
 * @return utf8_string A non-owning UTF-8 string reference (`utf8_string`) pointing to the same data.
 *
 * @note This function does not free or transfer ownership of the `owned_utf8_string`.
 *       The caller is responsible for managing the lifetime of the owned string.
 */
utf8_string as_utf8_string(const owned_utf8_string* owned_str);

/**
 * @brief Frees the memory allocated for an `owned_utf8_string`.
 *
 * @details The `free_owned_utf8_string` function deallocates the memory used by an `owned_utf8_string`
 *          and sets the `str` pointer to `NULL` and `byte_len` to 0.
 *
 * @param owned_str A pointer to the `owned_utf8_string` structure to be freed.
 *
 * @code
 * // Example usage:
 * owned_utf8_string owned_ustr = make_utf8_string_lossy("hello\xC0\xC0 world!");
 * free_owned_utf8_string(&owned_ustr);
 * @endcode
 */
void free_owned_utf8_string(owned_utf8_string* owned_str);

/**
 * @brief Creates a UTF-8 string slice from a specified range of bytes in the original string.
 *
 * @param ustr The original UTF-8 string.
 * @param byte_index The starting byte index of the slice.
 * @param byte_len The byte length of the slice.
 * @return A UTF-8 string representing the specified byte range [offset, offset + byte_len) if valid (range between UTF-8 char boundaries);
 * otherwise { .str = NULL, .byte_len = 0 }
 *
 * @note if `byte_index` >= strlen(ustr.str) then returns terminating '\0' of ustr.str { .str = '\0', .byte_len = 0 }
 * @note if `byte_index` + `byte_len` >= strlen(ustr.str) then only chars till terminating '\0' are considered.
 */
utf8_string slice_utf8_string(utf8_string ustr, size_t byte_index, size_t byte_len);

/**
 * @brief Creates an iterator for traversing UTF-8 characters within a string. (see next_utf8_char( .. ) for traversal)
 *
 * @param ustr The UTF-8 string to iterate over.
 * @return An iterator structure initialized to the start of the string.
 */
utf8_char_iter make_utf8_char_iter(utf8_string ustr);

/**
 * @brief Retrieves the next UTF-8 character from the iterator.
 *
 * @param iter Pointer to the UTF-8 character iterator.
 * @return The next UTF-8 character from the iterator.
 * @note If the iterator reaches the end, it keeps returning terminating '\0' of iter.str { .str = '\0', .byte_len = 0 }
 */
utf8_char next_utf8_char(utf8_char_iter* iter);

utf8_char peek_utf8_char(utf8_char_iter* iter);

/**
 * @brief Retrieves the UTF-8 character at the specified character index within a UTF-8 string in O(n) time.
 *
 * @details The `nth_utf8_char` function returns the UTF-8 character located at the specified character index
 * within the given UTF-8 string. The character index is zero-based, indicating the position of
 * the character in the string. If the index is out of bounds or invalid, the function returns
 * { .str = NULL, .byte_len = 0 }
 *
 * @param ustr The UTF-8 string from which to retrieve the character.
 * @param char_index The zero-based index of the character to retrieve.
 * @return The UTF-8 character at the specified index within the string.
 *
 * @code
 * // Example usage:
 * utf8_string str = make_utf8_string("Hello Здравствуйте こんにちは");
 * utf8_char char_at_index = nth_utf8_char(str, 7);    // д
 * @endcode
 */
utf8_char nth_utf8_char(utf8_string ustr, size_t char_index);

/**
 * @brief Counts the number of UTF-8 characters in the given utf8_string.
 *
 * @param ustr The UTF-8 string whose characters are to be counted.
 * @return The total number of characters in the UTF-8 string.
 */
size_t utf8_char_count(utf8_string ustr);

/**
 * @brief Checks if a given byte is the start of a UTF-8 character. ('\0' is also a valid character boundary)
 *
 * @param str Pointer to the byte to check.
 * @return `true` if the byte is the start of a UTF-8 character; otherwise, `false`.
 */
bool is_utf8_char_boundary(const char* str);

/**
 * @brief Converts a UTF-8 character to its corresponding Unicode code point (which is the same as a UTF-32 value).
 *
 * @param uchar The UTF-8 character to convert.
 * @return The Unicode code point.
 */
uint32_t unicode_code_point(utf8_char uchar);


#ifdef UTF8_IMPL
utf8_char_validity validate_utf8_char(const char* str, size_t offset) {
    // Single-byte UTF-8 characters have the form 0xxxxxxx
    if (((uint8_t)str[offset] & 0b10000000) == 0b00000000)
        return (utf8_char_validity) { .valid = true, .next_offset = offset + 1 };

    // Two-byte UTF-8 characters have the form 110xxxxx 10xxxxxx
    if (((uint8_t)str[offset + 0] & 0b11100000) == 0b11000000 &&
        ((uint8_t)str[offset + 1] & 0b11000000) == 0b10000000) {

        // Check for overlong encoding
        // 0(xxxxxxx)
        // 0(1111111)
        // 110(xxxxx) 10(xxxxxx)
        // 110(00001) 10(111111)
        // 110(00010) 10(000000)
        if (((uint8_t)str[offset] & 0b00011111) < 0b00000010)
            return (utf8_char_validity) { .valid = false, .next_offset = offset };

        return (utf8_char_validity) { .valid = true, .next_offset = offset + 2 };
    }

    // Three-byte UTF-8 characters have the form 1110xxxx 10xxxxxx 10xxxxxx
    if (((uint8_t)str[offset + 0] & 0b11110000) == 0b11100000 &&
        ((uint8_t)str[offset + 1] & 0b11000000) == 0b10000000 &&
        ((uint8_t)str[offset + 2] & 0b11000000) == 0b10000000) {

        // Check for overlong encoding
        // 110(xxxxx) 10(xxxxxx)
        // 110(11111) 10(111111)
        // 1110(xxxx) 10(xxxxxx) 10(xxxxxx)
        // 1110(0000) 10(011111) 10(111111)
        // 1110(0000) 10(100000) 10(000000)
        if (((uint8_t)str[offset + 0] & 0b00001111) == 0b00000000 &&
            ((uint8_t)str[offset + 1] & 0b00111111) < 0b00100000)
            return (utf8_char_validity) { .valid = false, .next_offset = offset };

        // Reject UTF-16 surrogates
        // U+D800 to U+DFFF
        // 1110(1101) 10(100000) 10(000000) ED A0 80 to 1110(1101) 10(111111) 10(111111) ED BF BF
        if ((uint8_t)str[offset + 0] == 0b11101101 &&
            (uint8_t)str[offset + 1] >= 0b10100000 &&
            (uint8_t)str[offset + 1] <= 0b10111111)
            return (utf8_char_validity) { .valid = false, .next_offset = offset };

        return (utf8_char_validity) { .valid = true, .next_offset = offset + 3 };
    }

    // Four-byte UTF-8 characters have the form 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if (((uint8_t)str[offset + 0] & 0b11111000) == 0b11110000 &&
        ((uint8_t)str[offset + 1] & 0b11000000) == 0b10000000 &&
        ((uint8_t)str[offset + 2] & 0b11000000) == 0b10000000 &&
        ((uint8_t)str[offset + 3] & 0b11000000) == 0b10000000) {

        // Check for overlong encoding
        // 1110(xxxx) 10(xxxxxx) 10(xxxxxx)
        // 1110(1111) 10(111111) 10(111111)
        // 11110(xxx) 10(xxxxxx) 10(xxxxxx) 10(xxxxxx)
        // 11110(000) 10(001111) 10(111111) 10(111111)
        // 11110(000) 10(010000) 10(000000) 10(000000)
        if (((uint8_t)str[offset + 0] & 0b00000111) == 0b00000000 &&
            ((uint8_t)str[offset + 1] & 0b00111111) < 0b00010000)
            return (utf8_char_validity) { .valid = false, .next_offset = offset };

        return (utf8_char_validity) { .valid = true, .next_offset = offset + 4 };
    }

    return (utf8_char_validity) { .valid = false, .next_offset = offset };
}

utf8_validity validate_utf8(const char* str) {
    if (str == NULL) return (utf8_validity) { .valid = false, .valid_upto = 0 };

    size_t offset = 0;
    utf8_char_validity char_validity;

    while (str[offset] != '\0') {
        char_validity = validate_utf8_char(str, offset);
        if (char_validity.valid) offset = char_validity.next_offset;
        else return (utf8_validity) { .valid = false, .valid_upto = offset };
    }

    return (utf8_validity) { .valid = true, .valid_upto = offset };
}

utf8_string make_utf8_string(const char* str) {
    utf8_validity validity = validate_utf8(str);
    if (validity.valid) return (utf8_string) { .str = str, .byte_len = validity.valid_upto };
    return (utf8_string) { .str = NULL, .byte_len = 0 };
}

owned_utf8_string make_utf8_string_lossy(const char* str) {
    if (str == NULL) return empty_owned_string();

    size_t len = strlen(str);

    // Worst case scenario: every byte is invalid and is replaced with 3 bytes for U+FFFD
    size_t worst_case_size = len * 3 + 1;

    // Allocate buffer for the lossy UTF-8 string
    char* buffer = (char*)malloc(worst_case_size);
    if (!buffer) return empty_owned_string(); // failed allocation

    size_t buffer_offset = 0;
    size_t offset = 0;
    utf8_char_validity char_validity;

    while (offset < len) {
        char_validity = validate_utf8_char(str, offset);

        if (char_validity.valid) {
            // Copy valid UTF-8 character sequence to the buffer
            size_t char_len = char_validity.next_offset - offset;
            memcpy(buffer + buffer_offset, str + offset, char_len);
            buffer_offset += char_len;
            offset = char_validity.next_offset;
        } else {
            // Insert the UTF-8 bytes for U+FFFD (�)
            // FFFD = 1111111111111101
            //      = (1111) (111111) (111101)
            //      = 1110(1111) 10(111111) 10(111101)
            //      = EF BF BD
            buffer[buffer_offset++] = 0xEF;
            buffer[buffer_offset++] = 0xBF;
            buffer[buffer_offset++] = 0xBD;
            offset++;
        }
    }

    buffer[buffer_offset] = '\0';

    return (owned_utf8_string) { .str = buffer, .byte_len = buffer_offset, .capacity = worst_case_size };
}

owned_utf8_string owned_string_with_capacity(size_t capacity) {
	char* buff = (char*) malloc(capacity);
	if (!buff) return empty_owned_string(); // failed allocation
	return (owned_utf8_string) { .str = buff, .byte_len = 0, .capacity = capacity };
}

void owned_string_push_back(owned_utf8_string* ustr, utf8_char ch) {
	if (ch.byte_len > (ustr->capacity - ustr->byte_len)) {
		const size_t new_cap = (ustr->capacity*2)+ch.byte_len;
		char* temp = (char*)realloc(ustr->str, new_cap); 
		if (!temp) return;  // failed reallocation
		ustr->str = temp;
		ustr->capacity = new_cap;
	}
	
	for (uint8_t i = 0; i < ch.byte_len; ++i) {
		ustr->str[ustr->byte_len] = ch.str[i];
		ustr->byte_len++;
	}
	ustr->str[ustr->byte_len] = '\0';
}

owned_utf8_string empty_owned_string() {
	return (owned_utf8_string) { .str = NULL, .byte_len = 0, .capacity = 0 };
}

utf8_string as_utf8_string(const owned_utf8_string* owned_str) {
    return (utf8_string) { .str = owned_str->str, .byte_len = owned_str->byte_len };
}

void free_owned_utf8_string(owned_utf8_string* owned_str) {
    if (owned_str->str) {
        free(owned_str->str);
        owned_str->str = NULL;
        owned_str->byte_len = 0;
        owned_str->capacity = 0;
    }
}

utf8_char_iter make_utf8_char_iter(utf8_string ustr) {
    return (utf8_char_iter) { .str = ustr.str };
}

bool is_utf8_char_boundary(const char* str) {
    return (uint8_t)*str <= 0b01111111 || (uint8_t)*str >= 0b11000000;
}

utf8_string slice_utf8_string(utf8_string ustr, size_t start_byte_index, size_t byte_len) {
    if (start_byte_index > ustr.byte_len) start_byte_index = ustr.byte_len;

    size_t excl_end_byte_index = start_byte_index + byte_len;
    if (excl_end_byte_index > ustr.byte_len) excl_end_byte_index = ustr.byte_len;

    if (is_utf8_char_boundary(ustr.str + start_byte_index) && is_utf8_char_boundary(ustr.str + excl_end_byte_index))
        return (utf8_string) { .str = ustr.str + start_byte_index, .byte_len = excl_end_byte_index - start_byte_index };

    return (utf8_string) { .str = NULL, .byte_len = 0 };
}

utf8_char next_utf8_char(utf8_char_iter* iter) {
    if (*iter->str == '\0') return (utf8_char) { .str = iter->str, .byte_len = 0 };

    // iter->str is at the current char's starting byte (char boundary).
    const char* curr_boundary = iter->str;

    iter->str++;
    uint8_t byte_len = 1;

    // find the next char's starting byte (next char boundary) and set the iter->str to that.
    while (!is_utf8_char_boundary(iter->str)) {
        iter->str++;
        byte_len++;
    }

    return (utf8_char) { .str = curr_boundary, .byte_len = byte_len };
}

utf8_char peek_utf8_char(utf8_char_iter* iter) {
	const char* save = iter->str;	
	utf8_char next = next_utf8_char(iter);
	iter->str = save;
	return next;
}

utf8_char nth_utf8_char(utf8_string ustr, size_t char_index) {
    utf8_char_iter iter = make_utf8_char_iter(ustr);

    utf8_char ch;
    while ((ch = next_utf8_char(&iter)).byte_len != 0 && char_index-- != 0) {}

    if (ch.byte_len == 0) return (utf8_char) { .str = NULL, .byte_len = 0 };
    return ch;
}

size_t utf8_char_count(utf8_string ustr) {
    utf8_char_iter iter = make_utf8_char_iter(ustr);

    size_t count = 0;
    while (next_utf8_char(&iter).byte_len > 0) count++;
    return count;
}

uint32_t unicode_code_point(utf8_char uchar) {
    switch (uchar.byte_len) {
    case 1: return uchar.str[0] & 0b01111111;
    case 2: return
        (uchar.str[0] & 0b00011111) << 6 |
        (uchar.str[1] & 0b00111111);
    case 3: return
        (uchar.str[0] & 0b00001111) << 12 |
        (uchar.str[1] & 0b00111111) << 6 |
        (uchar.str[2] & 0b00111111);
    case 4: return
        (uchar.str[0] & 0b00000111) << 18 |
        (uchar.str[1] & 0b00111111) << 12 |
        (uchar.str[2] & 0b00111111) << 6 |
        (uchar.str[3] & 0b00111111);
    }

    return 0; // unreachable
}
#endif /* UTF8_IMPL */

#endif
