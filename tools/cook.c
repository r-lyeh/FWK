#define COOK_ON_DEMAND 0
#define COOK_FROM_TERMINAL 1
#include "fwk.c"

int main(int argc, const char **argv) {
    double timer = time_ss();

    if( flag("--help") || flag("-h") || flag("-?") ) {
        printf("%s [/pathfiles/to/cook] [options]\n\n", argv[0]);
        printf("Options:\n");
        printf("\t--quiet\n\t--cook-ini=/pathfile/to/cook.ini\n\t--cook-debug\n\t--cook-additive\n\t--cook-jobs=N\n\t--cook-compressor=[level][use]");
        printf("\toverride compressor (level=0..10, use=PPP,ULZ,LZ4,CRSH,DEFL,LZP,LZMA,BALZ,LZW,LZSS,BCM)\n");

        return 1;
    }

    const char *ini_file = option("--cook-ini", COOK_INI);

    unsigned num_cooked = 0;
    for( int i = 1; i < argc; ++i ) {
        num_cooked += argv[i][0] != '-' && cook_start(ini_file, argv[i], COOK_ASYNC) ? cook_stop(), 1 : 0;
    }
    if( !num_cooked ) {
        if( cook_start(ini_file, "**", COOK_ASYNC) ) cook_stop();
    }

    if( !flag("--quiet") ) printf("Ok, %5.2fs.\n", time_ss() - timer);
    return 0;
}

/*
   compiled with:
   cc -ObjC cook.c -I../engine -o cook.osx -framework Cocoa -framework IOKit -framework audiotoolbox -framework coreaudio -O3
   cc cook.c       -I../engine -o cook.linux -lm -lpthread -ldl -lX11 -O3
   tcc cook.c      -I..\engine
   cl  cook.c      -I..\engine /openmp /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
   del *.o & del *.obj & del *.lib & del *.exp & del *.pdb
*/
