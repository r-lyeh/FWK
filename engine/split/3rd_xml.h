// original xml.h/xml.c files by tor andersson, public domain

#ifndef xml_h
#define xml_h

/* Parse UTF-8 string and return the XML root node, or NULL if there is a parse error. */
struct xml *xml_parse(char *buf, int preserve_white, char **error);

/* Free an XML node and all its children and siblings. */
void xml_free(struct xml *item);

/* Navigate the XML tree. */
struct xml *xml_prev(struct xml *item);
struct xml *xml_next(struct xml *item);
struct xml *xml_up(struct xml *item);
struct xml *xml_down(struct xml *item);

/* Return true if the tag name matches. */
int xml_is_tag(struct xml *item, const char *name);

/* Return tag name of XML node, or NULL if it's a text node. */
char *xml_tag(struct xml *item);

/* Return the value of an attribute of an XML node, or NULL if the attribute doesn't exist. */
char *xml_att(struct xml *item, const char *att);

/* Return the name of an attribute of an XML node, or NULL if the attribute doesn't exist. */
char *xml_att_name(struct xml *item, int index); //< @r-lyeh: new function

/* Return the text content of an XML node, or NULL if the node is a tag. */
char *xml_text(struct xml *item);

/* Find the first sibling with the given tag name (may be the same node). */
struct xml *xml_find(struct xml *item, const char *tag);

/* Find the next sibling with the given tag name (never the same node). */
struct xml *xml_find_next(struct xml *item, const char *tag);

/* Find the first child with the given tag name. */
struct xml *xml_find_down(struct xml *item, const char *tag);

#endif

#ifdef XML_C

#include <stdlib.h> /* malloc, free, strtol */
#include <string.h> /* memmove, strcmp */

static int runetochar(char *s, int c)
{
    if (c < 0x80) {
        s[0] = c;
        return 1;
    }
    if (c < 0x800) {
        s[0] = 0xC0 | (c >> 6);
        s[1] = 0x80 | (c & 0x3F);
        return 2;
    }
    if (c > 0x10FFFF)
        c = 0xFFFD;
    if (c < 0x1000) {
        s[0] = 0xE0 | (c >> 12);
        s[1] = 0x80 | ((c >> 6) & 0x3F);
        s[2] = 0x80 | (c & 0x3F);
        return 3;
    }
    s[0] = 0xf0 | (c >> 18);
    s[1] = 0x80 | ((c >> 12) & 0x3F);
    s[2] = 0x80 | ((c >> 6) & 0x3F);
    s[3] = 0x80 | (c & 0x3F);
    return 4;
}

struct {
    struct xml *head;
    int preserve_white;
    int depth;
} g;

struct xmlatt {
    char name[40];
    char *value;
    struct xmlatt *next;
};

struct xml {
    char name[40];
    char *text;
    struct xmlatt *atts;
    struct xml *up, *down, *tail, *prev, *next;
};

struct xml *xml_prev(struct xml *item)
{
    return item ? item->prev : NULL;
}

struct xml *xml_next(struct xml *item)
{
    return item ? item->next : NULL;
}

struct xml *xml_up(struct xml *item)
{
    return item ? item->up : NULL;
}

struct xml *xml_down(struct xml *item)
{
    return item ? item->down : NULL;
}

char *xml_text(struct xml *item)
{
    return item ? item->text : NULL;
}

char *xml_tag(struct xml *item)
{
    return item && item->name[0] ? item->name : NULL;
}

int xml_is_tag(struct xml *item, const char *name)
{
    if (!item)
        return 0;
    return !strcmp(item->name, name);
}

char *xml_att(struct xml *item, const char *name)
{
    struct xmlatt *att;
    if (!item)
        return NULL;
    for (att = item->atts; att; att = att->next)
        if (!strcmp(att->name, name))
            return att->value;
    return NULL;
}

char *xml_att_name(struct xml *item, int index) //< @r-lyeh: new function
{
    int i;
    struct xmlatt *att;
    if (!item)
        return NULL;
    for (att = item->atts, i = 0; att && i < index; att = att->next, ++i) {}
    return att ? att->name : NULL;
}

struct xml *xml_find(struct xml *item, const char *tag)
{
    while (item) {
        if (!strcmp(item->name, tag))
            return item;
        item = item->next;
    }
    return NULL;
}

struct xml *xml_find_next(struct xml *item, const char *tag)
{
    if (item)
        item = item->next;
    return xml_find(item, tag);
}

struct xml *xml_find_down(struct xml *item, const char *tag)
{
    if (item)
        item = item->down;
    return xml_find(item, tag);
}

static void xml_free_att(struct xmlatt *att)
{
    while (att) {
        struct xmlatt *next = att->next;
        if (att->value)
            free(att->value);
        free(att);
        att = next;
    }
}

void xml_free(struct xml *item)
{
    while (item) {
        struct xml *next = item->next;
        if (item->text)
            free(item->text);
        if (item->atts)
            xml_free_att(item->atts);
        if (item->down)
            xml_free(item->down);
        free(item);
        item = next;
    }
}

static int xml_parse_entity(int *c, char *a)
{
    char *b;
    if (a[1] == '#') {
        if (a[2] == 'x')
            *c = strtol(a + 3, &b, 16);
        else
            *c = strtol(a + 2, &b, 10);
        if (*b == ';')
            return b - a + 1;
    } else if (a[1] == 'l' && a[2] == 't' && a[3] == ';') {
        *c = '<';
        return 4;
    } else if (a[1] == 'g' && a[2] == 't' && a[3] == ';') {
        *c = '>';
        return 4;
    } else if (a[1] == 'a' && a[2] == 'm' && a[3] == 'p' && a[4] == ';') {
        *c = '&';
        return 5;
    } else if (a[1] == 'a' && a[2] == 'p' && a[3] == 'o' && a[4] == 's' && a[5] == ';') {
        *c = '\'';
        return 6;
    } else if (a[1] == 'q' && a[2] == 'u' && a[3] == 'o' && a[4] == 't' && a[5] == ';') {
        *c = '"';
        return 6;
    }
    *c = *a;
    return 1;
}

static int isname(int c)
{
    return c == '.' || c == '-' || c == '_' || c == ':' ||
        (c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z');
}

static int is_white(int c)
{
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

static void xml_emit_open_tag(char *a, char *b)
{
    struct xml *head, *tail;
    char *ns;

    /* skip namespace prefix */
    for (ns = a; ns < b; ++ns)
        if (*ns == ':')
            a = ns + 1;

    head = malloc(sizeof *head);
    if (b - a > (int)sizeof(head->name) - 1)
        b = a + sizeof(head->name) - 1;
    memmove(head->name, a, b - a);
    head->name[b - a] = 0;

    head->atts = NULL;
    head->text = NULL;
    head->up = g.head;
    head->down = NULL;
    head->prev = NULL;
    head->next = NULL;

    if (!g.head->down) {
        g.head->down = head;
        g.head->tail = head;
    } else {
        tail = g.head->tail;
        tail->next = head;
        head->prev = tail;
        g.head->tail = head;
    }

    g.head = head;
    g.depth++;
}

static void xml_emit_att_name(char *a, char *b)
{
    struct xml *head = g.head;
    struct xmlatt *att;

    att = malloc(sizeof *att);
    if (b - a > (int)sizeof(att->name) - 1)
        b = a + sizeof(att->name) - 1;
    memmove(att->name, a, b - a);
    att->name[b - a] = 0;
    att->value = NULL;
    att->next = head->atts;
    head->atts = att;
}

static void xml_emit_att_value(char *a, char *b)
{
    struct xml *head = g.head;
    struct xmlatt *att = head->atts;
    char *s;
    int c;

    /* entities are all longer than UTFmax so runetochar is safe */
    s = att->value = malloc(b - a + 1);
    while (a < b) {
        if (*a == '&') {
            a += xml_parse_entity(&c, a);
            s += runetochar(s, c);
        } else {
            *s++ = *a++;
        }
    }
    *s = 0;
}

static void xml_emit_close_tag(void)
{
    g.depth--;
    if (g.head->up)
        g.head = g.head->up;
}

static void xml_emit_text(char *a, char *b)
{
    static char *empty = "";
    struct xml *head;
    char *s;
    int c;

    /* Skip text outside the root tag */
    if (g.depth == 0)
        return;

    /* Skip all-whitespace text nodes */
    if (!g.preserve_white) {
        for (s = a; s < b; s++)
            if (!is_white(*s))
                break;
        if (s == b)
            return;
    }

    xml_emit_open_tag(empty, empty);
    head = g.head;

    /* entities are all longer than UTFmax so runetochar is safe */
    s = head->text = malloc(b - a + 1);
    while (a < b) {
        if (*a == '&') {
            a += xml_parse_entity(&c, a);
            s += runetochar(s, c);
        } else {
            *s++ = *a++;
        }
    }
    *s = 0;

    xml_emit_close_tag();
}

static void xml_emit_cdata(char *a, char *b)
{
    static char *empty = "";
    struct xml *head;
    char *s;

    xml_emit_open_tag(empty, empty);
    head = g.head;

    s = head->text = malloc(b - a + 1);
    while (a < b)
        *s++ = *a++;
    *s = 0;

    xml_emit_close_tag();
}

static char *xml_parse_imp(char *p)
{
    char *mark;
    int quote;

parse_text:
    mark = p;
    while (*p && *p != '<') ++p;
    if (mark != p) xml_emit_text(mark, p);
    if (*p == '<') { ++p; goto parse_element; }
    return NULL;

parse_element:
    if (*p == '/') { ++p; goto parse_closing_element; }
    if (*p == '!') { ++p; goto parse_comment; }
    if (*p == '?') { ++p; goto parse_processing_instruction; }
    while (is_white(*p)) ++p;
    if (isname(*p))
        goto parse_element_name;
    return "syntax error in element";

parse_comment:
    if (*p == '[') goto parse_cdata;
    if (*p == 'D' && !memcmp(p, "DOCTYPE", 7)) goto parse_declaration;
    if (*p == 'E' && !memcmp(p, "ENTITY", 6)) goto parse_declaration;
    if (*p++ != '-') return "syntax error in comment (<! not followed by --)";
    if (*p++ != '-') return "syntax error in comment (<!- not followed by -)";
    while (*p) {
        if (p[0] == '-' && p[1] == '-' && p[2] == '>') {
            p += 3;
            goto parse_text;
        }
        ++p;
    }
    return "end of data in comment";

parse_declaration:
    while (*p) if (*p++ == '>') goto parse_text;
    return "end of data in declaration";

parse_cdata:
    if (p[1] != 'C' || p[2] != 'D' || p[3] != 'A' || p[4] != 'T' || p[5] != 'A' || p[6] != '[')
        return "syntax error in CDATA section";
    p += 7;
    mark = p;
    while (*p) {
        if (p[0] == ']' && p[1] == ']' && p[2] == '>') {
            xml_emit_cdata(mark, p);
            p += 3;
            goto parse_text;
        }
        ++p;
    }
    return "end of data in CDATA section";

parse_processing_instruction:
    while (*p) {
        if (p[0] == '?' && p[1] == '>') {
            p += 2;
            goto parse_text;
        }
        ++p;
    }
    return "end of data in processing instruction";

parse_closing_element:
    while (is_white(*p)) ++p;
    while (isname(*p)) ++p;
    while (is_white(*p)) ++p;
    if (*p != '>')
        return "syntax error in closing element";
    xml_emit_close_tag();
    ++p;
    goto parse_text;

parse_element_name:
    mark = p;
    while (isname(*p)) ++p;
    xml_emit_open_tag(mark, p);
    if (*p == '>') { ++p; goto parse_text; }
    if (p[0] == '/' && p[1] == '>') {
        xml_emit_close_tag();
        p += 2;
        goto parse_text;
    }
    if (is_white(*p))
        goto parse_attributes;
    return "syntax error after element name";

parse_attributes:
    while (is_white(*p)) ++p;
    if (isname(*p))
        goto parse_attribute_name;
    if (*p == '>') { ++p; goto parse_text; }
    if (p[0] == '/' && p[1] == '>') {
        xml_emit_close_tag();
        p += 2;
        goto parse_text;
    }
    return "syntax error in attributes";

parse_attribute_name:
    mark = p;
    while (isname(*p)) ++p;
    xml_emit_att_name(mark, p);
    while (is_white(*p)) ++p;
    if (*p == '=') { ++p; goto parse_attribute_value; }
    return "syntax error after attribute name";

parse_attribute_value:
    while (is_white(*p)) ++p;
    quote = *p++;
    if (quote != '"' && quote != '\'')
        return "missing quote character";
    mark = p;
    while (*p && *p != quote) ++p;
    if (*p == quote) {
        xml_emit_att_value(mark, p++);
        goto parse_attributes;
    }
    return "end of data in attribute value";
}

struct xml *xml_parse(char *s, int preserve_white, char **errorp)
{
    struct xml root, *node;
    char *error;

    memset(&root, 0, sizeof root);
    g.head = &root;
    g.preserve_white = preserve_white;
    g.depth = 0;

    error = xml_parse_imp(s);
    if (error) {
        if (errorp)
            *errorp = error;
        xml_free(root.down);
        return NULL;
    }

    for (node = root.down; node; node = node->next)
        node->up = NULL;

    if (errorp)
        *errorp = NULL;
    return root.down;
}

#endif
