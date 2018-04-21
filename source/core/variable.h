

#ifndef CORE_VARIABLE_H
#define CORE_VARIABLE_H


typedef void variable;

typedef void        (*variable_new_t)(variable *var);
typedef void        (*variable_del_t)(variable *var);
typedef const char *(*variable_get_t)(variable *var);
typedef void        (*variable_set_t)(variable *var, const char *str);
typedef void        (*variable_cb_t)(void *data, variable *var);


typedef struct vartype {

	const char *type;

	size_t size;

	variable_new_t var_new;
	variable_del_t var_del;
	variable_get_t var_get;
	variable_set_t var_set;

} vartype;


variable *var_new(const char *name, const vartype *vtbl, const char *def);
void      var_del(variable *var);

const char    *var_type(const variable *var);
const vartype *var_vt(  const variable *var);
const char    *var_name(const variable *var);

const char *var_get(variable *var);
void        var_set(variable *var, const char *str);

void var_set_cb(variable *var, variable_cb_t func, void *data);

variable *var_find(const char *name);
variable *var_nget(const char *name, const char **buf);
variable *var_nset(const char *name, const char *str);

bool var_load(const char *file);
bool var_save(const char *file);


int    *var_new_int(  const char *name, const char *def);
float  *var_new_float(const char *name, const char *def);
char  **var_new_str(  const char *name, const char *def);


#define VARTYPE(vtbl, typename, datatype, n, d, g, s) \
	const vartype vtbl = {                  \
		.type    = (typename),          \
		.size    = sizeof(datatype),    \
		.var_new = (variable_new_t)(n), \
		.var_del = (variable_del_t)(d), \
		.var_get = (variable_get_t)(g), \
		.var_set = (variable_set_t)(s)  \
	}


#endif

