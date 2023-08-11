// tip: convert video to x265/mp4. note: higher crf to increase compression (default crf is 28)
// ffmpeg -i {{infile}} -c:v libx265 -crf 24 -c:a copy {{outfile}}

struct video_t {
    // mpeg player
    plm_t *plm;
    double previous_time;
    bool paused;
    bool has_ycbcr;
    bool has_audio;
    // yCbCr
    union {
        struct {
            texture_t textureY;
            texture_t textureCb;
            texture_t textureCr;
        };
        texture_t textures[3];
    };
    // rgb
    void *surface;
    texture_t texture;
};

static void mpeg_update_texture(GLuint unit, GLuint texture, plm_plane_t *plane) {
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RED, plane->width, plane->height, 0,
        GL_RED, GL_UNSIGNED_BYTE, plane->data
    );
}
static void mpeg_video_callback( plm_t* plm, plm_frame_t* frame, void* user ) {
    video_t *v = (video_t*)user;
    if(v->paused) return;

    if(v->has_ycbcr) {
        mpeg_update_texture(GL_TEXTURE0, v->textureY.id, &frame->y);
        mpeg_update_texture(GL_TEXTURE1, v->textureCb.id, &frame->cb);
        mpeg_update_texture(GL_TEXTURE2, v->textureCr.id, &frame->cr);
    } else {
        plm_frame_to_rgb( frame, v->surface, v->texture.w * 3 );
        texture_update( &v->texture, v->texture.w, v->texture.h, v->texture.n, v->surface, v->texture.flags );
    }
    (void)plm;
}
static void mpeg_audio_callback(plm_t *plm, plm_samples_t *samples, void *user) {
    video_t *v = (video_t*)user;
    audio_queue(v->paused ? NULL : samples->interleaved, samples->count, AUDIO_FLOAT | AUDIO_2CH | AUDIO_44KHZ );
    (void)plm;
}

video_t* video( const char *filename, int flags ) {
    plm_t* plm = plm_create_with_file( vfs_handle(filename), 1 );
    if ( !plm ) {
        PANIC( "!Cannot open '%s' file for reading\n", filename );
        return 0;
    }

    int w = plm_get_width( plm );
    int h = plm_get_height( plm );
    float fps = plm_get_framerate( plm );
    float rate = plm_get_samplerate( plm );

    video_t *v = MALLOC(sizeof(video_t)), zero = {0};
    *v = zero;

    v->has_ycbcr = flags & VIDEO_RGB ? 0 : 1;

    if( v->has_ycbcr ) {
        v->textureY = texture_create( w, h, 1, NULL, TEXTURE_R );
        v->textureCb = texture_create( w, h, 1, NULL, TEXTURE_R );
        v->textureCr = texture_create( w, h, 1, NULL, TEXTURE_R );
    } else {
        int w16 = (w+15) & ~15;
        int h16 = (h+15) & ~15;
        v->texture = texture_create( w16, h16, 3, NULL, TEXTURE_SRGB );
        v->surface = REALLOC( v->surface,  w16 * h16 * 3 );
    }

    v->plm = plm;

    v->has_audio = flags & VIDEO_NO_AUDIO ? 0 : 1;

    plm_set_loop(plm, false);
    plm_set_video_decode_callback(plm, mpeg_video_callback, v);
    if( v->has_audio ) {
        plm_set_audio_enabled(plm, true);
        plm_set_audio_stream(plm, 0);
        plm_set_audio_decode_callback(plm, mpeg_audio_callback, v);
    }

    PRINTF( "Video texture: %s (%dx%dx%d %.0ffps %.1fKHz)\n", file_name(filename), w, h, v->has_ycbcr ? 3:1, fps, rate / 1000 );
    return v;
}

texture_t* video_decode( video_t *v ) { // decodes next frame, returns associated texture(s)
    double current_time = time_ss();
    double elapsed_time = current_time - v->previous_time;
    if (elapsed_time > 1.0 / 30.0) {
        elapsed_time = 1.0 / 30.0;
    }
    v->previous_time = current_time;

    if(!v->paused)
    plm_decode(v->plm, elapsed_time);

    return v->has_ycbcr ? &v->textureY : &v->texture;
}

void video_destroy(video_t *v) {
    plm_destroy( v->plm );

    if( v->has_ycbcr ) {
        texture_destroy(&v->textureY);
        texture_destroy(&v->textureCr);
        texture_destroy(&v->textureCb);
    } else {
        texture_destroy(&v->texture);
        v->surface = REALLOC(v->surface, 0);
    }

    video_t zero = {0};
    *v = zero;
    FREE(v);
}

int video_has_finished(video_t *v) {
    return !!plm_has_ended(v->plm);
}
double video_duration(video_t *v) {
    return plm_get_duration(v->plm);
}
int video_seek(video_t *v, double seek_to) {
    plm_seek(v->plm, clampf(seek_to, 0, video_duration(v)), FALSE);
    if( v->has_audio ) audio_queue_clear();
    return 1;
}
double video_position(video_t *v) {
    return plm_get_time(v->plm);
}
void video_pause(video_t *v, bool paused) {
    v->paused = paused;
}
bool video_is_paused(video_t *v) {
    return v->paused;
}
bool video_is_rgb(video_t *v) {
    return !v->has_ycbcr;
}
texture_t* video_textures( video_t *v ) {
    return v->has_ycbcr ? &v->textureY : &v->texture;
}

// -----------------------------------------------------------------------------
// ffmpeg video recording
// [src] http://blog.mmacklin.com/2013/06/11/real-time-video-capture-with-ffmpeg/

static FILE* rec_ffmpeg;
static FILE* rec_mpeg1;

void record_stop(void) {
    if(rec_ffmpeg) ifdef(win32, _pclose, pclose)(rec_ffmpeg);
    rec_ffmpeg = 0;

    if(rec_mpeg1) fclose(rec_mpeg1);
    rec_mpeg1 = 0;
}

bool record_active() {
    return rec_ffmpeg || rec_mpeg1;
}

bool record_start(const char *outfile_mp4) {
    do_once atexit(record_stop);

    record_stop();

    // first choice: external ffmpeg encoder
    if( !rec_ffmpeg ) {
        extern const char *TOOLS;

        char *tools_native_path = strswap( va("%s/", TOOLS), ifdef(win32, "/", "\\"), ifdef(win32, "\\", "/") );

        char *cmd = va("%sffmpeg%s "
                    "-hide_banner -loglevel error " // less verbose
                    "-r %d -f rawvideo -pix_fmt bgr24 -s %dx%d " // raw BGR WxH-60Hz frames
                    // "-framerate 30 " // interpolating new video output frames from the source frames
                    "-i - "              // read frames from stdin
                    //"-draw_mouse 1 "
                    "-threads 0 "
                    //"-vsync vfr "
                    "-preset ultrafast " // collection of options that will provide a certain encoding speed [fast,ultrafast]
                    // "-tune zerolatency " // change settings based upon the specifics of your input
                    //"-crf 21 "           // range of the CRF scale [0(lossless)..23(default)..51(worst quality)]
                    "-pix_fmt yuv420p "  // compatible with Windows Media Player and Quicktime
                    "-vf vflip "         // flip Y
//                  "-vf \"pad=ceil(iw/2)*2:ceil(ih/2)*2\" "
                    "-y \"%s\"", tools_native_path, ifdef(win32, ".exe", ifdef(osx, ".osx",".linux")),
                    (int)window_fps(), window_width(), window_height(), outfile_mp4);    // overwrite output file

        // -rtbufsize 100M (https://trac.ffmpeg.org/wiki/DirectShow#BufferingLatency) Prevent some frames in the buffer from being dropped.
        // -probesize 10M (https://www.ffmpeg.org/ffmpeg-formats.html#Format-Options) Set probing size in bytes, i.e. the size of the data to analyze to get stream information. A higher value will enable detecting more information in case it is dispersed into the stream, but will increase latency. Must be an integer not lesser than 32. It is 5000000 by default.
        // -c:v libx264 (https://www.ffmpeg.org/ffmpeg.html#Main-options) Select an encoder (when used before an output file) or a decoder (when used before an input file) for one or more streams. codec is the name of a decoder/encoder or a special value copy (output only) to indicate that the stream is not to be re-encoded.

        // open pipe to ffmpeg's stdin in binary write mode
        rec_ffmpeg = ifdef(win32, _popen(cmd, "wb"), popen(cmd, "w"));
    }

    // fallback: built-in mpeg1 encoder
    if( !rec_ffmpeg ) {
        rec_mpeg1 = fopen(outfile_mp4, "wb"); // "a+b"
    }

    return record_active();
}

void record_frame() {
    if( record_active() ) {
        void* pixels = screenshot_async(-3); // 3 RGB, 4 RGBA, -3 BGR, -4 BGRA. ps: BGR is fastest on my intel discrete gpu

        if( rec_ffmpeg ) {
            fwrite(pixels, 3 * window_width() * window_height(), 1, rec_ffmpeg);
        }
        if( rec_mpeg1 ) {
            jo_write_mpeg(rec_mpeg1, pixels, window_width(), window_height(), 24);  // 24fps
        }
    }
}
