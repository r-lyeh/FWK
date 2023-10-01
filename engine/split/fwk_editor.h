// -----------------------------------------------------------------------------
// in-game editor
// - rlyeh, public domain.
//
// @todo: merge editor1.c and editor3.c internals into this api

//API void  editor();
//API bool  editor_active();
API vec3   editor_pick(float mouse_x, float mouse_y);
API char*  editor_path(const char *path);

API float* engine_getf(const char *key);
API int*   engine_geti(const char *key);
API char** engine_gets(const char *key);
API int    engine_send(const char *cmd, const char *optional_value);

API int    ui_debug();

// open file dialog

API char* dialog_load();
API char* dialog_save();

// transform gizmos

API int   gizmo(vec3 *pos, vec3 *rot, vec3 *sca);
API bool  gizmo_active();
API bool  gizmo_hover();

// localization kit (I18N, L10N)

API bool  kit_load( const char *filename ); // load translations file (xlsx)
API bool  kit_merge( const char *filename ); // merge translations file into existing context
API void  kit_insert( const char *id, const char *translation ); // insert single translation unit
API void  kit_clear(); // delete all translations

API void  kit_set( const char *variable, const char *value ); // set context variable
API void  kit_reset(); // reset all variables in context
API void  kit_dump_state( FILE *fp ); // debug

API char* kit_translate2( const char *id, const char *langcode_iso639_1 ); // perform a translation given explicit locale

API void  kit_locale( const char *langcode_iso639_1 ); // set current locale: enUS, ptBR, esES, ...
API char* kit_translate( const char *id ); // perform a translation, given current locale
