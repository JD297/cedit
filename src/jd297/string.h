/**
BSD 2-Clause License

Copyright (c) 2026, JD297
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef JD297_STRING_H
#define JD297_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JD297_STRING_NOT_FOUND (size_t)-1

typedef struct {
    char *elements;
    size_t len;
    size_t num;
} str_t;

extern void str_free(str_t *str);

extern str_t *str_from_cstr(const char *cstr);

extern size_t str_find(const str_t *str, const char *cstr);

extern str_t *str_substr(const str_t *str, size_t pos, size_t count);

extern str_t *str_assign_cstr(str_t *str, const char *cstr);

extern str_t *str_insert_char(str_t *str, size_t index, char c);

extern str_t *str_insert_cstr(str_t *str, size_t index, const char *cstr);

extern str_t *str_erase(str_t *str, size_t index, size_t count);

#ifdef __cplusplus
}
#endif

#define str_length(str_ptr) (str_ptr)->num

#define str_empty(str_ptr) (str_ptr)->num == 0

#define str_capacity(str_ptr) (str_ptr)->len

#define str_val(str_ptr) (str_ptr)->elements

#define str_at(str_ptr, index) (str_ptr)->elements[(index)]

#define str_append_char(str_ptr, c) str_insert_char((str_ptr), (str_ptr)->num, (c))

#define str_append_cstr(str_ptr, cstr) str_insert_cstr((str_ptr), (str_ptr)->num, (cstr))

#endif

#ifdef JD297_STRING_TESTSUITE
#define JD297_STRING_IMPLEMENTATION
#endif

#ifdef JD297_STRING_IMPLEMENTATION

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void str_free(str_t *str)
{
    free(str->elements);

    str->elements = NULL;
    str->len = 0;
    str->num = 0;
}

extern str_t *str_from_cstr(const char *cstr)
{
	str_t *str = (str_t *)malloc(sizeof(str_t));
	
	if (str == NULL || cstr == NULL) {
		return NULL;
	}

	str_length(str) = strlen(cstr);
	str_capacity(str) = str->num + 1;
	str_val(str) = (char *)malloc(str_capacity(str) * sizeof(char));

	if (str_val(str) == NULL) {
		return NULL;
	}

	snprintf(str_val(str), str_capacity(str), "%s", cstr);
	
	(void)memcpy(str_val(str), cstr, str_capacity(str));

	return str;
}

extern size_t str_find(const str_t *str, const char *cstr)
{
	char *pos = strstr(str_val(str), cstr);
	
	if (pos == NULL) {
		return JD297_STRING_NOT_FOUND;
	}
	
	return pos - str_val(str);
}

extern str_t *str_substr(const str_t *str, size_t pos, size_t count)
{
	size_t max_len;

	str_t *substr;

	if (pos > str_length(str)) {
		return NULL;
	}

	substr = (str_t *)malloc(sizeof(str_t));
	
	if (substr == NULL) {
		return NULL;
	}

	max_len = str_length(str) - pos;

	if (count > max_len) {
		str_length(substr) = max_len;
	} else {
		str_length(substr) = count;
	}

	str_capacity(substr) = str_length(substr) + 1;
	
	str_val(substr) = (char *)malloc(str_capacity(substr) * sizeof(char));

	if (str_val(str) == NULL) {
		return NULL;
	}

	(void)memcpy(str_val(substr), str_val(str) + pos, str_length(substr));
	str_at(substr, str_length(substr)) = '\0';

	return substr;
}

extern str_t *str_assign_cstr(str_t *str, const char *cstr)
{
	char *old_val = str_val(str);

	str_length(str) = strlen(cstr);
	str_capacity(str) = str_length(str) + 1;

	str_val(str) = (char *)realloc(str_val(str), str_capacity(str) * sizeof(char));

	if (str_val(str) == NULL) {
		free(old_val);

		return NULL;
	}

	(void)memcpy(str_val(str), cstr, str_length(str) + 1);

	return str;
}

extern str_t *str_insert_char(str_t *str, size_t index, char c)
{
	if (index > str_length(str)) {
		return NULL;
	}

	str_length(str) += 1;

	if (str_length(str) >= str_capacity(str)) {
		char *old_val = str_val(str);

		str_capacity(str) = str_length(str) + 1;
		
		str_val(str) = (char *)realloc(str_val(str), str_capacity(str) * sizeof(char));

		if (str_val(str) == NULL) {
			free(old_val);

			return NULL;
		}
	}

	(void)memmove(str_val(str) + index + 1, str_val(str) + index, str_length(str) + 1 - index);

	str_at(str, index) = c;

	return str;
}
#include <assert.h>
extern str_t *str_insert_cstr(str_t *str, size_t index, const char *cstr)
{
	size_t len_cstr = strlen(cstr);

	assert(cstr != NULL);

	if (index > str_length(str)) {
		return NULL;
	}

	if (len_cstr == 0) {
		return str;
	}

	str_length(str) += len_cstr;

	if (str_length(str) >= str_capacity(str)) {
		char *old_val = str_val(str);

		str_capacity(str) = str_length(str) + 1;
		
		str_val(str) = (char *)realloc(str_val(str), str_capacity(str) * sizeof(char));

		if (str_val(str) == NULL) {
			free(old_val);

			return NULL;
		}
	}

	(void)memmove(str_val(str) + index + len_cstr, str_val(str) + index, len_cstr);

	(void)memcpy(str_val(str) + index, cstr, len_cstr);

	return str;
}

extern str_t *str_erase(str_t *str, size_t index, size_t count)
{
	size_t max_len;

	if (index > str_length(str)) {
		return NULL;
	}

	if (count == 0) {
		return str;
	}

	max_len = str_length(str) - index;

	if (count > max_len) {
		str_length(str) -= max_len;
		
		str_at(str, str_length(str)) = '\0';
		
		return str;
	}
	
	str_length(str) -= count;

	(void)memmove(str_val(str) + index, str_val(str) + index + count, max_len);

	return str;
}

#ifdef __cplusplus
}
#endif

#endif

#ifdef JD297_STRING_TESTSUITE

#include <assert.h>
#include <stdio.h>

#if defined(__OpenBSD__)
#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#endif

static void jd297_string_test_str_from_cstr(void)
{
	str_t *a, *b;

	const char *a_cstr = "hello, world";
	const char *b_cstr = "";

	a = str_from_cstr(a_cstr);

	assert(a != NULL);
	assert(str_length(a) == strlen(a_cstr));
	assert(str_capacity(a) == strlen(a_cstr) + 1);
	assert(strcmp(str_val(a), a_cstr) == 0);

	b = str_from_cstr(b_cstr);

	assert(b != NULL);
	assert(str_length(b) == strlen(b_cstr));
	assert(str_capacity(b) == strlen(b_cstr) + 1);
	assert(strcmp(str_val(b), b_cstr) == 0);
}

static void jd297_string_test_str_find(void)
{
	str_t *a, *b;

	const char *a_cstr = "hello, world";
	const char *b_cstr = "";

	a = str_from_cstr(a_cstr);

	assert(0 == str_find(a, "hello"));
	assert(7 == str_find(a, "world"));
	assert(JD297_STRING_NOT_FOUND == str_find(a, "[no]"));

	b = str_from_cstr(b_cstr);

	assert(JD297_STRING_NOT_FOUND == str_find(b, "hello"));
	assert(JD297_STRING_NOT_FOUND == str_find(b, "world"));
	assert(JD297_STRING_NOT_FOUND == str_find(b, "[no]"));
}

static void jd297_string_test_str_substr(void)
{
	str_t *a, *b, *c, *d, *e;

	const char *ref_cstr = "hello, world";
	
	str_t *ref_str = str_from_cstr(ref_cstr);

	const char *a_cstr = "hello";
	const char *b_cstr = ",";
	const char *c_cstr = "world";
	const char *e_cstr = ref_cstr;

	a = str_substr(ref_str, 0, 5);

	assert(a != NULL);
	assert(str_length(a) == strlen(a_cstr));
	assert(str_capacity(a) == strlen(a_cstr) + 1);
	assert(strcmp(str_val(a), a_cstr) == 0);
	
	b = str_substr(ref_str, 5, 1);

	assert(b != NULL);
	assert(str_length(b) == strlen(b_cstr));
	assert(str_capacity(b) == strlen(b_cstr) + 1);
	assert(strcmp(str_val(b), b_cstr) == 0);

	c = str_substr(ref_str, 7, -1);

	assert(c != NULL);
	assert(str_length(c) == strlen(c_cstr));
	assert(str_capacity(c) == strlen(c_cstr) + 1);
	assert(strcmp(str_val(c), c_cstr) == 0);

	d = str_substr(ref_str, 30, -1);

	assert(d == NULL);

	e = str_substr(ref_str, 0, -1);

	assert(e != NULL);
	assert(str_length(e) == strlen(e_cstr));
	assert(str_capacity(e) == strlen(e_cstr) + 1);
	assert(strcmp(str_val(e), e_cstr) == 0);
}

static void jd297_string_test_str_assign_cstr(void)
{
	str_t str = { 0 };

	const char *a_cstr = "hello, world";
	const char *b_cstr = "12345";
	const char *c_cstr = "c programming language rocks";
	const char *d_cstr = "";

	assert(str_assign_cstr(&str, a_cstr) != NULL);
	assert(str_length(&str) == strlen(a_cstr));
	assert(str_capacity(&str) == strlen(a_cstr) + 1);
	assert(strcmp(str_val(&str), a_cstr) == 0);

	assert(str_assign_cstr(&str, b_cstr) != NULL);
	assert(str_length(&str) == strlen(b_cstr));
	assert(str_capacity(&str) == strlen(b_cstr) + 1);
	assert(strcmp(str_val(&str), b_cstr) == 0);

	assert(str_assign_cstr(&str, c_cstr) != NULL);
	assert(str_length(&str) == strlen(c_cstr));
	assert(str_capacity(&str) == strlen(c_cstr) + 1);
	assert(strcmp(str_val(&str), c_cstr) == 0);

	assert(str_assign_cstr(&str, d_cstr) != NULL);
	assert(str_length(&str) == strlen(d_cstr));
	assert(str_capacity(&str) == strlen(d_cstr) + 1);
	assert(strcmp(str_val(&str), d_cstr) == 0);
}

static void jd297_string_test_str_insert_char(void)
{
	str_t str = { 0 };

	const char *a_base_ref_cstr = "hello world";
	const char *a_cstr = "hello, world";
	const char *b_base_ref_cstr = "2345";
	const char *b_cstr = "12345";
	const char *c_base_ref_cstr = "c programming language rocks";
	const char *c_cstr = "c programming language rocks!";
	const char *d_base_ref_cstr = "";
	const char *d_cstr = "+";
	const char *e_base_ref_cstr = "exception";
	const char *e_cstr = NULL; (void)e_cstr;

	assert(str_assign_cstr(&str, a_base_ref_cstr) != NULL);
	assert(str_insert_char(&str, strlen("hello"), ',') != NULL);
	assert(str_length(&str) == strlen(a_cstr));
	assert(strlen(str_val(&str)) == strlen(a_cstr));
	assert(str_capacity(&str) == strlen(a_cstr) + 1);
	assert(strcmp(str_val(&str), a_cstr) == 0);
	
	assert(str_assign_cstr(&str, b_base_ref_cstr) != NULL);
	assert(str_insert_char(&str, 0, '1') != NULL);
	assert(str_length(&str) == strlen(b_cstr));
	assert(strlen(str_val(&str)) == strlen(b_cstr));
	assert(str_capacity(&str) == strlen(b_cstr) + 1);
	assert(strcmp(str_val(&str), b_cstr) == 0);
	
	assert(str_assign_cstr(&str, c_base_ref_cstr) != NULL);
	assert(str_append_char(&str, '!') != NULL);
	assert(str_length(&str) == strlen(c_cstr));
	assert(strlen(str_val(&str)) == strlen(c_cstr));
	assert(str_capacity(&str) == strlen(c_cstr) + 1);
	assert(strcmp(str_val(&str), c_cstr) == 0);

	assert(str_assign_cstr(&str, d_base_ref_cstr) != NULL);
	assert(str_append_char(&str, '+') != NULL);
	assert(str_length(&str) == strlen(d_cstr));
	assert(strlen(str_val(&str)) == strlen(d_cstr));
	assert(str_capacity(&str) == strlen(d_cstr) + 1);
	assert(strcmp(str_val(&str), d_cstr) == 0);

	assert(str_assign_cstr(&str, e_base_ref_cstr) != NULL);
	assert(str_insert_char(&str, strlen(e_base_ref_cstr) + 999, '~') == NULL);
}

static void jd297_string_test_str_insert_cstr(void)
{
	str_t str = { 0 };

	const char *a_base_ref_cstr = "horld";
	const char *a_cstr = "hello, world";
	const char *b_base_ref_cstr = "45";
	const char *b_cstr = "12345";
	const char *c_base_ref_cstr = "c programming";
	const char *c_cstr = "c programming language rocks!";
	const char *d_base_ref_cstr = "";
	const char *d_cstr = "+++++++";
	const char *e_base_ref_cstr = "exception";
	const char *e_cstr = NULL; (void)e_cstr;

	assert(str_assign_cstr(&str, a_base_ref_cstr) != NULL);
	assert(str_insert_cstr(&str, 1, "ello, w") != NULL);
	assert(str_length(&str) == strlen(a_cstr));
	assert(strlen(str_val(&str)) == strlen(a_cstr));
	assert(str_capacity(&str) == strlen(a_cstr) + 1);
	assert(strcmp(str_val(&str), a_cstr) == 0);
	
	assert(str_assign_cstr(&str, b_base_ref_cstr) != NULL);
	assert(str_insert_cstr(&str, 0, "123") != NULL);
	assert(str_length(&str) == strlen(b_cstr));
	assert(strlen(str_val(&str)) == strlen(b_cstr));
	assert(str_capacity(&str) == strlen(b_cstr) + 1);
	assert(strcmp(str_val(&str), b_cstr) == 0);
	
	assert(str_assign_cstr(&str, c_base_ref_cstr) != NULL);
	assert(str_append_cstr(&str, " language rocks!") != NULL);
	assert(str_length(&str) == strlen(c_cstr));
	assert(strlen(str_val(&str)) == strlen(c_cstr));
	assert(str_capacity(&str) == strlen(c_cstr) + 1);
	assert(strcmp(str_val(&str), c_cstr) == 0);

	assert(str_assign_cstr(&str, d_base_ref_cstr) != NULL);
	assert(str_append_cstr(&str, "+++++++") != NULL);
	assert(str_length(&str) == strlen(d_cstr));
	assert(strlen(str_val(&str)) == strlen(d_cstr));
	assert(str_capacity(&str) == strlen(d_cstr) + 1);
	assert(strcmp(str_val(&str), d_cstr) == 0);

	assert(str_assign_cstr(&str, e_base_ref_cstr) != NULL);
	assert(str_insert_cstr(&str, strlen(e_base_ref_cstr) + 999, "~~~") == NULL);
}

static void jd297_string_test_str_erase(void)
{
	str_t str = { 0 };

	const char *a_base_ref_cstr = "hello, world";
	const char *a_cstr = "hello world";
	const char *b_base_ref_cstr = "12345";
	const char *b_cstr = "45";
	const char *c_base_ref_cstr = "c programming language rocks!";
	const char *c_cstr = "c programming";
	const char *d_base_ref_cstr = "+++++++";
	const char *d_cstr = "";
	const char *e_base_ref_cstr = "exception";
	const char *e_cstr = NULL; (void)e_cstr;

	assert(str_assign_cstr(&str, a_base_ref_cstr) != NULL);
	assert(str_erase(&str, strlen("hello"), strlen(",")) != NULL);
	assert(str_length(&str) == strlen(a_cstr));
	assert(strlen(str_val(&str)) == strlen(a_cstr));
	assert(strcmp(str_val(&str), a_cstr) == 0);
	
	assert(str_assign_cstr(&str, b_base_ref_cstr) != NULL);
	assert(str_erase(&str, 0, strlen("123")) != NULL);
	assert(str_length(&str) == strlen(b_cstr));
	assert(strlen(str_val(&str)) == strlen(b_cstr));
	assert(strcmp(str_val(&str), b_cstr) == 0);
	
	assert(str_assign_cstr(&str, c_base_ref_cstr) != NULL);
	assert(str_erase(&str, strlen("c programming"), strlen(" language rocks!")) != NULL);
	assert(str_length(&str) == strlen(c_cstr));
	assert(strlen(str_val(&str)) == strlen(c_cstr));
	assert(strcmp(str_val(&str), c_cstr) == 0);

	assert(str_assign_cstr(&str, d_base_ref_cstr) != NULL);
	assert(str_erase(&str, 0, -1) != NULL);
	assert(str_length(&str) == strlen(d_cstr));
	assert(strlen(str_val(&str)) == strlen(d_cstr));
	assert(strcmp(str_val(&str), d_cstr) == 0);

	assert(str_assign_cstr(&str, e_base_ref_cstr) != NULL);
	assert(str_erase(&str, strlen(e_base_ref_cstr) + 999, 1) == NULL);
}

int main(void)
{
	#if defined(__OpenBSD__)
	if (pledge("stdio", NULL)) {
		err(EXIT_FAILURE, "pledge");
	}
	#endif

	jd297_string_test_str_from_cstr();

	jd297_string_test_str_find();

	jd297_string_test_str_substr();

	jd297_string_test_str_assign_cstr();
	
	jd297_string_test_str_insert_char();

	jd297_string_test_str_insert_cstr();

	jd297_string_test_str_erase();

	printf("%s(ok)\n", __FILE__);

	return 0;
}

#endif
