/*
------------------------------------------------------------------------------
          Licensing information can be found at the end of the file.
------------------------------------------------------------------------------

mid.h - v0.1 - Midi playback library using the TinySoundFont library.

Do this:
    #define MID_IMPLEMENTATION
before you include this file in *one* C/C++ file to create the implementation.
*/

#ifndef mid_h
#define mid_h

#define _CRT_NONSTDC_NO_DEPRECATE 
#define _CRT_SECURE_NO_WARNINGS
#include <stddef.h>

typedef struct mid_t mid_t;
typedef struct tsf tsf;
    
mid_t* mid_create( void const* midi_data, size_t midi_size, void* memctx );
void mid_destroy( mid_t* mid ); 

int mid_render_short( mid_t* mid, short* sample_pairs, int sample_pairs_count, tsf* sound_font );
int mid_render_float( mid_t* mid, float* sample_pairs, int sample_pairs_count, tsf* sound_font );

void mid_skip_leading_silence( mid_t* mid, tsf* sound_font );

#endif /* mid_h */

#ifdef MID_ENABLE_RAW

#ifndef mid_raw_h
#define mid_raw_h

#ifndef MID_U8
    #define MID_U8 unsigned char
#endif

#ifndef MID_U16
    #define MID_U16 unsigned short
#endif

#ifndef MID_U32
    #define MID_U32 unsigned int
#endif

#ifndef MID_U64
    #define MID_U64 unsigned long long
#endif

typedef struct mid_event_t
    {
    MID_U32 delay_us;
    MID_U8 channel;
    MID_U8 type;
    union 
        {
        struct { MID_U8 program; } program_change;
        struct { MID_U8 note; MID_U8 velocity; } note_on;
        struct { MID_U8 note; } note_off;
        struct { MID_U8 key; MID_U8 key_pressure; } key_pressure;
        struct { MID_U16 value; } pitch_bend;
        struct { MID_U8 control, control_value; } control_change;
        struct { MID_U8 channel_pressure; } channel_pressure;
        } data;
    } mid_event_t;


typedef struct mid_song_t
    {
    int event_count;
    mid_event_t* events;
    } mid_song_t;


struct mid_t
    {
    void* memctx;
    mid_song_t song;
    int percussion_preset;
    MID_U64 playback_accumulated_time_us;
    int playback_sample_pos;
    int playback_event_pos;
    };

int mid_init_raw( mid_t* mid, void const* raw_data, size_t raw_size );

size_t mid_save_raw( mid_t* mid, void* data, size_t capacity ); 


#endif /* MID_ENABLE_RAW */

#endif /* mid_raw_h */

/*
----------------------
    IMPLEMENTATION
----------------------
*/

#ifdef MID_IMPLEMENTATION
#undef MID_IMPLEMENTATION

#ifndef MID_U8
    #define MID_U8 unsigned char
#endif

#ifndef MID_U16
    #define MID_U16 unsigned short
#endif

#ifndef MID_U32
    #define MID_U32 unsigned int
#endif

#ifndef MID_U64
    #define MID_U64 unsigned long long
#endif

#ifndef MID_MALLOC
    #define _CRT_NONSTDC_NO_DEPRECATE 
    #define _CRT_SECURE_NO_WARNINGS
    #include <stdlib.h>
    #if defined(_cplusplus)
        #define MID_MALLOC( ctx, size ) ( ::malloc( size ) )
        #define MID_FREE( ctx, ptr ) ( ::free( ptr ) )
    #else
        #define MID_MALLOC( ctx, size ) ( malloc( size ) )
        #define MID_FREE( ctx, ptr ) ( free( ptr ) )
    #endif
#endif
#include <assert.h>
#define MID_LOG(...) (void) __VA_ARGS__

#include <string.h>

#pragma warning( push )
#pragma warning( disable: 4242 )
#pragma warning( disable: 4244 )
#pragma warning( disable: 4365 )
#pragma warning( disable: 4668 )
#pragma warning( disable: 4701 )
#pragma warning( disable: 4703 )

#ifndef MID_NO_TSF_IMPLEMENTATION
    #define TSF_NO_STDIO
    #define TSF_IMPLEMENTATION
#endif
#include "3rd_tsf.h"

#pragma warning( disable: 4201 )

#ifndef MID_NO_TML_IMPLEMENTATION
    #define TML_NO_STDIO
    #define TML_IMPLEMENTATION
#endif
#include "3rd_tml.h"

#pragma warning( pop )




#ifndef MID_ENABLE_RAW

typedef struct mid_event_t
    {
    MID_U32 delay_us;
    MID_U8 channel;
    MID_U8 type;
    union 
        {
        struct { MID_U8 program; } program_change;
        struct { MID_U8 note; MID_U8 velocity; } note_on;
        struct { MID_U8 note; } note_off;
        struct { MID_U8 key; MID_U8 key_pressure; } key_pressure;
        struct { MID_U16 value; } pitch_bend;
        struct { MID_U8 control, control_value; } control_change;
        struct { MID_U8 channel_pressure; } channel_pressure;
        } data;
    } mid_event_t;


typedef struct mid_song_t
    {
    int event_count;
    mid_event_t* events;
    } mid_song_t;


struct mid_t
    {
    void* memctx;
    mid_song_t song;
    int percussion_preset;
    MID_U64 playback_accumulated_time_us;
    int playback_sample_pos;
    int playback_event_pos;
    };


#endif /* MID_ENABLE_RAW */


mid_t* mid_create( void const* midi_data, size_t midi_size, void* memctx )
    {
    tml_message* mid_file = tml_load_memory( midi_data, (int) midi_size );
    if( !mid_file ) return NULL;
    int count = 0;
    tml_message* iter = mid_file;
    while( iter ) 
        {
        if( iter->type == TML_PROGRAM_CHANGE || iter->type == TML_NOTE_ON || iter->type == TML_NOTE_OFF || 
            iter->type == TML_PITCH_BEND || iter->type == TML_CONTROL_CHANGE )
            {
            ++count;
            }
        iter = iter->next;
        }

    mid_event_t* events = (mid_event_t*) malloc( sizeof( mid_event_t ) * count );
    int events_count = 0;
    unsigned int time = 0;
    tml_message* msg = mid_file;
    while( msg ) 
        {
        if( msg->type == TML_PROGRAM_CHANGE || msg->type == TML_NOTE_ON || msg->type == TML_NOTE_OFF || 
            msg->type == TML_PITCH_BEND || msg->type == TML_CONTROL_CHANGE )
            {
            mid_event_t* event = &events[ events_count++ ];
            event->delay_us = ( msg->time - time ) * 1000;
            time = msg->time;
            event->channel = msg->channel;
            event->type = msg->type;
            switch( msg->type ) 
                {
			    case TML_PROGRAM_CHANGE:
                    event->data.program_change.program = (MID_U8) msg->program;
				    break;
			    case TML_NOTE_ON: //play a note
                    event->data.note_on.note = (MID_U8) msg->key;
                    event->data.note_on.velocity = (MID_U8) msg->velocity;
				    break;
			    case TML_NOTE_OFF: //stop a note
                    event->data.note_off.note = (MID_U8) msg->key;
				    break;
			    case TML_PITCH_BEND: //pitch wheel modification
                    event->data.pitch_bend.value = (MID_U16) msg->pitch_bend;
				    break;
			    case TML_CONTROL_CHANGE: //MIDI controller messages
                    event->data.control_change.control = (MID_U8) msg->control;
                    event->data.control_change.control_value = (MID_U8) msg->control_value;
				    break;
                }
            }
        
        msg = msg->next;
        }

    tml_free( mid_file );

    mid_t* mid = (mid_t*) MID_MALLOC( memctx, sizeof( mid_t ) );
    mid->memctx = memctx;
    mid->song.event_count = events_count;
    mid->song.events = events;

    mid->playback_accumulated_time_us = 0ull;
    mid->playback_sample_pos = 0;
    mid->playback_event_pos = 0;

    return mid; 
    }


void mid_destroy( mid_t* mid )
    {
    if( mid->song.events ) MID_FREE( mid->memctx, mid->song.events );
    MID_FREE( mid->memctx, mid );
    }


int mid_init_raw( mid_t* mid, void const* raw_data, size_t raw_size )
    {
    int events_count = *(int*)raw_data;
    if( sizeof( mid_event_t ) * events_count != raw_size - sizeof( int ) ) return 0;

    mid->memctx = NULL;

    mid->song.event_count = events_count;
    mid->song.events = (mid_event_t*)( ( (int*)raw_data ) + 1 );

    mid->playback_accumulated_time_us = 0ull;
    mid->playback_sample_pos = 0;
    mid->playback_event_pos = 0;

    return 1; 
    }


size_t mid_save_raw( mid_t* mid, void* data, size_t capacity ) 
    {
    size_t size = sizeof( mid_event_t ) * mid->song.event_count + sizeof( int );
    if( data && capacity >= size ) 
        {
        *(int*)data = mid->song.event_count;
        memcpy( ( (int*)data ) + 1, mid->song.events, sizeof( mid_event_t ) * mid->song.event_count );
        }
    return size;
    }


void mid_skip_leading_silence( mid_t* mid, tsf* sound_font )
    {
    (void) sound_font;
    for( ; ; )
        {
        MID_U64 next_event_delay_us = mid->song.events[ mid->playback_event_pos ].delay_us;
        MID_U64 playback_time_us = ( mid->playback_sample_pos * 1000000ull ) / 44100ull;
        MID_U64 next_event_time_us = mid->playback_accumulated_time_us + next_event_delay_us;
        assert( next_event_time_us >= playback_time_us );
        MID_U64 time_until_next_event = next_event_time_us - playback_time_us;
        int samples_until_next_event = (int)( ( time_until_next_event * 44100ull ) / 1000000ull );       
        mid_event_t* event = &mid->song.events[ mid->playback_event_pos ];
        switch( event->type )
            {
			case TML_PROGRAM_CHANGE: 
				tsf_channel_set_presetnumber( sound_font, event->channel, event->data.program_change.program, ( event->channel == 9 ) );
				break;
			case TML_NOTE_ON:
				return;
			case TML_NOTE_OFF: //stop a note
				tsf_channel_note_off( sound_font, event->channel, event->data.note_off.note );
				break;
			case TML_PITCH_BEND: //pitch wheel modification
				tsf_channel_set_pitchwheel( sound_font, event->channel, event->data.pitch_bend.value );
				break;
			case TML_CONTROL_CHANGE: //MIDI controller messages
				tsf_channel_midi_control( sound_font, event->channel, event->data.control_change.control, event->data.control_change.control_value );
				break;
			}
        mid->playback_sample_pos += samples_until_next_event;
        mid->playback_accumulated_time_us += next_event_delay_us; 
        mid->playback_event_pos++;
        }
    }


int mid_render_short( mid_t* mid, short* sample_pairs, int sample_pairs_count, tsf* sound_font )
    {
    int samples_rendered = 0;
    memset( sample_pairs, 0, sample_pairs_count * sizeof( short ) * 2 );
    while( samples_rendered < sample_pairs_count )
        {
        MID_U64 next_event_delay_us = mid->song.events[ mid->playback_event_pos ].delay_us;
        MID_U64 playback_time_us = ( mid->playback_sample_pos * 1000000ull ) / 44100ull;
        MID_U64 next_event_time_us = mid->playback_accumulated_time_us + next_event_delay_us;
        assert( next_event_time_us >= playback_time_us );
        MID_U64 time_until_next_event = next_event_time_us - playback_time_us;
        int samples_until_next_event = (int)( ( time_until_next_event * 44100ull ) / 1000000ull );
        int samples_to_render = samples_until_next_event;
        if( samples_to_render > sample_pairs_count - samples_rendered )
            {
            samples_to_render = sample_pairs_count - samples_rendered;
            tsf_render_short( sound_font, sample_pairs + samples_rendered * 2, 
                samples_to_render, 1 );
            samples_rendered += samples_to_render;
            mid->playback_sample_pos += samples_to_render;
            return samples_rendered;
            }
        else
            {
            tsf_render_short( sound_font, sample_pairs + samples_rendered * 2, 
                samples_to_render, 1 );
            samples_rendered += samples_to_render;
            mid->playback_sample_pos += samples_to_render;
            }


        mid->playback_accumulated_time_us += next_event_delay_us; 
        mid_event_t* event = &mid->song.events[ mid->playback_event_pos++ ];
        switch( event->type )
            {
			case TML_PROGRAM_CHANGE: 
				tsf_channel_set_presetnumber( sound_font, event->channel, event->data.program_change.program, ( event->channel == 9 ) );
				break;
			case TML_NOTE_ON:
				tsf_channel_note_on( sound_font, event->channel, event->data.note_on.note, event->data.note_on.velocity / 127.0f );
				break;
			case TML_NOTE_OFF: //stop a note
				tsf_channel_note_off( sound_font, event->channel, event->data.note_off.note );
				break;
			case TML_PITCH_BEND: //pitch wheel modification
				tsf_channel_set_pitchwheel( sound_font, event->channel, event->data.pitch_bend.value );
				break;
			case TML_CONTROL_CHANGE: //MIDI controller messages
				tsf_channel_midi_control( sound_font, event->channel, event->data.control_change.control, event->data.control_change.control_value );
				break;
			}
        }
    
    return samples_rendered;
    }


#endif /* MID_IMPLEMENTATION */

/*
------------------------------------------------------------------------------

This software is available under 2 licenses - you may choose the one you like.

------------------------------------------------------------------------------

ALTERNATIVE A - MIT License

Copyright (c) 2016 Mattias Gustavsson

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

------------------------------------------------------------------------------

ALTERNATIVE B - Public Domain (www.unlicense.org)

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
software, either in source code form or as a compiled binary, for any purpose, 
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this 
software dedicate any and all copyright interest in the software to the public 
domain. We make this dedication for the benefit of the public at large and to 
the detriment of our heirs and successors. We intend this dedication to be an 
overt act of relinquishment in perpetuity of all present and future rights to 
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

------------------------------------------------------------------------------
*/
