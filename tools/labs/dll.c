#include "engine/fwk.c"

int main() {
	int (*dll_main)(int argc, char **argv);
	dll_main = dll("tools/ase2ini.exe","main"); // crashes
	printf("%p\n", dll_main);
	dll_main = dll("tools/ase2ini.dll","main");
	printf("%p\n", dll_main);

	benchmark {
		for( int i = 0; i < 10000; ++i ) {
			char *args[] = { "ase2ini.exe", "demos/art/sprites/Treasure Hunters/Captain Clown Nose.ase", 0 };
			dll_main(countof(args)-1, args);
		}
	}
}
