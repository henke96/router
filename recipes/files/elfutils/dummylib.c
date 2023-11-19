// elfutils is a pain to build on musl..
// Trick the configure script to avoid useless dependencies.
void argp_parse(void) {}
void fts_close(void) {}
void _obstack_free(void) {}
