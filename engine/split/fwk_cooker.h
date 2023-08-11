// -----------------------------------------------------------------------------
// asset pipeline framework
// - rlyeh, public domain.
//
// all cooked assets are stored inside zip file at root folder, which acts as an asset database.
// during game boot, the database gets rebuilt as follows: (note: step 0 is an optional optimization)
// 0. for N given cores, split list of infiles into N zipfiles. then, parallelize cooks.
// 1. compare local disk files against file in zip database. for each mismatch do:
// 2. - invalidate its entry in database, if local file was removed from disk.
// 3. - write its *cooked* contents into database, if local file was created or modified from disk.
// 4. mount any existing zipfile(s) after cooking.
//
// notes: meta-datas from every raw asset are stored into comment field, inside .cook.zip archive.
// @todo: fix leaks
// @todo: symlink exact files
// @todo: idle threads should steal jobs from busy threads (maybe use jobs/coroutines for this?) ...

enum COOK_FLAGS {
    COOK_SYNC = 0,
    COOK_ASYNC = 1,
    COOK_CANCELABLE = 2,
    COOK_DEBUGLOG = 4, // log all cooking commands to a batch file
};

API void cook_config( const char *path_to_cook_ini ); // "tools/cook.ini"
API bool cook_start( const char *path_to_cook_ini, const char *masks, int flags ); // COOK_INI, "**"
API void cook_stop();
API void cook_cancel();
API int  cook_jobs();     // [0..N]
API int  cook_progress(); // [0..100]

