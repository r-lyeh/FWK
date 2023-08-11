// mid2wav
// - rlyeh, public domain

#include "3rd_stb_vorbis.h" // for sf3

#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h> // stddef.h

// Tiny WAV writer: original code by jon olick, public domain
// Floating point support + pure C version by rlyeh, public domain | wtrmrkrlyeh
#include <stdio.h>
static void tinywav(FILE *fp, short numChannels, short bitsPerSample, int sampleRateHz, const void *data, int size, int is_floating) {
    short bpsamp;
    int length, bpsec;
    fwrite("RIFF", 1, 4, fp);
    length = size + 44 - 8;
    fwrite(&length, 1, 4, fp);
    fwrite(is_floating ? "WAVEfmt \x10\x00\x00\x00\x03\x00" : "WAVEfmt \x10\x00\x00\x00\x01\x00", 1, 14, fp);
    fwrite(&numChannels, 1, 2, fp);
    fwrite(&sampleRateHz, 1, 4, fp);
    bpsec = numChannels * sampleRateHz * bitsPerSample/8;
    fwrite(&bpsec, 1, 4, fp);
    bpsamp = numChannels * bitsPerSample/8;
    fwrite(&bpsamp, 1, 2, fp);
    fwrite(&bitsPerSample, 1, 2, fp);
    fwrite("data", 1, 4, fp);
    fwrite(&size, 1, 4, fp);
    fwrite(data, 1, size, fp);
}

#define MID_IMPLEMENTATION
#define MID_ENABLE_RAW
#include "3rd_mid.h"

// io
unsigned char *readfile(const char *pathfile, int *size) {
    char *bin = 0;
    for( FILE *fp = fopen(pathfile,"rb"); fp; fclose(fp), fp = 0) {
        fseek(fp, 0L, SEEK_END);
        size_t len = ftell(fp);
        if(size) *size = (int)len;
        fseek(fp, 0L, SEEK_SET);
        bin = malloc(len+1);
        if( bin && fread(bin, 1, len, fp) == len ) bin[len] = '\0';
        else free(bin), bin = 0;
    }
    return bin;
}

#define die(errmsg) exit((puts(errmsg),-__LINE__))

int main(int argc, char **argv) {
    if( argc != 3 && argc != 4 ) {
        printf("%s infile.mid outfile.wav [soundbank.sf2/soundbank.sf3]\n", argv[0]);
        return -1;
    }

#if 0
    // not sure why this sf2 cannot be loaded (it should!)
    char gm_dls[256];
    snprintf(gm_dls, 255, "%s/system32/gm.dls", getenv("SystemRoot"));
    puts(gm_dls);
#else
    const char *gm_dls = "AweROMGM.sf3";
#endif

    int mid_size = 0;
    void *mid_data = readfile(argv[1],&mid_size);
    if(!mid_data) die("cannot open midi file for reading");

    int sf2_size = 0;
    void *sf2_data = readfile(argc > 3 ? argv[3] : gm_dls, &sf2_size);
    if(!mid_data) die("cannot open soundfont file for reading");

    // set soundfont
    tsf *sound_font = tsf_load_memory( sf2_data, sf2_size );
    tsf_channel_set_bank_preset( sound_font, 9, 128, 0 );
    tsf_set_output( sound_font, TSF_STEREO_INTERLEAVED, 44100, 0.0f );

    // play song
    mid_t *mid = 0;
    mid = mid_create(mid_data, mid_size, NULL );
    if( mid && sound_font ) {
        mid_skip_leading_silence( mid, sound_font );

        // export song
        size_t raw_size = 16*1024*1024;
        void *raw_data = malloc(raw_size);
        int bytes_per_sample = 1*2*2;
        int samples = raw_size / bytes_per_sample;
        int ok = mid_render_short(mid, (short*)raw_data, samples, sound_font );

        FILE *fp = fopen(argv[2], "wb"); if(!fp) die("cannot open wav file for writing");
        tinywav(fp, 2, 16, 44100, raw_data, raw_size, 0);
        fclose(fp);
    }

    // clean up
    mid_destroy( mid );
    tsf_close( sound_font );

    return 0;
}

// cl mid2wav.c -I split /Os /Ox /O2 /Oy /MT /DFINAL /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
