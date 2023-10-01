// file browser for nuklear, based on https://github.com/vurtun/nuklear/blob/master/example/file_browser.c (public domain)
// - rlyeh, public domain
//
// changelog:
// - ported to FWK api
// - namespaced symbols
// - diverse win32 fixes
// - adaptive cols/rows
// - removed nk_begin()/nk_end() pairs
// - dangling nk_group_begin/end() pairs
// - simplified file<->media_group concept
// - minor cosmetics

#ifdef _WIN32
#include <direct.h> // _getcwd()
#else
#include <unistd.h> // getcwd()
#include <pwd.h>    // getpwuid()
#define _popen popen
#define _pclose pclose
#endif

const char** old_file_list(const char *cwd, const char *masks) {
    ASSERT(strend(cwd, "/"), "Error: dirs like '%s' must end with slash", cwd);

    static __thread array(char*) list = 0;
    const char *arg0 = cwd; // app_path();
    int larg0 = strlen(arg0);

    for( int i = 0; i < array_count(list); ++i ) {
        FREE(list[i]);
    }
    array_resize(list, 0);//array_free(list);

    for each_substring(masks,";",it) {
        int recurse = !!strstr(it, "**");
        #if is(win32)
        char *glob = va("dir %s/b/o:n \"%s\\%s\" 2> NUL", recurse ? "/s":"", cwd, it);
        #else // linux, osx
        char *glob = va("find %s %s -name \"%s\" | sort", cwd, !recurse ? "-maxdepth 1":"-type f", it);
        #endif
        for( FILE *in = _popen(glob, "r"); in; _pclose(in), in = 0) {
            char buf[1024], *line = buf;
            while( fgets(buf, sizeof(buf), in) ) {
                // clean up
                if( strstr(line, arg0) ) line = buf + larg0;
                if( !memcmp(line, "./", 2) ) line += 2;
                int len = strlen(line); while( len > 0 && line[len-1] < 32 ) line[--len] = 0;
                if( line[0] == '\0' ) continue;
                // do not insert system folders/files
                for(int i = 0; i < len; ++i ) if(line[i] == '\\') line[i] = '/';
                if( line[0] == '.' ) if( !strcmp(line,".git") || !strcmp(line,".vs") || !strcmp(line,".") || !strcmp(line,"..") ) continue;
                if( strstr(line, "/.") ) continue;
                // insert copy
                #if is(win32)
                char *copy = STRDUP(line); // full path already provided
                #else
                // while(line[0] == '/') ++line;
                char *copy = STRDUP(va("%s%s", cwd, line)); // need to prepend path
                #endif
                array_push(list, copy);
            }
        }
    }
    array_push(list, 0); // terminator
    return (const char**)list;
}

#if 1
#define BROWSER_PRINTF(...) do {} while(0)
#else
#define BROWSER_PRINTF      printf
#endif

enum browser_groups {
    BROWSER_FOLDER,
    BROWSER_HOME,
    BROWSER_DESKTOP,
    BROWSER_COMPUTER,
    BROWSER_PROJECT,
    BROWSER_MAXFOLDERS,

    BROWSER_MAXTYPES = 64,
};

struct browser_media_group {
    unsigned icon;
    const char *extensions;
};

struct browser_media {
    int font;
    int icon_sheet;
    struct nk_image custom_folders[BROWSER_MAXFOLDERS];
    struct nk_image custom_files[BROWSER_MAXTYPES];
    struct browser_media_group group[BROWSER_MAXTYPES];
} media = {0};

void browser_config_dir(struct nk_image icon, unsigned counter) {
    if( counter < BROWSER_MAXFOLDERS ) {
        media.custom_folders[ counter ] = icon;
    }
}

void browser_config_type(struct nk_image icon, const char *extensions) {
    static int counter = 0;
    if( counter < BROWSER_MAXTYPES ) {
        media.custom_files[ counter ] = icon;
        media.group[ counter ].icon = counter;
        media.group[ counter ].extensions = extensions;
        ++counter;
    }
}

#define BROWSER_MAX_PATH 512
struct browser {
    /* path */
    char file[BROWSER_MAX_PATH]; // selection
    char directory[BROWSER_MAX_PATH]; // current cwd while browsing

    char home[BROWSER_MAX_PATH];
    char desktop[BROWSER_MAX_PATH];
    char computer[BROWSER_MAX_PATH];
    char project[BROWSER_MAX_PATH]; // cwd when first invoked

    /* directory content */
    array(char*) files;
    array(char*) directories;
    size_t file_count;
    size_t dir_count;

    /* filtered directory content */
    array(char*) ffiles;
    array(char*) fdirectories;

    /* view mode */
    bool listing;
    float zooming;
};

static struct nk_image* media_icon_for_file(const char *file) {
    /* extract extension .xxx from file */
    char *ext = strrchr(file, '.');
    if( ext && strlen(ext) < 16 ) {
        char ext_dot[16+1];
        snprintf(ext_dot, 16, "%s.", ext);
        /* check for all file definition of all groups for fitting extension. skip first group (default) */
        for (int i = 1; i < BROWSER_MAXTYPES && media.group[i].extensions; ++i) {
            if( strstri(media.group[i].extensions, ext_dot) ) {
                return &media.custom_files[ media.group[i].icon ];
            }
        }
    }

    // return first (default) group
    return &media.custom_files[0];
}


static void browser_reload_directory_content(struct browser *browser, const char *path) {
    if(path[0] == '\0') path = va("./");
    if(!strend(path, "/")) path = va("%s/", path);

    for(int i = 0; i < array_count(browser->files); ++i) FREE(browser->files[i]);
    for(int i = 0; i < array_count(browser->directories); ++i) FREE(browser->directories[i]);

    array_resize(browser->files, 0);
    array_resize(browser->directories, 0);

    BROWSER_PRINTF("searching at %s\n", path);

    const char** list = old_file_list(path, "*");
    for( int i = 0; list[i]; ++i ) {

        char *absolute = file_pathabs(ifndef(win32, list[i], va("%s/%s", path, list[i]))); // ../dir/./file.ext -> c:/prj/dir/file.ext
        BROWSER_PRINTF("%s->%s %d->", list[i], absolute, file_directory(absolute) );

        if( file_directory(absolute) ) {
            // remove last '/' if present. ok to overwrite absolute var, file_*() API returns writeable strings.
            char *dir = absolute; if( dir[ strlen(dir) - 1 ] == '/' ) dir[ strlen(dir) - 1 ] = '\0';

            dir = file_name(dir);  // /home/rlyeh/prj/fwk/art -> art
            BROWSER_PRINTF("%s\n", dir);

            if( dir[0] != '.' ) // skip special files, folders and internal files like .git or .art.zip
            array_push(browser->directories, STRDUP(dir));
        } else {
            const char *fname = file_name(absolute);

            BROWSER_PRINTF("%s\n", fname);

            if( fname[0] != '.' ) // skip special files, folders and internal files like .git or .art.zip
            array_push(browser->files, STRDUP(fname));
        }
    }

    browser->file_count = array_count(browser->files);
    browser->dir_count = array_count(browser->directories);
}

static void browser_chdir_and_reload_directory_content(struct browser *browser, const char *path) {
    if( path != browser->directory ) strncpy(browser->directory, path, BROWSER_MAX_PATH);
    browser_reload_directory_content(browser, path);
}

static void browser_init(struct browser *browser) {
    memset(browser, 0, sizeof(*browser));
    {
        /* load files and sub-directory list */
        const char *home = getenv("HOME");
#ifdef _WIN32
        if (!home) home = getenv("USERPROFILE");
#else
        if (!home) home = getpwuid(getuid())->pw_dir;
#endif
        snprintf(browser->home, BROWSER_MAX_PATH, "%s/", home);
        snprintf(browser->desktop, BROWSER_MAX_PATH, "%s/Desktop/", home);
        snprintf(browser->computer, BROWSER_MAX_PATH, "%s", ifdef(win32, va("%.*s", 3, getenv("windir")), "/"));
        {
            ifdef(win32, _getcwd, getcwd)(browser->project, sizeof(browser->project) - 1); // -1 == room for '/'
            strcat(browser->project, "/");
        }

        BROWSER_PRINTF("%s\n", browser->home);
        BROWSER_PRINTF("%s\n", browser->desktop);
        BROWSER_PRINTF("%s\n", browser->computer);
        BROWSER_PRINTF("%s\n", browser->project);

        browser_chdir_and_reload_directory_content(browser, browser->project);
    }
}

static void browser_free(struct browser *browser) {
    for(int i = 0; i < array_count(browser->files); ++i) FREE(browser->files[i]);
    for(int i = 0; i < array_count(browser->directories); ++i) FREE(browser->directories[i]);
    array_free(browser->files);
    array_free(browser->directories);
    array_free(browser->ffiles);
    array_free(browser->fdirectories);

    memset(browser, 0, sizeof(*browser));
}

static int browser_run(struct nk_context *ctx, struct browser *browser, int windowed, struct nk_rect total_space) {
    int clicked = 0;

    static float ratio[] = {0.25f, NK_UNDEFINED};

    float spacing_x = ctx->style.window.spacing.x;

    /* output path directory selector in the menubar */
    ctx->style.window.spacing.x = 0;
    if( windowed ) nk_menubar_begin(ctx);
    {
        char *d = browser->directory;
#ifdef _WIN32
        char *begin = d;
#else
        char *begin = d + 1;
#endif
        nk_layout_row_template_begin(ctx, 25);
        nk_layout_row_template_push_variable(ctx, 40);
        nk_layout_row_template_push_variable(ctx, 40);
        nk_layout_row_template_push_variable(ctx, 40);
        nk_layout_row_template_end(ctx);

        if (nk_button_label(ctx, !browser->listing ? ICON_MD_LIST : ICON_MD_GRID_VIEW)) {
            browser->listing ^= 1;
        }

        while (*d++) {
            if (*d == '/') {
                *d = '\0';
                if (nk_button_label(ctx, va("%s" ICON_MD_ARROW_RIGHT, file_name(begin)))) {
                    *d++ = '/'; *d = '\0';
                    browser_chdir_and_reload_directory_content(browser, browser->directory);
                    break;
                }
                *d = '/';
                begin = d + 1;
            }
        }
    }
    if( windowed ) nk_menubar_end(ctx);
    ctx->style.window.spacing.x = spacing_x;

if(nk_window_has_focus(ctx)) {
browser->zooming = clampf( browser->zooming + (input(KEY_LCTRL) || input(KEY_RCTRL)) * input_diff(MOUSE_W) * 0.1, 1, 3);
}

bool compact = 0, tiny = browser->listing; // compact, no left panel. tiny, no large icons
size_t cols = total_space.w / (100 * browser->zooming);
int icon_height = (67 * browser->zooming) * (tiny ? 0.33 : 1.); // icon height (96) + button padding (??). originally: 135
/**/ if( tiny )      cols = (int)cols+1.5, cols /= 2, compact = total_space.w < 500; // cols <= 2;
else                 cols = (int)cols+1,     compact = total_space.w < 500; // cols <= 5;
if( cols < 1 ) cols=1;

    /* window layout */
    nk_layout_row(ctx, NK_DYNAMIC, total_space.h, compact ? 1 : 2, compact ? ratio+1 : ratio);
    if( !compact )
    if( nk_group_begin(ctx, "Special", NK_WINDOW_NO_SCROLLBAR) ) {
        nk_layout_row_dynamic(ctx, 40, 1);

        if (nk_button_image_label(ctx,media.custom_folders[BROWSER_HOME],"Home",NK_TEXT_RIGHT))
            browser_chdir_and_reload_directory_content(browser, browser->home);

        if (nk_button_image_label(ctx,media.custom_folders[BROWSER_DESKTOP],"Desktop",NK_TEXT_RIGHT))
            browser_chdir_and_reload_directory_content(browser, browser->desktop);

        if (nk_button_image_label(ctx,media.custom_folders[BROWSER_COMPUTER],"Computer",NK_TEXT_RIGHT))
            browser_chdir_and_reload_directory_content(browser, browser->computer);

        if (nk_button_image_label(ctx,media.custom_folders[BROWSER_PROJECT],"Project",NK_TEXT_RIGHT))
            browser_chdir_and_reload_directory_content(browser, browser->project);

        nk_group_end(ctx);
    }

    /* output directory content window */
    if(nk_group_begin(ctx, "Content", windowed ? NK_WINDOW_NO_SCROLLBAR : 0)) {

        array(char*) *directories = &browser->directories;
        array(char*) *files = &browser->files;

        if( ui_filter && ui_filter[0] ) {
            array_resize(browser->fdirectories, 0);
            array_resize(browser->ffiles, 0);

            for each_array(browser->directories,char*,k)
                if( strstri(k, ui_filter) )
                    array_push(browser->fdirectories, k);

            for each_array(browser->files,char*,k)
                if( strstri(k, ui_filter) )
                    array_push(browser->ffiles, k);

            directories = &browser->fdirectories;
            files = &browser->ffiles;
        }

        int dir_count = array_count(*directories);
        int file_count = array_count(*files);

        int index = -1;
        size_t i = 0, j = 0, k = 0;
        size_t rows = 0;
        size_t count = dir_count + file_count;

        rows = count / cols;
        for (i = 0; i <= rows; i += 1) {
            if(!tiny)
            {size_t n = j + cols;
            nk_layout_row_dynamic(ctx, icon_height, (int)cols);
            for (; j < count && j < n; ++j) {
                size_t t = j-dir_count;

                /* draw one row of icons */
                if (j < dir_count) {
                    /* draw and execute directory buttons */
                    if (nk_button_image(ctx,media.custom_folders[BROWSER_FOLDER]))
                        index = (int)j;
                } else {
                    /* draw and execute files buttons */
                    struct nk_image *icon;
                    size_t fileIndex = ((size_t)j - dir_count);
                    icon = media_icon_for_file((*files)[fileIndex]);
                    if (nk_button_image(ctx, *icon)) {
                        snprintf(browser->file, BROWSER_MAX_PATH, "%s%s", browser->directory, browser->files[fileIndex]);
                        clicked = 1;
                    }
                }
            }}
            if(!tiny)
            {size_t n = k + cols;
            nk_layout_row_dynamic(ctx, 20, (int)cols);
            for (; k < count && k < n; k++) {
                size_t t = k-dir_count;

                /* draw one row of labels */
                if (k < dir_count) {
                    nk_label(ctx, (*directories)[k], NK_TEXT_CENTERED);
                } else {
                    nk_label(ctx, (*files)[t], NK_TEXT_CENTERED);
                }
            }}
            if(tiny)
            {size_t n = j + cols;
            nk_layout_row_dynamic(ctx, icon_height, (int)cols);
            for (; j < count && j < n; ++j) {
                size_t t = j-dir_count;

                /* draw one row of icons */
                if (j < dir_count) {
                    /* draw and execute directory buttons */
                    if (nk_button_image_label(ctx,media.custom_folders[BROWSER_FOLDER], (*directories)[j], NK_TEXT_RIGHT))
                        index = (int)j;
                } else {
                    /* draw and execute files buttons */
                    struct nk_image *icon;
                    size_t fileIndex = ((size_t)j - dir_count);
                    icon = media_icon_for_file((*files)[fileIndex]);
                    if (nk_button_image_label(ctx, *icon, (*files)[t],NK_TEXT_RIGHT)) {
                        snprintf(browser->file, BROWSER_MAX_PATH, "%s%s", browser->directory, browser->files[fileIndex]);
                        clicked = 1;
                    }
                }
#if 0
                bool has_focus = nk_window_has_focus(ctx); // @fixme: move out of loop
                bool has_popups = ui_popups(); // @fixme: move out of loop
                if( !has_popups && has_focus ) {
                    struct nk_rect bounds = nk_widget_bounds(ctx);
                    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds) ) {

                        char *name = j < dir_count ? (*directories)[j] : (*files)[j-dir_count];

                        char fullpath[PATH_MAX];
                        snprintf(fullpath, PATH_MAX, "%s%s", browser->directory, name);

                        struct stat t = {0};
                        if( stat( fullpath, &t ) != -1 ) {
                            char tooltip[256];
                            snprintf(tooltip, 256,
                                "Path: %s\n"
                                "Type: %lld\n"  // file type and mode
                                "Size: %lld\n"  // file size
                                "Owner: %lld\n" // user ID of file owner
                                "Modified: %s (%lld)", // last modification date
                                name, (int64_t)t.st_mode, (int64_t)t.st_size, (int64_t)t.st_uid, ctime(&t.st_mtime), (int64_t)t.st_mtime
                            );
                            nk_tooltip(ctx, tooltip);
                        }
                    }
                }
#endif
            }}
        }

        if (index != -1) {
            BROWSER_PRINTF("%s + %s = ", browser->directory, browser->directories[index]);
            size_t n = strlen(browser->directory);
            snprintf(browser->directory + n, BROWSER_MAX_PATH - n, "%s/", browser->directories[index]);
            BROWSER_PRINTF("%s\n", browser->directory);
            browser_chdir_and_reload_directory_content(browser, browser->directory);
        }
        nk_group_end(ctx);
    }

    return clicked;
}

static struct nk_image icon_load(const char *filename) {
    texture_t t = texture(filename, 0);
    return nk_image_id((int)t.id);
}

static struct nk_image icon_load_rect(unsigned id, unsigned w, unsigned h, unsigned wcell, unsigned hcell, unsigned col, unsigned row) {
    return nk_subimage_id((int)id, w, h, (struct nk_rect){ wcell * col, hcell * row, wcell, hcell });
}


/*  demo:

    struct browser browser = {0};
    browser_init(&browser);
    browser_config_dir(nk_image, BROWSER_HOME);
    browser_config_dir(nk_image, BROWSER_PROJECT);
    // [...]
    browser_config_type(nk_image, ".ext1.ext2.ext3.");
    browser_config_type(nk_image, ".ext1.ext2.ext3.");
    browser_config_type(nk_image, ".ext1.ext2.ext3.");
    // [...]

    [...]

    if( nk_begin(ctx, "window", ...) ) {
        struct nk_rect total_space = nk_window_get_content_region(ctx);
        if( browser_run(ctx, &browser, 0, total_space) ) {
            puts( browser->directory );
            puts( browser->file );
        }
    }
    nk_end();
*/
