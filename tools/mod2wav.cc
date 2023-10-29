// music module to wav writer
// - rlyeh, public domain
//
// based on modplug123 by Konstanty Bialkowski - 2010 
// based on modplug play ( gurkan@linuks.mine.nu www.linuks.mine.nu - Copyright (C) 2003 Gürkan Sengün)

#define MODPLUG_STATIC
#define MODPLUG_CPP
#include "3rd_modplug.hpp"

#include <stdio.h>
#include <string.h>
#include <vector>

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

char *read_file(char *filename, long *size) {
    FILE *f = fopen(filename, "rb");
    if (f) {
        fseek(f, 0L, SEEK_END);
        (*size) = ftell(f);
        rewind(f);
        char *data = (char*)malloc(*size);
        if( fread(data, *size, 1, f) == 1 ) {
            fclose(f);
            return data;
        }
        free(data);
        fclose(f);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if(argc != 3) exit(-printf("%s\n", "mod2wav infile.mod outfile.wav"));

    char *filename = argv[1];
    long size; char *bin = read_file(filename, &size);
    if (!bin) exit(-printf("%s\n", "cannot read input file"));

    ModPlug_Settings settings;
    ModPlug_GetSettings(&settings);

    // settings must be set before ModPlug_Load()
    settings.mResamplingMode = MODPLUG_RESAMPLE_FIR; /* RESAMP */
    settings.mChannels = 2; // 1 if mono
    settings.mBits = 16;
    settings.mFrequency = 44100;
    settings.mStereoSeparation = 128;
    settings.mMaxMixChannels = 256;
#if 0
    settings.mFlags=MODPLUG_ENABLE_OVERSAMPLING | \
                    MODPLUG_ENABLE_NOISE_REDUCTION | \
            MODPLUG_ENABLE_REVERB | \
            MODPLUG_ENABLE_MEGABASS | \
            MODPLUG_ENABLE_SURROUND;
    settings.mReverbDepth = 100;   //  0 - 100     [REV--DLY--]
    settings.mReverbDelay = 200;   // 40 - 200 ms  00-FF  
    settings.mSurroundDepth = 100; //  0 - 100     [SUR--DLY--]
    settings.mSurroundDelay = 40;  //  5 -  40 ms   
    settings.mBassAmount  = 100;   //  0 - 100     [BAS--RNG--]
    settings.mBassRange   = 100;   // 10 - 100 hz  
#endif

    ModPlug_SetSettings(&settings);
    ModPlugFile *f2 = ModPlug_Load(bin, size);
    if (!f2) {
        // free(bin); // @leak
        exit(-printf("could not load %s\n", filename));
    }

    std::vector<unsigned char> raw;

    for( int mlen = 1; mlen > 0; ) {
        enum { BUF_SIZE = 4096 };
        unsigned char audio_buffer[BUF_SIZE];
        mlen = ModPlug_Read(f2,audio_buffer,sizeof(audio_buffer));

        if( mlen > 0 ) for( int i = 0; i < mlen; ++i ) {
            raw.push_back( audio_buffer[i] );
        }
    }

    if( raw.size() ) {
        FILE *fp = fopen(argv[2], "wb");
        if (!fp) exit(-printf("%s\n", "cannot open output file"));
        tinywav(fp, 2, 16, 44100, raw.data(), raw.size(), false /*is_floating*/);
        fclose(fp);
    }

    puts(ModPlug_GetName(f2));    
    // ModPlug_Unload(f2); // @leak
    // free(bin); // @leak

    return raw.size() ? 0 : -1;
}

// cl mod2wav.cc -I ..\engine\split /EHsc /O2 /Oy /DNDEBUG /MT
