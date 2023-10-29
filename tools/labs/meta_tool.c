#define FWK_C
#include "fwk.h"

bool parse_struct(const char *line) {
    return strstr(line, "s""truct ");
}
bool parse_union(const char *line) {
    return strstr(line, "u""nion");
}
bool parse_enum(const char *line) {
    return strstr(line, "e""num ");
}
bool parse_typedef(const char *line) {
    return strstr(line, "t""ypedef ");
}
bool parse_function(const char *line) {
    return strstr(line, "(") && strstr(line, ");");
}
bool parse_comment(const char *line) {
    return strstr(line, "//") || (strstr(line, "/*") && strstr(line, "*/"));
}
bool parse_variable(const char *line) {
    return strstr(line, "=");
}
bool parse_member(const char *line) {
    return strstr(line, ";");
}
const char* parse_any(const char *line) {
    if(parse_struct(line))   return "STRUCT";
    if(parse_union(line))    return "UNION";
    if(parse_enum(line))     return "ENUM";
    if(parse_typedef(line))  return "TYPEDEF";
    if(parse_variable(line)) return "VARIABLE";
    if(parse_function(line)) return "FUNCTION";
    if(parse_member(line))   return "MEMBER";
    return 0;
}

int main() {
    char *data = file_read(__FILE__);

    for each_substring(data, "\r\n", line) {
        const char *type = parse_any(line);
        printf("%s%s%s\n", line, type ? " -> " : "", type ? type : "");
    }
}
