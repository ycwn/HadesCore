

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/blob.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/pixelformat.h"
#include "gr/surface.h"
#include "gr/renderpass.h"
#include "gr/builder.h"


#define STB_C_LEX_C_DECIMAL_INTS    Y   //  "0|[1-9][0-9]*"                        CLEX_intlit
#define STB_C_LEX_C_HEX_INTS        Y   //  "0x[0-9a-fA-F]+"                       CLEX_intlit
#define STB_C_LEX_C_OCTAL_INTS      Y   //  "[0-7]+"                               CLEX_intlit
#define STB_C_LEX_C_DECIMAL_FLOATS  Y   //  "[0-9]*(.[0-9]*([eE][-+]?[0-9]+)?)     CLEX_floatlit
#define STB_C_LEX_C99_HEX_FLOATS    N   //  "0x{hex}+(.{hex}*)?[pP][-+]?{hex}+     CLEX_floatlit
#define STB_C_LEX_C_IDENTIFIERS     Y   //  "[_a-zA-Z][_a-zA-Z0-9]*"               CLEX_id
#define STB_C_LEX_C_DQ_STRINGS      Y   //  double-quote-delimited strings with escapes  CLEX_dqstring
#define STB_C_LEX_C_SQ_STRINGS      N   //  single-quote-delimited strings with escapes  CLEX_ssstring
#define STB_C_LEX_C_CHARS           Y   //  single-quote-delimited character with escape CLEX_charlits
#define STB_C_LEX_C_COMMENTS        Y   //  "/* comment */"
#define STB_C_LEX_CPP_COMMENTS      Y   //  "// comment to end of line\n"
#define STB_C_LEX_C_COMPARISONS     Y   //  "==" CLEX_eq  "!=" CLEX_noteq   "<=" CLEX_lesseq  ">=" CLEX_greatereq
#define STB_C_LEX_C_LOGICAL         Y   //  "&&"  CLEX_andand   "||"  CLEX_oror
#define STB_C_LEX_C_SHIFTS          Y   //  "<<"  CLEX_shl      ">>"  CLEX_shr
#define STB_C_LEX_C_INCREMENTS      Y   //  "++"  CLEX_plusplus "--"  CLEX_minusminus
#define STB_C_LEX_C_ARROW           Y   //  "->"  CLEX_arrow
#define STB_C_LEX_EQUAL_ARROW       N   //  "=>"  CLEX_eqarrow
#define STB_C_LEX_C_BITWISEEQ       Y   //  "&="  CLEX_andeq    "|="  CLEX_oreq     "^="  CLEX_xoreq
#define STB_C_LEX_C_ARITHEQ         Y   //  "+="  CLEX_pluseq   "-="  CLEX_minuseq
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


static const char *keywords[]={

	"if", "else",
	"import",
	"renderpass", "sampler", "shader", "surface",

	"load",  "store",
	"framebuffer",
	"clear", "input", "output", "preserve", "priority", "ignore",

	"width", "height", "format",

	NULL

};


enum {

	K_INIT = CLEX_first_unused_token,

	K_IF, K_ELSE,
	K_IMPORT,
	K_RENDERPASS, K_SAMPLER, K_SHADER, K_SURFACE,

	K_LOAD, K_STORE,
	K_FRAMEBUFFER,
	K_CLEAR, K_INPUT, K_OUTPUT, K_PRESERVE, K_PRIORITY, K_IGNORE,

	K_WIDTH, K_HEIGHT, K_FORMAT

};


typedef struct symbol {

	char *name;
	int   value;

} symbol;


static symbol *symbol_add(const char *name);
static void    symbol_del(symbol *sym);
static symbol *symbol_find(const char *name);

static int  parse_token(     stb_lexer *lex);
static bool parse_integer(   stb_lexer *lex, int *value);
static int  parse_number(    stb_lexer *lex, int *i, float *f);
static bool parse_format(    stb_lexer *lex, int *format);
static bool parse_surface(   stb_lexer *lex);
static bool parse_renderpass(stb_lexer *lex);
static bool parse_pipeline(  stb_lexer *lex);


static symbol *parser_symbols     = NULL;
static int     parser_symbols_num = 0;
static int     parser_symbols_max = 0;


#define PARSE_ERROR(msg, ...)                                      \
	do {                                                       \
		log_e("graphics: builder: " msg, ## __VA_ARGS__);  \
		return false;                                      \
	} while (0)


#define PARSE_ERROR_UNEXPECT(tok)                 \
	do {                                      \
		PARSE_ERROR("Unexpected token!"); \
	} while (0)



void gr_builder_create()
{
}



void gr_builder_destroy()
{
}



bool gr_builder_define(const char *name, int value)
{

	symbol *sym = symbol_find(name);

	if (sym != NULL) {

		log_e("graphics: builder: Symbol '%s' already defined to %d", name, sym->value);
		return false;

	}

	sym = symbol_add(name);
	sym->value = value;

	return true;

}



void gr_builder_undef(const char *name)
{

	symbol *sym = symbol_find(name);

	if (sym != NULL)
		symbol_del(sym);

}



bool gr_builder_parse(const char *file)
{

	return gr_builder_parsefd(blob_open(file, BLOB_REV_LAST));
}



bool gr_builder_parsefd(int fd)
{

	const char *buf = blob_get_data(fd);
	size_t      len = blob_get_length(fd);

	if (buf == NULL)
		return false;

	char store[len + 1];

	stb_lexer lex;
	stb_c_lexer_init(&lex, buf, buf + len, store, sizeof(store));

	if (!parse_pipeline(&lex)) {

		size_t           toklen = lex.where_lastchar - lex.where_firstchar + 1;
		char             token[toklen + 1];
		stb_lex_location loc;

		memcpy(token, lex.where_firstchar, toklen);
		token[toklen] = 0;

		stb_c_lexer_get_location(&lex, lex.where_firstchar, &loc);

		log_e("graphics: builder: %s: %d:%d: Parser stopped at '%s'",
			blob_get_name(fd),
			loc.line_number, loc.line_offset,
			token);

		return false;

	}

	return true;

}



symbol *symbol_add(const char *name)
{

	if (parser_symbols_num >= parser_symbols_max) {

		parser_symbols_max += 16;
		parser_symbols      = realloc(parser_symbols, sizeof(symbol) * parser_symbols_max);

	}

	symbol *sym = &parser_symbols[parser_symbols_num++];

	sym->name  = strdup(name);
	sym->value = 0;

	return sym;

}



void symbol_del(symbol *sym)
{

	int index = sym - &parser_symbols[0];

	if (index < 0 || index >= parser_symbols_num)
		return;

	free(sym->name);

	sym->name  = NULL;
	sym->value = 0;

	parser_symbols_num--;

	if (index < parser_symbols_num)
		parser_symbols[index] = parser_symbols[parser_symbols_num];

}



symbol *symbol_find(const char *name)
{

	for (int n=0; n < parser_symbols_num; n++)
		if (!strcmp(parser_symbols[n].name, name))
			return &parser_symbols[n];

	return NULL;

}



int parse_token(stb_lexer *lex)
{

	if (!stb_c_lexer_get_token(lex))
		return -1;

	if (lex->token != CLEX_id)
		return lex->token;

	for (int n=0; keywords[n] != NULL; n++)
		if (!strcmp(lex->string, keywords[n]))
			return K_INIT + n + 1;

	return CLEX_id;

}



bool parse_integer(stb_lexer *lex, int *value)
{

	int tok   = parse_token(lex);
	bool sign = tok == '-';

	if (tok == '+' || tok == '-')
		tok = parse_token(lex);

	if (tok == CLEX_id) {

		symbol *sym = symbol_find(lex->string);

		if (sym == NULL)
			return false;

		if (value != NULL)
			*value = sign? -sym->value: sym->value;

		return true;

	} else if (tok == CLEX_intlit) {

		if (value != NULL)
			*value = sign? -lex->int_number: lex->int_number;

		return true;

	}

	return false;

}



int parse_number(stb_lexer *lex, int *i, float *f)
{

	int tok   = parse_token(lex);
	bool sign = tok == '-';

	if (tok == '+' || tok == '-')
		tok = parse_token(lex);

	if (tok == CLEX_id) {

		symbol *sym = symbol_find(lex->string);

		if (sym == NULL)
			return CLEX_parse_error;

		if (i != NULL)
			*i = sign? -sym->value: sym->value;

		return CLEX_intlit;

	} else if (tok == CLEX_intlit) {

		if (i != NULL)
			*i = sign? -lex->int_number: lex->int_number;

		return CLEX_intlit;

	} else if (tok == CLEX_floatlit) {

		if (f != NULL)
			*f = sign? -lex->real_number: lex->real_number;

		return CLEX_intlit;

	}

	return CLEX_parse_error;

}



bool parse_format(stb_lexer *lex, int *format)
{

	int token = parse_token(lex);
	int form  = VK_FORMAT_UNDEFINED;

	if (token == '{') {

		while (true) {

			if (parse_token(lex) != CLEX_dqstring)
				PARSE_ERROR_UNEXPECT(CLEX_dqstring);

			const gr_pixelformat *pf = gr_pixelformat_find(lex->string);

			if (pf == NULL)
				PARSE_ERROR("Unknown pixelformat '%s'", lex->string);

			if ((form == VK_FORMAT_UNDEFINED) && (pf->properties.optimalTilingFeatures & (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)))
				form = pf->format;

			int sep = parse_token(lex);

			if      (sep == '}') break;
			else if (sep != ',') PARSE_ERROR_UNEXPECT(',');

		}

		if (form == VK_FORMAT_UNDEFINED)
			PARSE_ERROR("No supported pixelformats");

	} else {

		if (token != CLEX_dqstring)
			PARSE_ERROR_UNEXPECT(CLEX_dqstring);

		const gr_pixelformat *pf = gr_pixelformat_find(lex->string);

		if (pf == NULL)
			PARSE_ERROR("Unknown pixelformat '%s'", lex->string);

		if ((pf->properties.optimalTilingFeatures & (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) == 0)
			PARSE_ERROR("Pixelformat %s is not supported", pf->name);

		form = pf->format;

	}

	if (format != NULL)
		*format = form;

	return form != VK_FORMAT_UNDEFINED;

}



bool parse_surface(stb_lexer *lex)
{

	if (parse_token(lex) != CLEX_id)
		return false;

	int width  = -1;
	int height = -1;
	int format = VK_FORMAT_UNDEFINED;

	char name[lex->string_len + 1];
	strcpy(name, lex->string);

	gr_surface *s = gr_surface_find(lex->string);

	if (s != NULL)
		PARSE_ERROR("Another surface with name '%s' already exists", lex->string);

	if (parse_token(lex) != '{')
		PARSE_ERROR_UNEXPECT('{');

	while (true) {

		int stmt = parse_token(lex);

		if      (stmt == '}') break;
		else if (stmt == K_WIDTH)  { if (!parse_integer(lex, &width)  || parse_token(lex) != ';') return false; }
		else if (stmt == K_HEIGHT) { if (!parse_integer(lex, &height) || parse_token(lex) != ';') return false; }
		else if (stmt == K_FORMAT) { if (!parse_format( lex, &format)) return false; }
		else
			PARSE_ERROR("Surface definition statement expected");

	}

	if (width  < 0)                    PARSE_ERROR("Surface '%s' has no width", name);
	if (height < 0)                    PARSE_ERROR("Surface '%s' has no height", name);
	if (format == VK_FORMAT_UNDEFINED) PARSE_ERROR("Surface '%s' has no format", name);

	gr_surface *surf = gr_surface_attachment(name, width, height, format, true);

	if (surf == NULL)
		PARSE_ERROR("Failed to create surface '%s'", name);

	log_d("Created '%s' surface as %dx%d %s", surf->name, surf->width, surf->height, surf->pf->name);

	return true;

}



bool parse_renderpass(stb_lexer *lex)
{

	if (parse_token(lex) != CLEX_id)
		PARSE_ERROR_UNEXPECT(CLEX_id);

	log_d("Starting renderpass %s\n", lex->string);

	gr_renderpass *rp = gr_renderpass_find(lex->string);

	if (rp != NULL)
		PARSE_ERROR("Renderpass '%s' already exists!", lex->string);

	rp = gr_renderpass_new(lex->string);

	if (parse_token(lex) != '{')
		return false;

	int priority = -1;
	int width    = -1;
	int height   = -1;

	while (true) {

		int tok = parse_token(lex);

		if      (tok == '}')        break;
		else if (tok == K_PRIORITY) {

			int priority = 0;

			if (!parse_integer(lex, &priority))
				return false;

			if (parse_token(lex) != ';')
				PARSE_ERROR_UNEXPECT(';');

		} else if (tok == K_WIDTH) {

			if (!parse_integer(lex, &width))
				return false;

			if (parse_token(lex) != ';')
				PARSE_ERROR_UNEXPECT(';');

		} else if (tok == K_HEIGHT) {

			if (!parse_integer(lex, &height))
				return false;

			if (parse_token(lex) != ';')
				PARSE_ERROR_UNEXPECT(';');

		} else if (tok == K_INPUT) {

			int pos = 0;

			if (parse_token(lex) != CLEX_id)
				PARSE_ERROR_UNEXPECT(CLEX_id);

			gr_surface *surf = gr_surface_find(lex->string);

			if (surf == NULL)
				PARSE_ERROR("Surface '%s' doesn't exist", lex->string);

			if (parse_token(lex) != ',')
				PARSE_ERROR_UNEXPECT(',');

			if (!parse_integer(lex, &pos))
				return false;

			if (parse_token(lex) != ';')
				PARSE_ERROR_UNEXPECT(';');

			gr_renderpass_surface(rp, surf);

		} else if (tok == K_OUTPUT) {

			int           last        = -1;
			int           load_mode   = 0;
			int           store_mode  = 0;
			int           image_mode  = 0;
			VkClearValue  clear_value = { 0 };
			gr_surface   *surf        = NULL;

			int tok = parse_token(lex);

			if (tok != K_FRAMEBUFFER) {

				if (tok != CLEX_id)
					PARSE_ERROR_UNEXPECT(CLEX_id);

				surf = gr_surface_find(lex->string);

				if (surf == NULL)
					PARSE_ERROR("Surface '%s' doesn't exist", lex->string);

				image_mode =
					(surf->pf->aspect & VK_IMAGE_ASPECT_COLOR_BIT)?   GR_RENDERPASS_LAYOUT_OUT_C_OPTIMAL:
					(surf->pf->aspect & VK_IMAGE_ASPECT_DEPTH_BIT)?   GR_RENDERPASS_LAYOUT_OUT_DS_OPTIMAL:
					(surf->pf->aspect & VK_IMAGE_ASPECT_STENCIL_BIT)? GR_RENDERPASS_LAYOUT_OUT_DS_OPTIMAL:
					0;

			} else
				image_mode = GR_RENDERPASS_LAYOUT_OUT_PRESENT;

			tok = parse_token(lex);

			if (tok == ':') {

				while (true) {

					tok = parse_token(lex);

					if (tok == K_LOAD) {

						if (parse_token(lex) != ':')
							PARSE_ERROR_UNEXPECT(':');

						tok  = parse_token(lex);
						last = tok;

						if      (tok == K_PRESERVE) { load_mode = GR_RENDERPASS_LOAD_PRESERVE; }
						else if (tok == K_IGNORE)   { load_mode = GR_RENDERPASS_LOAD_DONTCARE; }
						else if (tok == K_CLEAR)    { load_mode = GR_RENDERPASS_LOAD_CLEAR; }
						else
							PARSE_ERROR("Invalid attachment load mode");

					} else if (tok == K_STORE) {

						if (parse_token(lex) != ':')
							PARSE_ERROR_UNEXPECT(':');

						tok = parse_token(lex);
						last = -1;

						if      (tok == K_PRESERVE) { store_mode = GR_RENDERPASS_STORE_PRESERVE; }
						else if (tok == K_IGNORE)   { store_mode = GR_RENDERPASS_STORE_DONTCARE; }
						else
							PARSE_ERROR("Invalid attachment store mode");

					} else
						PARSE_ERROR("Output attachment attribute expected");

					tok = parse_token(lex);

					if (tok == '(') {

						for (int n=0;; n++) {

							if (n >= 4)
								PARSE_ERROR("Too many components");

							if (parse_number(lex, &clear_value.color.int32[n], &clear_value.color.float32[n]) == CLEX_parse_error)
								return false;

							int sep  = parse_token(lex);

							if      (sep == ')') break;
							else if (sep != ',') return false;

						}

						tok = parse_token(lex);

					}

					if      (tok == ';') break;
					else if (tok != ',') PARSE_ERROR("Output attachment attribute expected");

				}

			}

			if (tok != ';')
				PARSE_ERROR_UNEXPECT(';');

			if (surf != NULL)
				gr_renderpass_attach(rp, surf->image_view, &clear_value, surf->pf->format, load_mode | store_mode | image_mode);

			else
				gr_renderpass_attach(rp, NULL, &clear_value, VK_FORMAT_UNDEFINED, load_mode | store_mode | image_mode);

		} else
			PARSE_ERROR("Renderpass definition statement expected");

	}

	rp->width  = width;
	rp->height = height;

	gr_renderpass_build(rp);

	return true;

}



bool parse_pipeline(stb_lexer *lex)
{

	while (true) {

		int token = parse_token(lex);

		if      (token < 0)             return true;
		else if (token == K_SURFACE)    { if (!parse_surface(lex))    return false; }
		else if (token == K_RENDERPASS) { if (!parse_renderpass(lex)) return false; }
		else
			return false;
	}

	return true;

}

