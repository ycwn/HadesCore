

alias variable_cb = void function(void *data, variable *var);

struct variable {

//	string text;
	int    i;
	float  f;

}


extern(C) {

	variable *var_new(const char *name, const char *def);
	void      var_del(variable *self);

	const(char) *var_name(const variable *self);

	variable *var_find(const char *name);
	variable *var_set( const char *name, const char *value);

	void var_set_s(variable *self, const char *value);
	void var_set_f(variable *self, float       value);
	void var_set_i(variable *self, int         value);

	void var_set_cb(variable *self, variable_cb func, void *data);

	bool var_load(const(char) *file);
	bool var_save(const(char) *file);

}

