// data pipeline
// - rlyeh, public domain.
// ----------------------------------------------------------------------------
// @todo: threads should steal workloads from job queue
// @todo: restore errno/errorlevel checks
// @todo: +=, -=, that_asset.ini
// @todo: @dae FLAGS+=-U
// @todo: SF2_SOUNDBANK=TOOLS/soundbank.sf2
// @fixme: leaks (worth?)
// -----------------------------------------------------------------------------

#ifndef COOK_INI_PATHFILE
#define COOK_INI_PATHFILE "tools/cook.ini"
#endif

const char *ART = "art/";
const char *TOOLS = "tools/bin/";
const char *EDITOR = "tools/";
const char *COOK_INI = COOK_INI_PATHFILE;

static unsigned ART_SKIP_ROOT; // number of chars to skip the base root in ART folder
static unsigned ART_LEN;       // dupe

typedef struct cook_subscript_t {
    char *infile;
    char *outfile;    // can be either infile, or a totally different file
    char *script;
    char *outname;
    int compress_level;
    uint64_t pass_ns, gen_ns, exe_ns, zip_ns;
} cook_subscript_t;

typedef struct cook_script_t {
    cook_subscript_t cs[8];
    int num_passes;
    uint64_t pass_ns, gen_ns, exe_ns, zip_ns;
} cook_script_t;

static
cook_script_t cook_script(const char *rules, const char *infile, const char *outfile) {
    cook_script_t mcs = { 0 };

    // pass loop: some asset rules may require multiple cook passes
    for( int pass = 0; pass < countof(mcs.cs); ++pass ) {
        // by default, assume:
        // - no script is going to be generated (empty script)
        // - if no script is going to be generated, output is in fact input file.
        // - no compression is going to be required.
        cook_subscript_t cs = { 0 };
        cs.gen_ns -= time_ns();

            // reuse script heap from last call if possible (optimization)
            static __thread char *script = 0;
            if(script) script[0] = 0;

            // reuse parsing maps if possible (optimization)
            static __thread map(char*, char*) symbols = 0;   if(!symbols) map_init_str(symbols);
            static __thread map(char*, char*) groups = 0;    if(!groups)  map_init_str(groups);
            static __thread set(char*) passes = 0;           if(!passes)  set_init_str(passes);
            map_clear(symbols);
            map_clear(groups);


            map_find_or_add(symbols, "INFILE", STRDUP(infile));
            map_find_or_add(symbols, "INPUT", STRDUP(infile));
            map_find_or_add(symbols, "PRETTY", STRDUP(infile + ART_SKIP_ROOT)); // pretty (truncated) input (C:/prj/FWK/art/file.wav -> file.wav)
            map_find_or_add(symbols, "OUTPUT", STRDUP(outfile));
            map_find_or_add(symbols, "TOOLS", STRDUP(TOOLS));
            map_find_or_add(symbols, "EDITOR", STRDUP(EDITOR));
            map_find_or_add(symbols, "PROGRESS", STRDUP(va("%03d", cook_progress())));

        // clear pass counter
        set_clear(passes);

        // start parsing. parsing is enabled by default
        int enabled = 1;
        array(char*)lines = strsplit(rules, "\r\n");
        for( int i = 0, end = array_count(lines); i < end; ++i ) {
            // skip blanks
            int blanks = strspn(lines[i], " \t");
            char *line = lines[i] + blanks;

            // discard full comments
            if( line[0] == ';' ) continue;
            // truncate inline comments
            if( strstr(line, ";") ) *strstr(line, ";") = 0;
            // trim ending spaces
            char *eos = line + strlen(line); while(eos > line && eos[-1] == ' ' ) *--eos = 0;
            // discard non-specific lines
            if( line[0] == '@' ) {
                int with_wine = flag("--cook-wine") && !!strstr(line, "@win");
                int parse = 0
                    | ifdef(win32, (!!strstr(line, "@win")), 0)
                    | ifdef(linux, (!!strstr(line, "@lin") ? 1 : with_wine), 0)
                    | ifdef(osx,   (!!strstr(line, "@osx") ? 1 : with_wine), 0);

                if( !parse ) continue;

                line = strchr(line+1, ' ');
                if(!line) continue;
                line += strspn(line, " \t");
            }
            // execute `shell` commands
            if( line[0] == '`' ) {
                char *eos = strrchr(++line, '`');
                if( eos ) *eos = 0;

                    // replace all symbols
                    char* nl = STRDUP(line); // @leak
                    for each_map(symbols, char*, key, char*, val) {
                        strrepl(&nl, key, val);
                    }
                    lines[i] = line = nl;

#if 0
                static thread_mutex_t lock, *init = 0; if(!init) thread_mutex_init(init = &lock);
                thread_mutex_lock( &lock );
                    system(line); // strcatf(&script, "%s\n", line);
                thread_mutex_unlock( &lock );
#else
                // append line
                strcatf(&script, "%s\n", line);
#endif

                continue;
            }
            // process [sections]
            if( line[0] == '[' ) {
                enabled = 1;
                int is_cook = !!strstr(line, "[cook]");
                int is_compress = !!strstr(line, "[compress]");
                if( !is_cook && !is_compress ) { // if not a special section...
                    // remove hint cook tag if present. that's informative only.
                    if(strbegi(line, "[cook ") ) memcpy(line+1, "    ", 4); // line += 6;

                    // start parsing expressions like `[media && !avi && mp3]`
                    array(char*) tags = strsplit(line, " []&");

                    // let's check whether INPUT belongs to tags above
                    char **INPUT = map_find(symbols, "INPUT");
                    bool found_in_set = true;

                    for( int i = 0, end = array_count(tags); i < end; ++i) {
                        bool negate = false;
                        char *tag = tags[i];
                        while(*tag == '!') negate ^= 1, ++tag;

                        // find tag in groups map
                        // either a group or an extension
                        char **is_group = map_find(groups, tag);
                        if( is_group ) {
                            char *list = *is_group;
                            char *INPUT_EXT = file_ext(infile); INPUT_EXT = strrchr(INPUT_EXT, '.'); // .ext1.ext -> .ext
                            char *ext = INPUT_EXT; ext += ext[0] == '.'; // dotless
                            bool in_list = strbegi(list, ext) || strendi(list, va(",%s",ext)) || strstri(list, va(",%s,",ext));
                            if( !in_list ^ negate ) { found_in_set = false; break; }
                        } else {
                            char *ext = va(".%s", tag);
                            bool found = !!strendi(*INPUT, ext);
                            if( !found ^ negate ) { found_in_set = false; break; }
                        }
                    }
                    if( found_in_set ) {
                        // inc pass
                        set_find_or_add(passes, STRDUP(*tags)); // @leak
                        // check whether we keep searching
                        int num_passes = set_count(passes);
                        found_in_set = ( pass == (num_passes-1) );
                    }
                    //
                    enabled = found_in_set ? 1 : 0;
                }
            }
            // either SYMBOL=, group=, or regular script line
            if( enabled && line[0] != '[' ) {
                enum { group, symbol, regular } type = regular;
                int tokenlen = strspn(line, "-+_.|0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
                char *token = va("%.*s", tokenlen, line);
                char *equal = strchr(line, '=');
                if( equal ) {
                    if( equal == &line[tokenlen] ) { // if key=value expression found
                        // discriminate: symbols are uppercase and never begin with digits. groups are [0-9]+[|][a-z].
                        type = strcmp(strupper(token), token) || isdigit(token[0]) ? group : symbol;
                    }
                }
                if( type == group  ) map_find_or_add(groups, token, STRDUP(equal+1));
                if( type == symbol ) {
                    // @todo: perform the replacement/union/intersection on set here
                    bool is_add = strendi(token, "+");
                    bool is_del = strendi(token, "-");

                    // if present, remove last sign from token -> (FLAGS1+)=, (FLAGS1-)=
                    if(is_add || is_del) token[strlen(token) - 1] = 0;

                    map_find_or_add(symbols, token, STRDUP(equal+1));
                }
                // for each_map(symbols, char*, key, char*, val) printf("%s=%s,", key, val); puts("");
                // for each_map(groups, char*, key, char*, val) printf("%s=%s,", key, val); puts("");
                // if( type != regular ) printf("%s found >> %s\n", type == group ? "group" : "symbol", line);

                if( type == regular ) {
                    char** INPUT = map_find(symbols, "INPUT");
                    char** OUTPUT = map_find(symbols, "OUTPUT");

                    // parse return code
                    char *has_errorlevel = strstr(line, "=="); //==N form
                    int errorlevel = has_errorlevel ? atoi(has_errorlevel + 2) : 0;
                    if( has_errorlevel ) memcpy(has_errorlevel, "   ", 3);

                    // detect if newer extension or filename is present, and thus update OUTPUT if needed
                    char *newer_extension = strstr(line, "->"); if(newer_extension) {
                        *newer_extension = 0;
                        newer_extension += 2 + strspn(newer_extension + 2, " ");

                        if( strchr(newer_extension, '.') ) {
                            // newer filename
                            cs.outname = stringf("%s@%s", cs.outname ? cs.outname : infile, newer_extension); // @leak // special char (multi-pass cooks)
                            newer_extension = NULL;
                        } else {
                            strcatf(&*OUTPUT, ".%s", newer_extension);
                        }
                    }

                    // replace all symbols
                    char* nl = STRDUP(line); // @leak
                    for each_map(symbols, char*, key, char*, val) {
                        strrepl(&nl, key, val);
                    }
                    lines[i] = line = nl;

                    // convert slashes
                    ifdef(win32,
                        strswap(line, "/", "\\")
                    , // else
                        strswap(line, "\\", "/")
                    );

                    // append line
                    strcatf(&script, "%s\n", line);

                    // handle return code here
                    // if(has_errorlevel)
                    // strcatf(&script, "IF NOT '%%ERRORLEVEL%%'=='%d' echo ERROR!\n", errorlevel);

                    // rename output->input for further chaining, in case it is needed
                    if( newer_extension ) {
                        *INPUT[0] = 0;
                        strcatf(&*INPUT, "%s", *OUTPUT);
                    }
                }
            }
        }

        char** OUTPUT = map_find(symbols, "OUTPUT");
        int ext_num_groups = 0;

        // compression
        if( 1 ) {
            char* ext = file_ext(infile); ext = strrchr(ext, '.'); ext += ext[0] == '.'; // dotless INPUT_EXT
            char* belongs_to = 0;
            for each_map(groups, char*, key, char*, val) {
                if( !isdigit(key[0]) ) {
                    char *comma = va(",%s,", ext);
                    if( !strcmpi(val,ext) || strbegi(val, comma+1) || strstri(val, comma) || strendi(val, va(",%s", ext))) {
                        belongs_to = key;
                        ext_num_groups++;
                    }
                }
            }
            char *compression = 0;
            for each_map_ptr_sorted(groups, char*, key, char*, val) { // sorted iteration, so hopefully '0' no compression gets evaluated first
                if( !compression && isdigit((*key)[0]) ) {
                    char *comma = va(",%s,", ext);
                    if( !strcmpi(*val,ext) || strbegi(*val, comma+1) || strstri(*val, comma) || strendi(*val, va(",%s", ext))) {
                        compression = (*key);
                    }
                    comma = va(",%s,", belongs_to);
                    if( !strcmpi(*val,ext) || strbegi(*val, comma+1) || strstri(*val, comma) || strendi(*val, va(",%s", ext))) {
                        compression = (*key);
                    }
                }
            }

            cs.compress_level = 0;
            if( compression ) {
                // last chance to optionally override the compressor at command-line level
                static const char *compressor_override;
                do_once compressor_override = option("--cook-compressor", "");
                if( compressor_override[0] ) compression = (char*)compressor_override;

                /**/ if(strstri(compression, "PPP"))  cs.compress_level = atoi(compression) | PPP;
                else if(strstri(compression, "ULZ"))  cs.compress_level = atoi(compression) | ULZ;
                else if(strstri(compression, "LZ4"))  cs.compress_level = atoi(compression) | LZ4X;
                else if(strstri(compression, "CRSH")) cs.compress_level = atoi(compression) | CRSH;
                else if(strstri(compression, "DEFL")) cs.compress_level = isdigit(compression[0]) ? atoi(compression) : 6 /*| DEFL*/;
                //else if(strstri(compression, "LZP"))  cs.compress_level = atoi(compression) | LZP1; // not supported
                else if(strstri(compression, "LZMA")) cs.compress_level = atoi(compression) | LZMA;
                else if(strstri(compression, "BALZ")) cs.compress_level = atoi(compression) | BALZ;
                else if(strstri(compression, "LZW"))  cs.compress_level = atoi(compression) | LZW3;
                else if(strstri(compression, "LZSS")) cs.compress_level = atoi(compression) | LZSS;
                else if(strstri(compression, "BCM"))  cs.compress_level = atoi(compression) | BCM;
                else                                  cs.compress_level = isdigit(compression[0]) ? atoi(compression) : 6 /*| DEFL*/;
            }
        }

        // if script was generated...
        if( script && script[0] && strstr(script, ifdef(win32, file_normalize(va("%s",infile)), infile )) ) {
            // update outfile
            cs.outfile = *OUTPUT;

            // amalgamate script
            array(char*) lines = strsplit(script, "\r\n");

            #if is(win32)
                char *joint = strjoin(lines, " && ");
                cs.script = joint;
            #else
                if( flag("--cook-wine") ) {
                    // dear linux/osx/bsd users:
                    // tools going wrong for any reason? cant compile them maybe?
                    // small hack to use win32 pipeline tools instead
                    char *joint = strjoin(lines, " && wine " );
                    cs.script = va("wine %s", /*TOOLS,*/ joint);
                } else {
                    char *joint = strjoin(lines, " && " );
                    cs.script = va("export LD_LIBRARY_PATH=%s && %s", TOOLS, joint);
                }
            #endif
        } else {
            // if( script && script[0] ) system(script); //< @todo: un-comment this line if we want to get the shell command prints invoked per entry

            // ... else bypass infile->outfile
            char** INFILE = map_find(symbols, "INFILE");
            cs.outfile = *INFILE;

            // and return an empty script
            cs.script = "";
        }

        cs.outname = cs.outname ? cs.outname : (char*)infile;
        cs.gen_ns += time_ns();

        ASSERT(mcs.num_passes < countof(mcs.cs));
        mcs.cs[mcs.num_passes++] = cs;

        bool next_pass_required = mcs.num_passes < ext_num_groups;
        if( !next_pass_required ) break;
    }

    return mcs;
}

// ----------------------------------------------------------------------------

struct fs {
    char *fname, status;
    uint64_t stamp;
    uint64_t bytes;
};

static array(struct fs) fs_now;
static __thread array(char*) added;
static __thread array(char*) changed;
static __thread array(char*) deleted;
static __thread array(char*) uncooked;

static
array(struct fs) zipscan_filter(int threadid, int numthreads) {
    // iterate all previously scanned files
    array(struct fs) fs = 0;
    for( int i = 0, end = array_count(fs_now); i < end; ++i ) {
        // during workload distribution, we assign random files to specific thread buckets.
        // we achieve this by hashing the basename of the file. we used to hash also the path
        // long time ago but that is less resilient to file relocations across the repository.
        // excluding the file extension from the hash also helps from external file conversions.
        char *fname = file_name(fs_now[i].fname);
        char *sign = strrchr(fname, '@'); if(sign) *sign = '\0'; // special char (multi-pass cooks)
        char *dot = strrchr(fname, '.'); if(dot) *dot = '\0';

        // skip if list item does not belong to this thread bucket
        uint64_t hash = hash_str(fname);
        unsigned bucket = (hash /*>> 32*/) % numthreads;
        if(bucket != threadid) continue;

        array_push(fs, fs_now[i]);
    }
    return fs;
}

static
int zipscan_diff( zip* old, array(struct fs) now ) {
    array_free(added);
    array_free(changed);
    array_free(deleted);
    array_free(uncooked);

    // if not zipfile is present, all files are new and must be added
    if( !old ) {
        for( int i = 0; i < array_count(now); ++i ) {
            array_push(uncooked, STRDUP(now[i].fname));
        }
        return 1;
    }

    // compare for new & changed files
    for( int i = 0; i < array_count(now); ++i ) {
        int found = zip_find(old, now[i].fname);
        if( found < 0 ) {
            array_push(added, STRDUP(now[i].fname));
            array_push(uncooked, STRDUP(now[i].fname));
        } else {
            uint64_t oldsize = atoi64(zip_comment(old,found)); // zip_size(old, found); returns sizeof processed asset. return original size of unprocessed asset, which we store in comment section
            uint64_t oldstamp = atoi64(zip_modt(old,found)+20); // format is "YYYY/MM/DD hh:mm:ss", then +20 chars later a hidden epoch timestamp in base10 can be found
            int64_t diffstamp = oldstamp < now[i].stamp ? now[i].stamp - oldstamp : oldstamp - now[i].stamp;
            if( oldsize != now[i].bytes || diffstamp > 1 ) { // @fixme: should use hash instead. hashof(tool) ^ hashof(args used) ^ hashof(rawsize) ^ hashof(rawdate)
                printf("%s:\t%u vs %u, %llu vs %llu\n", now[i].fname, (unsigned)oldsize,(unsigned)now[i].bytes, (long long unsigned)oldstamp, (long long unsigned)now[i].stamp);
                array_push(changed, STRDUP(now[i].fname));
                array_push(uncooked, STRDUP(now[i].fname));
            }
        }
    }
    // compare for deleted files
    for( int i = 0; i < zip_count(old); ++i ) {
        char *oldname = zip_name(old, i);
        //if( strchr(oldname, '@') ) oldname = va("%*.s", (int)(strchr(oldname, '@') - oldname), oldname ); // special char (multi-pass cooks)

        int idx = zip_find(old, oldname); // find latest versioned file in zip
        unsigned oldsize = zip_size(old, idx);
        if (!oldsize) continue;

        struct fs *found = 0; // zipscan_locate(now, oldname);
        for(int j = 0; j < array_count(now); ++j) {
            if( !strcmp(now[j].fname,oldname)) {
                found = &now[j];
                break;
            }
        }

        if( !found ) {
            array_push(deleted, STRDUP(oldname));
        }
    }
    return 1;
}

// ----------------------------------------------------------------------------

typedef struct cook_worker {
    const char **files;
    const char *rules;
    int threadid, numthreads;
    thread_ptr_t self;
    volatile int progress;
    thread_mutex_t *lock;
} cook_worker;

enum { JOBS_MAX = 256 };
static cook_worker jobs[JOBS_MAX] = {0};
static volatile bool cook_cancelable = false, cook_cancelling = false, cook_debug = false;

#ifndef COOK_ON_DEMAND
#define COOK_ON_DEMAND ifdef(cook, optioni("--cook-on-demand", 1), false)
#endif

static
int cook(void *userdata) {
    cook_worker *job = (cook_worker*)userdata;

    // start progress
    volatile int *progress = &job->progress;
    *progress = 0;

    // preload a few large binaries
//    dll("tools/furnace.exe", 0);
//    dll("tools/assimp-vc143-mt.dll", 0);
//    dll("tools/ffmpeg.exe", 0);

    // scan disk from fs_now snapshot
    array(struct fs) filtered = zipscan_filter(job->threadid, job->numthreads);
    //printf("Scanned: %d items found\n", array_count(now));

    // prepare out tempname
    char COOK_TMPFILE[64]; snprintf(COOK_TMPFILE, 64, "temp_%02d", job->threadid);

    // prepare zip
    char zipfile[64]; snprintf(zipfile, 64, ".art[%02x].zip", job->threadid);
    if( file_size(zipfile) == 0 ) unlink(zipfile);

    // populate added/deleted/changed arrays by examining current disk vs last cache
    zip *z;
    {
        z = zip_open(zipfile, "r+b");
        zipscan_diff(z, filtered);
        if( z ) zip_close(z);

        fflush(0);

        z = zip_open(zipfile, "a+b");
        if( !z ) {
            unlink(zipfile);
            z = zip_open(zipfile, "a+b"); // try again
            if(!z) PANIC("cannot open file for updating: %s", zipfile);
        }
    }

    // deleted files. --cook-additive runs are append-only, so they skip this block
    if( !flag("--cook-additive") )
    for( int i = 0, end = array_count(deleted); i < end; ++i ) {
        printf("Deleting %03d%% %s\n", (i+1) == end ? 100 : (i * 100) / end, deleted[i]);
        FILE* out = fopen(COOK_TMPFILE, "wb"); fclose(out);
        FILE* in = fopen(COOK_TMPFILE, "rb");
        char *comment = "0";
        zip_append_file/*_timeinfo*/(z, deleted[i], comment, in, 0/*, tm_now*/);
        fclose(in);
    }

//  if(array_count(uncooked))
//  PRINTF("cook_jobs[%d]=%d\n", job->threadid, array_count(uncooked));

    // generate cook metrics. you usually do `game.exe --cook-stats && (type *.csv | sort /R > cook.csv)`
    static __thread FILE *statsfile = 0;
    if(flag("--cook-stats"))
    fseek(statsfile = fopen(va("cook%d.csv",job->threadid), "a+t"), 0L, SEEK_END);
    if(statsfile && !job->threadid && ftell(statsfile) == 0) fprintf(statsfile,"%10s,%10s,%10s,%10s,%10s, %s\n","+total_ms","gen_ms","exe_ms","zip_ms","pass","file");

    // added or changed files
    for( int i = 0, end = array_count(uncooked); i < end && !cook_cancelling; ++i ) {
        *progress = ((i+1) == end ? 90 : (i * 90) / end); // (i+i>0) * 100.f / end;

        // start cook
        const char *infile = uncooked[i]; //job->files[j];
        int inlen = file_size(infile);

        // generate a cooking script for this asset
        cook_script_t mcs = cook_script(job->rules, infile, COOK_TMPFILE);
        // puts(cs.script);

        for(int pass = 0; pass < mcs.num_passes; ++pass) {
            cook_subscript_t cs = mcs.cs[pass];

            // log to batch file for forensic purposes, if explicitly requested
            static __thread int logging = -1; if(logging < 0) logging = !!flag("--cook-debug") || cook_debug;
            if( logging ) {
                static __thread FILE *logfile = 0; if(!logfile) fseek(logfile = fopen(va("cook%d.cmd",job->threadid), "a+t"), 0L, SEEK_END);
                if( logfile ) {
                    fprintf(logfile, "@rem %s\n%s\n", cs.outname, cs.script);
                    fprintf(logfile, "for %%%%i in (\"%s\") do md _cook\\%%%%~pi\\%%%%~ni%%%%~xi 1>nul 2>nul\n", infile);
                    fprintf(logfile, "for %%%%i in (\"%s\") do xcopy /y %s _cook\\%%%%~pi\\%%%%~ni%%%%~xi\n\n", infile, file_normalize(cs.outfile));
                }
            }

            // invoke cooking script
            mcs.cs[pass].exe_ns -= time_ns();
                // invoke cooking script
                const char *rc_output = app_exec(cs.script);
                // recap status
                int rc = atoi(rc_output);
                // int outlen = file_size(cs.outfile);
                int failed = rc; // cs.script[0] ? rc || !outlen : 0;
                // print errors
                if( failed ) {
                    PRINTF("Import failed: %s while executing:\n%s\nReturned:\n%s\n", cs.outname, cs.script, rc_output);
                    continue;
                }
                if( pass > 0 ) { // (multi-pass cook)
                    // newly generated file: refresh values
                    // ensure newly created files by cook are also present on repo/disc for further cook passes
                    file_delete(cs.outname);
                    file_move(cs.outfile, cs.outname);
                    inlen = file_size(infile = cs.outfile = cs.outname);
                }
            mcs.cs[pass].exe_ns += time_ns();

            // process only if included. may include optional compression.
            mcs.cs[pass].zip_ns -= time_ns();
            if( cs.compress_level >= 0 ) {
                FILE *in = fopen(cs.outfile ? cs.outfile : infile, "rb");
                if(!in) in = fopen(infile, "rb");

                    char *comment = va("%d", inlen);
                    if( !zip_append_file(z, infile, comment, in, cs.compress_level) ) {
                        PANIC("failed to add processed file into %s: %s(%s)", zipfile, cs.outname, infile);
                    }

                fclose(in);
            }
            mcs.cs[pass].zip_ns += time_ns();

            // stats per subscript
            mcs.cs[pass].pass_ns = mcs.cs[pass].gen_ns + mcs.cs[pass].exe_ns + mcs.cs[pass].zip_ns;
            if(statsfile) fprintf(statsfile, "%10.f,%10.f,%10.f,%10.f,%10d, \"%s\"\n", mcs.cs[pass].pass_ns/1e6, mcs.cs[pass].gen_ns/1e6, mcs.cs[pass].exe_ns/1e6, mcs.cs[pass].zip_ns/1e6, pass+1, infile);
        }
    }

    zip_close(z);

    // end progress
    if( file_size(zipfile) == 0 ) unlink(zipfile);
    *progress = 100;

    return 1;
}

static
int cook_async( void *userdata ) {
#if COOK_FROM_TERMINAL
    // nothing to do...
#else
    while(!window_handle()) sleep_ms(100); // wait for window handle to be created
#endif

    // boost cook thread #0, which happens to be the only spawn thread when num_jobs=1 (tcc case, cook-sync case).
    // also in multi-threaded scenarios, it is not bad at all to have one high priority thread...
    // in any case, game view is not going to look bad because the game will be displaying a progress bar at that time.
    cook_worker *job = (cook_worker*)userdata;
    if( job->threadid == 0 ) thread_set_high_priority();

    // tcc: only a single running thread shall pass, because of racing shared state due to missing thread_local support at compiler level
    ifdef(tcc, thread_mutex_lock( job->lock ));
    ifdef(osx, thread_mutex_lock( job->lock ));   // @todo: remove silicon mac M1 hack

    int ret = cook(userdata);

    // tcc: only a single running thread shall pass, because of racing shared state due to missing thread_local support at compiler level
    ifdef(osx, thread_mutex_unlock( job->lock )); // @todo: remove silicon mac M1 hack
    ifdef(tcc, thread_mutex_unlock( job->lock ));

    thread_exit( ret );
    return ret;
}

bool cook_start( const char *cook_ini, const char *masks, int flags ) {
    cook_ini = cook_ini ? cook_ini : COOK_INI;

    char *rules_ = file_read(cook_ini);
    if(!rules_ || rules_[0] == 0) return false;

    static char *rules; do_once rules = STRDUP(rules_);

    do_once {
    #if 0
        const char *HOME = file_pathabs(cook_ini); // ../tools/cook.ini -> c:/prj/fwk/tools/cook.ini
        if( strbeg(HOME, app_path() ) ) HOME = STRDUP( file_path( HOME += strlen(app_path()) ) ); // -> tools/ @leak
    #else
        char *HOME = STRDUP(file_pathabs(cook_ini)); // ../tools/cook.ini -> c:/prj/fwk/tools/cook.ini
        HOME[ strlen(HOME) - strlen(file_name(cook_ini)) ] = '\0'; // -> tools/ @leak
    #endif

        ART_LEN = 0; //strlen(app_path());
        /* = MAX_PATH;
        for each_substring(ART, ",", art_folder) {
            ART_LEN = mini(ART_LEN, strlen(art_folder));
        }*/

        if( strstr(rules, "ART=") ) {
            ART = va( "%s", strstr(rules, "ART=") + 4 );
            char *r = strchr( ART, '\r' ); if(r) *r = 0;
            char *n = strchr( ART, '\n' ); if(n) *n = 0;
            char *s = strchr( ART, ';' );  if(s) *s = 0;
            char *w = strchr( ART, ' ' );  if(w) *w = 0;
            char *out = 0; const char *sep = "";
            for each_substring(ART, ",", t) {
                char *tmp = file_pathabs(va("%s%s", HOME, t)) + ART_LEN;
                for(int i = 0; tmp[i]; ++i) if(tmp[i]=='\\') tmp[i] = '/';
                strcatf(&out, "%s%s%s", sep, tmp, strendi(tmp, "/") ? "" : "/");
                assert( out[strlen(out) - 1] == '/' );

                sep = ",";
            }
            ART = out; // @leak
        }

        if( strstr(rules, "TOOLS=") ) {
            TOOLS = va( "%s", strstr(rules, "TOOLS=") + 6 );
            char *r = strchr( TOOLS, '\r' ); if(r) *r = 0;
            char *n = strchr( TOOLS, '\n' ); if(n) *n = 0;
            char *s = strchr( TOOLS, ';' );  if(s) *s = 0;
            char *w = strchr( TOOLS, ' ' );  if(w) *w = 0;
            char *cat = va("%s%s", HOME, TOOLS), *out = 0;
            for(int i = 0; cat[i]; ++i) if(cat[i]=='\\') cat[i] = '/';
            strcatf(&out, "%s%s", cat, strend(cat, "/") ? "" : "/");
            TOOLS = out; // @leak
            assert( TOOLS[strlen(TOOLS) - 1] == '/' );

            // last chance to autodetect tools folder (from cook.ini path)
            if( !file_directory(TOOLS) ) {
                out = STRDUP(cook_ini);
                for(int i = 0; out[i]; ++i) if(out[i]=='\\') out[i] = '/';
                TOOLS = out; // @leak
            }
        }

        if( strstr(rules, "EDITOR=") ) {
            EDITOR = va( "%s", strstr(rules, "EDITOR=") + 7 );
            char *r = strchr( EDITOR, '\r' ); if(r) *r = 0;
            char *n = strchr( EDITOR, '\n' ); if(n) *n = 0;
            char *s = strchr( EDITOR, ';' );  if(s) *s = 0;
            char *w = strchr( EDITOR, ' ' );  if(w) *w = 0;
            char *cat = va("%s%s", HOME, EDITOR), *out = 0;
            for(int i = 0; cat[i]; ++i) if(cat[i]=='\\') cat[i] = '/';
            strcatf(&out, "%s%s", cat, strend(cat, "/") ? "" : "/");
            EDITOR = out; // @leak
            assert( EDITOR[strlen(EDITOR) - 1] == '/' );
        }

        // small optimization for upcoming parser: remove whole comments from file
        array(char*) lines = strsplit(rules, "\r\n");
        for( int i = 0; i < array_count(lines); ) {
            if( lines[i][0] == ';' ) array_erase_slow(lines, i);
            else ++i;
        }
        rules = STRDUP( strjoin(lines, "\n") );
    }

    if( !masks ) {
        return true; // nothing to do
    }

    // estimate ART_SKIP_ROOT (C:/prj/fwk/demos/assets/file.png -> strlen(C:/prj/fwk/) -> 11)
    {
        array(char*) dirs = 0;
        for each_substring(ART, ",", art_folder) {
            array_push(dirs, file_pathabs(art_folder));
        }
        if( array_count(dirs) > 1 )  {
            for( int ok = 1, ch = dirs[0][ART_SKIP_ROOT]; ch && ok; ch = dirs[0][++ART_SKIP_ROOT] ) {
                for( int i = 1; i < array_count(dirs) && ok; ++i ) {
                    ok = dirs[i][ART_SKIP_ROOT] == ch;
                }
            }
        }
        while( ART_SKIP_ROOT > 0 && !strchr("\\/", dirs[0][ART_SKIP_ROOT-1]) ) --ART_SKIP_ROOT;
        array_free(dirs);
    }

    if( COOK_ON_DEMAND ) {
        return true; // cooking is deferred
    }

    // scan disk: all subfolders in ART (comma-separated)
    static array(char *) list = 0; // @leak
    for each_substring(ART, ",", art_folder) {
        array(char *) glob = file_list(va("%s**",art_folder)); // art_folder ends with '/'
        for( unsigned i = 0, end = array_count(glob); i < end; ++i ) {
            const char *fname = glob[i];
            if( !strmatchi(fname, masks)) continue;

            // skip special files, folders and internal files like .art.zip
            const char *dir = file_path(fname);
            if( dir[0] == '.' ) continue; // discard system dirs and hidden files
            if( strbegi(dir, TOOLS) ) continue; // discard tools folder
            if( !file_ext(fname)[0] ) continue; // discard extensionless entries
            if( !file_size(fname)) continue; // skip dirs and empty files

            // exclude vc c/c++ .obj files. they're not 3d wavefront .obj files
            if( strend(fname, ".obj") ) {
                char header[4] = {0};
                for( FILE *in = fopen(fname, "rb"); in; fclose(in), in = NULL) {
                    fread(header, 2, 1, in);
                }
                if( !memcmp(header, "\x64\x86", 2) ) continue;
                if( !memcmp(header, "\x00\x00", 2) ) continue;
            }

            char *dot = strrchr(fname, '.');
            if( dot ) {
                char extdot[32];
                snprintf(extdot, 32, "%s.", dot); // .png -> .png.
                // exclude vc/gcc/clang files
                if( strstr(fname, ".a.o.pdb.lib.ilk.exp.dSYM.") ) // must end with dot
                    continue;
            }

            // @todo: normalize path & rebase here (absolute to local)
            // [...]
            // fi.normalized = ; tolower->to_underscore([]();:+ )->remove_extra_underscores

            if (file_name(fname)[0] == '.') continue; // skip system files
            if (file_name(fname)[0] == ';') continue; // skip comment files

            array_push(list, STRDUP(fname));
        }
    }

    // inspect disk
    for( int i = 0, end = array_count(list); i < end; ++i ) {
        char *fname = list[i];

        struct fs fi = {0};
        fi.fname = fname; // STRDUP(fname);
        fi.bytes = file_size(fname);
        fi.stamp = file_stamp10(fname); // timestamp in base10(yyyymmddhhmmss)

        array_push(fs_now, fi);
    }

    cook_debug = !!( flags & COOK_DEBUGLOG );
    cook_cancelable = !!( flags & COOK_CANCELABLE );

    // spawn all the threads
    int num_jobs = cook_jobs();
    for( int i = 0; i < num_jobs; ++i ) {
        jobs[i].self = 0;
        jobs[i].threadid = i;
        jobs[i].numthreads = flags & COOK_ASYNC ? num_jobs : 1;
        jobs[i].files = (const char **)list;
        jobs[i].rules = rules;
        jobs[i].progress = -1;
        static thread_mutex_t lock; do_once thread_mutex_init(&lock);
        jobs[i].lock = &lock;
    }
    for( int i = 0; i < num_jobs; ++i ) {
        if( flags & COOK_ASYNC ) {
            jobs[i].self = thread_init(cook_async, &jobs[i], "cook_async()", 0/*STACK_SIZE*/);
            continue;
        }

        if(!cook(&jobs[i])) return false;
    }

    return true;
}

void cook_stop() {
    // join all threads
    int num_jobs = cook_jobs();
    for( int i = 0; i < num_jobs; ++i ) {
        if(jobs[i].self) thread_join(jobs[i].self);
    }
    // remove all temporary outfiles
    for each_array(file_list("temp_*"), char*, tempfile) unlink(tempfile);
}

int cook_progress() {
    int count = 0, sum = 0;
    for( int i = 0, end = cook_jobs(); i < end; ++i ) {
        sum += jobs[i].progress;
        ++count;
    }
    return cook_jobs() ? sum / (count+!count) : 100;
}

void cook_cancel() {
    if( cook_cancelable ) cook_cancelling = true;
}

int cook_jobs() {
    int num_jobs = optioni("--cook-jobs", maxf(1.15,app_cores()) * 1.75), max_jobs = countof(jobs);
    ifdef(ems, num_jobs = 0);
    ifdef(retail, num_jobs = 0);
    ifdef(nocook, num_jobs = 0);
    return clampi(num_jobs, 0, max_jobs);
}

void cook_config( const char *pathfile_to_cook_ini ) { // @todo: test run-from-"bin/" case on Linux.
    COOK_INI = pathfile_to_cook_ini;
    ASSERT( file_exist(COOK_INI) );
}

bool have_tools() {
    static bool found; do_once found = file_exist(COOK_INI);
    return ifdef(retail, false, found);
}
