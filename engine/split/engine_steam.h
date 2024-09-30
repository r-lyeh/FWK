// ----------------------------------------------------------------------------
// steam framework

API bool steam_init(unsigned app_id);
API void steam_tick();
API void steam_trophy(const char *trophy_id, bool redeem);
API void steam_screenshot();
API void steam_destroy();

API int  ui_steam();
