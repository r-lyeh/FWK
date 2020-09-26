/**
 * Encode a code point using UTF-8
 *
 * @author Ondřej Hruška <ondra@ondrovo.com>
 * @license MIT
 *
 * @param out - output buffer (min 5 characters), will be 0-terminated
 * @param utf - code point 0-0x10FFFF
 * @return number of bytes on success, 0 on failure (also produces U+FFFD, which uses 3 bytes)
 */
int utf8_encode(char *out, unsigned utf)
{
  if (utf <= 0x7F) {
    // Plain ASCII
    out[0] = (char) utf;
    out[1] = 0;
    return 1;
  }
  else if (utf <= 0x07FF) {
    // 2-byte unicode
    out[0] = (char) (((utf >> 6) & 0x1F) | 0xC0);
    out[1] = (char) (((utf >> 0) & 0x3F) | 0x80);
    out[2] = 0;
    return 2;
  }
  else if (utf <= 0xFFFF) {
    // 3-byte unicode
    out[0] = (char) (((utf >> 12) & 0x0F) | 0xE0);
    out[1] = (char) (((utf >>  6) & 0x3F) | 0x80);
    out[2] = (char) (((utf >>  0) & 0x3F) | 0x80);
    out[3] = 0;
    return 3;
  }
  else if (utf <= 0x10FFFF) {
    // 4-byte unicode
    out[0] = (char) (((utf >> 18) & 0x07) | 0xF0);
    out[1] = (char) (((utf >> 12) & 0x3F) | 0x80);
    out[2] = (char) (((utf >>  6) & 0x3F) | 0x80);
    out[3] = (char) (((utf >>  0) & 0x3F) | 0x80);
    out[4] = 0;
    return 4;
  }
  else {
    // error - use replacement character
    out[0] = (char) 0xEF;
    out[1] = (char) 0xBF;
    out[2] = (char) 0xBD;
    out[3] = 0;
    return 0;
  }
}

#include <stdio.h>
#include <ctype.h>

int main() {
    unsigned mincp = ~0u, maxcp = 0;

    for( FILE *fp = fopen("materialicons-regular.codepoints", "rt"); fp; fclose(fp), fp = 0) {
        char buffer[128];
        while( fgets(buffer, 128, fp) ) {
            while(buffer[strlen(buffer)-1] < 32) buffer[strlen(buffer)-1] = '\0';
            //puts(buffer);

            char name[128];
            unsigned cp;

            sscanf(buffer, "%s %x", name, &cp);
            for(int i = 0; name[i]; ++i) name[i] = toupper(name[i]);

            char utf8[128];
            int utf8len = utf8_encode(utf8, cp);

            printf("#define ICON_MD_%s \"", name);
            for( int i = 0; i < utf8len; ++i ) printf("\\x%02x", (unsigned char)utf8[i]);
            printf("\" // U+%x\n", cp);

            mincp = mincp < cp ? mincp : cp;
            maxcp = maxcp > cp ? maxcp : cp;
        }
    }

    printf("#define ICON_MD_MIN %#x\n", mincp);
    printf("#define ICON_MD_MAX %#x\n", maxcp);
}
