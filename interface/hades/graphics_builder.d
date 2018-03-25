

extern(C) {

	void gr_builder_create();
	void gr_builder_destroy();

	bool gr_builder_define(const(char) *name, int value);
	void gr_builder_undef(const(char) *name);

	bool gr_builder_has_include_path(const(char) *dir);
	bool gr_builder_add_include_path(const(char) *dir);

	bool gr_builder_parse(const(char) *file);
	bool gr_builder_parsefd(int fd);

}

