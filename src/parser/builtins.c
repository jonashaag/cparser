/*
 * This file is part of cparser.
 * Copyright (C) 2012 Matthias Braun <matze@braunis.de>
 */
#include "builtins.h"

#include "adt/strutil.h"
#include "ast/dialect.h"
#include "ast/entity_t.h"
#include "ast/symbol_t.h"
#include "ast/symbol_table.h"
#include "ast/type_t.h"
#include "ast/types.h"
#include "driver/diagnostic.h"
#include "driver/warning.h"
#include "parser_t.h"

static entity_t *create_builtin_function(builtin_kind_t kind, symbol_t *symbol,
                                         type_t *function_type)
{
	entity_t *const entity = allocate_entity_zero(ENTITY_FUNCTION, NAMESPACE_NORMAL, symbol, &builtin_position);
	entity->declaration.storage_class          = STORAGE_CLASS_EXTERN;
	entity->declaration.declared_storage_class = STORAGE_CLASS_EXTERN;
	entity->declaration.type                   = function_type;
	entity->declaration.implicit               = true;
	entity->function.btk                       = kind;

	return entity;
}

static entity_t *record_builtin_function(builtin_kind_t kind, symbol_t *symbol,
                                         type_t *function_type)
{
	entity_t *entity = create_builtin_function(kind, symbol, function_type);
	record_entity(entity, /*is_definition=*/false);
	return entity;
}

static symbol_t *finalize_symbol_string(void)
{
	char     *const string = obstack_nul_finish(&symbol_obstack);
	symbol_t *const symbol = symbol_table_insert(string);
	if (symbol->string != string)
		obstack_free(&symbol_obstack, string);
	return symbol;
}

static entity_t *create_gnu_builtin(builtin_kind_t kind, const char *name,
                                    type_t *type)
{
	obstack_printf(&symbol_obstack, "__builtin_%s", name);
	symbol_t *symbol = finalize_symbol_string();
	entity_t *entity = record_builtin_function(kind, symbol, type);
	return entity;
}

static entity_t *create_gnu_builtin_firm(ir_builtin_kind kind, const char *name,
                                         type_t *type)
{
	obstack_printf(&symbol_obstack, "__builtin_%s", name);
	symbol_t *symbol = finalize_symbol_string();
	entity_t *entity = record_builtin_function(BUILTIN_FIRM, symbol, type);
	entity->function.b.firm_builtin_kind = kind;
	return entity;
}

static entity_t *create_builtin_firm(ir_builtin_kind kind, const char *name,
                                     type_t *type)
{
	symbol_t *symbol = symbol_table_insert(name);
	entity_t *entity = record_builtin_function(BUILTIN_FIRM, symbol, type);
	entity->function.b.firm_builtin_kind = kind;
	return entity;
}

static entity_t *create_gnu_builtin_libc(const char *name, type_t *type)
{
	obstack_printf(&symbol_obstack, "__builtin_%s", name);
	symbol_t *symbol = finalize_symbol_string();
	entity_t *entity = record_builtin_function(BUILTIN_LIBC, symbol, type);
	entity->function.builtin_in_lib = true;
	entity->function.actual_name    = symbol_table_insert(name);
	return entity;
}

static entity_t *create_gnu_builtin_chk(const char *name, unsigned chk_arg_pos,
                                        type_t *type)
{
	obstack_printf(&symbol_obstack, "__builtin___%s_chk", name);
	symbol_t *symbol = finalize_symbol_string();
	entity_t *entity = record_builtin_function(BUILTIN_LIBC_CHECK, symbol, type);
	entity->function.builtin_in_lib = true;
	entity->function.actual_name    = symbol_table_insert(name);
	entity->function.b.chk_arg_pos  = chk_arg_pos;
	return entity;
}

void create_gnu_builtins(void)
{
	entity_t *(*b)(builtin_kind_t,const char*,type_t*) = create_gnu_builtin;
	b(BUILTIN_ALLOCA,      "alloca",         make_function_1_type(type_void_ptr, type_size_t, DM_NONE));
	b(BUILTIN_INF,         "huge_val",       make_function_0_type(type_double, DM_CONST));
	b(BUILTIN_INF,         "huge_valf",      make_function_0_type(type_float, DM_CONST));
	b(BUILTIN_INF,         "huge_vall",      make_function_0_type(type_long_double, DM_CONST));
	b(BUILTIN_INF,         "inf",            make_function_0_type(type_double, DM_CONST));
	b(BUILTIN_INF,         "inff",           make_function_0_type(type_float, DM_CONST));
	b(BUILTIN_INF,         "infl",           make_function_0_type(type_long_double, DM_CONST));
	b(BUILTIN_NAN,         "nan",            make_function_1_type(type_double, type_char_ptr, DM_CONST));
	b(BUILTIN_NAN,         "nanf",           make_function_1_type(type_float, type_char_ptr, DM_CONST));
	b(BUILTIN_NAN,         "nanl",           make_function_1_type(type_long_double, type_char_ptr, DM_CONST));
	b(BUILTIN_VA_END,      "va_end",         make_function_1_type(type_void, type_valist_arg, DM_NONE));
	b(BUILTIN_EXPECT,      "expect",         make_function_2_type(type_long, type_long, type_long, DM_CONST));
	b(BUILTIN_OBJECT_SIZE, "object_size",    make_function_2_type(type_size_t, type_void_ptr, type_int, DM_CONST));

	entity_t *(*f)(ir_builtin_kind,const char*,type_t*)
		= create_gnu_builtin_firm;
	f(ir_bk_bswap,          "bswap32",        make_function_1_type(type_int32_t, type_int32_t, DM_CONST));
	f(ir_bk_bswap,          "bswap64",        make_function_1_type(type_int64_t, type_int64_t, DM_CONST));
	f(ir_bk_clz,            "clz",            make_function_1_type(type_int, type_unsigned_int, DM_CONST));
	f(ir_bk_clz,            "clzl",           make_function_1_type(type_int, type_unsigned_long, DM_CONST));
	f(ir_bk_clz,            "clzll",          make_function_1_type(type_int, type_unsigned_long_long, DM_CONST));
	f(ir_bk_ctz,            "ctz",            make_function_1_type(type_int, type_unsigned_int, DM_CONST));
	f(ir_bk_ctz,            "ctzl",           make_function_1_type(type_int, type_unsigned_long, DM_CONST));
	f(ir_bk_ctz,            "ctzll",          make_function_1_type(type_int, type_unsigned_long_long, DM_CONST));
	f(ir_bk_ffs,            "ffs",            make_function_1_type(type_int, type_unsigned_int, DM_CONST));
	f(ir_bk_ffs,            "ffsl",           make_function_1_type(type_int, type_unsigned_long, DM_CONST));
	f(ir_bk_ffs,            "ffsll",          make_function_1_type(type_int, type_unsigned_long_long, DM_CONST));
	f(ir_bk_frame_address,  "frame_address",  make_function_1_type(type_void_ptr, type_unsigned_int, DM_CONST));
	f(ir_bk_parity,         "parity",         make_function_1_type(type_int, type_unsigned_int, DM_CONST));
	f(ir_bk_parity,         "parityl",        make_function_1_type(type_int, type_unsigned_long, DM_CONST));
	f(ir_bk_parity,         "parityll",       make_function_1_type(type_int, type_unsigned_long_long, DM_CONST));
	f(ir_bk_popcount,       "popcount",       make_function_1_type(type_int, type_unsigned_int, DM_CONST));
	f(ir_bk_popcount,       "popcountl",      make_function_1_type(type_int, type_unsigned_long, DM_CONST));
	f(ir_bk_popcount,       "popcountll",     make_function_1_type(type_int, type_unsigned_long_long, DM_CONST));
	f(ir_bk_prefetch,       "prefetch",       make_function_1_type_variadic(type_float, type_void_ptr, DM_NONE));
	f(ir_bk_return_address, "return_address", make_function_1_type(type_void_ptr, type_unsigned_int, DM_CONST));
	f(ir_bk_trap,           "trap",           make_function_type(type_void, 0, NULL, DM_NORETURN));

	entity_t *(*s)(ir_builtin_kind,const char*,type_t*) = create_builtin_firm;
	type_t *template     = type_builtin_template;
	type_t *template_ptr = type_builtin_template_ptr;
	s(ir_bk_compare_swap, "__sync_val_compare_and_swap", make_function_type(template, 3, (type_t*[]) { template_ptr, template, template }, DM_NONE));
	s(ir_bk_may_alias,    "__builtin_may_alias",         make_function_type(type_int, 2, (type_t*[]) { type_const_void_ptr, type_const_void_ptr }, DM_NONE));

	entity_t *(*l)(const char*,type_t*) = create_gnu_builtin_libc;
	l("abort",   make_function_type(type_void, 0, NULL, DM_NORETURN));
	l("abs",     make_function_type(type_int, 1, (type_t *[]) { type_int }, DM_CONST));
	l("atan2l",  make_function_type(type_long_double, 2, (type_t*[]) { type_long_double, type_long_double }, DM_CONST));
	l("exit",    make_function_type(type_void, 1, (type_t *[]) { type_int }, DM_NORETURN));
	l("fabs",    make_function_type(type_double, 1, (type_t *[]) { type_double }, DM_CONST));
	l("fabsf",   make_function_type(type_float, 1, (type_t *[]) { type_float }, DM_CONST));
	l("fabsl",   make_function_type(type_long_double, 1, (type_t *[]) { type_long_double }, DM_CONST));
	l("labs",    make_function_type(type_long, 1, (type_t *[]) { type_long }, DM_CONST));
	l("llabs",   make_function_type(type_long_long, 1, (type_t *[]) { type_long_long }, DM_CONST));
	l("malloc",  make_function_type(type_void_ptr, 1, (type_t *[]) { type_size_t }, DM_MALLOC));
	l("memcmp",  make_function_type(type_int, 3, (type_t *[]) { type_const_void_ptr, type_const_void_ptr, type_size_t }, DM_PURE));
	l("memcpy",  make_function_type(type_void_ptr, 3, (type_t *[]) { type_void_ptr_restrict, type_const_void_ptr_restrict, type_size_t }, DM_NONE));
	l("memmove", make_function_type(type_void_ptr, 3, (type_t *[]) { type_void_ptr_restrict, type_const_void_ptr_restrict, type_size_t }, DM_NONE));
	l("memset",  make_function_type(type_void_ptr, 3, (type_t *[]) { type_void_ptr, type_int, type_size_t }, DM_NONE));
	l("stpcpy",  make_function_type(type_char_ptr, 2, (type_t *[]) { type_char_ptr_restrict, type_const_char_ptr_restrict }, DM_NONE));
	l("strcat",  make_function_type(type_char_ptr, 2, (type_t *[]) { type_char_ptr_restrict, type_const_char_ptr_restrict }, DM_NONE));
	l("strchr",  make_function_type(type_char_ptr, 2, (type_t *[]) { type_const_char_ptr, type_int }, DM_NONE));
	l("strcmp",  make_function_type(type_int, 2, (type_t *[]) { type_const_char_ptr, type_const_char_ptr }, DM_PURE));
	l("strcpy",  make_function_type(type_char_ptr, 2, (type_t *[]) { type_char_ptr_restrict, type_const_char_ptr_restrict }, DM_NONE));
	l("strlen",  make_function_type(type_size_t, 1, (type_t *[]) { type_const_char_ptr }, DM_PURE));
	l("strncat", make_function_type(type_char_ptr, 3, (type_t *[]) { type_char_ptr_restrict, type_const_char_ptr_restrict, type_size_t }, DM_NONE));
	l("strncpy", make_function_type(type_char_ptr, 3, (type_t *[]) { type_char_ptr_restrict, type_const_char_ptr_restrict, type_size_t }, DM_NONE));

	entity_t *(*c)(const char*,unsigned,type_t*) = create_gnu_builtin_chk;
	c("memcpy",  3, make_function_type(type_void_ptr, 4, (type_t *[]) { type_void_ptr_restrict, type_const_void_ptr_restrict, type_size_t, type_size_t}, DM_NONE));
	c("memmove", 3, make_function_type(type_void_ptr, 4, (type_t *[]) { type_void_ptr_restrict, type_const_void_ptr_restrict, type_size_t, type_size_t }, DM_NONE));
	c("memset",  3, make_function_type(type_void_ptr, 4, (type_t *[]) { type_void_ptr, type_int, type_size_t, type_size_t }, DM_NONE));
	c("stpcpy",  2, make_function_type(type_char_ptr, 3, (type_t *[]) { type_char_ptr_restrict, type_const_char_ptr_restrict, type_size_t }, DM_NONE));
	c("stpncpy", 3, make_function_type(type_char_ptr, 4, (type_t *[]) { type_char_ptr_restrict, type_const_char_ptr_restrict, type_size_t, type_size_t }, DM_NONE));
	c("strcat",  2, make_function_type(type_char_ptr, 3, (type_t *[]) { type_char_ptr_restrict, type_const_char_ptr_restrict, type_size_t }, DM_NONE));
	c("strcpy",  2, make_function_type(type_char_ptr, 3, (type_t *[]) { type_char_ptr_restrict, type_const_char_ptr_restrict, type_size_t }, DM_NONE));
	c("strncat", 3, make_function_type(type_char_ptr, 4, (type_t *[]) { type_char_ptr_restrict, type_const_char_ptr_restrict, type_size_t, type_size_t }, DM_NONE));
	c("strncpy", 3, make_function_type(type_char_ptr, 4, (type_t *[]) { type_char_ptr_restrict, type_const_char_ptr_restrict, type_size_t, type_size_t }, DM_NONE));

	/* TODO: gcc has a LONG list of builtin functions (nearly everything from
	 * C89-C99 and others). Complete this */
}

static entity_t *find_existing_entity(const char *name)
{
	symbol_t *symbol = symbol_table_insert(name);
	return get_entity(symbol, NAMESPACE_NORMAL);
}

static void merge_builtin(entity_t *def, builtin_kind_t kind, type_t *type)
{
	if (def->kind != ENTITY_FUNCTION) {
		warningf(WARN_OTHER, &def->base.pos,
				 "language standard mandates that '%N' is a function", def);
		return;
	} else {
		/* check if the type is compatible with the libc specified one */
		type_t *def_type    = def->declaration.type;
		type_t *def_skipped = skip_typeref(def_type);
		if (!types_compatible(def_skipped, type)) {
			warningf(WARN_OTHER, &def->base.pos,
					 "declaration of '%N' with type '%T' is incompatible with language standard specified '%T'",
					 def, def_type, type);
			return;
		}
	}
	/* produce a new declaration for the builtin */
	symbol_t *symbol       = def->base.symbol;
	entity_t *builtin_func = create_builtin_function(kind, symbol, type);
	builtin_func->function.builtin_in_lib = true;
	merge_into_decl(def, builtin_func);
}

void find_known_libc_functions(void)
{
	if (dialect.freestanding)
		return;
	entity_t *(*f)(const char*) = find_existing_entity;
	void (*m)(entity_t*,builtin_kind_t,type_t*) = merge_builtin;

	if (dialect.c99) {
		type_t *type_complex_float
			= make_complex_type(ATOMIC_TYPE_FLOAT, TYPE_QUALIFIER_NONE);
		type_t *type_complex_double
			= make_complex_type(ATOMIC_TYPE_DOUBLE, TYPE_QUALIFIER_NONE);
		type_t *type_complex_ldouble
			= make_complex_type(ATOMIC_TYPE_LONG_DOUBLE, TYPE_QUALIFIER_NONE);
		type_t *type_ldouble = type_long_double;

		entity_t *cimag = f("cimag");
		if (cimag != NULL)  m(cimag,  BUILTIN_CIMAG, make_function_type(type_double,  1, (type_t *[]) { type_complex_double }, DM_CONST));
		entity_t *cimagf = f("cimagf");
		if (cimagf != NULL) m(cimagf, BUILTIN_CIMAG, make_function_type(type_float,   1, (type_t *[]) { type_complex_float },  DM_CONST));
		entity_t *cimagl = f("cimagl");
		if (cimagl != NULL) m(cimagl, BUILTIN_CIMAG, make_function_type(type_ldouble, 1, (type_t *[]) { type_complex_ldouble },  DM_CONST));

		entity_t *creal = f("creal");
		if (creal != NULL)  m(creal,  BUILTIN_CREAL, make_function_type(type_double,  1, (type_t *[]) { type_complex_double }, DM_CONST));
		entity_t *crealf = f("crealf");
		if (crealf != NULL) m(crealf, BUILTIN_CREAL, make_function_type(type_float,   1, (type_t *[]) { type_complex_float },  DM_CONST));
		entity_t *creall = f("creall");
		if (creall != NULL) m(creall, BUILTIN_CREAL, make_function_type(type_ldouble, 1, (type_t *[]) { type_complex_ldouble },  DM_CONST));
	}
}

static entity_t *create_intrinsic_firm(ir_builtin_kind kind, const char *name,
                                       type_t *type)
{
	symbol_t *symbol = symbol_table_insert(name);
	entity_t *entity = record_builtin_function(BUILTIN_FIRM, symbol, type);
	entity->function.b.firm_builtin_kind = kind;
	return entity;
}

static entity_t *create_intrinsic(builtin_kind_t kind, const char *name,
                                  type_t *type)
{
	symbol_t *symbol = symbol_table_insert(name);
	entity_t *entity = record_builtin_function(kind, symbol, type);
	return entity;
}

void create_microsoft_intrinsics(void)
{
	entity_t *(*i)(builtin_kind_t,const char*,type_t*) = create_intrinsic;
	entity_t *(*f)(ir_builtin_kind,const char*,type_t*) = create_intrinsic_firm;
	/* intrinsics for all architectures */
	i(BUILTIN_ROTL,   "_rotl",                make_function_2_type(type_unsigned_int,   type_unsigned_int, type_int, DM_CONST));
	i(BUILTIN_ROTL,   "_rotl64",              make_function_2_type(type_unsigned_int64, type_unsigned_int64, type_int, DM_CONST));
	i(BUILTIN_ROTR,   "_rotr",                make_function_2_type(type_unsigned_int,   type_unsigned_int, type_int, DM_CONST));
	i(BUILTIN_ROTR,   "_rotr64",              make_function_2_type(type_unsigned_int64, type_unsigned_int64, type_int, DM_CONST));

	f(ir_bk_bswap,    "_byteswap_ushort",     make_function_1_type(type_unsigned_short, type_unsigned_short, DM_CONST));
	f(ir_bk_bswap,    "_byteswap_ulong",      make_function_1_type(type_unsigned_long,  type_unsigned_long, DM_CONST));
	f(ir_bk_bswap,    "_byteswap_uint64",     make_function_1_type(type_unsigned_int64, type_unsigned_int64, DM_CONST));

	f(ir_bk_debugbreak,     "__debugbreak",   make_function_0_type(type_void, DM_NONE));
	f(ir_bk_return_address, "_ReturnAddress", make_function_0_type(type_void_ptr, DM_NONE));
	f(ir_bk_popcount,       "__popcount",     make_function_1_type(type_unsigned_int, type_unsigned_int, DM_CONST));

	/* x86/x64 only */
	f(ir_bk_inport,   "__inbyte",             make_function_1_type(type_unsigned_char, type_unsigned_short, DM_NONE));
	f(ir_bk_inport,   "__inword",             make_function_1_type(type_unsigned_short, type_unsigned_short, DM_NONE));
	f(ir_bk_inport,   "__indword",            make_function_1_type(type_unsigned_long, type_unsigned_short, DM_NONE));
	f(ir_bk_outport,  "__outbyte",            make_function_2_type(type_void, type_unsigned_short, type_unsigned_char, DM_NONE));
	f(ir_bk_outport,  "__outword",            make_function_2_type(type_void, type_unsigned_short, type_unsigned_short, DM_NONE));
	f(ir_bk_outport,  "__outdword",           make_function_2_type(type_void, type_unsigned_short, type_unsigned_long, DM_NONE));
	f(ir_bk_trap,     "__ud2",                make_function_type(type_void, 0, NULL, DM_NORETURN));
}

static type_t *add_type_modifier(type_t *orig_type, decl_modifiers_t modifiers)
{
	type_t *type = skip_typeref(orig_type);

	assert(type->kind == TYPE_FUNCTION);
	if ((type->function.modifiers & modifiers) == modifiers)
		return orig_type;

	type_t *new_type = duplicate_type(type);
	new_type->function.modifiers |= modifiers;
	return identify_new_type(new_type);
}

void adapt_special_functions(function_t *function)
{
	symbol_t *symbol = function->base.base.symbol;
	if (symbol == NULL)
		return;
	const char *name = symbol->string;

	/* the following list of names is taken from gcc (calls.c) */

	/* Disregard prefix _, __, __x or __builtin_.  */
	if (name[0] == '_') {
		if (strstart(name + 1, "_builtin_"))
			name += 10;
		else if (name[1] == '_' && name[2] == 'x')
			name += 3;
		else if (name[1] == '_')
			name += 2;
		else
			name += 1;
	}

	if (name[0] == 's') {
		if ((name[1] == 'e' && (streq(name, "setjmp")
		                     || streq(name, "setjmp_syscall")))
		    || (name[1] == 'i' && streq(name, "sigsetjmp"))
		    || (name[1] == 'a' && streq(name, "savectx"))) {
			function->base.type
				= add_type_modifier(function->base.type, DM_RETURNS_TWICE);
		} else if (name[1] == 'i' && streq(name, "siglongjmp")) {
			function->base.type
				= add_type_modifier(function->base.type, DM_NORETURN);
		}
	} else if ((name[0] == 'q' && streq(name, "qsetjmp"))
	           || (name[0] == 'v' && streq(name, "vfork"))
	           || (name[0] == 'g' && streq(name, "getcontext"))) {
		function->base.type
			= add_type_modifier(function->base.type, DM_RETURNS_TWICE);
	} else if (name[0] == 'l' && streq(name, "longjmp")) {
		function->base.type
			= add_type_modifier(function->base.type, DM_NORETURN);
	}
}
