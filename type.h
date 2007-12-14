#ifndef TYPE_H
#define TYPE_H

#include <stdio.h>
#include <stdbool.h>
#include "symbol.h"

typedef struct type_base_t           type_base_t;
typedef struct atomic_type_t         atomic_type_t;
typedef struct pointer_type_t        pointer_type_t;
typedef struct function_parameter_t  function_parameter_t;
typedef struct function_type_t       function_type_t;
typedef struct compound_type_t       compound_type_t;
typedef struct enum_type_t           enum_type_t;
typedef struct builtin_type_t        builtin_type_t;
typedef struct array_type_t          array_type_t;
typedef struct typedef_type_t        typedef_type_t;
typedef struct bitfield_type_t       bitfield_type_t;
typedef struct typeof_type_t         typeof_type_t;
typedef union  type_t                type_t;

void init_types(void);
void exit_types(void);

void print_type(const type_t *type);

/**
 * prints a human readable form of @p type. prints an abstract typename
 * if symbol is NULL
 */
void print_type_ext(const type_t *type, const symbol_t *symbol,
                    const scope_t *scope);

void print_type_qualifiers(unsigned qualifiers);

void print_enum_definition(const declaration_t *declaration);
void print_compound_definition(const declaration_t *declaration);

/**
 * set output stream for the type printer
 */
void type_set_output(FILE *out);

void inc_type_visited(void);

/**
 * returns true if type contains integer numbers
 */
bool is_type_integer(const type_t *type);

/**
 * return true if type contains signed numbers
 */
bool is_type_signed(const type_t *type);

/**
 * returns true if type contains floating point numbers
 */
bool is_type_floating(const type_t *type);

/**
 * returns true if the type is valid. A type is valid if it contains no
 * unresolved references anymore and is not of TYPE_INVALID.
 */
bool type_valid(const type_t *type);

/**
 * returns true if the type is an arithmetic type (6.2.18)
 */
bool is_type_arithmetic(const type_t *type);

/**
 * returns true if the type is a scalar type (6.2.21)
 */
bool is_type_scalar(const type_t *type);

bool is_type_incomplete(const type_t *type);

bool types_compatible(const type_t *type1, const type_t *type2);

bool pointers_compatible(const type_t *type1, const type_t *type2);

type_t *get_unqualified_type(type_t *type);
type_t *skip_typeref(type_t *type);

#endif
