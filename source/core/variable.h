

#ifndef __CORE_VARIABLE_H
#define __CORE_VARIABLE_H


typedef struct _variable_t      variable;
typedef struct _variable_node_t variable_node;

typedef void (*_variable_cb_t)(void *data, variable *var);


typedef struct _variable_t {

	string text;
	int    integer;
	float  real;

} variable;


variable *var_new(const char *name, const char *def);
void      var_del(variable *self);

const char *var_name(const variable *self);

variable *var_find(const char *name);
variable *var_set( const char *name, const char *value);

void var_set_s(variable *self, const char *value);
void var_set_f(variable *self, float       value);
void var_set_i(variable *self, int         value);

void var_set_cb(variable *self, _variable_cb_t func, void *data);

bool var_read( const void *buf, size_t len);
bool var_write(void *buf,       size_t len);


#endif

