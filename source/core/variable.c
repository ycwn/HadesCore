

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/string.h"
#include "core/variable.h"


typedef struct _variable_node_t {

	variable  var;
	list      node;

	const char *name;

	void           *cb_data;
	_variable_cb_t  cb_func;

} variable_node;


static list vars = LIST_INIT(&vars, NULL);



variable *var_new(const char *name, const char *def)
{

	if (name == NULL || *name == '\0')
		return NULL;

	variable_node *self = malloc(sizeof(variable_node));

	str_new(&self->var.text);

	self->var.integer = 0;
	self->var.real    = 0.0f;
	self->name        = name;
	self->cb_data     = NULL;
	self->cb_func     = NULL;

	list_init(&self->node, self);
	list_append(&vars, &self->node);

	if (def != NULL && *def != '\0')
		var_set_s(&self->var, def);

	return &self->var;

}



void var_del(variable *_self)
{

	variable_node *self = (variable_node*)_self;

	list_remove(&self->node);
	str_del(&self->var.text);
	free(self);

}



variable *var_find(const char *name)
{

	for (list *var=list_begin(&vars); var != list_end(&vars); var=var->next)
		if (!strcmp(LIST_PTR(variable_node, var)->name, name))
			return var->ptr;

	return NULL;

}



variable *var_set(const char *name, const char *value)
{

	variable *var = var_find(name);

	if (var == NULL)
		return NULL;

	var_set_s(var, value);

	log_i("var: Set '%s' to '%s'", name, value);

	return var;

}



void var_set_s(variable *_self, const char *value)
{

	if (_self == NULL || value == NULL)
		return;

	variable_node *self = (variable_node*)_self;

	str_dup(&self->var.text, value);

	self->var.real    = atof(value);
	self->var.integer = atoi(value);

	if (self->cb_func != NULL)
		(self->cb_func)(self->cb_data, &self->var);

}



void var_set_f(variable *_self, float value)
{

	if (_self == NULL)
		return;

	variable_node *self = (variable_node*)_self;

	str_printf(&self->var.text, "%f", value);

	self->var.real    = value;
	self->var.integer = (int)value;

	if (self->cb_func != NULL)
		(self->cb_func)(self->cb_data, &self->var);

}



void var_set_i(variable *_self, int value)
{

	if (_self == NULL)
		return;

	variable_node *self = (variable_node*)_self;

	str_printf(&self->var.text, "%i", value);

	self->var.real    = (float)value;
	self->var.integer = value;

	if (self->cb_func != NULL)
		(self->cb_func)(self->cb_data, &self->var);

}



void var_set_cb(variable *_self, _variable_cb_t func, void *data)
{

	if (_self == NULL)
		return;

	variable_node *self = (variable_node*)_self;

	self->cb_func = func;
	self->cb_data = data;

}



bool var_parse(const void *buf, size_t len)
{

	return false;

}

