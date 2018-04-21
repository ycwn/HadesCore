

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/algorithm.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"


#define STB_C_LEX_C_DECIMAL_INTS    N   //  "0|[1-9][0-9]*"                        CLEX_intlit
#define STB_C_LEX_C_HEX_INTS        N   //  "0x[0-9a-fA-F]+"                       CLEX_intlit
#define STB_C_LEX_C_OCTAL_INTS      N   //  "[0-7]+"                               CLEX_intlit
#define STB_C_LEX_C_DECIMAL_FLOATS  N   //  "[0-9]*(.[0-9]*([eE][-+]?[0-9]+)?)     CLEX_floatlit
#define STB_C_LEX_C99_HEX_FLOATS    N   //  "0x{hex}+(.{hex}*)?[pP][-+]?{hex}+     CLEX_floatlit
#define STB_C_LEX_C_IDENTIFIERS     Y   //  "[_a-zA-Z][_a-zA-Z0-9]*"               CLEX_id
#define STB_C_LEX_C_DQ_STRINGS      Y   //  double-quote-delimited strings with escapes  CLEX_dqstring
#define STB_C_LEX_C_SQ_STRINGS      N   //  single-quote-delimited strings with escapes  CLEX_ssstring
#define STB_C_LEX_C_CHARS           N   //  single-quote-delimited character with escape CLEX_charlits
#define STB_C_LEX_C_COMMENTS        Y   //  "/* comment */"
#define STB_C_LEX_CPP_COMMENTS      Y   //  "// comment to end of line\n"
#define STB_C_LEX_C_COMPARISONS     N   //  "==" CLEX_eq  "!=" CLEX_noteq   "<=" CLEX_lesseq  ">=" CLEX_greatereq
#define STB_C_LEX_C_LOGICAL         N   //  "&&"  CLEX_andand   "||"  CLEX_oror
#define STB_C_LEX_C_SHIFTS          N   //  "<<"  CLEX_shl      ">>"  CLEX_shr
#define STB_C_LEX_C_INCREMENTS      N   //  "++"  CLEX_plusplus "--"  CLEX_minusminus
#define STB_C_LEX_C_ARROW           N   //  "->"  CLEX_arrow
#define STB_C_LEX_EQUAL_ARROW       N   //  "=>"  CLEX_eqarrow
#define STB_C_LEX_C_BITWISEEQ       N   //  "&="  CLEX_andeq    "|="  CLEX_oreq     "^="  CLEX_xoreq
#define STB_C_LEX_C_ARITHEQ         N   //  "+="  CLEX_pluseq   "-="  CLEX_minuseq
                                        //  "*="  CLEX_muleq    "/="  CLEX_diveq    "%=" CLEX_modeq
                                        //  if both STB_C_LEX_SHIFTS & STB_C_LEX_ARITHEQ:
                                        //                      "<<=" CLEX_shleq    ">>=" CLEX_shreq

#define STB_C_LEX_PARSE_SUFFIXES    N   // letters after numbers are parsed as part of those numbers, and must be in suffix list below
#define STB_C_LEX_DECIMAL_SUFFIXES  ""  // decimal integer suffixes e.g. "uUlL" -- these are returned as-is in string storage
#define STB_C_LEX_HEX_SUFFIXES      ""  // e.g. "uUlL"
#define STB_C_LEX_OCTAL_SUFFIXES    ""  // e.g. "uUlL"
#define STB_C_LEX_FLOAT_SUFFIXES    ""  //

#define STB_C_LEX_0_IS_EOF             N  // if Y, ends parsing at '\0'; if N, returns '\0' as token
#define STB_C_LEX_INTEGERS_AS_DOUBLES  N  // parses integers as doubles so they can be larger than 'int', but only if STB_C_LEX_STDLIB==N
#define STB_C_LEX_MULTILINE_DSTRINGS   N  // allow newlines in double-quoted strings
#define STB_C_LEX_MULTILINE_SSTRINGS   N  // allow newlines in single-quoted strings
#define STB_C_LEX_USE_STDLIB           Y  // use strtod,strtol for parsing #s; otherwise inaccurate hack
#define STB_C_LEX_DOLLAR_IDENTIFIER    Y  // allow $ as an identifier character
#define STB_C_LEX_FLOAT_NO_DECIMAL     Y  // allow floats that have no decimal point if they have an exponent

#define STB_C_LEX_DEFINE_ALL_TOKEN_NAMES  N   // if Y, all CLEX_ token names are defined, even if never returned
                                              // leaving it as N should help you catch config bugs

#define STB_C_LEX_DISCARD_PREPROCESSOR    Y   // discard C-preprocessor directives (e.g. after prepocess
                                              // still have #line, #pragma, etc)
#define STB_C_LEXER_IMPLEMENTATION
#define STB_C_LEXER_DEFINITIONS         // This line prevents the header file from replacing your definitions

#include <STB/stb_c_lexer.h>


#define VARPTR(self) ((varnode*)(((const u8*)self) - offsetof(varnode, payload)))


typedef struct varnode {

	const char    *name;
	const vartype *vtbl;

	void          *cb_data;
	variable_cb_t  cb_func;

	list node;
	u8   payload[];

} varnode;


typedef struct varstr {

	char   *str;
	size_t  capacity;

} varstr;


static const char *var_i_get(int *var);
static void        var_i_set(int *var, const char *str);

static const char *var_f_get(float *var);
static void        var_f_set(float *var, const char *str);

static void        var_s_new(varstr *var);
static void        var_s_del(varstr *var);
static const char *var_s_get(varstr *var);
static void        var_s_set(varstr *var, const char *str);

static void var_dump_r(FILE *f, variable *vars[], int start, int end, int offset, int level);
static bool var_parse_r(stb_lexer *lex, const char *ns, int nsl);


static list vars = LIST_INIT(&vars, NULL);

static VARTYPE(vtbl_i, "integer", int,    NULL,       NULL,       &var_i_get, &var_i_set);
static VARTYPE(vtbl_f, "float",   float,  NULL,       NULL,       &var_f_get, &var_f_set);
static VARTYPE(vtbl_s, "string",  varstr, &var_s_new, &var_s_del, &var_s_get, &var_s_set);



static inline int compare_variable_names(const variable **a, const variable **b)
{
	return strcmp(var_name(*a), var_name(*b));
}

GENERATE_SHELL_SORT(sort_variables, variable*, compare_variable_names);



variable *var_new(const char *name, const vartype *vtbl, const char *def)
{

	if (name == NULL || vtbl == NULL || *name == '\0')
		return NULL;

	varnode  *self = malloc(sizeof(varnode) + vtbl->size + strlen(name) + 1);
	variable *var  = &self->payload;

	self->name = (const char*)&self->payload[vtbl->size];
	self->vtbl = vtbl;

	self->cb_data = NULL;
	self->cb_func = NULL;

	strcpy((char*)self->name, name);

	list_init(&self->node, self);
	list_append(&vars, &self->node);

	if (vtbl->var_new != NULL)
		vtbl->var_new(var);

	if (def != NULL && *def != '\0')
		var_set(var, def);

	return var;

}



void var_del(variable *var)
{

	varnode *self = VARPTR(var);

	if (self->vtbl->var_del != NULL)
		self->vtbl->var_del(var);

	list_remove(&self->node);
	free(self);

}



const char *var_type(const variable *var)
{

	return VARPTR(var)->vtbl->type;

}



const vartype *var_vt(const variable *var)
{

 	return VARPTR(var)->vtbl;

}



const char *var_name(const variable *var)
{

	return VARPTR(var)->name;

}



const char *var_get(variable *var)
{

	varnode *self = VARPTR(var);

	if (self->vtbl->var_get != NULL)
		return self->vtbl->var_get(var);

	return NULL;

}



void var_set(variable *var, const char *str)
{

	varnode *self = VARPTR(var);

	if (self->vtbl->var_set != NULL)
		self->vtbl->var_set(var, str);

}



void var_set_cb(variable *var, variable_cb_t func, void *data)
{

	varnode *self = VARPTR(var);

	self->cb_func = func;
	self->cb_data = data;

}



variable *var_find(const char *name)
{

	for (list *var=list_begin(&vars); var != list_end(&vars); var=var->next)
		if (!strcmp(LIST_PTR(varnode, var)->name, name))
			return &LIST_PTR(varnode, var)->payload;

	return NULL;

}



variable *var_nget(const char *name, const char **buf)
{

	variable *var = var_find(name);

	if (var == NULL)
		return NULL;

	if (buf != NULL)
		*buf = var_get(var);

	return var;

}



variable *var_nset(const char *name, const char *str)
{

	variable *var = var_find(name);

	if (var == NULL)
		return NULL;

	if (str != NULL) {

		var_set(var, str);
		log_i("var: Set '%s' to '%s'", name, str);

	}

	return var;

}



bool var_load(const char *file)
{

	FILE *cfg = fopen(file, "r");

	if (cfg == NULL)
		return false;

	fseek(cfg, 0, SEEK_END);

	size_t len = ftell(cfg);
	char   buf[len + 1];
	char   tmp[len + 1];

	fseek(cfg, 0, SEEK_SET);
	fread(buf, len, 1, cfg);
	fclose(cfg);

	buf[len] = '\0';

	stb_lexer lex;
	stb_c_lexer_init(&lex, buf, buf + len, tmp, sizeof(tmp));

	if (!var_parse_r(&lex, "", 0)) {

		size_t           toklen = lex.where_lastchar - lex.where_firstchar + 1;
		char             token[toklen + 1];
		stb_lex_location loc;

		memcpy(token, lex.where_firstchar, toklen);
		token[toklen] = 0;

		stb_c_lexer_get_location(&lex, lex.where_firstchar, &loc);

		log_e("config: %s: %d:%d: Parser stopped at '%s'",
			file,
			loc.line_number, loc.line_offset,
			token);

		return false;

	}

	return true;

}



bool var_save(const char *file)
{

	FILE *cfg = fopen(file, "w");

	if (cfg == NULL)
		return false;

	int num = list_size(&vars);
	int n   = 0;

	variable *nodes[num];

	for (list *var=list_begin(&vars); var != list_end(&vars); var=var->next)
		nodes[n++] = &LIST_PTR(varnode, var)->payload;

	sort_variables(nodes, num);
	var_dump_r(cfg, nodes, 0, num, 0, 0);

	fclose(cfg);

	return true;

}



int *var_new_int(const char *name, const char *def)
{

	return var_new(name, &vtbl_i, def);

}



float *var_new_float(const char *name, const char *def)
{

	return var_new(name, &vtbl_f, def);

}



char **var_new_str(const char *name, const char *def)
{

	return var_new(name, &vtbl_s, def);

}



const char *var_i_get(int *var)
{
	static char buf[32];

	sprintf(buf, "%d", *var);
	return buf;

}



void var_i_set(int *var, const char *str)
{

	*var = atoi(str);

}



const char *var_f_get(float *var)
{
	static char buf[32];

	sprintf(buf, "%f", *var);
	return buf;

}



void var_f_set(float *var, const char *str)
{

	*var = atof(str);

}



void var_s_new(varstr *var)
{

	var->str      = NULL;
	var->capacity = 0;

}



void var_s_del(varstr *var)
{

	free(var->str);

}



const char *var_s_get(varstr *var)
{

	return var->str;

}



void var_s_set(varstr *var, const char *str)
{

	size_t len = strlen(str);

	if (len >= var->capacity) {

		var->capacity = len + 1;
		var->str = realloc(var->str, var->capacity);

	}

	strcpy(var->str, str);

}



void var_dump_r(FILE *f, variable *vars[], int start, int end, int offset, int level)
{

	char tabs[level + 1];

	memset(tabs, '\t', level);
	tabs[level] = '\0';

	while (start < end) {

		const char *name = var_name(vars[start]);
		const char *dot  = strchr(name + offset, '.');

		if (dot != NULL) {

			int num = 0;
			int len = dot - (name + offset);

			for (num=1; start + num < end; num++) {

				const char *nn = var_name(vars[start + num]);
				const char *dd = strchr(nn + offset, '.');

				if (dd == NULL)
					break;

				int ll = dd - (nn + offset);

				if (ll != len || strncmp(name + offset, nn + offset, len) != 0)
					break;

			}

			fprintf(f, "\n%s%.*s {\n", tabs, len, offset + name);
			var_dump_r(f, vars, start, start + num, offset + len + 1, level + 1);

			fprintf(f, "%s}\n\n", tabs);
			start += num;

		} else {

			fprintf(f, "%s%s = \"%s\";\n", tabs, name + offset, var_get(vars[start]));
			start++;

		}

	}

}



bool var_parse_r(stb_lexer *lex, const char *ns, int nsl)
{

	while (true) {

		if (!stb_c_lexer_get_token(lex))
			break;

		if (lex->token == '}' && nsl > 0)
			return true;

		if (lex->token != CLEX_id)
			return false;

		int  len = lex->string_len;
		char name[nsl + len + 2];

		strcpy(name, ns);
		strcat(name, lex->string);

		int tok = stb_c_lexer_get_token(lex)? lex->token: 0;

		if (tok == '=') {

			if (!stb_c_lexer_get_token(lex) || lex->token != CLEX_dqstring)
				return false;

			if (var_nset(name, lex->string) == NULL)
				log_w("config: unknown variable '%s', skipping...", name);

			if (!stb_c_lexer_get_token(lex) || lex->token != ';')
				return false;

		} else if (tok == '{') {

			strcat(name, ".");

			if (!var_parse_r(lex, name, nsl + len + 1))
				return false;

		} else
			return false;

	}

	return true;

}
