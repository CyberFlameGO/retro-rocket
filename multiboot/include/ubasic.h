/*
 * Copyright (c) 2006, Adam Dunkels
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#ifndef __UBASIC_H__
#define __UBASIC_H__

#include "video.h"

#define MAX_STRINGLEN 1024
#define MAX_GOSUB_STACK_DEPTH 255
#define MAX_FOR_STACK_DEPTH 50

struct for_state
{
        int line_after_for;
        char* for_variable;
        int to;
        int step;
};

struct ub_var_int
{
	char* varname;
	int value;
	struct ub_var_int* next;
};

struct ub_var_string
{
	char* varname; /* Not including the $ on the end! */
	char* value;
	struct ub_var_string* next;
};

struct ub_var_int_array
{
	char* varname;
	struct ub_var_int* values;
	int itemcount;
};

struct ub_var_string_array
{
	char* varname;
	struct ub_var_string* values;
	int itemcount;
};

struct ubasic_ctx
{
        char const *ptr, *nextptr;
        int current_token;
	int current_linenum;
        char const *program_ptr;
        char string[MAX_STRINGLEN];
        int gosub_stack[MAX_GOSUB_STACK_DEPTH];
        int gosub_stack_ptr;
        struct for_state for_stack[MAX_FOR_STACK_DEPTH];
        int for_stack_ptr;
        struct ub_var_int* int_variables;
	struct ub_var_string* str_variables;
	struct ub_var_int_array* int_array_variables;
	struct ub_var_string_array* string_array_variables;
	console* cons;
        int ended;

};

struct ubasic_ctx* ubasic_init(const char *program, console* cons);
void ubasic_destroy(struct ubasic_ctx* ctx);
void ubasic_run(struct ubasic_ctx* ctx);
int ubasic_finished(struct ubasic_ctx* ctx);
int ubasic_get_int_variable(const char* varname, struct ubasic_ctx* ctx);
const char* ubasic_get_string_variable(const char* var, struct ubasic_ctx* ctx);
void ubasic_set_variable(const char* varname, const char* value, struct ubasic_ctx* ctx);

#endif /* __UBASIC_H__ */
