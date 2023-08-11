// -----------------------------------------------------------------------------
// video decoder (mpeg)
// - rlyeh, public domain
//
// [ref] https://github.com/phoboslab/pl_mpeg/blob/master/pl_mpeg_player.c
// [use] ffmpeg -i infile.mp4 -c:v mpeg1video -c:a mp2 -format mpeg outfile.mpg

enum VIDEO_FLAGS {
    VIDEO_YCBCR = 0,
    VIDEO_RGB = 1,

    VIDEO_AUDIO = 0,
    VIDEO_NO_AUDIO = 2,
};

typedef struct video_t video_t;

API video_t*   video( const char *filename, int flags );
API texture_t*  video_decode( video_t *v ); // decodes next frame, returns associated texture(s)
API texture_t*  video_textures( video_t *v ); // returns last video textures. does not perform any decoding.
API int         video_has_finished(video_t *v);
API double      video_duration(video_t *v);
API int         video_seek(video_t *v, double seek_to);
API double      video_position(video_t *v);
API void        video_pause(video_t *v, bool paused);
API bool        video_is_paused(video_t *v);
API bool        video_is_rgb(video_t *v);
API void       video_destroy( video_t *v );

// -----------------------------------------------------------------------------
// video recorder (uses external ffmpeg and fallbacks to built-in mpeg1 encoder)
// - rlyeh, public domain
//
// @fixme: MSAA can cause some artifacts with Intel PBOs: either use glDisable(GL_MULTISAMPLE) before recording or do not create window with WINDOW_MSAA at all.

API bool       record_start(const char *outfile_mp4);
API bool        record_active();
API void       record_stop(void);
