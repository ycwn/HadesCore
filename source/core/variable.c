

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"


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


static list vars = LIST_INIT(&vars, NULL);

static VARTYPE(vtbl_i, "integer", int,    NULL,       NULL,       &var_i_get, &var_i_set);
static VARTYPE(vtbl_f, "float",   float,  NULL,       NULL,       &var_f_get, &var_f_set);
static VARTYPE(vtbl_s, "string",  varstr, &var_s_new, &var_s_del, &var_s_get, &var_s_set);



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

