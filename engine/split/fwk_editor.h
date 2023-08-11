// -----------------------------------------------------------------------------
// in-game editor
// - rlyeh, public domain.
//
// @todo: merge editor1.c and editor2.c internals into this api

//API void  editor();
//API bool  editor_active();
API vec3  editor_pick(float mouse_x, float mouse_y);
API char* editor_path(const char *path);

// open file dialog

API char* dialog_load();
API char* dialog_save();

// transform gizmos

API int   gizmo(vec3 *pos, vec3 *rot, vec3 *sca);
API bool  gizmo_active();
API bool  gizmo_hover();

// localization kit (I18N, L10N)

API void  kit_locale( const char *langcode_iso639_1 ); // set context language: enUS, ptBR, esES, ...
API void  kit_set( const char *variable, const char *value ); // set context variable
API void  kit_reset(); // reset all variables in context

API void  kit_insert( const char *id, const char *translation ); // insert single translation
API bool  kit_load( const char *filename ); // load translations file (xlsx)
API bool  kit_merge( const char *filename ); // merge translations file into existing context
API void  kit_clear(); // delete all translations

API char* kit_translate( const char *id ); // perform a translation, given current locale
API char* kit_translate2( const char *id, const char *langcode_iso639_1 ); // perform a translation given explicit locale

API void  kit_dump_state( FILE *fp );
