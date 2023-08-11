// XLSX exporter v1.04. (tags: xlsx, ini, c++11, i18n, L10n)
// - rlyeh, public domain

// This tool exports any given xlsx:
//
//   ,________,_______,
//   | sheet1 \ sheet2 \ ... \
//   |---------------------------------------------------
//  1| LABEL    | NOTES    | ENGLISH   | SPANISH   | ...
//   |----------+----------+-----------+-----------+-----
//  2| ID_HI    | Informal | Hi!       | Ey!       | ...
//  3| ID_HELLO | Formal   | Hello {0} | Hola {0}  | ...
// ..| ...      | ...      | ...       | ...       | ...
//
// as:
// [xlsx2ini]
// ENGLISH.ID_HI=Hi!
// ENGLISH.ID_HELLO=Hello {0}
// SPANISH.ID_HI=Ey!
// SPANISH.ID_HELLO=Hola {0}
// ...

// Features:
// - [x] parses xlsx
// - [x] parses all sheets
// - [x] exports .ini and .cpp files; both utf8
// - [x] column header (@COL:1) must be a text identifier
// - [x] columns are skipped if no column header is present (blank cell @COL:1)

// Build:
// - [x] osx: clang++ xlsx2ini.cc -o xlsx2ini.osx -O2 -std=c++11 -stdlib=libc++
// - [x] win: cl xlsx2ini.cc /MD /Ox /Oy /DNDEBUG /EHsc

#define VERSION "1.04"

// Including pugixml and miniz now.
// Check real program at bottom of file.

// #define PUGIXML_NO_STL
#define PUGIXML_HAS_LONG_LONG
#define PUGIXML_NO_EXCEPTIONS
#define PUGIXML_HEADER_ONLY
#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
#define MINIZ_LITTLE_ENDIAN                  1
#define MINIZ_HAS_64BIT_REGISTERS            1

/*
#pragma comment(lib, "advapi32")
#include <stdlib.h>
#define ENABLE_OVERRIDE 1
#include "rpmalloc/rpmalloc.c"
*/

// ----------------------
// pugixml.hpp

/**
 * pugixml parser - version 1.12
 * --------------------------------------------------------
 * Copyright (C) 2006-2022, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
 * Report bugs and download new versions at https://pugixml.org/
 *
 * This library is distributed under the MIT License. See notice at the end
 * of this file.
 *
 * This work is based on the pugxml parser, which is:
 * Copyright (C) 2003, by Kristen Wegner (kristen@tima.net)
 */

// Define version macro; evaluates to major * 1000 + minor * 10 + patch so that it's safe to use in less-than comparisons
// Note: pugixml used major * 100 + minor * 10 + patch format up until 1.9 (which had version identifier 190); starting from pugixml 1.10, the minor version number is two digits
#ifndef PUGIXML_VERSION
#   define PUGIXML_VERSION 1120 // 1.12
#endif

// Uncomment this to enable wchar_t mode
// #define PUGIXML_WCHAR_MODE

// Uncomment this to enable compact mode
// #define PUGIXML_COMPACT

// Uncomment this to disable XPath
// #define PUGIXML_NO_XPATH

// Uncomment this to disable STL
// #define PUGIXML_NO_STL

// Uncomment this to disable exceptions
// #define PUGIXML_NO_EXCEPTIONS

// Set this to control attributes for public classes/functions, i.e.:
// #define PUGIXML_API __declspec(dllexport) // to export all public symbols from DLL
// #define PUGIXML_CLASS __declspec(dllimport) // to import all classes from DLL
// #define PUGIXML_FUNCTION __fastcall // to set calling conventions to all public functions to fastcall
// In absence of PUGIXML_CLASS/PUGIXML_FUNCTION definitions PUGIXML_API is used instead

// Tune these constants to adjust memory-related behavior
// #define PUGIXML_MEMORY_PAGE_SIZE 32768
// #define PUGIXML_MEMORY_OUTPUT_STACK 10240
// #define PUGIXML_MEMORY_XPATH_PAGE_SIZE 4096

// Tune this constant to adjust max nesting for XPath queries
// #define PUGIXML_XPATH_DEPTH_LIMIT 1024

// Uncomment this to switch to header-only version
// #define PUGIXML_HEADER_ONLY

// Uncomment this to enable long long support
// #define PUGIXML_HAS_LONG_LONG


#ifndef HEADER_PUGIXML_HPP
#define HEADER_PUGIXML_HPP

// Include stddef.h for size_t and ptrdiff_t
#include <stddef.h>

// Include exception header for XPath
#if !defined(PUGIXML_NO_XPATH) && !defined(PUGIXML_NO_EXCEPTIONS)
#   include <exception>
#endif

// Include STL headers
#ifndef PUGIXML_NO_STL
#   include <iterator>
#   include <iosfwd>
#   include <string>
#endif

// Macro for deprecated features
#ifndef PUGIXML_DEPRECATED
#   if defined(__GNUC__)
#       define PUGIXML_DEPRECATED __attribute__((deprecated))
#   elif defined(_MSC_VER) && _MSC_VER >= 1300
#       define PUGIXML_DEPRECATED __declspec(deprecated)
#   else
#       define PUGIXML_DEPRECATED
#   endif
#endif

// If no API is defined, assume default
#ifndef PUGIXML_API
#   define PUGIXML_API
#endif

// If no API for classes is defined, assume default
#ifndef PUGIXML_CLASS
#   define PUGIXML_CLASS PUGIXML_API
#endif

// If no API for functions is defined, assume default
#ifndef PUGIXML_FUNCTION
#   define PUGIXML_FUNCTION PUGIXML_API
#endif

// If the platform is known to have long long support, enable long long functions
#ifndef PUGIXML_HAS_LONG_LONG
#   if __cplusplus >= 201103
#       define PUGIXML_HAS_LONG_LONG
#   elif defined(_MSC_VER) && _MSC_VER >= 1400
#       define PUGIXML_HAS_LONG_LONG
#   endif
#endif

// If the platform is known to have move semantics support, compile move ctor/operator implementation
#ifndef PUGIXML_HAS_MOVE
#   if __cplusplus >= 201103
#       define PUGIXML_HAS_MOVE
#   elif defined(_MSC_VER) && _MSC_VER >= 1600
#       define PUGIXML_HAS_MOVE
#   endif
#endif

// If C++ is 2011 or higher, add 'noexcept' specifiers
#ifndef PUGIXML_NOEXCEPT
#   if __cplusplus >= 201103
#       define PUGIXML_NOEXCEPT noexcept
#   elif defined(_MSC_VER) && _MSC_VER >= 1900
#       define PUGIXML_NOEXCEPT noexcept
#   else
#       define PUGIXML_NOEXCEPT
#   endif
#endif

// Some functions can not be noexcept in compact mode
#ifdef PUGIXML_COMPACT
#   define PUGIXML_NOEXCEPT_IF_NOT_COMPACT
#else
#   define PUGIXML_NOEXCEPT_IF_NOT_COMPACT PUGIXML_NOEXCEPT
#endif

// If C++ is 2011 or higher, add 'override' qualifiers
#ifndef PUGIXML_OVERRIDE
#   if __cplusplus >= 201103
#       define PUGIXML_OVERRIDE override
#   elif defined(_MSC_VER) && _MSC_VER >= 1700
#       define PUGIXML_OVERRIDE override
#   else
#       define PUGIXML_OVERRIDE
#   endif
#endif

// If C++ is 2011 or higher, use 'nullptr'
#ifndef PUGIXML_NULL
#   if __cplusplus >= 201103
#       define PUGIXML_NULL nullptr
#   else
#       define PUGIXML_NULL 0
#   endif
#endif

// Character interface macros
#ifdef PUGIXML_WCHAR_MODE
#   define PUGIXML_TEXT(t) L ## t
#   define PUGIXML_CHAR wchar_t
#else
#   define PUGIXML_TEXT(t) t
#   define PUGIXML_CHAR char
#endif

namespace pugi
{
    // Character type used for all internal storage and operations; depends on PUGIXML_WCHAR_MODE
    typedef PUGIXML_CHAR char_t;

#ifndef PUGIXML_NO_STL
    // String type used for operations that work with STL string; depends on PUGIXML_WCHAR_MODE
    typedef std::basic_string<PUGIXML_CHAR, std::char_traits<PUGIXML_CHAR>, std::allocator<PUGIXML_CHAR> > string_t;
#endif
}

// The PugiXML namespace
namespace pugi
{
    // Tree node types
    enum xml_node_type
    {
        node_null,          // Empty (null) node handle
        node_document,      // A document tree's absolute root
        node_element,       // Element tag, i.e. '<node/>'
        node_pcdata,        // Plain character data, i.e. 'text'
        node_cdata,         // Character data, i.e. '<![CDATA[text]]>'
        node_comment,       // Comment tag, i.e. '<!-- text -->'
        node_pi,            // Processing instruction, i.e. '<?name?>'
        node_declaration,   // Document declaration, i.e. '<?xml version="1.0"?>'
        node_doctype        // Document type declaration, i.e. '<!DOCTYPE doc>'
    };

    // Parsing options

    // Minimal parsing mode (equivalent to turning all other flags off).
    // Only elements and PCDATA sections are added to the DOM tree, no text conversions are performed.
    const unsigned int parse_minimal = 0x0000;

    // This flag determines if processing instructions (node_pi) are added to the DOM tree. This flag is off by default.
    const unsigned int parse_pi = 0x0001;

    // This flag determines if comments (node_comment) are added to the DOM tree. This flag is off by default.
    const unsigned int parse_comments = 0x0002;

    // This flag determines if CDATA sections (node_cdata) are added to the DOM tree. This flag is on by default.
    const unsigned int parse_cdata = 0x0004;

    // This flag determines if plain character data (node_pcdata) that consist only of whitespace are added to the DOM tree.
    // This flag is off by default; turning it on usually results in slower parsing and more memory consumption.
    const unsigned int parse_ws_pcdata = 0x0008;

    // This flag determines if character and entity references are expanded during parsing. This flag is on by default.
    const unsigned int parse_escapes = 0x0010;

    // This flag determines if EOL characters are normalized (converted to #xA) during parsing. This flag is on by default.
    const unsigned int parse_eol = 0x0020;

    // This flag determines if attribute values are normalized using CDATA normalization rules during parsing. This flag is on by default.
    const unsigned int parse_wconv_attribute = 0x0040;

    // This flag determines if attribute values are normalized using NMTOKENS normalization rules during parsing. This flag is off by default.
    const unsigned int parse_wnorm_attribute = 0x0080;

    // This flag determines if document declaration (node_declaration) is added to the DOM tree. This flag is off by default.
    const unsigned int parse_declaration = 0x0100;

    // This flag determines if document type declaration (node_doctype) is added to the DOM tree. This flag is off by default.
    const unsigned int parse_doctype = 0x0200;

    // This flag determines if plain character data (node_pcdata) that is the only child of the parent node and that consists only
    // of whitespace is added to the DOM tree.
    // This flag is off by default; turning it on may result in slower parsing and more memory consumption.
    const unsigned int parse_ws_pcdata_single = 0x0400;

    // This flag determines if leading and trailing whitespace is to be removed from plain character data. This flag is off by default.
    const unsigned int parse_trim_pcdata = 0x0800;

    // This flag determines if plain character data that does not have a parent node is added to the DOM tree, and if an empty document
    // is a valid document. This flag is off by default.
    const unsigned int parse_fragment = 0x1000;

    // This flag determines if plain character data is be stored in the parent element's value. This significantly changes the structure of
    // the document; this flag is only recommended for parsing documents with many PCDATA nodes in memory-constrained environments.
    // This flag is off by default.
    const unsigned int parse_embed_pcdata = 0x2000;

    // The default parsing mode.
    // Elements, PCDATA and CDATA sections are added to the DOM tree, character/reference entities are expanded,
    // End-of-Line characters are normalized, attribute values are normalized using CDATA normalization rules.
    const unsigned int parse_default = parse_cdata | parse_escapes | parse_wconv_attribute | parse_eol;

    // The full parsing mode.
    // Nodes of all types are added to the DOM tree, character/reference entities are expanded,
    // End-of-Line characters are normalized, attribute values are normalized using CDATA normalization rules.
    const unsigned int parse_full = parse_default | parse_pi | parse_comments | parse_declaration | parse_doctype;

    // These flags determine the encoding of input data for XML document
    enum xml_encoding
    {
        encoding_auto,      // Auto-detect input encoding using BOM or < / <? detection; use UTF8 if BOM is not found
        encoding_utf8,      // UTF8 encoding
        encoding_utf16_le,  // Little-endian UTF16
        encoding_utf16_be,  // Big-endian UTF16
        encoding_utf16,     // UTF16 with native endianness
        encoding_utf32_le,  // Little-endian UTF32
        encoding_utf32_be,  // Big-endian UTF32
        encoding_utf32,     // UTF32 with native endianness
        encoding_wchar,     // The same encoding wchar_t has (either UTF16 or UTF32)
        encoding_latin1
    };

    // Formatting flags

    // Indent the nodes that are written to output stream with as many indentation strings as deep the node is in DOM tree. This flag is on by default.
    const unsigned int format_indent = 0x01;

    // Write encoding-specific BOM to the output stream. This flag is off by default.
    const unsigned int format_write_bom = 0x02;

    // Use raw output mode (no indentation and no line breaks are written). This flag is off by default.
    const unsigned int format_raw = 0x04;

    // Omit default XML declaration even if there is no declaration in the document. This flag is off by default.
    const unsigned int format_no_declaration = 0x08;

    // Don't escape attribute values and PCDATA contents. This flag is off by default.
    const unsigned int format_no_escapes = 0x10;

    // Open file using text mode in xml_document::save_file. This enables special character (i.e. new-line) conversions on some systems. This flag is off by default.
    const unsigned int format_save_file_text = 0x20;

    // Write every attribute on a new line with appropriate indentation. This flag is off by default.
    const unsigned int format_indent_attributes = 0x40;

    // Don't output empty element tags, instead writing an explicit start and end tag even if there are no children. This flag is off by default.
    const unsigned int format_no_empty_element_tags = 0x80;

    // Skip characters belonging to range [0; 32) instead of "&#xNN;" encoding. This flag is off by default.
    const unsigned int format_skip_control_chars = 0x100;

    // Use single quotes ' instead of double quotes " for enclosing attribute values. This flag is off by default.
    const unsigned int format_attribute_single_quote = 0x200;

    // The default set of formatting flags.
    // Nodes are indented depending on their depth in DOM tree, a default declaration is output if document has none.
    const unsigned int format_default = format_indent;

    const int default_double_precision = 17;
    const int default_float_precision = 9;

    // Forward declarations
    struct xml_attribute_struct;
    struct xml_node_struct;

    class xml_node_iterator;
    class xml_attribute_iterator;
    class xml_named_node_iterator;

    class xml_tree_walker;

    struct xml_parse_result;

    class xml_node;

    class xml_text;

    #ifndef PUGIXML_NO_XPATH
    class xpath_node;
    class xpath_node_set;
    class xpath_query;
    class xpath_variable_set;
    #endif

    // Range-based for loop support
    template <typename It> class xml_object_range
    {
    public:
        typedef It const_iterator;
        typedef It iterator;

        xml_object_range(It b, It e): _begin(b), _end(e)
        {
        }

        It begin() const { return _begin; }
        It end() const { return _end; }

        bool empty() const { return _begin == _end; }

    private:
        It _begin, _end;
    };

    // Writer interface for node printing (see xml_node::print)
    class PUGIXML_CLASS xml_writer
    {
    public:
        virtual ~xml_writer() {}

        // Write memory chunk into stream/file/whatever
        virtual void write(const void* data, size_t size) = 0;
    };

    // xml_writer implementation for FILE*
    class PUGIXML_CLASS xml_writer_file: public xml_writer
    {
    public:
        // Construct writer from a FILE* object; void* is used to avoid header dependencies on stdio
        xml_writer_file(void* file);

        virtual void write(const void* data, size_t size) PUGIXML_OVERRIDE;

    private:
        void* file;
    };

    #ifndef PUGIXML_NO_STL
    // xml_writer implementation for streams
    class PUGIXML_CLASS xml_writer_stream: public xml_writer
    {
    public:
        // Construct writer from an output stream object
        xml_writer_stream(std::basic_ostream<char, std::char_traits<char> >& stream);
        xml_writer_stream(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& stream);

        virtual void write(const void* data, size_t size) PUGIXML_OVERRIDE;

    private:
        std::basic_ostream<char, std::char_traits<char> >* narrow_stream;
        std::basic_ostream<wchar_t, std::char_traits<wchar_t> >* wide_stream;
    };
    #endif

    // A light-weight handle for manipulating attributes in DOM tree
    class PUGIXML_CLASS xml_attribute
    {
        friend class xml_attribute_iterator;
        friend class xml_node;

    private:
        xml_attribute_struct* _attr;

        typedef void (*unspecified_bool_type)(xml_attribute***);

    public:
        // Default constructor. Constructs an empty attribute.
        xml_attribute();

        // Constructs attribute from internal pointer
        explicit xml_attribute(xml_attribute_struct* attr);

        // Safe bool conversion operator
        operator unspecified_bool_type() const;

        // Borland C++ workaround
        bool operator!() const;

        // Comparison operators (compares wrapped attribute pointers)
        bool operator==(const xml_attribute& r) const;
        bool operator!=(const xml_attribute& r) const;
        bool operator<(const xml_attribute& r) const;
        bool operator>(const xml_attribute& r) const;
        bool operator<=(const xml_attribute& r) const;
        bool operator>=(const xml_attribute& r) const;

        // Check if attribute is empty
        bool empty() const;

        // Get attribute name/value, or "" if attribute is empty
        const char_t* name() const;
        const char_t* value() const;

        // Get attribute value, or the default value if attribute is empty
        const char_t* as_string(const char_t* def = PUGIXML_TEXT("")) const;

        // Get attribute value as a number, or the default value if conversion did not succeed or attribute is empty
        int as_int(int def = 0) const;
        unsigned int as_uint(unsigned int def = 0) const;
        double as_double(double def = 0) const;
        float as_float(float def = 0) const;

    #ifdef PUGIXML_HAS_LONG_LONG
        long long as_llong(long long def = 0) const;
        unsigned long long as_ullong(unsigned long long def = 0) const;
    #endif

        // Get attribute value as bool (returns true if first character is in '1tTyY' set), or the default value if attribute is empty
        bool as_bool(bool def = false) const;

        // Set attribute name/value (returns false if attribute is empty or there is not enough memory)
        bool set_name(const char_t* rhs);
        bool set_value(const char_t* rhs);

        // Set attribute value with type conversion (numbers are converted to strings, boolean is converted to "true"/"false")
        bool set_value(int rhs);
        bool set_value(unsigned int rhs);
        bool set_value(long rhs);
        bool set_value(unsigned long rhs);
        bool set_value(double rhs);
        bool set_value(double rhs, int precision);
        bool set_value(float rhs);
        bool set_value(float rhs, int precision);
        bool set_value(bool rhs);

    #ifdef PUGIXML_HAS_LONG_LONG
        bool set_value(long long rhs);
        bool set_value(unsigned long long rhs);
    #endif

        // Set attribute value (equivalent to set_value without error checking)
        xml_attribute& operator=(const char_t* rhs);
        xml_attribute& operator=(int rhs);
        xml_attribute& operator=(unsigned int rhs);
        xml_attribute& operator=(long rhs);
        xml_attribute& operator=(unsigned long rhs);
        xml_attribute& operator=(double rhs);
        xml_attribute& operator=(float rhs);
        xml_attribute& operator=(bool rhs);

    #ifdef PUGIXML_HAS_LONG_LONG
        xml_attribute& operator=(long long rhs);
        xml_attribute& operator=(unsigned long long rhs);
    #endif

        // Get next/previous attribute in the attribute list of the parent node
        xml_attribute next_attribute() const;
        xml_attribute previous_attribute() const;

        // Get hash value (unique for handles to the same object)
        size_t hash_value() const;

        // Get internal pointer
        xml_attribute_struct* internal_object() const;
    };

#ifdef __BORLANDC__
    // Borland C++ workaround
    bool PUGIXML_FUNCTION operator&&(const xml_attribute& lhs, bool rhs);
    bool PUGIXML_FUNCTION operator||(const xml_attribute& lhs, bool rhs);
#endif

    // A light-weight handle for manipulating nodes in DOM tree
    class PUGIXML_CLASS xml_node
    {
        friend class xml_attribute_iterator;
        friend class xml_node_iterator;
        friend class xml_named_node_iterator;

    protected:
        xml_node_struct* _root;

        typedef void (*unspecified_bool_type)(xml_node***);

    public:
        // Default constructor. Constructs an empty node.
        xml_node();

        // Constructs node from internal pointer
        explicit xml_node(xml_node_struct* p);

        // Safe bool conversion operator
        operator unspecified_bool_type() const;

        // Borland C++ workaround
        bool operator!() const;

        // Comparison operators (compares wrapped node pointers)
        bool operator==(const xml_node& r) const;
        bool operator!=(const xml_node& r) const;
        bool operator<(const xml_node& r) const;
        bool operator>(const xml_node& r) const;
        bool operator<=(const xml_node& r) const;
        bool operator>=(const xml_node& r) const;

        // Check if node is empty.
        bool empty() const;

        // Get node type
        xml_node_type type() const;

        // Get node name, or "" if node is empty or it has no name
        const char_t* name() const;

        // Get node value, or "" if node is empty or it has no value
        // Note: For <node>text</node> node.value() does not return "text"! Use child_value() or text() methods to access text inside nodes.
        const char_t* value() const;

        // Get attribute list
        xml_attribute first_attribute() const;
        xml_attribute last_attribute() const;

        // Get children list
        xml_node first_child() const;
        xml_node last_child() const;

        // Get next/previous sibling in the children list of the parent node
        xml_node next_sibling() const;
        xml_node previous_sibling() const;

        // Get parent node
        xml_node parent() const;

        // Get root of DOM tree this node belongs to
        xml_node root() const;

        // Get text object for the current node
        xml_text text() const;

        // Get child, attribute or next/previous sibling with the specified name
        xml_node child(const char_t* name) const;
        xml_attribute attribute(const char_t* name) const;
        xml_node next_sibling(const char_t* name) const;
        xml_node previous_sibling(const char_t* name) const;

        // Get attribute, starting the search from a hint (and updating hint so that searching for a sequence of attributes is fast)
        xml_attribute attribute(const char_t* name, xml_attribute& hint) const;

        // Get child value of current node; that is, value of the first child node of type PCDATA/CDATA
        const char_t* child_value() const;

        // Get child value of child with specified name. Equivalent to child(name).child_value().
        const char_t* child_value(const char_t* name) const;

        // Set node name/value (returns false if node is empty, there is not enough memory, or node can not have name/value)
        bool set_name(const char_t* rhs);
        bool set_value(const char_t* rhs);

        // Add attribute with specified name. Returns added attribute, or empty attribute on errors.
        xml_attribute append_attribute(const char_t* name);
        xml_attribute prepend_attribute(const char_t* name);
        xml_attribute insert_attribute_after(const char_t* name, const xml_attribute& attr);
        xml_attribute insert_attribute_before(const char_t* name, const xml_attribute& attr);

        // Add a copy of the specified attribute. Returns added attribute, or empty attribute on errors.
        xml_attribute append_copy(const xml_attribute& proto);
        xml_attribute prepend_copy(const xml_attribute& proto);
        xml_attribute insert_copy_after(const xml_attribute& proto, const xml_attribute& attr);
        xml_attribute insert_copy_before(const xml_attribute& proto, const xml_attribute& attr);

        // Add child node with specified type. Returns added node, or empty node on errors.
        xml_node append_child(xml_node_type type = node_element);
        xml_node prepend_child(xml_node_type type = node_element);
        xml_node insert_child_after(xml_node_type type, const xml_node& node);
        xml_node insert_child_before(xml_node_type type, const xml_node& node);

        // Add child element with specified name. Returns added node, or empty node on errors.
        xml_node append_child(const char_t* name);
        xml_node prepend_child(const char_t* name);
        xml_node insert_child_after(const char_t* name, const xml_node& node);
        xml_node insert_child_before(const char_t* name, const xml_node& node);

        // Add a copy of the specified node as a child. Returns added node, or empty node on errors.
        xml_node append_copy(const xml_node& proto);
        xml_node prepend_copy(const xml_node& proto);
        xml_node insert_copy_after(const xml_node& proto, const xml_node& node);
        xml_node insert_copy_before(const xml_node& proto, const xml_node& node);

        // Move the specified node to become a child of this node. Returns moved node, or empty node on errors.
        xml_node append_move(const xml_node& moved);
        xml_node prepend_move(const xml_node& moved);
        xml_node insert_move_after(const xml_node& moved, const xml_node& node);
        xml_node insert_move_before(const xml_node& moved, const xml_node& node);

        // Remove specified attribute
        bool remove_attribute(const xml_attribute& a);
        bool remove_attribute(const char_t* name);

        // Remove all attributes
        bool remove_attributes();

        // Remove specified child
        bool remove_child(const xml_node& n);
        bool remove_child(const char_t* name);

        // Remove all children
        bool remove_children();

        // Parses buffer as an XML document fragment and appends all nodes as children of the current node.
        // Copies/converts the buffer, so it may be deleted or changed after the function returns.
        // Note: append_buffer allocates memory that has the lifetime of the owning document; removing the appended nodes does not immediately reclaim that memory.
        xml_parse_result append_buffer(const void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

        // Find attribute using predicate. Returns first attribute for which predicate returned true.
        template <typename Predicate> xml_attribute find_attribute(Predicate pred) const
        {
            if (!_root) return xml_attribute();

            for (xml_attribute attrib = first_attribute(); attrib; attrib = attrib.next_attribute())
                if (pred(attrib))
                    return attrib;

            return xml_attribute();
        }

        // Find child node using predicate. Returns first child for which predicate returned true.
        template <typename Predicate> xml_node find_child(Predicate pred) const
        {
            if (!_root) return xml_node();

            for (xml_node node = first_child(); node; node = node.next_sibling())
                if (pred(node))
                    return node;

            return xml_node();
        }

        // Find node from subtree using predicate. Returns first node from subtree (depth-first), for which predicate returned true.
        template <typename Predicate> xml_node find_node(Predicate pred) const
        {
            if (!_root) return xml_node();

            xml_node cur = first_child();

            while (cur._root && cur._root != _root)
            {
                if (pred(cur)) return cur;

                if (cur.first_child()) cur = cur.first_child();
                else if (cur.next_sibling()) cur = cur.next_sibling();
                else
                {
                    while (!cur.next_sibling() && cur._root != _root) cur = cur.parent();

                    if (cur._root != _root) cur = cur.next_sibling();
                }
            }

            return xml_node();
        }

        // Find child node by attribute name/value
        xml_node find_child_by_attribute(const char_t* name, const char_t* attr_name, const char_t* attr_value) const;
        xml_node find_child_by_attribute(const char_t* attr_name, const char_t* attr_value) const;

    #ifndef PUGIXML_NO_STL
        // Get the absolute node path from root as a text string.
        string_t path(char_t delimiter = '/') const;
    #endif

        // Search for a node by path consisting of node names and . or .. elements.
        xml_node first_element_by_path(const char_t* path, char_t delimiter = '/') const;

        // Recursively traverse subtree with xml_tree_walker
        bool traverse(xml_tree_walker& walker);

    #ifndef PUGIXML_NO_XPATH
        // Select single node by evaluating XPath query. Returns first node from the resulting node set.
        xpath_node select_node(const char_t* query, xpath_variable_set* variables = PUGIXML_NULL) const;
        xpath_node select_node(const xpath_query& query) const;

        // Select node set by evaluating XPath query
        xpath_node_set select_nodes(const char_t* query, xpath_variable_set* variables = PUGIXML_NULL) const;
        xpath_node_set select_nodes(const xpath_query& query) const;

        // (deprecated: use select_node instead) Select single node by evaluating XPath query.
        PUGIXML_DEPRECATED xpath_node select_single_node(const char_t* query, xpath_variable_set* variables = PUGIXML_NULL) const;
        PUGIXML_DEPRECATED xpath_node select_single_node(const xpath_query& query) const;

    #endif

        // Print subtree using a writer object
        void print(xml_writer& writer, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto, unsigned int depth = 0) const;

    #ifndef PUGIXML_NO_STL
        // Print subtree to stream
        void print(std::basic_ostream<char, std::char_traits<char> >& os, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto, unsigned int depth = 0) const;
        void print(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& os, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, unsigned int depth = 0) const;
    #endif

        // Child nodes iterators
        typedef xml_node_iterator iterator;

        iterator begin() const;
        iterator end() const;

        // Attribute iterators
        typedef xml_attribute_iterator attribute_iterator;

        attribute_iterator attributes_begin() const;
        attribute_iterator attributes_end() const;

        // Range-based for support
        xml_object_range<xml_node_iterator> children() const;
        xml_object_range<xml_named_node_iterator> children(const char_t* name) const;
        xml_object_range<xml_attribute_iterator> attributes() const;

        // Get node offset in parsed file/string (in char_t units) for debugging purposes
        ptrdiff_t offset_debug() const;

        // Get hash value (unique for handles to the same object)
        size_t hash_value() const;

        // Get internal pointer
        xml_node_struct* internal_object() const;
    };

#ifdef __BORLANDC__
    // Borland C++ workaround
    bool PUGIXML_FUNCTION operator&&(const xml_node& lhs, bool rhs);
    bool PUGIXML_FUNCTION operator||(const xml_node& lhs, bool rhs);
#endif

    // A helper for working with text inside PCDATA nodes
    class PUGIXML_CLASS xml_text
    {
        friend class xml_node;

        xml_node_struct* _root;

        typedef void (*unspecified_bool_type)(xml_text***);

        explicit xml_text(xml_node_struct* root);

        xml_node_struct* _data_new();
        xml_node_struct* _data() const;

    public:
        // Default constructor. Constructs an empty object.
        xml_text();

        // Safe bool conversion operator
        operator unspecified_bool_type() const;

        // Borland C++ workaround
        bool operator!() const;

        // Check if text object is empty
        bool empty() const;

        // Get text, or "" if object is empty
        const char_t* get() const;

        // Get text, or the default value if object is empty
        const char_t* as_string(const char_t* def = PUGIXML_TEXT("")) const;

        // Get text as a number, or the default value if conversion did not succeed or object is empty
        int as_int(int def = 0) const;
        unsigned int as_uint(unsigned int def = 0) const;
        double as_double(double def = 0) const;
        float as_float(float def = 0) const;

    #ifdef PUGIXML_HAS_LONG_LONG
        long long as_llong(long long def = 0) const;
        unsigned long long as_ullong(unsigned long long def = 0) const;
    #endif

        // Get text as bool (returns true if first character is in '1tTyY' set), or the default value if object is empty
        bool as_bool(bool def = false) const;

        // Set text (returns false if object is empty or there is not enough memory)
        bool set(const char_t* rhs);

        // Set text with type conversion (numbers are converted to strings, boolean is converted to "true"/"false")
        bool set(int rhs);
        bool set(unsigned int rhs);
        bool set(long rhs);
        bool set(unsigned long rhs);
        bool set(double rhs);
        bool set(double rhs, int precision);
        bool set(float rhs);
        bool set(float rhs, int precision);
        bool set(bool rhs);

    #ifdef PUGIXML_HAS_LONG_LONG
        bool set(long long rhs);
        bool set(unsigned long long rhs);
    #endif

        // Set text (equivalent to set without error checking)
        xml_text& operator=(const char_t* rhs);
        xml_text& operator=(int rhs);
        xml_text& operator=(unsigned int rhs);
        xml_text& operator=(long rhs);
        xml_text& operator=(unsigned long rhs);
        xml_text& operator=(double rhs);
        xml_text& operator=(float rhs);
        xml_text& operator=(bool rhs);

    #ifdef PUGIXML_HAS_LONG_LONG
        xml_text& operator=(long long rhs);
        xml_text& operator=(unsigned long long rhs);
    #endif

        // Get the data node (node_pcdata or node_cdata) for this object
        xml_node data() const;
    };

#ifdef __BORLANDC__
    // Borland C++ workaround
    bool PUGIXML_FUNCTION operator&&(const xml_text& lhs, bool rhs);
    bool PUGIXML_FUNCTION operator||(const xml_text& lhs, bool rhs);
#endif

    // Child node iterator (a bidirectional iterator over a collection of xml_node)
    class PUGIXML_CLASS xml_node_iterator
    {
        friend class xml_node;

    private:
        mutable xml_node _wrap;
        xml_node _parent;

        xml_node_iterator(xml_node_struct* ref, xml_node_struct* parent);

    public:
        // Iterator traits
        typedef ptrdiff_t difference_type;
        typedef xml_node value_type;
        typedef xml_node* pointer;
        typedef xml_node& reference;

    #ifndef PUGIXML_NO_STL
        typedef std::bidirectional_iterator_tag iterator_category;
    #endif

        // Default constructor
        xml_node_iterator();

        // Construct an iterator which points to the specified node
        xml_node_iterator(const xml_node& node);

        // Iterator operators
        bool operator==(const xml_node_iterator& rhs) const;
        bool operator!=(const xml_node_iterator& rhs) const;

        xml_node& operator*() const;
        xml_node* operator->() const;

        xml_node_iterator& operator++();
        xml_node_iterator operator++(int);

        xml_node_iterator& operator--();
        xml_node_iterator operator--(int);
    };

    // Attribute iterator (a bidirectional iterator over a collection of xml_attribute)
    class PUGIXML_CLASS xml_attribute_iterator
    {
        friend class xml_node;

    private:
        mutable xml_attribute _wrap;
        xml_node _parent;

        xml_attribute_iterator(xml_attribute_struct* ref, xml_node_struct* parent);

    public:
        // Iterator traits
        typedef ptrdiff_t difference_type;
        typedef xml_attribute value_type;
        typedef xml_attribute* pointer;
        typedef xml_attribute& reference;

    #ifndef PUGIXML_NO_STL
        typedef std::bidirectional_iterator_tag iterator_category;
    #endif

        // Default constructor
        xml_attribute_iterator();

        // Construct an iterator which points to the specified attribute
        xml_attribute_iterator(const xml_attribute& attr, const xml_node& parent);

        // Iterator operators
        bool operator==(const xml_attribute_iterator& rhs) const;
        bool operator!=(const xml_attribute_iterator& rhs) const;

        xml_attribute& operator*() const;
        xml_attribute* operator->() const;

        xml_attribute_iterator& operator++();
        xml_attribute_iterator operator++(int);

        xml_attribute_iterator& operator--();
        xml_attribute_iterator operator--(int);
    };

    // Named node range helper
    class PUGIXML_CLASS xml_named_node_iterator
    {
        friend class xml_node;

    public:
        // Iterator traits
        typedef ptrdiff_t difference_type;
        typedef xml_node value_type;
        typedef xml_node* pointer;
        typedef xml_node& reference;

    #ifndef PUGIXML_NO_STL
        typedef std::bidirectional_iterator_tag iterator_category;
    #endif

        // Default constructor
        xml_named_node_iterator();

        // Construct an iterator which points to the specified node
        xml_named_node_iterator(const xml_node& node, const char_t* name);

        // Iterator operators
        bool operator==(const xml_named_node_iterator& rhs) const;
        bool operator!=(const xml_named_node_iterator& rhs) const;

        xml_node& operator*() const;
        xml_node* operator->() const;

        xml_named_node_iterator& operator++();
        xml_named_node_iterator operator++(int);

        xml_named_node_iterator& operator--();
        xml_named_node_iterator operator--(int);

    private:
        mutable xml_node _wrap;
        xml_node _parent;
        const char_t* _name;

        xml_named_node_iterator(xml_node_struct* ref, xml_node_struct* parent, const char_t* name);
    };

    // Abstract tree walker class (see xml_node::traverse)
    class PUGIXML_CLASS xml_tree_walker
    {
        friend class xml_node;

    private:
        int _depth;

    protected:
        // Get current traversal depth
        int depth() const;

    public:
        xml_tree_walker();
        virtual ~xml_tree_walker();

        // Callback that is called when traversal begins
        virtual bool begin(xml_node& node);

        // Callback that is called for each node traversed
        virtual bool for_each(xml_node& node) = 0;

        // Callback that is called when traversal ends
        virtual bool end(xml_node& node);
    };

    // Parsing status, returned as part of xml_parse_result object
    enum xml_parse_status
    {
        status_ok = 0,              // No error

        status_file_not_found,      // File was not found during load_file()
        status_io_error,            // Error reading from file/stream
        status_out_of_memory,       // Could not allocate memory
        status_internal_error,      // Internal error occurred

        status_unrecognized_tag,    // Parser could not determine tag type

        status_bad_pi,              // Parsing error occurred while parsing document declaration/processing instruction
        status_bad_comment,         // Parsing error occurred while parsing comment
        status_bad_cdata,           // Parsing error occurred while parsing CDATA section
        status_bad_doctype,         // Parsing error occurred while parsing document type declaration
        status_bad_pcdata,          // Parsing error occurred while parsing PCDATA section
        status_bad_start_element,   // Parsing error occurred while parsing start element tag
        status_bad_attribute,       // Parsing error occurred while parsing element attribute
        status_bad_end_element,     // Parsing error occurred while parsing end element tag
        status_end_element_mismatch,// There was a mismatch of start-end tags (closing tag had incorrect name, some tag was not closed or there was an excessive closing tag)

        status_append_invalid_root, // Unable to append nodes since root type is not node_element or node_document (exclusive to xml_node::append_buffer)

        status_no_document_element  // Parsing resulted in a document without element nodes
    };

    // Parsing result
    struct PUGIXML_CLASS xml_parse_result
    {
        // Parsing status (see xml_parse_status)
        xml_parse_status status;

        // Last parsed offset (in char_t units from start of input data)
        ptrdiff_t offset;

        // Source document encoding
        xml_encoding encoding;

        // Default constructor, initializes object to failed state
        xml_parse_result();

        // Cast to bool operator
        operator bool() const;

        // Get error description
        const char* description() const;
    };

    // Document class (DOM tree root)
    class PUGIXML_CLASS xml_document: public xml_node
    {
    private:
        char_t* _buffer;

        char _memory[192];

        // Non-copyable semantics
        xml_document(const xml_document&);
        xml_document& operator=(const xml_document&);

        void _create();
        void _destroy();
        void _move(xml_document& rhs) PUGIXML_NOEXCEPT_IF_NOT_COMPACT;

    public:
        // Default constructor, makes empty document
        xml_document();

        // Destructor, invalidates all node/attribute handles to this document
        ~xml_document();

    #ifdef PUGIXML_HAS_MOVE
        // Move semantics support
        xml_document(xml_document&& rhs) PUGIXML_NOEXCEPT_IF_NOT_COMPACT;
        xml_document& operator=(xml_document&& rhs) PUGIXML_NOEXCEPT_IF_NOT_COMPACT;
    #endif

        // Removes all nodes, leaving the empty document
        void reset();

        // Removes all nodes, then copies the entire contents of the specified document
        void reset(const xml_document& proto);

    #ifndef PUGIXML_NO_STL
        // Load document from stream.
        xml_parse_result load(std::basic_istream<char, std::char_traits<char> >& stream, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);
        xml_parse_result load(std::basic_istream<wchar_t, std::char_traits<wchar_t> >& stream, unsigned int options = parse_default);
    #endif

        // (deprecated: use load_string instead) Load document from zero-terminated string. No encoding conversions are applied.
        PUGIXML_DEPRECATED xml_parse_result load(const char_t* contents, unsigned int options = parse_default);

        // Load document from zero-terminated string. No encoding conversions are applied.
        xml_parse_result load_string(const char_t* contents, unsigned int options = parse_default);

        // Load document from file
        xml_parse_result load_file(const char* path, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);
        xml_parse_result load_file(const wchar_t* path, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

        // Load document from buffer. Copies/converts the buffer, so it may be deleted or changed after the function returns.
        xml_parse_result load_buffer(const void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

        // Load document from buffer, using the buffer for in-place parsing (the buffer is modified and used for storage of document data).
        // You should ensure that buffer data will persist throughout the document's lifetime, and free the buffer memory manually once document is destroyed.
        xml_parse_result load_buffer_inplace(void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

        // Load document from buffer, using the buffer for in-place parsing (the buffer is modified and used for storage of document data).
        // You should allocate the buffer with pugixml allocation function; document will free the buffer when it is no longer needed (you can't use it anymore).
        xml_parse_result load_buffer_inplace_own(void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

        // Save XML document to writer (semantics is slightly different from xml_node::print, see documentation for details).
        void save(xml_writer& writer, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;

    #ifndef PUGIXML_NO_STL
        // Save XML document to stream (semantics is slightly different from xml_node::print, see documentation for details).
        void save(std::basic_ostream<char, std::char_traits<char> >& stream, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;
        void save(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& stream, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default) const;
    #endif

        // Save XML to file
        bool save_file(const char* path, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;
        bool save_file(const wchar_t* path, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;

        // Get document element
        xml_node document_element() const;
    };

#ifndef PUGIXML_NO_XPATH
    // XPath query return type
    enum xpath_value_type
    {
        xpath_type_none,      // Unknown type (query failed to compile)
        xpath_type_node_set,  // Node set (xpath_node_set)
        xpath_type_number,    // Number
        xpath_type_string,    // String
        xpath_type_boolean    // Boolean
    };

    // XPath parsing result
    struct PUGIXML_CLASS xpath_parse_result
    {
        // Error message (0 if no error)
        const char* error;

        // Last parsed offset (in char_t units from string start)
        ptrdiff_t offset;

        // Default constructor, initializes object to failed state
        xpath_parse_result();

        // Cast to bool operator
        operator bool() const;

        // Get error description
        const char* description() const;
    };

    // A single XPath variable
    class PUGIXML_CLASS xpath_variable
    {
        friend class xpath_variable_set;

    protected:
        xpath_value_type _type;
        xpath_variable* _next;

        xpath_variable(xpath_value_type type);

        // Non-copyable semantics
        xpath_variable(const xpath_variable&);
        xpath_variable& operator=(const xpath_variable&);

    public:
        // Get variable name
        const char_t* name() const;

        // Get variable type
        xpath_value_type type() const;

        // Get variable value; no type conversion is performed, default value (false, NaN, empty string, empty node set) is returned on type mismatch error
        bool get_boolean() const;
        double get_number() const;
        const char_t* get_string() const;
        const xpath_node_set& get_node_set() const;

        // Set variable value; no type conversion is performed, false is returned on type mismatch error
        bool set(bool value);
        bool set(double value);
        bool set(const char_t* value);
        bool set(const xpath_node_set& value);
    };

    // A set of XPath variables
    class PUGIXML_CLASS xpath_variable_set
    {
    private:
        xpath_variable* _data[64];

        void _assign(const xpath_variable_set& rhs);
        void _swap(xpath_variable_set& rhs);

        xpath_variable* _find(const char_t* name) const;

        static bool _clone(xpath_variable* var, xpath_variable** out_result);
        static void _destroy(xpath_variable* var);

    public:
        // Default constructor/destructor
        xpath_variable_set();
        ~xpath_variable_set();

        // Copy constructor/assignment operator
        xpath_variable_set(const xpath_variable_set& rhs);
        xpath_variable_set& operator=(const xpath_variable_set& rhs);

    #ifdef PUGIXML_HAS_MOVE
        // Move semantics support
        xpath_variable_set(xpath_variable_set&& rhs) PUGIXML_NOEXCEPT;
        xpath_variable_set& operator=(xpath_variable_set&& rhs) PUGIXML_NOEXCEPT;
    #endif

        // Add a new variable or get the existing one, if the types match
        xpath_variable* add(const char_t* name, xpath_value_type type);

        // Set value of an existing variable; no type conversion is performed, false is returned if there is no such variable or if types mismatch
        bool set(const char_t* name, bool value);
        bool set(const char_t* name, double value);
        bool set(const char_t* name, const char_t* value);
        bool set(const char_t* name, const xpath_node_set& value);

        // Get existing variable by name
        xpath_variable* get(const char_t* name);
        const xpath_variable* get(const char_t* name) const;
    };

    // A compiled XPath query object
    class PUGIXML_CLASS xpath_query
    {
    private:
        void* _impl;
        xpath_parse_result _result;

        typedef void (*unspecified_bool_type)(xpath_query***);

        // Non-copyable semantics
        xpath_query(const xpath_query&);
        xpath_query& operator=(const xpath_query&);

    public:
        // Construct a compiled object from XPath expression.
        // If PUGIXML_NO_EXCEPTIONS is not defined, throws xpath_exception on compilation errors.
        explicit xpath_query(const char_t* query, xpath_variable_set* variables = PUGIXML_NULL);

        // Constructor
        xpath_query();

        // Destructor
        ~xpath_query();

    #ifdef PUGIXML_HAS_MOVE
        // Move semantics support
        xpath_query(xpath_query&& rhs) PUGIXML_NOEXCEPT;
        xpath_query& operator=(xpath_query&& rhs) PUGIXML_NOEXCEPT;
    #endif

        // Get query expression return type
        xpath_value_type return_type() const;

        // Evaluate expression as boolean value in the specified context; performs type conversion if necessary.
        // If PUGIXML_NO_EXCEPTIONS is not defined, throws std::bad_alloc on out of memory errors.
        bool evaluate_boolean(const xpath_node& n) const;

        // Evaluate expression as double value in the specified context; performs type conversion if necessary.
        // If PUGIXML_NO_EXCEPTIONS is not defined, throws std::bad_alloc on out of memory errors.
        double evaluate_number(const xpath_node& n) const;

    #ifndef PUGIXML_NO_STL
        // Evaluate expression as string value in the specified context; performs type conversion if necessary.
        // If PUGIXML_NO_EXCEPTIONS is not defined, throws std::bad_alloc on out of memory errors.
        string_t evaluate_string(const xpath_node& n) const;
    #endif

        // Evaluate expression as string value in the specified context; performs type conversion if necessary.
        // At most capacity characters are written to the destination buffer, full result size is returned (includes terminating zero).
        // If PUGIXML_NO_EXCEPTIONS is not defined, throws std::bad_alloc on out of memory errors.
        // If PUGIXML_NO_EXCEPTIONS is defined, returns empty  set instead.
        size_t evaluate_string(char_t* buffer, size_t capacity, const xpath_node& n) const;

        // Evaluate expression as node set in the specified context.
        // If PUGIXML_NO_EXCEPTIONS is not defined, throws xpath_exception on type mismatch and std::bad_alloc on out of memory errors.
        // If PUGIXML_NO_EXCEPTIONS is defined, returns empty node set instead.
        xpath_node_set evaluate_node_set(const xpath_node& n) const;

        // Evaluate expression as node set in the specified context.
        // Return first node in document order, or empty node if node set is empty.
        // If PUGIXML_NO_EXCEPTIONS is not defined, throws xpath_exception on type mismatch and std::bad_alloc on out of memory errors.
        // If PUGIXML_NO_EXCEPTIONS is defined, returns empty node instead.
        xpath_node evaluate_node(const xpath_node& n) const;

        // Get parsing result (used to get compilation errors in PUGIXML_NO_EXCEPTIONS mode)
        const xpath_parse_result& result() const;

        // Safe bool conversion operator
        operator unspecified_bool_type() const;

        // Borland C++ workaround
        bool operator!() const;
    };

    #ifndef PUGIXML_NO_EXCEPTIONS
        #if defined(_MSC_VER)
          // C4275 can be ignored in Visual C++ if you are deriving
          // from a type in the Standard C++ Library
          #pragma warning(push)
          #pragma warning(disable: 4275)
        #endif
    // XPath exception class
    class PUGIXML_CLASS xpath_exception: public std::exception
    {
    private:
        xpath_parse_result _result;

    public:
        // Construct exception from parse result
        explicit xpath_exception(const xpath_parse_result& result);

        // Get error message
        virtual const char* what() const throw() PUGIXML_OVERRIDE;

        // Get parse result
        const xpath_parse_result& result() const;
    };
        #if defined(_MSC_VER)
          #pragma warning(pop)
        #endif
    #endif

    // XPath node class (either xml_node or xml_attribute)
    class PUGIXML_CLASS xpath_node
    {
    private:
        xml_node _node;
        xml_attribute _attribute;

        typedef void (*unspecified_bool_type)(xpath_node***);

    public:
        // Default constructor; constructs empty XPath node
        xpath_node();

        // Construct XPath node from XML node/attribute
        xpath_node(const xml_node& node);
        xpath_node(const xml_attribute& attribute, const xml_node& parent);

        // Get node/attribute, if any
        xml_node node() const;
        xml_attribute attribute() const;

        // Get parent of contained node/attribute
        xml_node parent() const;

        // Safe bool conversion operator
        operator unspecified_bool_type() const;

        // Borland C++ workaround
        bool operator!() const;

        // Comparison operators
        bool operator==(const xpath_node& n) const;
        bool operator!=(const xpath_node& n) const;
    };

#ifdef __BORLANDC__
    // Borland C++ workaround
    bool PUGIXML_FUNCTION operator&&(const xpath_node& lhs, bool rhs);
    bool PUGIXML_FUNCTION operator||(const xpath_node& lhs, bool rhs);
#endif

    // A fixed-size collection of XPath nodes
    class PUGIXML_CLASS xpath_node_set
    {
    public:
        // Collection type
        enum type_t
        {
            type_unsorted,          // Not ordered
            type_sorted,            // Sorted by document order (ascending)
            type_sorted_reverse     // Sorted by document order (descending)
        };

        // Constant iterator type
        typedef const xpath_node* const_iterator;

        // We define non-constant iterator to be the same as constant iterator so that various generic algorithms (i.e. boost foreach) work
        typedef const xpath_node* iterator;

        // Default constructor. Constructs empty set.
        xpath_node_set();

        // Constructs a set from iterator range; data is not checked for duplicates and is not sorted according to provided type, so be careful
        xpath_node_set(const_iterator begin, const_iterator end, type_t type = type_unsorted);

        // Destructor
        ~xpath_node_set();

        // Copy constructor/assignment operator
        xpath_node_set(const xpath_node_set& ns);
        xpath_node_set& operator=(const xpath_node_set& ns);

    #ifdef PUGIXML_HAS_MOVE
        // Move semantics support
        xpath_node_set(xpath_node_set&& rhs) PUGIXML_NOEXCEPT;
        xpath_node_set& operator=(xpath_node_set&& rhs) PUGIXML_NOEXCEPT;
    #endif

        // Get collection type
        type_t type() const;

        // Get collection size
        size_t size() const;

        // Indexing operator
        const xpath_node& operator[](size_t index) const;

        // Collection iterators
        const_iterator begin() const;
        const_iterator end() const;

        // Sort the collection in ascending/descending order by document order
        void sort(bool reverse = false);

        // Get first node in the collection by document order
        xpath_node first() const;

        // Check if collection is empty
        bool empty() const;

    private:
        type_t _type;

        xpath_node _storage[1];

        xpath_node* _begin;
        xpath_node* _end;

        void _assign(const_iterator begin, const_iterator end, type_t type);
        void _move(xpath_node_set& rhs) PUGIXML_NOEXCEPT;
    };
#endif

#ifndef PUGIXML_NO_STL
    // Convert wide string to UTF8
    std::basic_string<char, std::char_traits<char>, std::allocator<char> > PUGIXML_FUNCTION as_utf8(const wchar_t* str);
    std::basic_string<char, std::char_traits<char>, std::allocator<char> > PUGIXML_FUNCTION as_utf8(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >& str);

    // Convert UTF8 to wide string
    std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > PUGIXML_FUNCTION as_wide(const char* str);
    std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > PUGIXML_FUNCTION as_wide(const std::basic_string<char, std::char_traits<char>, std::allocator<char> >& str);
#endif

    // Memory allocation function interface; returns pointer to allocated memory or NULL on failure
    typedef void* (*allocation_function)(size_t size);

    // Memory deallocation function interface
    typedef void (*deallocation_function)(void* ptr);

    // Override default memory management functions. All subsequent allocations/deallocations will be performed via supplied functions.
    void PUGIXML_FUNCTION set_memory_management_functions(allocation_function allocate, deallocation_function deallocate);

    // Get current memory management functions
    allocation_function PUGIXML_FUNCTION get_memory_allocation_function();
    deallocation_function PUGIXML_FUNCTION get_memory_deallocation_function();
}

#if !defined(PUGIXML_NO_STL) && (defined(_MSC_VER) || defined(__ICC))
namespace std
{
    // Workarounds for (non-standard) iterator category detection for older versions (MSVC7/IC8 and earlier)
    std::bidirectional_iterator_tag PUGIXML_FUNCTION _Iter_cat(const pugi::xml_node_iterator&);
    std::bidirectional_iterator_tag PUGIXML_FUNCTION _Iter_cat(const pugi::xml_attribute_iterator&);
    std::bidirectional_iterator_tag PUGIXML_FUNCTION _Iter_cat(const pugi::xml_named_node_iterator&);
}
#endif

#if !defined(PUGIXML_NO_STL) && defined(__SUNPRO_CC)
namespace std
{
    // Workarounds for (non-standard) iterator category detection
    std::bidirectional_iterator_tag PUGIXML_FUNCTION __iterator_category(const pugi::xml_node_iterator&);
    std::bidirectional_iterator_tag PUGIXML_FUNCTION __iterator_category(const pugi::xml_attribute_iterator&);
    std::bidirectional_iterator_tag PUGIXML_FUNCTION __iterator_category(const pugi::xml_named_node_iterator&);
}
#endif

#endif

// Make sure implementation is included in header-only mode
// Use macro expansion in #include to work around QMake (QTBUG-11923)
#if defined(PUGIXML_HEADER_ONLY) && !defined(PUGIXML_SOURCE)
#   define PUGIXML_SOURCE "pugixml.cpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#ifdef PUGIXML_WCHAR_MODE
#   include <wchar.h>
#endif

#ifndef PUGIXML_NO_XPATH
#   include <math.h>
#   include <float.h>
#endif

#ifndef PUGIXML_NO_STL
#   include <istream>
#   include <ostream>
#   include <string>
#endif

// For placement new
#include <new>

#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable: 4127) // conditional expression is constant
#   pragma warning(disable: 4324) // structure was padded due to __declspec(align())
#   pragma warning(disable: 4702) // unreachable code
#   pragma warning(disable: 4996) // this function or variable may be unsafe
#endif

#if defined(_MSC_VER) && defined(__c2__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated" // this function or variable may be unsafe
#endif

#ifdef __INTEL_COMPILER
#   pragma warning(disable: 177) // function was declared but never referenced
#   pragma warning(disable: 279) // controlling expression is constant
#   pragma warning(disable: 1478 1786) // function was declared "deprecated"
#   pragma warning(disable: 1684) // conversion from pointer to same-sized integral type
#endif

#if defined(__BORLANDC__) && defined(PUGIXML_HEADER_ONLY)
#   pragma warn -8080 // symbol is declared but never used; disabling this inside push/pop bracket does not make the warning go away
#endif

#ifdef __BORLANDC__
#   pragma option push
#   pragma warn -8008 // condition is always false
#   pragma warn -8066 // unreachable code
#endif

#ifdef __SNC__
// Using diag_push/diag_pop does not disable the warnings inside templates due to a compiler bug
#   pragma diag_suppress=178 // function was declared but never referenced
#   pragma diag_suppress=237 // controlling expression is constant
#endif

#ifdef __TI_COMPILER_VERSION__
#   pragma diag_suppress 179 // function was declared but never referenced
#endif

// Inlining controls
#if defined(_MSC_VER) && _MSC_VER >= 1300
#   define PUGI__NO_INLINE __declspec(noinline)
#elif defined(__GNUC__)
#   define PUGI__NO_INLINE __attribute__((noinline))
#else
#   define PUGI__NO_INLINE
#endif

// Branch weight controls
#if defined(__GNUC__) && !defined(__c2__)
#   define PUGI__UNLIKELY(cond) __builtin_expect(cond, 0)
#else
#   define PUGI__UNLIKELY(cond) (cond)
#endif

// Simple static assertion
#define PUGI__STATIC_ASSERT(cond) { static const char condition_failed[(cond) ? 1 : -1] = {0}; (void)condition_failed[0]; }

// Digital Mars C++ bug workaround for passing char loaded from memory via stack
#ifdef __DMC__
#   define PUGI__DMC_VOLATILE volatile
#else
#   define PUGI__DMC_VOLATILE
#endif

// Integer sanitizer workaround; we only apply this for clang since gcc8 has no_sanitize but not unsigned-integer-overflow and produces "attribute directive ignored" warnings
#if defined(__clang__) && defined(__has_attribute)
#   if __has_attribute(no_sanitize)
#       define PUGI__UNSIGNED_OVERFLOW __attribute__((no_sanitize("unsigned-integer-overflow")))
#   else
#       define PUGI__UNSIGNED_OVERFLOW
#   endif
#else
#   define PUGI__UNSIGNED_OVERFLOW
#endif

// Borland C++ bug workaround for not defining ::memcpy depending on header include order (can't always use std::memcpy because some compilers don't have it at all)
#if defined(__BORLANDC__) && !defined(__MEM_H_USING_LIST)
using std::memcpy;
using std::memmove;
using std::memset;
#endif

// Some MinGW/GCC versions have headers that erroneously omit LLONG_MIN/LLONG_MAX/ULLONG_MAX definitions from limits.h in some configurations
#if defined(PUGIXML_HAS_LONG_LONG) && defined(__GNUC__) && !defined(LLONG_MAX) && !defined(LLONG_MIN) && !defined(ULLONG_MAX)
#   define LLONG_MIN (-LLONG_MAX - 1LL)
#   define LLONG_MAX __LONG_LONG_MAX__
#   define ULLONG_MAX (LLONG_MAX * 2ULL + 1ULL)
#endif

// In some environments MSVC is a compiler but the CRT lacks certain MSVC-specific features
#if defined(_MSC_VER) && !defined(__S3E__) && !defined(_WIN32_WCE)
#   define PUGI__MSVC_CRT_VERSION _MSC_VER
#elif defined(_WIN32_WCE)
#   define PUGI__MSVC_CRT_VERSION 1310 // MSVC7.1
#endif

// Not all platforms have snprintf; we define a wrapper that uses snprintf if possible. This only works with buffers with a known size.
#if __cplusplus >= 201103
#   define PUGI__SNPRINTF(buf, ...) snprintf(buf, sizeof(buf), __VA_ARGS__)
#elif defined(PUGI__MSVC_CRT_VERSION) && PUGI__MSVC_CRT_VERSION >= 1400
#   define PUGI__SNPRINTF(buf, ...) _snprintf_s(buf, _countof(buf), _TRUNCATE, __VA_ARGS__)
#else
#   define PUGI__SNPRINTF sprintf
#endif

// We put implementation details into an anonymous namespace in source mode, but have to keep it in non-anonymous namespace in header-only mode to prevent binary bloat.
#ifdef PUGIXML_HEADER_ONLY
#   define PUGI__NS_BEGIN namespace pugi { namespace impl {
#   define PUGI__NS_END } }
#   define PUGI__FN inline
#   define PUGI__FN_NO_INLINE inline
#else
#   if defined(_MSC_VER) && _MSC_VER < 1300 // MSVC6 seems to have an amusing bug with anonymous namespaces inside namespaces
#       define PUGI__NS_BEGIN namespace pugi { namespace impl {
#       define PUGI__NS_END } }
#   else
#       define PUGI__NS_BEGIN namespace pugi { namespace impl { namespace {
#       define PUGI__NS_END } } }
#   endif
#   define PUGI__FN
#   define PUGI__FN_NO_INLINE PUGI__NO_INLINE
#endif

// uintptr_t
#if (defined(_MSC_VER) && _MSC_VER < 1600) || (defined(__BORLANDC__) && __BORLANDC__ < 0x561)
namespace pugi
{
#   ifndef _UINTPTR_T_DEFINED
    typedef size_t uintptr_t;
#   endif

    typedef unsigned __int8 uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
}
#else
#   include <stdint.h>
#endif

// Memory allocation
PUGI__NS_BEGIN
    PUGI__FN void* default_allocate(size_t size)
    {
        return malloc(size);
    }

    PUGI__FN void default_deallocate(void* ptr)
    {
        free(ptr);
    }

    template <typename T>
    struct xml_memory_management_function_storage
    {
        static allocation_function allocate;
        static deallocation_function deallocate;
    };

    // Global allocation functions are stored in class statics so that in header mode linker deduplicates them
    // Without a template<> we'll get multiple definitions of the same static
    template <typename T> allocation_function xml_memory_management_function_storage<T>::allocate = default_allocate;
    template <typename T> deallocation_function xml_memory_management_function_storage<T>::deallocate = default_deallocate;

    typedef xml_memory_management_function_storage<int> xml_memory;
PUGI__NS_END

// String utilities
PUGI__NS_BEGIN
    // Get string length
    PUGI__FN size_t strlength(const char_t* s)
    {
        assert(s);

    #ifdef PUGIXML_WCHAR_MODE
        return wcslen(s);
    #else
        return strlen(s);
    #endif
    }

    // Compare two strings
    PUGI__FN bool strequal(const char_t* src, const char_t* dst)
    {
        assert(src && dst);

    #ifdef PUGIXML_WCHAR_MODE
        return wcscmp(src, dst) == 0;
    #else
        return strcmp(src, dst) == 0;
    #endif
    }

    // Compare lhs with [rhs_begin, rhs_end)
    PUGI__FN bool strequalrange(const char_t* lhs, const char_t* rhs, size_t count)
    {
        for (size_t i = 0; i < count; ++i)
            if (lhs[i] != rhs[i])
                return false;

        return lhs[count] == 0;
    }

    // Get length of wide string, even if CRT lacks wide character support
    PUGI__FN size_t strlength_wide(const wchar_t* s)
    {
        assert(s);

    #ifdef PUGIXML_WCHAR_MODE
        return wcslen(s);
    #else
        const wchar_t* end = s;
        while (*end) end++;
        return static_cast<size_t>(end - s);
    #endif
    }
PUGI__NS_END

// auto_ptr-like object for exception recovery
PUGI__NS_BEGIN
    template <typename T> struct auto_deleter
    {
        typedef void (*D)(T*);

        T* data;
        D deleter;

        auto_deleter(T* data_, D deleter_): data(data_), deleter(deleter_)
        {
        }

        ~auto_deleter()
        {
            if (data) deleter(data);
        }

        T* release()
        {
            T* result = data;
            data = 0;
            return result;
        }
    };
PUGI__NS_END

#ifdef PUGIXML_COMPACT
PUGI__NS_BEGIN
    class compact_hash_table
    {
    public:
        compact_hash_table(): _items(0), _capacity(0), _count(0)
        {
        }

        void clear()
        {
            if (_items)
            {
                xml_memory::deallocate(_items);
                _items = 0;
                _capacity = 0;
                _count = 0;
            }
        }

        void* find(const void* key)
        {
            if (_capacity == 0) return 0;

            item_t* item = get_item(key);
            assert(item);
            assert(item->key == key || (item->key == 0 && item->value == 0));

            return item->value;
        }

        void insert(const void* key, void* value)
        {
            assert(_capacity != 0 && _count < _capacity - _capacity / 4);

            item_t* item = get_item(key);
            assert(item);

            if (item->key == 0)
            {
                _count++;
                item->key = key;
            }

            item->value = value;
        }

        bool reserve(size_t extra = 16)
        {
            if (_count + extra >= _capacity - _capacity / 4)
                return rehash(_count + extra);

            return true;
        }

    private:
        struct item_t
        {
            const void* key;
            void* value;
        };

        item_t* _items;
        size_t _capacity;

        size_t _count;

        bool rehash(size_t count);

        item_t* get_item(const void* key)
        {
            assert(key);
            assert(_capacity > 0);

            size_t hashmod = _capacity - 1;
            size_t bucket = hash(key) & hashmod;

            for (size_t probe = 0; probe <= hashmod; ++probe)
            {
                item_t& probe_item = _items[bucket];

                if (probe_item.key == key || probe_item.key == 0)
                    return &probe_item;

                // hash collision, quadratic probing
                bucket = (bucket + probe + 1) & hashmod;
            }

            assert(false && "Hash table is full"); // unreachable
            return 0;
        }

        static PUGI__UNSIGNED_OVERFLOW unsigned int hash(const void* key)
        {
            unsigned int h = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(key) & 0xffffffff);

            // MurmurHash3 32-bit finalizer
            h ^= h >> 16;
            h *= 0x85ebca6bu;
            h ^= h >> 13;
            h *= 0xc2b2ae35u;
            h ^= h >> 16;

            return h;
        }
    };

    PUGI__FN_NO_INLINE bool compact_hash_table::rehash(size_t count)
    {
        size_t capacity = 32;
        while (count >= capacity - capacity / 4)
            capacity *= 2;

        compact_hash_table rt;
        rt._capacity = capacity;
        rt._items = static_cast<item_t*>(xml_memory::allocate(sizeof(item_t) * capacity));

        if (!rt._items)
            return false;

        memset(rt._items, 0, sizeof(item_t) * capacity);

        for (size_t i = 0; i < _capacity; ++i)
            if (_items[i].key)
                rt.insert(_items[i].key, _items[i].value);

        if (_items)
            xml_memory::deallocate(_items);

        _capacity = capacity;
        _items = rt._items;

        assert(_count == rt._count);

        return true;
    }

PUGI__NS_END
#endif

PUGI__NS_BEGIN
#ifdef PUGIXML_COMPACT
    static const uintptr_t xml_memory_block_alignment = 4;
#else
    static const uintptr_t xml_memory_block_alignment = sizeof(void*);
#endif

    // extra metadata bits
    static const uintptr_t xml_memory_page_contents_shared_mask = 64;
    static const uintptr_t xml_memory_page_name_allocated_mask = 32;
    static const uintptr_t xml_memory_page_value_allocated_mask = 16;
    static const uintptr_t xml_memory_page_type_mask = 15;

    // combined masks for string uniqueness
    static const uintptr_t xml_memory_page_name_allocated_or_shared_mask = xml_memory_page_name_allocated_mask | xml_memory_page_contents_shared_mask;
    static const uintptr_t xml_memory_page_value_allocated_or_shared_mask = xml_memory_page_value_allocated_mask | xml_memory_page_contents_shared_mask;

#ifdef PUGIXML_COMPACT
    #define PUGI__GETHEADER_IMPL(object, page, flags) // unused
    #define PUGI__GETPAGE_IMPL(header) (header).get_page()
#else
    #define PUGI__GETHEADER_IMPL(object, page, flags) (((reinterpret_cast<char*>(object) - reinterpret_cast<char*>(page)) << 8) | (flags))
    // this macro casts pointers through void* to avoid 'cast increases required alignment of target type' warnings
    #define PUGI__GETPAGE_IMPL(header) static_cast<impl::xml_memory_page*>(const_cast<void*>(static_cast<const void*>(reinterpret_cast<const char*>(&header) - (header >> 8))))
#endif

    #define PUGI__GETPAGE(n) PUGI__GETPAGE_IMPL((n)->header)
    #define PUGI__NODETYPE(n) static_cast<xml_node_type>((n)->header & impl::xml_memory_page_type_mask)

    struct xml_allocator;

    struct xml_memory_page
    {
        static xml_memory_page* construct(void* memory)
        {
            xml_memory_page* result = static_cast<xml_memory_page*>(memory);

            result->allocator = 0;
            result->prev = 0;
            result->next = 0;
            result->busy_size = 0;
            result->freed_size = 0;

        #ifdef PUGIXML_COMPACT
            result->compact_string_base = 0;
            result->compact_shared_parent = 0;
            result->compact_page_marker = 0;
        #endif

            return result;
        }

        xml_allocator* allocator;

        xml_memory_page* prev;
        xml_memory_page* next;

        size_t busy_size;
        size_t freed_size;

    #ifdef PUGIXML_COMPACT
        char_t* compact_string_base;
        void* compact_shared_parent;
        uint32_t* compact_page_marker;
    #endif
    };

    static const size_t xml_memory_page_size =
    #ifdef PUGIXML_MEMORY_PAGE_SIZE
        (PUGIXML_MEMORY_PAGE_SIZE)
    #else
        32768
    #endif
        - sizeof(xml_memory_page);

    struct xml_memory_string_header
    {
        uint16_t page_offset; // offset from page->data
        uint16_t full_size; // 0 if string occupies whole page
    };

    struct xml_allocator
    {
        xml_allocator(xml_memory_page* root): _root(root), _busy_size(root->busy_size)
        {
        #ifdef PUGIXML_COMPACT
            _hash = 0;
        #endif
        }

        xml_memory_page* allocate_page(size_t data_size)
        {
            size_t size = sizeof(xml_memory_page) + data_size;

            // allocate block with some alignment, leaving memory for worst-case padding
            void* memory = xml_memory::allocate(size);
            if (!memory) return 0;

            // prepare page structure
            xml_memory_page* page = xml_memory_page::construct(memory);
            assert(page);

            assert(this == _root->allocator);
            page->allocator = this;

            return page;
        }

        static void deallocate_page(xml_memory_page* page)
        {
            xml_memory::deallocate(page);
        }

        void* allocate_memory_oob(size_t size, xml_memory_page*& out_page);

        void* allocate_memory(size_t size, xml_memory_page*& out_page)
        {
            if (PUGI__UNLIKELY(_busy_size + size > xml_memory_page_size))
                return allocate_memory_oob(size, out_page);

            void* buf = reinterpret_cast<char*>(_root) + sizeof(xml_memory_page) + _busy_size;

            _busy_size += size;

            out_page = _root;

            return buf;
        }

    #ifdef PUGIXML_COMPACT
        void* allocate_object(size_t size, xml_memory_page*& out_page)
        {
            void* result = allocate_memory(size + sizeof(uint32_t), out_page);
            if (!result) return 0;

            // adjust for marker
            ptrdiff_t offset = static_cast<char*>(result) - reinterpret_cast<char*>(out_page->compact_page_marker);

            if (PUGI__UNLIKELY(static_cast<uintptr_t>(offset) >= 256 * xml_memory_block_alignment))
            {
                // insert new marker
                uint32_t* marker = static_cast<uint32_t*>(result);

                *marker = static_cast<uint32_t>(reinterpret_cast<char*>(marker) - reinterpret_cast<char*>(out_page));
                out_page->compact_page_marker = marker;

                // since we don't reuse the page space until we reallocate it, we can just pretend that we freed the marker block
                // this will make sure deallocate_memory correctly tracks the size
                out_page->freed_size += sizeof(uint32_t);

                return marker + 1;
            }
            else
            {
                // roll back uint32_t part
                _busy_size -= sizeof(uint32_t);

                return result;
            }
        }
    #else
        void* allocate_object(size_t size, xml_memory_page*& out_page)
        {
            return allocate_memory(size, out_page);
        }
    #endif

        void deallocate_memory(void* ptr, size_t size, xml_memory_page* page)
        {
            if (page == _root) page->busy_size = _busy_size;

            assert(ptr >= reinterpret_cast<char*>(page) + sizeof(xml_memory_page) && ptr < reinterpret_cast<char*>(page) + sizeof(xml_memory_page) + page->busy_size);
            (void)!ptr;

            page->freed_size += size;
            assert(page->freed_size <= page->busy_size);

            if (page->freed_size == page->busy_size)
            {
                if (page->next == 0)
                {
                    assert(_root == page);

                    // top page freed, just reset sizes
                    page->busy_size = 0;
                    page->freed_size = 0;

                #ifdef PUGIXML_COMPACT
                    // reset compact state to maximize efficiency
                    page->compact_string_base = 0;
                    page->compact_shared_parent = 0;
                    page->compact_page_marker = 0;
                #endif

                    _busy_size = 0;
                }
                else
                {
                    assert(_root != page);
                    assert(page->prev);

                    // remove from the list
                    page->prev->next = page->next;
                    page->next->prev = page->prev;

                    // deallocate
                    deallocate_page(page);
                }
            }
        }

        char_t* allocate_string(size_t length)
        {
            static const size_t max_encoded_offset = (1 << 16) * xml_memory_block_alignment;

            PUGI__STATIC_ASSERT(xml_memory_page_size <= max_encoded_offset);

            // allocate memory for string and header block
            size_t size = sizeof(xml_memory_string_header) + length * sizeof(char_t);

            // round size up to block alignment boundary
            size_t full_size = (size + (xml_memory_block_alignment - 1)) & ~(xml_memory_block_alignment - 1);

            xml_memory_page* page;
            xml_memory_string_header* header = static_cast<xml_memory_string_header*>(allocate_memory(full_size, page));

            if (!header) return 0;

            // setup header
            ptrdiff_t page_offset = reinterpret_cast<char*>(header) - reinterpret_cast<char*>(page) - sizeof(xml_memory_page);

            assert(page_offset % xml_memory_block_alignment == 0);
            assert(page_offset >= 0 && static_cast<size_t>(page_offset) < max_encoded_offset);
            header->page_offset = static_cast<uint16_t>(static_cast<size_t>(page_offset) / xml_memory_block_alignment);

            // full_size == 0 for large strings that occupy the whole page
            assert(full_size % xml_memory_block_alignment == 0);
            assert(full_size < max_encoded_offset || (page->busy_size == full_size && page_offset == 0));
            header->full_size = static_cast<uint16_t>(full_size < max_encoded_offset ? full_size / xml_memory_block_alignment : 0);

            // round-trip through void* to avoid 'cast increases required alignment of target type' warning
            // header is guaranteed a pointer-sized alignment, which should be enough for char_t
            return static_cast<char_t*>(static_cast<void*>(header + 1));
        }

        void deallocate_string(char_t* string)
        {
            // this function casts pointers through void* to avoid 'cast increases required alignment of target type' warnings
            // we're guaranteed the proper (pointer-sized) alignment on the input string if it was allocated via allocate_string

            // get header
            xml_memory_string_header* header = static_cast<xml_memory_string_header*>(static_cast<void*>(string)) - 1;
            assert(header);

            // deallocate
            size_t page_offset = sizeof(xml_memory_page) + header->page_offset * xml_memory_block_alignment;
            xml_memory_page* page = reinterpret_cast<xml_memory_page*>(static_cast<void*>(reinterpret_cast<char*>(header) - page_offset));

            // if full_size == 0 then this string occupies the whole page
            size_t full_size = header->full_size == 0 ? page->busy_size : header->full_size * xml_memory_block_alignment;

            deallocate_memory(header, full_size, page);
        }

        bool reserve()
        {
        #ifdef PUGIXML_COMPACT
            return _hash->reserve();
        #else
            return true;
        #endif
        }

        xml_memory_page* _root;
        size_t _busy_size;

    #ifdef PUGIXML_COMPACT
        compact_hash_table* _hash;
    #endif
    };

    PUGI__FN_NO_INLINE void* xml_allocator::allocate_memory_oob(size_t size, xml_memory_page*& out_page)
    {
        const size_t large_allocation_threshold = xml_memory_page_size / 4;

        xml_memory_page* page = allocate_page(size <= large_allocation_threshold ? xml_memory_page_size : size);
        out_page = page;

        if (!page) return 0;

        if (size <= large_allocation_threshold)
        {
            _root->busy_size = _busy_size;

            // insert page at the end of linked list
            page->prev = _root;
            _root->next = page;
            _root = page;

            _busy_size = size;
        }
        else
        {
            // insert page before the end of linked list, so that it is deleted as soon as possible
            // the last page is not deleted even if it's empty (see deallocate_memory)
            assert(_root->prev);

            page->prev = _root->prev;
            page->next = _root;

            _root->prev->next = page;
            _root->prev = page;

            page->busy_size = size;
        }

        return reinterpret_cast<char*>(page) + sizeof(xml_memory_page);
    }
PUGI__NS_END

#ifdef PUGIXML_COMPACT
PUGI__NS_BEGIN
    static const uintptr_t compact_alignment_log2 = 2;
    static const uintptr_t compact_alignment = 1 << compact_alignment_log2;

    class compact_header
    {
    public:
        compact_header(xml_memory_page* page, unsigned int flags)
        {
            PUGI__STATIC_ASSERT(xml_memory_block_alignment == compact_alignment);

            ptrdiff_t offset = (reinterpret_cast<char*>(this) - reinterpret_cast<char*>(page->compact_page_marker));
            assert(offset % compact_alignment == 0 && static_cast<uintptr_t>(offset) < 256 * compact_alignment);

            _page = static_cast<unsigned char>(offset >> compact_alignment_log2);
            _flags = static_cast<unsigned char>(flags);
        }

        void operator&=(uintptr_t mod)
        {
            _flags &= static_cast<unsigned char>(mod);
        }

        void operator|=(uintptr_t mod)
        {
            _flags |= static_cast<unsigned char>(mod);
        }

        uintptr_t operator&(uintptr_t mod) const
        {
            return _flags & mod;
        }

        xml_memory_page* get_page() const
        {
            // round-trip through void* to silence 'cast increases required alignment of target type' warnings
            const char* page_marker = reinterpret_cast<const char*>(this) - (_page << compact_alignment_log2);
            const char* page = page_marker - *reinterpret_cast<const uint32_t*>(static_cast<const void*>(page_marker));

            return const_cast<xml_memory_page*>(reinterpret_cast<const xml_memory_page*>(static_cast<const void*>(page)));
        }

    private:
        unsigned char _page;
        unsigned char _flags;
    };

    PUGI__FN xml_memory_page* compact_get_page(const void* object, int header_offset)
    {
        const compact_header* header = reinterpret_cast<const compact_header*>(static_cast<const char*>(object) - header_offset);

        return header->get_page();
    }

    template <int header_offset, typename T> PUGI__FN_NO_INLINE T* compact_get_value(const void* object)
    {
        return static_cast<T*>(compact_get_page(object, header_offset)->allocator->_hash->find(object));
    }

    template <int header_offset, typename T> PUGI__FN_NO_INLINE void compact_set_value(const void* object, T* value)
    {
        compact_get_page(object, header_offset)->allocator->_hash->insert(object, value);
    }

    template <typename T, int header_offset, int start = -126> class compact_pointer
    {
    public:
        compact_pointer(): _data(0)
        {
        }

        void operator=(const compact_pointer& rhs)
        {
            *this = rhs + 0;
        }

        void operator=(T* value)
        {
            if (value)
            {
                // value is guaranteed to be compact-aligned; 'this' is not
                // our decoding is based on 'this' aligned to compact alignment downwards (see operator T*)
                // so for negative offsets (e.g. -3) we need to adjust the diff by compact_alignment - 1 to
                // compensate for arithmetic shift rounding for negative values
                ptrdiff_t diff = reinterpret_cast<char*>(value) - reinterpret_cast<char*>(this);
                ptrdiff_t offset = ((diff + int(compact_alignment - 1)) >> compact_alignment_log2) - start;

                if (static_cast<uintptr_t>(offset) <= 253)
                    _data = static_cast<unsigned char>(offset + 1);
                else
                {
                    compact_set_value<header_offset>(this, value);

                    _data = 255;
                }
            }
            else
                _data = 0;
        }

        operator T*() const
        {
            if (_data)
            {
                if (_data < 255)
                {
                    uintptr_t base = reinterpret_cast<uintptr_t>(this) & ~(compact_alignment - 1);

                    return reinterpret_cast<T*>(base + (_data - 1 + start) * compact_alignment);
                }
                else
                    return compact_get_value<header_offset, T>(this);
            }
            else
                return 0;
        }

        T* operator->() const
        {
            return *this;
        }

    private:
        unsigned char _data;
    };

    template <typename T, int header_offset> class compact_pointer_parent
    {
    public:
        compact_pointer_parent(): _data(0)
        {
        }

        void operator=(const compact_pointer_parent& rhs)
        {
            *this = rhs + 0;
        }

        void operator=(T* value)
        {
            if (value)
            {
                // value is guaranteed to be compact-aligned; 'this' is not
                // our decoding is based on 'this' aligned to compact alignment downwards (see operator T*)
                // so for negative offsets (e.g. -3) we need to adjust the diff by compact_alignment - 1 to
                // compensate for arithmetic shift behavior for negative values
                ptrdiff_t diff = reinterpret_cast<char*>(value) - reinterpret_cast<char*>(this);
                ptrdiff_t offset = ((diff + int(compact_alignment - 1)) >> compact_alignment_log2) + 65533;

                if (static_cast<uintptr_t>(offset) <= 65533)
                {
                    _data = static_cast<unsigned short>(offset + 1);
                }
                else
                {
                    xml_memory_page* page = compact_get_page(this, header_offset);

                    if (PUGI__UNLIKELY(page->compact_shared_parent == 0))
                        page->compact_shared_parent = value;

                    if (page->compact_shared_parent == value)
                    {
                        _data = 65534;
                    }
                    else
                    {
                        compact_set_value<header_offset>(this, value);

                        _data = 65535;
                    }
                }
            }
            else
            {
                _data = 0;
            }
        }

        operator T*() const
        {
            if (_data)
            {
                if (_data < 65534)
                {
                    uintptr_t base = reinterpret_cast<uintptr_t>(this) & ~(compact_alignment - 1);

                    return reinterpret_cast<T*>(base + (_data - 1 - 65533) * compact_alignment);
                }
                else if (_data == 65534)
                    return static_cast<T*>(compact_get_page(this, header_offset)->compact_shared_parent);
                else
                    return compact_get_value<header_offset, T>(this);
            }
            else
                return 0;
        }

        T* operator->() const
        {
            return *this;
        }

    private:
        uint16_t _data;
    };

    template <int header_offset, int base_offset> class compact_string
    {
    public:
        compact_string(): _data(0)
        {
        }

        void operator=(const compact_string& rhs)
        {
            *this = rhs + 0;
        }

        void operator=(char_t* value)
        {
            if (value)
            {
                xml_memory_page* page = compact_get_page(this, header_offset);

                if (PUGI__UNLIKELY(page->compact_string_base == 0))
                    page->compact_string_base = value;

                ptrdiff_t offset = value - page->compact_string_base;

                if (static_cast<uintptr_t>(offset) < (65535 << 7))
                {
                    // round-trip through void* to silence 'cast increases required alignment of target type' warnings
                    uint16_t* base = reinterpret_cast<uint16_t*>(static_cast<void*>(reinterpret_cast<char*>(this) - base_offset));

                    if (*base == 0)
                    {
                        *base = static_cast<uint16_t>((offset >> 7) + 1);
                        _data = static_cast<unsigned char>((offset & 127) + 1);
                    }
                    else
                    {
                        ptrdiff_t remainder = offset - ((*base - 1) << 7);

                        if (static_cast<uintptr_t>(remainder) <= 253)
                        {
                            _data = static_cast<unsigned char>(remainder + 1);
                        }
                        else
                        {
                            compact_set_value<header_offset>(this, value);

                            _data = 255;
                        }
                    }
                }
                else
                {
                    compact_set_value<header_offset>(this, value);

                    _data = 255;
                }
            }
            else
            {
                _data = 0;
            }
        }

        operator char_t*() const
        {
            if (_data)
            {
                if (_data < 255)
                {
                    xml_memory_page* page = compact_get_page(this, header_offset);

                    // round-trip through void* to silence 'cast increases required alignment of target type' warnings
                    const uint16_t* base = reinterpret_cast<const uint16_t*>(static_cast<const void*>(reinterpret_cast<const char*>(this) - base_offset));
                    assert(*base);

                    ptrdiff_t offset = ((*base - 1) << 7) + (_data - 1);

                    return page->compact_string_base + offset;
                }
                else
                {
                    return compact_get_value<header_offset, char_t>(this);
                }
            }
            else
                return 0;
        }

    private:
        unsigned char _data;
    };
PUGI__NS_END
#endif

#ifdef PUGIXML_COMPACT
namespace pugi
{
    struct xml_attribute_struct
    {
        xml_attribute_struct(impl::xml_memory_page* page): header(page, 0), namevalue_base(0)
        {
            PUGI__STATIC_ASSERT(sizeof(xml_attribute_struct) == 8);
        }

        impl::compact_header header;

        uint16_t namevalue_base;

        impl::compact_string<4, 2> name;
        impl::compact_string<5, 3> value;

        impl::compact_pointer<xml_attribute_struct, 6> prev_attribute_c;
        impl::compact_pointer<xml_attribute_struct, 7, 0> next_attribute;
    };

    struct xml_node_struct
    {
        xml_node_struct(impl::xml_memory_page* page, xml_node_type type): header(page, type), namevalue_base(0)
        {
            PUGI__STATIC_ASSERT(sizeof(xml_node_struct) == 12);
        }

        impl::compact_header header;

        uint16_t namevalue_base;

        impl::compact_string<4, 2> name;
        impl::compact_string<5, 3> value;

        impl::compact_pointer_parent<xml_node_struct, 6> parent;

        impl::compact_pointer<xml_node_struct, 8, 0> first_child;

        impl::compact_pointer<xml_node_struct,  9>    prev_sibling_c;
        impl::compact_pointer<xml_node_struct, 10, 0> next_sibling;

        impl::compact_pointer<xml_attribute_struct, 11, 0> first_attribute;
    };
}
#else
namespace pugi
{
    struct xml_attribute_struct
    {
        xml_attribute_struct(impl::xml_memory_page* page): name(0), value(0), prev_attribute_c(0), next_attribute(0)
        {
            header = PUGI__GETHEADER_IMPL(this, page, 0);
        }

        uintptr_t header;

        char_t* name;
        char_t* value;

        xml_attribute_struct* prev_attribute_c;
        xml_attribute_struct* next_attribute;
    };

    struct xml_node_struct
    {
        xml_node_struct(impl::xml_memory_page* page, xml_node_type type): name(0), value(0), parent(0), first_child(0), prev_sibling_c(0), next_sibling(0), first_attribute(0)
        {
            header = PUGI__GETHEADER_IMPL(this, page, type);
        }

        uintptr_t header;

        char_t* name;
        char_t* value;

        xml_node_struct* parent;

        xml_node_struct* first_child;

        xml_node_struct* prev_sibling_c;
        xml_node_struct* next_sibling;

        xml_attribute_struct* first_attribute;
    };
}
#endif

PUGI__NS_BEGIN
    struct xml_extra_buffer
    {
        char_t* buffer;
        xml_extra_buffer* next;
    };

    struct xml_document_struct: public xml_node_struct, public xml_allocator
    {
        xml_document_struct(xml_memory_page* page): xml_node_struct(page, node_document), xml_allocator(page), buffer(0), extra_buffers(0)
        {
        }

        const char_t* buffer;

        xml_extra_buffer* extra_buffers;

    #ifdef PUGIXML_COMPACT
        compact_hash_table hash;
    #endif
    };

    template <typename Object> inline xml_allocator& get_allocator(const Object* object)
    {
        assert(object);

        return *PUGI__GETPAGE(object)->allocator;
    }

    template <typename Object> inline xml_document_struct& get_document(const Object* object)
    {
        assert(object);

        return *static_cast<xml_document_struct*>(PUGI__GETPAGE(object)->allocator);
    }
PUGI__NS_END

// Low-level DOM operations
PUGI__NS_BEGIN
    inline xml_attribute_struct* allocate_attribute(xml_allocator& alloc)
    {
        xml_memory_page* page;
        void* memory = alloc.allocate_object(sizeof(xml_attribute_struct), page);
        if (!memory) return 0;

        return new (memory) xml_attribute_struct(page);
    }

    inline xml_node_struct* allocate_node(xml_allocator& alloc, xml_node_type type)
    {
        xml_memory_page* page;
        void* memory = alloc.allocate_object(sizeof(xml_node_struct), page);
        if (!memory) return 0;

        return new (memory) xml_node_struct(page, type);
    }

    inline void destroy_attribute(xml_attribute_struct* a, xml_allocator& alloc)
    {
        if (a->header & impl::xml_memory_page_name_allocated_mask)
            alloc.deallocate_string(a->name);

        if (a->header & impl::xml_memory_page_value_allocated_mask)
            alloc.deallocate_string(a->value);

        alloc.deallocate_memory(a, sizeof(xml_attribute_struct), PUGI__GETPAGE(a));
    }

    inline void destroy_node(xml_node_struct* n, xml_allocator& alloc)
    {
        if (n->header & impl::xml_memory_page_name_allocated_mask)
            alloc.deallocate_string(n->name);

        if (n->header & impl::xml_memory_page_value_allocated_mask)
            alloc.deallocate_string(n->value);

        for (xml_attribute_struct* attr = n->first_attribute; attr; )
        {
            xml_attribute_struct* next = attr->next_attribute;

            destroy_attribute(attr, alloc);

            attr = next;
        }

        for (xml_node_struct* child = n->first_child; child; )
        {
            xml_node_struct* next = child->next_sibling;

            destroy_node(child, alloc);

            child = next;
        }

        alloc.deallocate_memory(n, sizeof(xml_node_struct), PUGI__GETPAGE(n));
    }

    inline void append_node(xml_node_struct* child, xml_node_struct* node)
    {
        child->parent = node;

        xml_node_struct* head = node->first_child;

        if (head)
        {
            xml_node_struct* tail = head->prev_sibling_c;

            tail->next_sibling = child;
            child->prev_sibling_c = tail;
            head->prev_sibling_c = child;
        }
        else
        {
            node->first_child = child;
            child->prev_sibling_c = child;
        }
    }

    inline void prepend_node(xml_node_struct* child, xml_node_struct* node)
    {
        child->parent = node;

        xml_node_struct* head = node->first_child;

        if (head)
        {
            child->prev_sibling_c = head->prev_sibling_c;
            head->prev_sibling_c = child;
        }
        else
            child->prev_sibling_c = child;

        child->next_sibling = head;
        node->first_child = child;
    }

    inline void insert_node_after(xml_node_struct* child, xml_node_struct* node)
    {
        xml_node_struct* parent = node->parent;

        child->parent = parent;

        if (node->next_sibling)
            node->next_sibling->prev_sibling_c = child;
        else
            parent->first_child->prev_sibling_c = child;

        child->next_sibling = node->next_sibling;
        child->prev_sibling_c = node;

        node->next_sibling = child;
    }

    inline void insert_node_before(xml_node_struct* child, xml_node_struct* node)
    {
        xml_node_struct* parent = node->parent;

        child->parent = parent;

        if (node->prev_sibling_c->next_sibling)
            node->prev_sibling_c->next_sibling = child;
        else
            parent->first_child = child;

        child->prev_sibling_c = node->prev_sibling_c;
        child->next_sibling = node;

        node->prev_sibling_c = child;
    }

    inline void remove_node(xml_node_struct* node)
    {
        xml_node_struct* parent = node->parent;

        if (node->next_sibling)
            node->next_sibling->prev_sibling_c = node->prev_sibling_c;
        else
            parent->first_child->prev_sibling_c = node->prev_sibling_c;

        if (node->prev_sibling_c->next_sibling)
            node->prev_sibling_c->next_sibling = node->next_sibling;
        else
            parent->first_child = node->next_sibling;

        node->parent = 0;
        node->prev_sibling_c = 0;
        node->next_sibling = 0;
    }

    inline void append_attribute(xml_attribute_struct* attr, xml_node_struct* node)
    {
        xml_attribute_struct* head = node->first_attribute;

        if (head)
        {
            xml_attribute_struct* tail = head->prev_attribute_c;

            tail->next_attribute = attr;
            attr->prev_attribute_c = tail;
            head->prev_attribute_c = attr;
        }
        else
        {
            node->first_attribute = attr;
            attr->prev_attribute_c = attr;
        }
    }

    inline void prepend_attribute(xml_attribute_struct* attr, xml_node_struct* node)
    {
        xml_attribute_struct* head = node->first_attribute;

        if (head)
        {
            attr->prev_attribute_c = head->prev_attribute_c;
            head->prev_attribute_c = attr;
        }
        else
            attr->prev_attribute_c = attr;

        attr->next_attribute = head;
        node->first_attribute = attr;
    }

    inline void insert_attribute_after(xml_attribute_struct* attr, xml_attribute_struct* place, xml_node_struct* node)
    {
        if (place->next_attribute)
            place->next_attribute->prev_attribute_c = attr;
        else
            node->first_attribute->prev_attribute_c = attr;

        attr->next_attribute = place->next_attribute;
        attr->prev_attribute_c = place;
        place->next_attribute = attr;
    }

    inline void insert_attribute_before(xml_attribute_struct* attr, xml_attribute_struct* place, xml_node_struct* node)
    {
        if (place->prev_attribute_c->next_attribute)
            place->prev_attribute_c->next_attribute = attr;
        else
            node->first_attribute = attr;

        attr->prev_attribute_c = place->prev_attribute_c;
        attr->next_attribute = place;
        place->prev_attribute_c = attr;
    }

    inline void remove_attribute(xml_attribute_struct* attr, xml_node_struct* node)
    {
        if (attr->next_attribute)
            attr->next_attribute->prev_attribute_c = attr->prev_attribute_c;
        else
            node->first_attribute->prev_attribute_c = attr->prev_attribute_c;

        if (attr->prev_attribute_c->next_attribute)
            attr->prev_attribute_c->next_attribute = attr->next_attribute;
        else
            node->first_attribute = attr->next_attribute;

        attr->prev_attribute_c = 0;
        attr->next_attribute = 0;
    }

    PUGI__FN_NO_INLINE xml_node_struct* append_new_node(xml_node_struct* node, xml_allocator& alloc, xml_node_type type = node_element)
    {
        if (!alloc.reserve()) return 0;

        xml_node_struct* child = allocate_node(alloc, type);
        if (!child) return 0;

        append_node(child, node);

        return child;
    }

    PUGI__FN_NO_INLINE xml_attribute_struct* append_new_attribute(xml_node_struct* node, xml_allocator& alloc)
    {
        if (!alloc.reserve()) return 0;

        xml_attribute_struct* attr = allocate_attribute(alloc);
        if (!attr) return 0;

        append_attribute(attr, node);

        return attr;
    }
PUGI__NS_END

// Helper classes for code generation
PUGI__NS_BEGIN
    struct opt_false
    {
        enum { value = 0 };
    };

    struct opt_true
    {
        enum { value = 1 };
    };
PUGI__NS_END

// Unicode utilities
PUGI__NS_BEGIN
    inline uint16_t endian_swap(uint16_t value)
    {
        return static_cast<uint16_t>(((value & 0xff) << 8) | (value >> 8));
    }

    inline uint32_t endian_swap(uint32_t value)
    {
        return ((value & 0xff) << 24) | ((value & 0xff00) << 8) | ((value & 0xff0000) >> 8) | (value >> 24);
    }

    struct utf8_counter
    {
        typedef size_t value_type;

        static value_type low(value_type result, uint32_t ch)
        {
            // U+0000..U+007F
            if (ch < 0x80) return result + 1;
            // U+0080..U+07FF
            else if (ch < 0x800) return result + 2;
            // U+0800..U+FFFF
            else return result + 3;
        }

        static value_type high(value_type result, uint32_t)
        {
            // U+10000..U+10FFFF
            return result + 4;
        }
    };

    struct utf8_writer
    {
        typedef uint8_t* value_type;

        static value_type low(value_type result, uint32_t ch)
        {
            // U+0000..U+007F
            if (ch < 0x80)
            {
                *result = static_cast<uint8_t>(ch);
                return result + 1;
            }
            // U+0080..U+07FF
            else if (ch < 0x800)
            {
                result[0] = static_cast<uint8_t>(0xC0 | (ch >> 6));
                result[1] = static_cast<uint8_t>(0x80 | (ch & 0x3F));
                return result + 2;
            }
            // U+0800..U+FFFF
            else
            {
                result[0] = static_cast<uint8_t>(0xE0 | (ch >> 12));
                result[1] = static_cast<uint8_t>(0x80 | ((ch >> 6) & 0x3F));
                result[2] = static_cast<uint8_t>(0x80 | (ch & 0x3F));
                return result + 3;
            }
        }

        static value_type high(value_type result, uint32_t ch)
        {
            // U+10000..U+10FFFF
            result[0] = static_cast<uint8_t>(0xF0 | (ch >> 18));
            result[1] = static_cast<uint8_t>(0x80 | ((ch >> 12) & 0x3F));
            result[2] = static_cast<uint8_t>(0x80 | ((ch >> 6) & 0x3F));
            result[3] = static_cast<uint8_t>(0x80 | (ch & 0x3F));
            return result + 4;
        }

        static value_type any(value_type result, uint32_t ch)
        {
            return (ch < 0x10000) ? low(result, ch) : high(result, ch);
        }
    };

    struct utf16_counter
    {
        typedef size_t value_type;

        static value_type low(value_type result, uint32_t)
        {
            return result + 1;
        }

        static value_type high(value_type result, uint32_t)
        {
            return result + 2;
        }
    };

    struct utf16_writer
    {
        typedef uint16_t* value_type;

        static value_type low(value_type result, uint32_t ch)
        {
            *result = static_cast<uint16_t>(ch);

            return result + 1;
        }

        static value_type high(value_type result, uint32_t ch)
        {
            uint32_t msh = static_cast<uint32_t>(ch - 0x10000) >> 10;
            uint32_t lsh = static_cast<uint32_t>(ch - 0x10000) & 0x3ff;

            result[0] = static_cast<uint16_t>(0xD800 + msh);
            result[1] = static_cast<uint16_t>(0xDC00 + lsh);

            return result + 2;
        }

        static value_type any(value_type result, uint32_t ch)
        {
            return (ch < 0x10000) ? low(result, ch) : high(result, ch);
        }
    };

    struct utf32_counter
    {
        typedef size_t value_type;

        static value_type low(value_type result, uint32_t)
        {
            return result + 1;
        }

        static value_type high(value_type result, uint32_t)
        {
            return result + 1;
        }
    };

    struct utf32_writer
    {
        typedef uint32_t* value_type;

        static value_type low(value_type result, uint32_t ch)
        {
            *result = ch;

            return result + 1;
        }

        static value_type high(value_type result, uint32_t ch)
        {
            *result = ch;

            return result + 1;
        }

        static value_type any(value_type result, uint32_t ch)
        {
            *result = ch;

            return result + 1;
        }
    };

    struct latin1_writer
    {
        typedef uint8_t* value_type;

        static value_type low(value_type result, uint32_t ch)
        {
            *result = static_cast<uint8_t>(ch > 255 ? '?' : ch);

            return result + 1;
        }

        static value_type high(value_type result, uint32_t ch)
        {
            (void)ch;

            *result = '?';

            return result + 1;
        }
    };

    struct utf8_decoder
    {
        typedef uint8_t type;

        template <typename Traits> static inline typename Traits::value_type process(const uint8_t* data, size_t size, typename Traits::value_type result, Traits)
        {
            const uint8_t utf8_byte_mask = 0x3f;

            while (size)
            {
                uint8_t lead = *data;

                // 0xxxxxxx -> U+0000..U+007F
                if (lead < 0x80)
                {
                    result = Traits::low(result, lead);
                    data += 1;
                    size -= 1;

                    // process aligned single-byte (ascii) blocks
                    if ((reinterpret_cast<uintptr_t>(data) & 3) == 0)
                    {
                        // round-trip through void* to silence 'cast increases required alignment of target type' warnings
                        while (size >= 4 && (*static_cast<const uint32_t*>(static_cast<const void*>(data)) & 0x80808080) == 0)
                        {
                            result = Traits::low(result, data[0]);
                            result = Traits::low(result, data[1]);
                            result = Traits::low(result, data[2]);
                            result = Traits::low(result, data[3]);
                            data += 4;
                            size -= 4;
                        }
                    }
                }
                // 110xxxxx -> U+0080..U+07FF
                else if (static_cast<unsigned int>(lead - 0xC0) < 0x20 && size >= 2 && (data[1] & 0xc0) == 0x80)
                {
                    result = Traits::low(result, ((lead & ~0xC0) << 6) | (data[1] & utf8_byte_mask));
                    data += 2;
                    size -= 2;
                }
                // 1110xxxx -> U+0800-U+FFFF
                else if (static_cast<unsigned int>(lead - 0xE0) < 0x10 && size >= 3 && (data[1] & 0xc0) == 0x80 && (data[2] & 0xc0) == 0x80)
                {
                    result = Traits::low(result, ((lead & ~0xE0) << 12) | ((data[1] & utf8_byte_mask) << 6) | (data[2] & utf8_byte_mask));
                    data += 3;
                    size -= 3;
                }
                // 11110xxx -> U+10000..U+10FFFF
                else if (static_cast<unsigned int>(lead - 0xF0) < 0x08 && size >= 4 && (data[1] & 0xc0) == 0x80 && (data[2] & 0xc0) == 0x80 && (data[3] & 0xc0) == 0x80)
                {
                    result = Traits::high(result, ((lead & ~0xF0) << 18) | ((data[1] & utf8_byte_mask) << 12) | ((data[2] & utf8_byte_mask) << 6) | (data[3] & utf8_byte_mask));
                    data += 4;
                    size -= 4;
                }
                // 10xxxxxx or 11111xxx -> invalid
                else
                {
                    data += 1;
                    size -= 1;
                }
            }

            return result;
        }
    };

    template <typename opt_swap> struct utf16_decoder
    {
        typedef uint16_t type;

        template <typename Traits> static inline typename Traits::value_type process(const uint16_t* data, size_t size, typename Traits::value_type result, Traits)
        {
            while (size)
            {
                uint16_t lead = opt_swap::value ? endian_swap(*data) : *data;

                // U+0000..U+D7FF
                if (lead < 0xD800)
                {
                    result = Traits::low(result, lead);
                    data += 1;
                    size -= 1;
                }
                // U+E000..U+FFFF
                else if (static_cast<unsigned int>(lead - 0xE000) < 0x2000)
                {
                    result = Traits::low(result, lead);
                    data += 1;
                    size -= 1;
                }
                // surrogate pair lead
                else if (static_cast<unsigned int>(lead - 0xD800) < 0x400 && size >= 2)
                {
                    uint16_t next = opt_swap::value ? endian_swap(data[1]) : data[1];

                    if (static_cast<unsigned int>(next - 0xDC00) < 0x400)
                    {
                        result = Traits::high(result, 0x10000 + ((lead & 0x3ff) << 10) + (next & 0x3ff));
                        data += 2;
                        size -= 2;
                    }
                    else
                    {
                        data += 1;
                        size -= 1;
                    }
                }
                else
                {
                    data += 1;
                    size -= 1;
                }
            }

            return result;
        }
    };

    template <typename opt_swap> struct utf32_decoder
    {
        typedef uint32_t type;

        template <typename Traits> static inline typename Traits::value_type process(const uint32_t* data, size_t size, typename Traits::value_type result, Traits)
        {
            while (size)
            {
                uint32_t lead = opt_swap::value ? endian_swap(*data) : *data;

                // U+0000..U+FFFF
                if (lead < 0x10000)
                {
                    result = Traits::low(result, lead);
                    data += 1;
                    size -= 1;
                }
                // U+10000..U+10FFFF
                else
                {
                    result = Traits::high(result, lead);
                    data += 1;
                    size -= 1;
                }
            }

            return result;
        }
    };

    struct latin1_decoder
    {
        typedef uint8_t type;

        template <typename Traits> static inline typename Traits::value_type process(const uint8_t* data, size_t size, typename Traits::value_type result, Traits)
        {
            while (size)
            {
                result = Traits::low(result, *data);
                data += 1;
                size -= 1;
            }

            return result;
        }
    };

    template <size_t size> struct wchar_selector;

    template <> struct wchar_selector<2>
    {
        typedef uint16_t type;
        typedef utf16_counter counter;
        typedef utf16_writer writer;
        typedef utf16_decoder<opt_false> decoder;
    };

    template <> struct wchar_selector<4>
    {
        typedef uint32_t type;
        typedef utf32_counter counter;
        typedef utf32_writer writer;
        typedef utf32_decoder<opt_false> decoder;
    };

    typedef wchar_selector<sizeof(wchar_t)>::counter wchar_counter;
    typedef wchar_selector<sizeof(wchar_t)>::writer wchar_writer;

    struct wchar_decoder
    {
        typedef wchar_t type;

        template <typename Traits> static inline typename Traits::value_type process(const wchar_t* data, size_t size, typename Traits::value_type result, Traits traits)
        {
            typedef wchar_selector<sizeof(wchar_t)>::decoder decoder;

            return decoder::process(reinterpret_cast<const typename decoder::type*>(data), size, result, traits);
        }
    };

#ifdef PUGIXML_WCHAR_MODE
    PUGI__FN void convert_wchar_endian_swap(wchar_t* result, const wchar_t* data, size_t length)
    {
        for (size_t i = 0; i < length; ++i)
            result[i] = static_cast<wchar_t>(endian_swap(static_cast<wchar_selector<sizeof(wchar_t)>::type>(data[i])));
    }
#endif
PUGI__NS_END

PUGI__NS_BEGIN
    enum chartype_t
    {
        ct_parse_pcdata = 1,    // \0, &, \r, <
        ct_parse_attr = 2,      // \0, &, \r, ', "
        ct_parse_attr_ws = 4,   // \0, &, \r, ', ", \n, tab
        ct_space = 8,           // \r, \n, space, tab
        ct_parse_cdata = 16,    // \0, ], >, \r
        ct_parse_comment = 32,  // \0, -, >, \r
        ct_symbol = 64,         // Any symbol > 127, a-z, A-Z, 0-9, _, :, -, .
        ct_start_symbol = 128   // Any symbol > 127, a-z, A-Z, _, :
    };

    static const unsigned char chartype_table[256] =
    {
        55,  0,   0,   0,   0,   0,   0,   0,      0,   12,  12,  0,   0,   63,  0,   0,   // 0-15
        0,   0,   0,   0,   0,   0,   0,   0,      0,   0,   0,   0,   0,   0,   0,   0,   // 16-31
        8,   0,   6,   0,   0,   0,   7,   6,      0,   0,   0,   0,   0,   96,  64,  0,   // 32-47
        64,  64,  64,  64,  64,  64,  64,  64,     64,  64,  192, 0,   1,   0,   48,  0,   // 48-63
        0,   192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192, // 64-79
        192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 0,   0,   16,  0,   192, // 80-95
        0,   192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192, // 96-111
        192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 0, 0, 0, 0, 0,           // 112-127

        192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192, // 128+
        192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
        192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
        192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
        192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
        192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
        192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192,
        192, 192, 192, 192, 192, 192, 192, 192,    192, 192, 192, 192, 192, 192, 192, 192
    };

    enum chartypex_t
    {
        ctx_special_pcdata = 1,   // Any symbol >= 0 and < 32 (except \t, \r, \n), &, <, >
        ctx_special_attr = 2,     // Any symbol >= 0 and < 32, &, <, ", '
        ctx_start_symbol = 4,     // Any symbol > 127, a-z, A-Z, _
        ctx_digit = 8,            // 0-9
        ctx_symbol = 16           // Any symbol > 127, a-z, A-Z, 0-9, _, -, .
    };

    static const unsigned char chartypex_table[256] =
    {
        3,  3,  3,  3,  3,  3,  3,  3,     3,  2,  2,  3,  3,  2,  3,  3,     // 0-15
        3,  3,  3,  3,  3,  3,  3,  3,     3,  3,  3,  3,  3,  3,  3,  3,     // 16-31
        0,  0,  2,  0,  0,  0,  3,  2,     0,  0,  0,  0,  0, 16, 16,  0,     // 32-47
        24, 24, 24, 24, 24, 24, 24, 24,    24, 24, 0,  0,  3,  0,  1,  0,     // 48-63

        0,  20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 20, 20, 20, 20, 20,    // 64-79
        20, 20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 0,  0,  0,  0,  20,    // 80-95
        0,  20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 20, 20, 20, 20, 20,    // 96-111
        20, 20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 0,  0,  0,  0,  0,     // 112-127

        20, 20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 20, 20, 20, 20, 20,    // 128+
        20, 20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 20, 20, 20, 20, 20,
        20, 20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 20, 20, 20, 20, 20,
        20, 20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 20, 20, 20, 20, 20,
        20, 20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 20, 20, 20, 20, 20,
        20, 20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 20, 20, 20, 20, 20,
        20, 20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 20, 20, 20, 20, 20,
        20, 20, 20, 20, 20, 20, 20, 20,    20, 20, 20, 20, 20, 20, 20, 20
    };

#ifdef PUGIXML_WCHAR_MODE
    #define PUGI__IS_CHARTYPE_IMPL(c, ct, table) ((static_cast<unsigned int>(c) < 128 ? table[static_cast<unsigned int>(c)] : table[128]) & (ct))
#else
    #define PUGI__IS_CHARTYPE_IMPL(c, ct, table) (table[static_cast<unsigned char>(c)] & (ct))
#endif

    #define PUGI__IS_CHARTYPE(c, ct) PUGI__IS_CHARTYPE_IMPL(c, ct, chartype_table)
    #define PUGI__IS_CHARTYPEX(c, ct) PUGI__IS_CHARTYPE_IMPL(c, ct, chartypex_table)

    PUGI__FN bool is_little_endian()
    {
        unsigned int ui = 1;

        return *reinterpret_cast<unsigned char*>(&ui) == 1;
    }

    PUGI__FN xml_encoding get_wchar_encoding()
    {
        PUGI__STATIC_ASSERT(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4);

        if (sizeof(wchar_t) == 2)
            return is_little_endian() ? encoding_utf16_le : encoding_utf16_be;
        else
            return is_little_endian() ? encoding_utf32_le : encoding_utf32_be;
    }

    PUGI__FN bool parse_declaration_encoding(const uint8_t* data, size_t size, const uint8_t*& out_encoding, size_t& out_length)
    {
    #define PUGI__SCANCHAR(ch) { if (offset >= size || data[offset] != ch) return false; offset++; }
    #define PUGI__SCANCHARTYPE(ct) { while (offset < size && PUGI__IS_CHARTYPE(data[offset], ct)) offset++; }

        // check if we have a non-empty XML declaration
        if (size < 6 || !((data[0] == '<') & (data[1] == '?') & (data[2] == 'x') & (data[3] == 'm') & (data[4] == 'l') && PUGI__IS_CHARTYPE(data[5], ct_space)))
            return false;

        // scan XML declaration until the encoding field
        for (size_t i = 6; i + 1 < size; ++i)
        {
            // declaration can not contain ? in quoted values
            if (data[i] == '?')
                return false;

            if (data[i] == 'e' && data[i + 1] == 'n')
            {
                size_t offset = i;

                // encoding follows the version field which can't contain 'en' so this has to be the encoding if XML is well formed
                PUGI__SCANCHAR('e'); PUGI__SCANCHAR('n'); PUGI__SCANCHAR('c'); PUGI__SCANCHAR('o');
                PUGI__SCANCHAR('d'); PUGI__SCANCHAR('i'); PUGI__SCANCHAR('n'); PUGI__SCANCHAR('g');

                // S? = S?
                PUGI__SCANCHARTYPE(ct_space);
                PUGI__SCANCHAR('=');
                PUGI__SCANCHARTYPE(ct_space);

                // the only two valid delimiters are ' and "
                uint8_t delimiter = (offset < size && data[offset] == '"') ? '"' : '\'';

                PUGI__SCANCHAR(delimiter);

                size_t start = offset;

                out_encoding = data + offset;

                PUGI__SCANCHARTYPE(ct_symbol);

                out_length = offset - start;

                PUGI__SCANCHAR(delimiter);

                return true;
            }
        }

        return false;

    #undef PUGI__SCANCHAR
    #undef PUGI__SCANCHARTYPE
    }

    PUGI__FN xml_encoding guess_buffer_encoding(const uint8_t* data, size_t size)
    {
        // skip encoding autodetection if input buffer is too small
        if (size < 4) return encoding_utf8;

        uint8_t d0 = data[0], d1 = data[1], d2 = data[2], d3 = data[3];

        // look for BOM in first few bytes
        if (d0 == 0 && d1 == 0 && d2 == 0xfe && d3 == 0xff) return encoding_utf32_be;
        if (d0 == 0xff && d1 == 0xfe && d2 == 0 && d3 == 0) return encoding_utf32_le;
        if (d0 == 0xfe && d1 == 0xff) return encoding_utf16_be;
        if (d0 == 0xff && d1 == 0xfe) return encoding_utf16_le;
        if (d0 == 0xef && d1 == 0xbb && d2 == 0xbf) return encoding_utf8;

        // look for <, <? or <?xm in various encodings
        if (d0 == 0 && d1 == 0 && d2 == 0 && d3 == 0x3c) return encoding_utf32_be;
        if (d0 == 0x3c && d1 == 0 && d2 == 0 && d3 == 0) return encoding_utf32_le;
        if (d0 == 0 && d1 == 0x3c && d2 == 0 && d3 == 0x3f) return encoding_utf16_be;
        if (d0 == 0x3c && d1 == 0 && d2 == 0x3f && d3 == 0) return encoding_utf16_le;

        // look for utf16 < followed by node name (this may fail, but is better than utf8 since it's zero terminated so early)
        if (d0 == 0 && d1 == 0x3c) return encoding_utf16_be;
        if (d0 == 0x3c && d1 == 0) return encoding_utf16_le;

        // no known BOM detected; parse declaration
        const uint8_t* enc = 0;
        size_t enc_length = 0;

        if (d0 == 0x3c && d1 == 0x3f && d2 == 0x78 && d3 == 0x6d && parse_declaration_encoding(data, size, enc, enc_length))
        {
            // iso-8859-1 (case-insensitive)
            if (enc_length == 10
                && (enc[0] | ' ') == 'i' && (enc[1] | ' ') == 's' && (enc[2] | ' ') == 'o'
                && enc[3] == '-' && enc[4] == '8' && enc[5] == '8' && enc[6] == '5' && enc[7] == '9'
                && enc[8] == '-' && enc[9] == '1')
                return encoding_latin1;

            // latin1 (case-insensitive)
            if (enc_length == 6
                && (enc[0] | ' ') == 'l' && (enc[1] | ' ') == 'a' && (enc[2] | ' ') == 't'
                && (enc[3] | ' ') == 'i' && (enc[4] | ' ') == 'n'
                && enc[5] == '1')
                return encoding_latin1;
        }

        return encoding_utf8;
    }

    PUGI__FN xml_encoding get_buffer_encoding(xml_encoding encoding, const void* contents, size_t size)
    {
        // replace wchar encoding with utf implementation
        if (encoding == encoding_wchar) return get_wchar_encoding();

        // replace utf16 encoding with utf16 with specific endianness
        if (encoding == encoding_utf16) return is_little_endian() ? encoding_utf16_le : encoding_utf16_be;

        // replace utf32 encoding with utf32 with specific endianness
        if (encoding == encoding_utf32) return is_little_endian() ? encoding_utf32_le : encoding_utf32_be;

        // only do autodetection if no explicit encoding is requested
        if (encoding != encoding_auto) return encoding;

        // try to guess encoding (based on XML specification, Appendix F.1)
        const uint8_t* data = static_cast<const uint8_t*>(contents);

        return guess_buffer_encoding(data, size);
    }

    PUGI__FN bool get_mutable_buffer(char_t*& out_buffer, size_t& out_length, const void* contents, size_t size, bool is_mutable)
    {
        size_t length = size / sizeof(char_t);

        if (is_mutable)
        {
            out_buffer = static_cast<char_t*>(const_cast<void*>(contents));
            out_length = length;
        }
        else
        {
            char_t* buffer = static_cast<char_t*>(xml_memory::allocate((length + 1) * sizeof(char_t)));
            if (!buffer) return false;

            if (contents)
                memcpy(buffer, contents, length * sizeof(char_t));
            else
                assert(length == 0);

            buffer[length] = 0;

            out_buffer = buffer;
            out_length = length + 1;
        }

        return true;
    }

#ifdef PUGIXML_WCHAR_MODE
    PUGI__FN bool need_endian_swap_utf(xml_encoding le, xml_encoding re)
    {
        return (le == encoding_utf16_be && re == encoding_utf16_le) || (le == encoding_utf16_le && re == encoding_utf16_be) ||
               (le == encoding_utf32_be && re == encoding_utf32_le) || (le == encoding_utf32_le && re == encoding_utf32_be);
    }

    PUGI__FN bool convert_buffer_endian_swap(char_t*& out_buffer, size_t& out_length, const void* contents, size_t size, bool is_mutable)
    {
        const char_t* data = static_cast<const char_t*>(contents);
        size_t length = size / sizeof(char_t);

        if (is_mutable)
        {
            char_t* buffer = const_cast<char_t*>(data);

            convert_wchar_endian_swap(buffer, data, length);

            out_buffer = buffer;
            out_length = length;
        }
        else
        {
            char_t* buffer = static_cast<char_t*>(xml_memory::allocate((length + 1) * sizeof(char_t)));
            if (!buffer) return false;

            convert_wchar_endian_swap(buffer, data, length);
            buffer[length] = 0;

            out_buffer = buffer;
            out_length = length + 1;
        }

        return true;
    }

    template <typename D> PUGI__FN bool convert_buffer_generic(char_t*& out_buffer, size_t& out_length, const void* contents, size_t size, D)
    {
        const typename D::type* data = static_cast<const typename D::type*>(contents);
        size_t data_length = size / sizeof(typename D::type);

        // first pass: get length in wchar_t units
        size_t length = D::process(data, data_length, 0, wchar_counter());

        // allocate buffer of suitable length
        char_t* buffer = static_cast<char_t*>(xml_memory::allocate((length + 1) * sizeof(char_t)));
        if (!buffer) return false;

        // second pass: convert utf16 input to wchar_t
        wchar_writer::value_type obegin = reinterpret_cast<wchar_writer::value_type>(buffer);
        wchar_writer::value_type oend = D::process(data, data_length, obegin, wchar_writer());

        assert(oend == obegin + length);
        *oend = 0;

        out_buffer = buffer;
        out_length = length + 1;

        return true;
    }

    PUGI__FN bool convert_buffer(char_t*& out_buffer, size_t& out_length, xml_encoding encoding, const void* contents, size_t size, bool is_mutable)
    {
        // get native encoding
        xml_encoding wchar_encoding = get_wchar_encoding();

        // fast path: no conversion required
        if (encoding == wchar_encoding)
            return get_mutable_buffer(out_buffer, out_length, contents, size, is_mutable);

        // only endian-swapping is required
        if (need_endian_swap_utf(encoding, wchar_encoding))
            return convert_buffer_endian_swap(out_buffer, out_length, contents, size, is_mutable);

        // source encoding is utf8
        if (encoding == encoding_utf8)
            return convert_buffer_generic(out_buffer, out_length, contents, size, utf8_decoder());

        // source encoding is utf16
        if (encoding == encoding_utf16_be || encoding == encoding_utf16_le)
        {
            xml_encoding native_encoding = is_little_endian() ? encoding_utf16_le : encoding_utf16_be;

            return (native_encoding == encoding) ?
                convert_buffer_generic(out_buffer, out_length, contents, size, utf16_decoder<opt_false>()) :
                convert_buffer_generic(out_buffer, out_length, contents, size, utf16_decoder<opt_true>());
        }

        // source encoding is utf32
        if (encoding == encoding_utf32_be || encoding == encoding_utf32_le)
        {
            xml_encoding native_encoding = is_little_endian() ? encoding_utf32_le : encoding_utf32_be;

            return (native_encoding == encoding) ?
                convert_buffer_generic(out_buffer, out_length, contents, size, utf32_decoder<opt_false>()) :
                convert_buffer_generic(out_buffer, out_length, contents, size, utf32_decoder<opt_true>());
        }

        // source encoding is latin1
        if (encoding == encoding_latin1)
            return convert_buffer_generic(out_buffer, out_length, contents, size, latin1_decoder());

        assert(false && "Invalid encoding"); // unreachable
        return false;
    }
#else
    template <typename D> PUGI__FN bool convert_buffer_generic(char_t*& out_buffer, size_t& out_length, const void* contents, size_t size, D)
    {
        const typename D::type* data = static_cast<const typename D::type*>(contents);
        size_t data_length = size / sizeof(typename D::type);

        // first pass: get length in utf8 units
        size_t length = D::process(data, data_length, 0, utf8_counter());

        // allocate buffer of suitable length
        char_t* buffer = static_cast<char_t*>(xml_memory::allocate((length + 1) * sizeof(char_t)));
        if (!buffer) return false;

        // second pass: convert utf16 input to utf8
        uint8_t* obegin = reinterpret_cast<uint8_t*>(buffer);
        uint8_t* oend = D::process(data, data_length, obegin, utf8_writer());

        assert(oend == obegin + length);
        *oend = 0;

        out_buffer = buffer;
        out_length = length + 1;

        return true;
    }

    PUGI__FN size_t get_latin1_7bit_prefix_length(const uint8_t* data, size_t size)
    {
        for (size_t i = 0; i < size; ++i)
            if (data[i] > 127)
                return i;

        return size;
    }

    PUGI__FN bool convert_buffer_latin1(char_t*& out_buffer, size_t& out_length, const void* contents, size_t size, bool is_mutable)
    {
        const uint8_t* data = static_cast<const uint8_t*>(contents);
        size_t data_length = size;

        // get size of prefix that does not need utf8 conversion
        size_t prefix_length = get_latin1_7bit_prefix_length(data, data_length);
        assert(prefix_length <= data_length);

        const uint8_t* postfix = data + prefix_length;
        size_t postfix_length = data_length - prefix_length;

        // if no conversion is needed, just return the original buffer
        if (postfix_length == 0) return get_mutable_buffer(out_buffer, out_length, contents, size, is_mutable);

        // first pass: get length in utf8 units
        size_t length = prefix_length + latin1_decoder::process(postfix, postfix_length, 0, utf8_counter());

        // allocate buffer of suitable length
        char_t* buffer = static_cast<char_t*>(xml_memory::allocate((length + 1) * sizeof(char_t)));
        if (!buffer) return false;

        // second pass: convert latin1 input to utf8
        memcpy(buffer, data, prefix_length);

        uint8_t* obegin = reinterpret_cast<uint8_t*>(buffer);
        uint8_t* oend = latin1_decoder::process(postfix, postfix_length, obegin + prefix_length, utf8_writer());

        assert(oend == obegin + length);
        *oend = 0;

        out_buffer = buffer;
        out_length = length + 1;

        return true;
    }

    PUGI__FN bool convert_buffer(char_t*& out_buffer, size_t& out_length, xml_encoding encoding, const void* contents, size_t size, bool is_mutable)
    {
        // fast path: no conversion required
        if (encoding == encoding_utf8)
            return get_mutable_buffer(out_buffer, out_length, contents, size, is_mutable);

        // source encoding is utf16
        if (encoding == encoding_utf16_be || encoding == encoding_utf16_le)
        {
            xml_encoding native_encoding = is_little_endian() ? encoding_utf16_le : encoding_utf16_be;

            return (native_encoding == encoding) ?
                convert_buffer_generic(out_buffer, out_length, contents, size, utf16_decoder<opt_false>()) :
                convert_buffer_generic(out_buffer, out_length, contents, size, utf16_decoder<opt_true>());
        }

        // source encoding is utf32
        if (encoding == encoding_utf32_be || encoding == encoding_utf32_le)
        {
            xml_encoding native_encoding = is_little_endian() ? encoding_utf32_le : encoding_utf32_be;

            return (native_encoding == encoding) ?
                convert_buffer_generic(out_buffer, out_length, contents, size, utf32_decoder<opt_false>()) :
                convert_buffer_generic(out_buffer, out_length, contents, size, utf32_decoder<opt_true>());
        }

        // source encoding is latin1
        if (encoding == encoding_latin1)
            return convert_buffer_latin1(out_buffer, out_length, contents, size, is_mutable);

        assert(false && "Invalid encoding"); // unreachable
        return false;
    }
#endif

    PUGI__FN size_t as_utf8_begin(const wchar_t* str, size_t length)
    {
        // get length in utf8 characters
        return wchar_decoder::process(str, length, 0, utf8_counter());
    }

    PUGI__FN void as_utf8_end(char* buffer, size_t size, const wchar_t* str, size_t length)
    {
        // convert to utf8
        uint8_t* begin = reinterpret_cast<uint8_t*>(buffer);
        uint8_t* end = wchar_decoder::process(str, length, begin, utf8_writer());

        assert(begin + size == end);
        (void)!end;
        (void)!size;
    }

#ifndef PUGIXML_NO_STL
    PUGI__FN std::string as_utf8_impl(const wchar_t* str, size_t length)
    {
        // first pass: get length in utf8 characters
        size_t size = as_utf8_begin(str, length);

        // allocate resulting string
        std::string result;
        result.resize(size);

        // second pass: convert to utf8
        if (size > 0) as_utf8_end(&result[0], size, str, length);

        return result;
    }

    PUGI__FN std::basic_string<wchar_t> as_wide_impl(const char* str, size_t size)
    {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(str);

        // first pass: get length in wchar_t units
        size_t length = utf8_decoder::process(data, size, 0, wchar_counter());

        // allocate resulting string
        std::basic_string<wchar_t> result;
        result.resize(length);

        // second pass: convert to wchar_t
        if (length > 0)
        {
            wchar_writer::value_type begin = reinterpret_cast<wchar_writer::value_type>(&result[0]);
            wchar_writer::value_type end = utf8_decoder::process(data, size, begin, wchar_writer());

            assert(begin + length == end);
            (void)!end;
        }

        return result;
    }
#endif

    template <typename Header>
    inline bool strcpy_insitu_allow(size_t length, const Header& header, uintptr_t header_mask, char_t* target)
    {
        // never reuse shared memory
        if (header & xml_memory_page_contents_shared_mask) return false;

        size_t target_length = strlength(target);

        // always reuse document buffer memory if possible
        if ((header & header_mask) == 0) return target_length >= length;

        // reuse heap memory if waste is not too great
        const size_t reuse_threshold = 32;

        return target_length >= length && (target_length < reuse_threshold || target_length - length < target_length / 2);
    }

    template <typename String, typename Header>
    PUGI__FN bool strcpy_insitu(String& dest, Header& header, uintptr_t header_mask, const char_t* source, size_t source_length)
    {
        if (source_length == 0)
        {
            // empty string and null pointer are equivalent, so just deallocate old memory
            xml_allocator* alloc = PUGI__GETPAGE_IMPL(header)->allocator;

            if (header & header_mask) alloc->deallocate_string(dest);

            // mark the string as not allocated
            dest = 0;
            header &= ~header_mask;

            return true;
        }
        else if (dest && strcpy_insitu_allow(source_length, header, header_mask, dest))
        {
            // we can reuse old buffer, so just copy the new data (including zero terminator)
            memcpy(dest, source, source_length * sizeof(char_t));
            dest[source_length] = 0;

            return true;
        }
        else
        {
            xml_allocator* alloc = PUGI__GETPAGE_IMPL(header)->allocator;

            if (!alloc->reserve()) return false;

            // allocate new buffer
            char_t* buf = alloc->allocate_string(source_length + 1);
            if (!buf) return false;

            // copy the string (including zero terminator)
            memcpy(buf, source, source_length * sizeof(char_t));
            buf[source_length] = 0;

            // deallocate old buffer (*after* the above to protect against overlapping memory and/or allocation failures)
            if (header & header_mask) alloc->deallocate_string(dest);

            // the string is now allocated, so set the flag
            dest = buf;
            header |= header_mask;

            return true;
        }
    }

    struct gap
    {
        char_t* end;
        size_t size;

        gap(): end(0), size(0)
        {
        }

        // Push new gap, move s count bytes further (skipping the gap).
        // Collapse previous gap.
        void push(char_t*& s, size_t count)
        {
            if (end) // there was a gap already; collapse it
            {
                // Move [old_gap_end, new_gap_start) to [old_gap_start, ...)
                assert(s >= end);
                memmove(end - size, end, reinterpret_cast<char*>(s) - reinterpret_cast<char*>(end));
            }

            s += count; // end of current gap

            // "merge" two gaps
            end = s;
            size += count;
        }

        // Collapse all gaps, return past-the-end pointer
        char_t* flush(char_t* s)
        {
            if (end)
            {
                // Move [old_gap_end, current_pos) to [old_gap_start, ...)
                assert(s >= end);
                memmove(end - size, end, reinterpret_cast<char*>(s) - reinterpret_cast<char*>(end));

                return s - size;
            }
            else return s;
        }
    };

    PUGI__FN char_t* strconv_escape(char_t* s, gap& g)
    {
        char_t* stre = s + 1;

        switch (*stre)
        {
            case '#':   // &#...
            {
                unsigned int ucsc = 0;

                if (stre[1] == 'x') // &#x... (hex code)
                {
                    stre += 2;

                    char_t ch = *stre;

                    if (ch == ';') return stre;

                    for (;;)
                    {
                        if (static_cast<unsigned int>(ch - '0') <= 9)
                            ucsc = 16 * ucsc + (ch - '0');
                        else if (static_cast<unsigned int>((ch | ' ') - 'a') <= 5)
                            ucsc = 16 * ucsc + ((ch | ' ') - 'a' + 10);
                        else if (ch == ';')
                            break;
                        else // cancel
                            return stre;

                        ch = *++stre;
                    }

                    ++stre;
                }
                else    // &#... (dec code)
                {
                    char_t ch = *++stre;

                    if (ch == ';') return stre;

                    for (;;)
                    {
                        if (static_cast<unsigned int>(ch - '0') <= 9)
                            ucsc = 10 * ucsc + (ch - '0');
                        else if (ch == ';')
                            break;
                        else // cancel
                            return stre;

                        ch = *++stre;
                    }

                    ++stre;
                }

            #ifdef PUGIXML_WCHAR_MODE
                s = reinterpret_cast<char_t*>(wchar_writer::any(reinterpret_cast<wchar_writer::value_type>(s), ucsc));
            #else
                s = reinterpret_cast<char_t*>(utf8_writer::any(reinterpret_cast<uint8_t*>(s), ucsc));
            #endif

                g.push(s, stre - s);
                return stre;
            }

            case 'a':   // &a
            {
                ++stre;

                if (*stre == 'm') // &am
                {
                    if (*++stre == 'p' && *++stre == ';') // &amp;
                    {
                        *s++ = '&';
                        ++stre;

                        g.push(s, stre - s);
                        return stre;
                    }
                }
                else if (*stre == 'p') // &ap
                {
                    if (*++stre == 'o' && *++stre == 's' && *++stre == ';') // &apos;
                    {
                        *s++ = '\'';
                        ++stre;

                        g.push(s, stre - s);
                        return stre;
                    }
                }
                break;
            }

            case 'g': // &g
            {
                if (*++stre == 't' && *++stre == ';') // &gt;
                {
                    *s++ = '>';
                    ++stre;

                    g.push(s, stre - s);
                    return stre;
                }
                break;
            }

            case 'l': // &l
            {
                if (*++stre == 't' && *++stre == ';') // &lt;
                {
                    *s++ = '<';
                    ++stre;

                    g.push(s, stre - s);
                    return stre;
                }
                break;
            }

            case 'q': // &q
            {
                if (*++stre == 'u' && *++stre == 'o' && *++stre == 't' && *++stre == ';') // &quot;
                {
                    *s++ = '"';
                    ++stre;

                    g.push(s, stre - s);
                    return stre;
                }
                break;
            }

            default:
                break;
        }

        return stre;
    }

    // Parser utilities
    #define PUGI__ENDSWITH(c, e)        ((c) == (e) || ((c) == 0 && endch == (e)))
    #define PUGI__SKIPWS()              { while (PUGI__IS_CHARTYPE(*s, ct_space)) ++s; }
    #define PUGI__OPTSET(OPT)           ( optmsk & (OPT) )
    #define PUGI__PUSHNODE(TYPE)        { cursor = append_new_node(cursor, *alloc, TYPE); if (!cursor) PUGI__THROW_ERROR(status_out_of_memory, s); }
    #define PUGI__POPNODE()             { cursor = cursor->parent; }
    #define PUGI__SCANFOR(X)            { while (*s != 0 && !(X)) ++s; }
    #define PUGI__SCANWHILE(X)          { while (X) ++s; }
    #define PUGI__SCANWHILE_UNROLL(X)   { for (;;) { char_t ss = s[0]; if (PUGI__UNLIKELY(!(X))) { break; } ss = s[1]; if (PUGI__UNLIKELY(!(X))) { s += 1; break; } ss = s[2]; if (PUGI__UNLIKELY(!(X))) { s += 2; break; } ss = s[3]; if (PUGI__UNLIKELY(!(X))) { s += 3; break; } s += 4; } }
    #define PUGI__ENDSEG()              { ch = *s; *s = 0; ++s; }
    #define PUGI__THROW_ERROR(err, m)   return error_offset = m, error_status = err, static_cast<char_t*>(0)
    #define PUGI__CHECK_ERROR(err, m)   { if (*s == 0) PUGI__THROW_ERROR(err, m); }

    PUGI__FN char_t* strconv_comment(char_t* s, char_t endch)
    {
        gap g;

        while (true)
        {
            PUGI__SCANWHILE_UNROLL(!PUGI__IS_CHARTYPE(ss, ct_parse_comment));

            if (*s == '\r') // Either a single 0x0d or 0x0d 0x0a pair
            {
                *s++ = '\n'; // replace first one with 0x0a

                if (*s == '\n') g.push(s, 1);
            }
            else if (s[0] == '-' && s[1] == '-' && PUGI__ENDSWITH(s[2], '>')) // comment ends here
            {
                *g.flush(s) = 0;

                return s + (s[2] == '>' ? 3 : 2);
            }
            else if (*s == 0)
            {
                return 0;
            }
            else ++s;
        }
    }

    PUGI__FN char_t* strconv_cdata(char_t* s, char_t endch)
    {
        gap g;

        while (true)
        {
            PUGI__SCANWHILE_UNROLL(!PUGI__IS_CHARTYPE(ss, ct_parse_cdata));

            if (*s == '\r') // Either a single 0x0d or 0x0d 0x0a pair
            {
                *s++ = '\n'; // replace first one with 0x0a

                if (*s == '\n') g.push(s, 1);
            }
            else if (s[0] == ']' && s[1] == ']' && PUGI__ENDSWITH(s[2], '>')) // CDATA ends here
            {
                *g.flush(s) = 0;

                return s + 1;
            }
            else if (*s == 0)
            {
                return 0;
            }
            else ++s;
        }
    }

    typedef char_t* (*strconv_pcdata_t)(char_t*);

    template <typename opt_trim, typename opt_eol, typename opt_escape> struct strconv_pcdata_impl
    {
        static char_t* parse(char_t* s)
        {
            gap g;

            char_t* begin = s;

            while (true)
            {
                PUGI__SCANWHILE_UNROLL(!PUGI__IS_CHARTYPE(ss, ct_parse_pcdata));

                if (*s == '<') // PCDATA ends here
                {
                    char_t* end = g.flush(s);

                    if (opt_trim::value)
                        while (end > begin && PUGI__IS_CHARTYPE(end[-1], ct_space))
                            --end;

                    *end = 0;

                    return s + 1;
                }
                else if (opt_eol::value && *s == '\r') // Either a single 0x0d or 0x0d 0x0a pair
                {
                    *s++ = '\n'; // replace first one with 0x0a

                    if (*s == '\n') g.push(s, 1);
                }
                else if (opt_escape::value && *s == '&')
                {
                    s = strconv_escape(s, g);
                }
                else if (*s == 0)
                {
                    char_t* end = g.flush(s);

                    if (opt_trim::value)
                        while (end > begin && PUGI__IS_CHARTYPE(end[-1], ct_space))
                            --end;

                    *end = 0;

                    return s;
                }
                else ++s;
            }
        }
    };

    PUGI__FN strconv_pcdata_t get_strconv_pcdata(unsigned int optmask)
    {
        PUGI__STATIC_ASSERT(parse_escapes == 0x10 && parse_eol == 0x20 && parse_trim_pcdata == 0x0800);

        switch (((optmask >> 4) & 3) | ((optmask >> 9) & 4)) // get bitmask for flags (trim eol escapes); this simultaneously checks 3 options from assertion above
        {
        case 0: return strconv_pcdata_impl<opt_false, opt_false, opt_false>::parse;
        case 1: return strconv_pcdata_impl<opt_false, opt_false, opt_true>::parse;
        case 2: return strconv_pcdata_impl<opt_false, opt_true, opt_false>::parse;
        case 3: return strconv_pcdata_impl<opt_false, opt_true, opt_true>::parse;
        case 4: return strconv_pcdata_impl<opt_true, opt_false, opt_false>::parse;
        case 5: return strconv_pcdata_impl<opt_true, opt_false, opt_true>::parse;
        case 6: return strconv_pcdata_impl<opt_true, opt_true, opt_false>::parse;
        case 7: return strconv_pcdata_impl<opt_true, opt_true, opt_true>::parse;
        default: assert(false); return 0; // unreachable
        }
    }

    typedef char_t* (*strconv_attribute_t)(char_t*, char_t);

    template <typename opt_escape> struct strconv_attribute_impl
    {
        static char_t* parse_wnorm(char_t* s, char_t end_quote)
        {
            gap g;

            // trim leading whitespaces
            if (PUGI__IS_CHARTYPE(*s, ct_space))
            {
                char_t* str = s;

                do ++str;
                while (PUGI__IS_CHARTYPE(*str, ct_space));

                g.push(s, str - s);
            }

            while (true)
            {
                PUGI__SCANWHILE_UNROLL(!PUGI__IS_CHARTYPE(ss, ct_parse_attr_ws | ct_space));

                if (*s == end_quote)
                {
                    char_t* str = g.flush(s);

                    do *str-- = 0;
                    while (PUGI__IS_CHARTYPE(*str, ct_space));

                    return s + 1;
                }
                else if (PUGI__IS_CHARTYPE(*s, ct_space))
                {
                    *s++ = ' ';

                    if (PUGI__IS_CHARTYPE(*s, ct_space))
                    {
                        char_t* str = s + 1;
                        while (PUGI__IS_CHARTYPE(*str, ct_space)) ++str;

                        g.push(s, str - s);
                    }
                }
                else if (opt_escape::value && *s == '&')
                {
                    s = strconv_escape(s, g);
                }
                else if (!*s)
                {
                    return 0;
                }
                else ++s;
            }
        }

        static char_t* parse_wconv(char_t* s, char_t end_quote)
        {
            gap g;

            while (true)
            {
                PUGI__SCANWHILE_UNROLL(!PUGI__IS_CHARTYPE(ss, ct_parse_attr_ws));

                if (*s == end_quote)
                {
                    *g.flush(s) = 0;

                    return s + 1;
                }
                else if (PUGI__IS_CHARTYPE(*s, ct_space))
                {
                    if (*s == '\r')
                    {
                        *s++ = ' ';

                        if (*s == '\n') g.push(s, 1);
                    }
                    else *s++ = ' ';
                }
                else if (opt_escape::value && *s == '&')
                {
                    s = strconv_escape(s, g);
                }
                else if (!*s)
                {
                    return 0;
                }
                else ++s;
            }
        }

        static char_t* parse_eol(char_t* s, char_t end_quote)
        {
            gap g;

            while (true)
            {
                PUGI__SCANWHILE_UNROLL(!PUGI__IS_CHARTYPE(ss, ct_parse_attr));

                if (*s == end_quote)
                {
                    *g.flush(s) = 0;

                    return s + 1;
                }
                else if (*s == '\r')
                {
                    *s++ = '\n';

                    if (*s == '\n') g.push(s, 1);
                }
                else if (opt_escape::value && *s == '&')
                {
                    s = strconv_escape(s, g);
                }
                else if (!*s)
                {
                    return 0;
                }
                else ++s;
            }
        }

        static char_t* parse_simple(char_t* s, char_t end_quote)
        {
            gap g;

            while (true)
            {
                PUGI__SCANWHILE_UNROLL(!PUGI__IS_CHARTYPE(ss, ct_parse_attr));

                if (*s == end_quote)
                {
                    *g.flush(s) = 0;

                    return s + 1;
                }
                else if (opt_escape::value && *s == '&')
                {
                    s = strconv_escape(s, g);
                }
                else if (!*s)
                {
                    return 0;
                }
                else ++s;
            }
        }
    };

    PUGI__FN strconv_attribute_t get_strconv_attribute(unsigned int optmask)
    {
        PUGI__STATIC_ASSERT(parse_escapes == 0x10 && parse_eol == 0x20 && parse_wconv_attribute == 0x40 && parse_wnorm_attribute == 0x80);

        switch ((optmask >> 4) & 15) // get bitmask for flags (wnorm wconv eol escapes); this simultaneously checks 4 options from assertion above
        {
        case 0:  return strconv_attribute_impl<opt_false>::parse_simple;
        case 1:  return strconv_attribute_impl<opt_true>::parse_simple;
        case 2:  return strconv_attribute_impl<opt_false>::parse_eol;
        case 3:  return strconv_attribute_impl<opt_true>::parse_eol;
        case 4:  return strconv_attribute_impl<opt_false>::parse_wconv;
        case 5:  return strconv_attribute_impl<opt_true>::parse_wconv;
        case 6:  return strconv_attribute_impl<opt_false>::parse_wconv;
        case 7:  return strconv_attribute_impl<opt_true>::parse_wconv;
        case 8:  return strconv_attribute_impl<opt_false>::parse_wnorm;
        case 9:  return strconv_attribute_impl<opt_true>::parse_wnorm;
        case 10: return strconv_attribute_impl<opt_false>::parse_wnorm;
        case 11: return strconv_attribute_impl<opt_true>::parse_wnorm;
        case 12: return strconv_attribute_impl<opt_false>::parse_wnorm;
        case 13: return strconv_attribute_impl<opt_true>::parse_wnorm;
        case 14: return strconv_attribute_impl<opt_false>::parse_wnorm;
        case 15: return strconv_attribute_impl<opt_true>::parse_wnorm;
        default: assert(false); return 0; // unreachable
        }
    }

    inline xml_parse_result make_parse_result(xml_parse_status status, ptrdiff_t offset = 0)
    {
        xml_parse_result result;
        result.status = status;
        result.offset = offset;

        return result;
    }

    struct xml_parser
    {
        xml_allocator* alloc;
        char_t* error_offset;
        xml_parse_status error_status;

        xml_parser(xml_allocator* alloc_): alloc(alloc_), error_offset(0), error_status(status_ok)
        {
        }

        // DOCTYPE consists of nested sections of the following possible types:
        // <!-- ... -->, <? ... ?>, "...", '...'
        // <![...]]>
        // <!...>
        // First group can not contain nested groups
        // Second group can contain nested groups of the same type
        // Third group can contain all other groups
        char_t* parse_doctype_primitive(char_t* s)
        {
            if (*s == '"' || *s == '\'')
            {
                // quoted string
                char_t ch = *s++;
                PUGI__SCANFOR(*s == ch);
                if (!*s) PUGI__THROW_ERROR(status_bad_doctype, s);

                s++;
            }
            else if (s[0] == '<' && s[1] == '?')
            {
                // <? ... ?>
                s += 2;
                PUGI__SCANFOR(s[0] == '?' && s[1] == '>'); // no need for ENDSWITH because ?> can't terminate proper doctype
                if (!*s) PUGI__THROW_ERROR(status_bad_doctype, s);

                s += 2;
            }
            else if (s[0] == '<' && s[1] == '!' && s[2] == '-' && s[3] == '-')
            {
                s += 4;
                PUGI__SCANFOR(s[0] == '-' && s[1] == '-' && s[2] == '>'); // no need for ENDSWITH because --> can't terminate proper doctype
                if (!*s) PUGI__THROW_ERROR(status_bad_doctype, s);

                s += 3;
            }
            else PUGI__THROW_ERROR(status_bad_doctype, s);

            return s;
        }

        char_t* parse_doctype_ignore(char_t* s)
        {
            size_t depth = 0;

            assert(s[0] == '<' && s[1] == '!' && s[2] == '[');
            s += 3;

            while (*s)
            {
                if (s[0] == '<' && s[1] == '!' && s[2] == '[')
                {
                    // nested ignore section
                    s += 3;
                    depth++;
                }
                else if (s[0] == ']' && s[1] == ']' && s[2] == '>')
                {
                    // ignore section end
                    s += 3;

                    if (depth == 0)
                        return s;

                    depth--;
                }
                else s++;
            }

            PUGI__THROW_ERROR(status_bad_doctype, s);
        }

        char_t* parse_doctype_group(char_t* s, char_t endch)
        {
            size_t depth = 0;

            assert((s[0] == '<' || s[0] == 0) && s[1] == '!');
            s += 2;

            while (*s)
            {
                if (s[0] == '<' && s[1] == '!' && s[2] != '-')
                {
                    if (s[2] == '[')
                    {
                        // ignore
                        s = parse_doctype_ignore(s);
                        if (!s) return s;
                    }
                    else
                    {
                        // some control group
                        s += 2;
                        depth++;
                    }
                }
                else if (s[0] == '<' || s[0] == '"' || s[0] == '\'')
                {
                    // unknown tag (forbidden), or some primitive group
                    s = parse_doctype_primitive(s);
                    if (!s) return s;
                }
                else if (*s == '>')
                {
                    if (depth == 0)
                        return s;

                    depth--;
                    s++;
                }
                else s++;
            }

            if (depth != 0 || endch != '>') PUGI__THROW_ERROR(status_bad_doctype, s);

            return s;
        }

        char_t* parse_exclamation(char_t* s, xml_node_struct* cursor, unsigned int optmsk, char_t endch)
        {
            // parse node contents, starting with exclamation mark
            ++s;

            if (*s == '-') // '<!-...'
            {
                ++s;

                if (*s == '-') // '<!--...'
                {
                    ++s;

                    if (PUGI__OPTSET(parse_comments))
                    {
                        PUGI__PUSHNODE(node_comment); // Append a new node on the tree.
                        cursor->value = s; // Save the offset.
                    }

                    if (PUGI__OPTSET(parse_eol) && PUGI__OPTSET(parse_comments))
                    {
                        s = strconv_comment(s, endch);

                        if (!s) PUGI__THROW_ERROR(status_bad_comment, cursor->value);
                    }
                    else
                    {
                        // Scan for terminating '-->'.
                        PUGI__SCANFOR(s[0] == '-' && s[1] == '-' && PUGI__ENDSWITH(s[2], '>'));
                        PUGI__CHECK_ERROR(status_bad_comment, s);

                        if (PUGI__OPTSET(parse_comments))
                            *s = 0; // Zero-terminate this segment at the first terminating '-'.

                        s += (s[2] == '>' ? 3 : 2); // Step over the '\0->'.
                    }
                }
                else PUGI__THROW_ERROR(status_bad_comment, s);
            }
            else if (*s == '[')
            {
                // '<![CDATA[...'
                if (*++s=='C' && *++s=='D' && *++s=='A' && *++s=='T' && *++s=='A' && *++s == '[')
                {
                    ++s;

                    if (PUGI__OPTSET(parse_cdata))
                    {
                        PUGI__PUSHNODE(node_cdata); // Append a new node on the tree.
                        cursor->value = s; // Save the offset.

                        if (PUGI__OPTSET(parse_eol))
                        {
                            s = strconv_cdata(s, endch);

                            if (!s) PUGI__THROW_ERROR(status_bad_cdata, cursor->value);
                        }
                        else
                        {
                            // Scan for terminating ']]>'.
                            PUGI__SCANFOR(s[0] == ']' && s[1] == ']' && PUGI__ENDSWITH(s[2], '>'));
                            PUGI__CHECK_ERROR(status_bad_cdata, s);

                            *s++ = 0; // Zero-terminate this segment.
                        }
                    }
                    else // Flagged for discard, but we still have to scan for the terminator.
                    {
                        // Scan for terminating ']]>'.
                        PUGI__SCANFOR(s[0] == ']' && s[1] == ']' && PUGI__ENDSWITH(s[2], '>'));
                        PUGI__CHECK_ERROR(status_bad_cdata, s);

                        ++s;
                    }

                    s += (s[1] == '>' ? 2 : 1); // Step over the last ']>'.
                }
                else PUGI__THROW_ERROR(status_bad_cdata, s);
            }
            else if (s[0] == 'D' && s[1] == 'O' && s[2] == 'C' && s[3] == 'T' && s[4] == 'Y' && s[5] == 'P' && PUGI__ENDSWITH(s[6], 'E'))
            {
                s -= 2;

                if (cursor->parent) PUGI__THROW_ERROR(status_bad_doctype, s);

                char_t* mark = s + 9;

                s = parse_doctype_group(s, endch);
                if (!s) return s;

                assert((*s == 0 && endch == '>') || *s == '>');
                if (*s) *s++ = 0;

                if (PUGI__OPTSET(parse_doctype))
                {
                    while (PUGI__IS_CHARTYPE(*mark, ct_space)) ++mark;

                    PUGI__PUSHNODE(node_doctype);

                    cursor->value = mark;
                }
            }
            else if (*s == 0 && endch == '-') PUGI__THROW_ERROR(status_bad_comment, s);
            else if (*s == 0 && endch == '[') PUGI__THROW_ERROR(status_bad_cdata, s);
            else PUGI__THROW_ERROR(status_unrecognized_tag, s);

            return s;
        }

        char_t* parse_question(char_t* s, xml_node_struct*& ref_cursor, unsigned int optmsk, char_t endch)
        {
            // load into registers
            xml_node_struct* cursor = ref_cursor;
            char_t ch = 0;

            // parse node contents, starting with question mark
            ++s;

            // read PI target
            char_t* target = s;

            if (!PUGI__IS_CHARTYPE(*s, ct_start_symbol)) PUGI__THROW_ERROR(status_bad_pi, s);

            PUGI__SCANWHILE(PUGI__IS_CHARTYPE(*s, ct_symbol));
            PUGI__CHECK_ERROR(status_bad_pi, s);

            // determine node type; stricmp / strcasecmp is not portable
            bool declaration = (target[0] | ' ') == 'x' && (target[1] | ' ') == 'm' && (target[2] | ' ') == 'l' && target + 3 == s;

            if (declaration ? PUGI__OPTSET(parse_declaration) : PUGI__OPTSET(parse_pi))
            {
                if (declaration)
                {
                    // disallow non top-level declarations
                    if (cursor->parent) PUGI__THROW_ERROR(status_bad_pi, s);

                    PUGI__PUSHNODE(node_declaration);
                }
                else
                {
                    PUGI__PUSHNODE(node_pi);
                }

                cursor->name = target;

                PUGI__ENDSEG();

                // parse value/attributes
                if (ch == '?')
                {
                    // empty node
                    if (!PUGI__ENDSWITH(*s, '>')) PUGI__THROW_ERROR(status_bad_pi, s);
                    s += (*s == '>');

                    PUGI__POPNODE();
                }
                else if (PUGI__IS_CHARTYPE(ch, ct_space))
                {
                    PUGI__SKIPWS();

                    // scan for tag end
                    char_t* value = s;

                    PUGI__SCANFOR(s[0] == '?' && PUGI__ENDSWITH(s[1], '>'));
                    PUGI__CHECK_ERROR(status_bad_pi, s);

                    if (declaration)
                    {
                        // replace ending ? with / so that 'element' terminates properly
                        *s = '/';

                        // we exit from this function with cursor at node_declaration, which is a signal to parse() to go to LOC_ATTRIBUTES
                        s = value;
                    }
                    else
                    {
                        // store value and step over >
                        cursor->value = value;

                        PUGI__POPNODE();

                        PUGI__ENDSEG();

                        s += (*s == '>');
                    }
                }
                else PUGI__THROW_ERROR(status_bad_pi, s);
            }
            else
            {
                // scan for tag end
                PUGI__SCANFOR(s[0] == '?' && PUGI__ENDSWITH(s[1], '>'));
                PUGI__CHECK_ERROR(status_bad_pi, s);

                s += (s[1] == '>' ? 2 : 1);
            }

            // store from registers
            ref_cursor = cursor;

            return s;
        }

        char_t* parse_tree(char_t* s, xml_node_struct* root, unsigned int optmsk, char_t endch)
        {
            strconv_attribute_t strconv_attribute = get_strconv_attribute(optmsk);
            strconv_pcdata_t strconv_pcdata = get_strconv_pcdata(optmsk);

            char_t ch = 0;
            xml_node_struct* cursor = root;
            char_t* mark = s;

            while (*s != 0)
            {
                if (*s == '<')
                {
                    ++s;

                LOC_TAG:
                    if (PUGI__IS_CHARTYPE(*s, ct_start_symbol)) // '<#...'
                    {
                        PUGI__PUSHNODE(node_element); // Append a new node to the tree.

                        cursor->name = s;

                        PUGI__SCANWHILE_UNROLL(PUGI__IS_CHARTYPE(ss, ct_symbol)); // Scan for a terminator.
                        PUGI__ENDSEG(); // Save char in 'ch', terminate & step over.

                        if (ch == '>')
                        {
                            // end of tag
                        }
                        else if (PUGI__IS_CHARTYPE(ch, ct_space))
                        {
                        LOC_ATTRIBUTES:
                            while (true)
                            {
                                PUGI__SKIPWS(); // Eat any whitespace.

                                if (PUGI__IS_CHARTYPE(*s, ct_start_symbol)) // <... #...
                                {
                                    xml_attribute_struct* a = append_new_attribute(cursor, *alloc); // Make space for this attribute.
                                    if (!a) PUGI__THROW_ERROR(status_out_of_memory, s);

                                    a->name = s; // Save the offset.

                                    PUGI__SCANWHILE_UNROLL(PUGI__IS_CHARTYPE(ss, ct_symbol)); // Scan for a terminator.
                                    PUGI__ENDSEG(); // Save char in 'ch', terminate & step over.

                                    if (PUGI__IS_CHARTYPE(ch, ct_space))
                                    {
                                        PUGI__SKIPWS(); // Eat any whitespace.

                                        ch = *s;
                                        ++s;
                                    }

                                    if (ch == '=') // '<... #=...'
                                    {
                                        PUGI__SKIPWS(); // Eat any whitespace.

                                        if (*s == '"' || *s == '\'') // '<... #="...'
                                        {
                                            ch = *s; // Save quote char to avoid breaking on "''" -or- '""'.
                                            ++s; // Step over the quote.
                                            a->value = s; // Save the offset.

                                            s = strconv_attribute(s, ch);

                                            if (!s) PUGI__THROW_ERROR(status_bad_attribute, a->value);

                                            // After this line the loop continues from the start;
                                            // Whitespaces, / and > are ok, symbols and EOF are wrong,
                                            // everything else will be detected
                                            if (PUGI__IS_CHARTYPE(*s, ct_start_symbol)) PUGI__THROW_ERROR(status_bad_attribute, s);
                                        }
                                        else PUGI__THROW_ERROR(status_bad_attribute, s);
                                    }
                                    else PUGI__THROW_ERROR(status_bad_attribute, s);
                                }
                                else if (*s == '/')
                                {
                                    ++s;

                                    if (*s == '>')
                                    {
                                        PUGI__POPNODE();
                                        s++;
                                        break;
                                    }
                                    else if (*s == 0 && endch == '>')
                                    {
                                        PUGI__POPNODE();
                                        break;
                                    }
                                    else PUGI__THROW_ERROR(status_bad_start_element, s);
                                }
                                else if (*s == '>')
                                {
                                    ++s;

                                    break;
                                }
                                else if (*s == 0 && endch == '>')
                                {
                                    break;
                                }
                                else PUGI__THROW_ERROR(status_bad_start_element, s);
                            }

                            // !!!
                        }
                        else if (ch == '/') // '<#.../'
                        {
                            if (!PUGI__ENDSWITH(*s, '>')) PUGI__THROW_ERROR(status_bad_start_element, s);

                            PUGI__POPNODE(); // Pop.

                            s += (*s == '>');
                        }
                        else if (ch == 0)
                        {
                            // we stepped over null terminator, backtrack & handle closing tag
                            --s;

                            if (endch != '>') PUGI__THROW_ERROR(status_bad_start_element, s);
                        }
                        else PUGI__THROW_ERROR(status_bad_start_element, s);
                    }
                    else if (*s == '/')
                    {
                        ++s;

                        mark = s;

                        char_t* name = cursor->name;
                        if (!name) PUGI__THROW_ERROR(status_end_element_mismatch, mark);

                        while (PUGI__IS_CHARTYPE(*s, ct_symbol))
                        {
                            if (*s++ != *name++) PUGI__THROW_ERROR(status_end_element_mismatch, mark);
                        }

                        if (*name)
                        {
                            if (*s == 0 && name[0] == endch && name[1] == 0) PUGI__THROW_ERROR(status_bad_end_element, s);
                            else PUGI__THROW_ERROR(status_end_element_mismatch, mark);
                        }

                        PUGI__POPNODE(); // Pop.

                        PUGI__SKIPWS();

                        if (*s == 0)
                        {
                            if (endch != '>') PUGI__THROW_ERROR(status_bad_end_element, s);
                        }
                        else
                        {
                            if (*s != '>') PUGI__THROW_ERROR(status_bad_end_element, s);
                            ++s;
                        }
                    }
                    else if (*s == '?') // '<?...'
                    {
                        s = parse_question(s, cursor, optmsk, endch);
                        if (!s) return s;

                        assert(cursor);
                        if (PUGI__NODETYPE(cursor) == node_declaration) goto LOC_ATTRIBUTES;
                    }
                    else if (*s == '!') // '<!...'
                    {
                        s = parse_exclamation(s, cursor, optmsk, endch);
                        if (!s) return s;
                    }
                    else if (*s == 0 && endch == '?') PUGI__THROW_ERROR(status_bad_pi, s);
                    else PUGI__THROW_ERROR(status_unrecognized_tag, s);
                }
                else
                {
                    mark = s; // Save this offset while searching for a terminator.

                    PUGI__SKIPWS(); // Eat whitespace if no genuine PCDATA here.

                    if (*s == '<' || !*s)
                    {
                        // We skipped some whitespace characters because otherwise we would take the tag branch instead of PCDATA one
                        assert(mark != s);

                        if (!PUGI__OPTSET(parse_ws_pcdata | parse_ws_pcdata_single) || PUGI__OPTSET(parse_trim_pcdata))
                        {
                            continue;
                        }
                        else if (PUGI__OPTSET(parse_ws_pcdata_single))
                        {
                            if (s[0] != '<' || s[1] != '/' || cursor->first_child) continue;
                        }
                    }

                    if (!PUGI__OPTSET(parse_trim_pcdata))
                        s = mark;

                    if (cursor->parent || PUGI__OPTSET(parse_fragment))
                    {
                        if (PUGI__OPTSET(parse_embed_pcdata) && cursor->parent && !cursor->first_child && !cursor->value)
                        {
                            cursor->value = s; // Save the offset.
                        }
                        else
                        {
                            PUGI__PUSHNODE(node_pcdata); // Append a new node on the tree.

                            cursor->value = s; // Save the offset.

                            PUGI__POPNODE(); // Pop since this is a standalone.
                        }

                        s = strconv_pcdata(s);

                        if (!*s) break;
                    }
                    else
                    {
                        PUGI__SCANFOR(*s == '<'); // '...<'
                        if (!*s) break;

                        ++s;
                    }

                    // We're after '<'
                    goto LOC_TAG;
                }
            }

            // check that last tag is closed
            if (cursor != root) PUGI__THROW_ERROR(status_end_element_mismatch, s);

            return s;
        }

    #ifdef PUGIXML_WCHAR_MODE
        static char_t* parse_skip_bom(char_t* s)
        {
            unsigned int bom = 0xfeff;
            return (s[0] == static_cast<wchar_t>(bom)) ? s + 1 : s;
        }
    #else
        static char_t* parse_skip_bom(char_t* s)
        {
            return (s[0] == '\xef' && s[1] == '\xbb' && s[2] == '\xbf') ? s + 3 : s;
        }
    #endif

        static bool has_element_node_siblings(xml_node_struct* node)
        {
            while (node)
            {
                if (PUGI__NODETYPE(node) == node_element) return true;

                node = node->next_sibling;
            }

            return false;
        }

        static xml_parse_result parse(char_t* buffer, size_t length, xml_document_struct* xmldoc, xml_node_struct* root, unsigned int optmsk)
        {
            // early-out for empty documents
            if (length == 0)
                return make_parse_result(PUGI__OPTSET(parse_fragment) ? status_ok : status_no_document_element);

            // get last child of the root before parsing
            xml_node_struct* last_root_child = root->first_child ? root->first_child->prev_sibling_c + 0 : 0;

            // create parser on stack
            xml_parser parser(static_cast<xml_allocator*>(xmldoc));

            // save last character and make buffer zero-terminated (speeds up parsing)
            char_t endch = buffer[length - 1];
            buffer[length - 1] = 0;

            // skip BOM to make sure it does not end up as part of parse output
            char_t* buffer_data = parse_skip_bom(buffer);

            // perform actual parsing
            parser.parse_tree(buffer_data, root, optmsk, endch);

            xml_parse_result result = make_parse_result(parser.error_status, parser.error_offset ? parser.error_offset - buffer : 0);
            assert(result.offset >= 0 && static_cast<size_t>(result.offset) <= length);

            if (result)
            {
                // since we removed last character, we have to handle the only possible false positive (stray <)
                if (endch == '<')
                    return make_parse_result(status_unrecognized_tag, length - 1);

                // check if there are any element nodes parsed
                xml_node_struct* first_root_child_parsed = last_root_child ? last_root_child->next_sibling + 0 : root->first_child+ 0;

                if (!PUGI__OPTSET(parse_fragment) && !has_element_node_siblings(first_root_child_parsed))
                    return make_parse_result(status_no_document_element, length - 1);
            }
            else
            {
                // roll back offset if it occurs on a null terminator in the source buffer
                if (result.offset > 0 && static_cast<size_t>(result.offset) == length - 1 && endch == 0)
                    result.offset--;
            }

            return result;
        }
    };

    // Output facilities
    PUGI__FN xml_encoding get_write_native_encoding()
    {
    #ifdef PUGIXML_WCHAR_MODE
        return get_wchar_encoding();
    #else
        return encoding_utf8;
    #endif
    }

    PUGI__FN xml_encoding get_write_encoding(xml_encoding encoding)
    {
        // replace wchar encoding with utf implementation
        if (encoding == encoding_wchar) return get_wchar_encoding();

        // replace utf16 encoding with utf16 with specific endianness
        if (encoding == encoding_utf16) return is_little_endian() ? encoding_utf16_le : encoding_utf16_be;

        // replace utf32 encoding with utf32 with specific endianness
        if (encoding == encoding_utf32) return is_little_endian() ? encoding_utf32_le : encoding_utf32_be;

        // only do autodetection if no explicit encoding is requested
        if (encoding != encoding_auto) return encoding;

        // assume utf8 encoding
        return encoding_utf8;
    }

    template <typename D, typename T> PUGI__FN size_t convert_buffer_output_generic(typename T::value_type dest, const char_t* data, size_t length, D, T)
    {
        PUGI__STATIC_ASSERT(sizeof(char_t) == sizeof(typename D::type));

        typename T::value_type end = D::process(reinterpret_cast<const typename D::type*>(data), length, dest, T());

        return static_cast<size_t>(end - dest) * sizeof(*dest);
    }

    template <typename D, typename T> PUGI__FN size_t convert_buffer_output_generic(typename T::value_type dest, const char_t* data, size_t length, D, T, bool opt_swap)
    {
        PUGI__STATIC_ASSERT(sizeof(char_t) == sizeof(typename D::type));

        typename T::value_type end = D::process(reinterpret_cast<const typename D::type*>(data), length, dest, T());

        if (opt_swap)
        {
            for (typename T::value_type i = dest; i != end; ++i)
                *i = endian_swap(*i);
        }

        return static_cast<size_t>(end - dest) * sizeof(*dest);
    }

#ifdef PUGIXML_WCHAR_MODE
    PUGI__FN size_t get_valid_length(const char_t* data, size_t length)
    {
        if (length < 1) return 0;

        // discard last character if it's the lead of a surrogate pair
        return (sizeof(wchar_t) == 2 && static_cast<unsigned int>(static_cast<uint16_t>(data[length - 1]) - 0xD800) < 0x400) ? length - 1 : length;
    }

    PUGI__FN size_t convert_buffer_output(char_t* r_char, uint8_t* r_u8, uint16_t* r_u16, uint32_t* r_u32, const char_t* data, size_t length, xml_encoding encoding)
    {
        // only endian-swapping is required
        if (need_endian_swap_utf(encoding, get_wchar_encoding()))
        {
            convert_wchar_endian_swap(r_char, data, length);

            return length * sizeof(char_t);
        }

        // convert to utf8
        if (encoding == encoding_utf8)
            return convert_buffer_output_generic(r_u8, data, length, wchar_decoder(), utf8_writer());

        // convert to utf16
        if (encoding == encoding_utf16_be || encoding == encoding_utf16_le)
        {
            xml_encoding native_encoding = is_little_endian() ? encoding_utf16_le : encoding_utf16_be;

            return convert_buffer_output_generic(r_u16, data, length, wchar_decoder(), utf16_writer(), native_encoding != encoding);
        }

        // convert to utf32
        if (encoding == encoding_utf32_be || encoding == encoding_utf32_le)
        {
            xml_encoding native_encoding = is_little_endian() ? encoding_utf32_le : encoding_utf32_be;

            return convert_buffer_output_generic(r_u32, data, length, wchar_decoder(), utf32_writer(), native_encoding != encoding);
        }

        // convert to latin1
        if (encoding == encoding_latin1)
            return convert_buffer_output_generic(r_u8, data, length, wchar_decoder(), latin1_writer());

        assert(false && "Invalid encoding"); // unreachable
        return 0;
    }
#else
    PUGI__FN size_t get_valid_length(const char_t* data, size_t length)
    {
        if (length < 5) return 0;

        for (size_t i = 1; i <= 4; ++i)
        {
            uint8_t ch = static_cast<uint8_t>(data[length - i]);

            // either a standalone character or a leading one
            if ((ch & 0xc0) != 0x80) return length - i;
        }

        // there are four non-leading characters at the end, sequence tail is broken so might as well process the whole chunk
        return length;
    }

    PUGI__FN size_t convert_buffer_output(char_t* /* r_char */, uint8_t* r_u8, uint16_t* r_u16, uint32_t* r_u32, const char_t* data, size_t length, xml_encoding encoding)
    {
        if (encoding == encoding_utf16_be || encoding == encoding_utf16_le)
        {
            xml_encoding native_encoding = is_little_endian() ? encoding_utf16_le : encoding_utf16_be;

            return convert_buffer_output_generic(r_u16, data, length, utf8_decoder(), utf16_writer(), native_encoding != encoding);
        }

        if (encoding == encoding_utf32_be || encoding == encoding_utf32_le)
        {
            xml_encoding native_encoding = is_little_endian() ? encoding_utf32_le : encoding_utf32_be;

            return convert_buffer_output_generic(r_u32, data, length, utf8_decoder(), utf32_writer(), native_encoding != encoding);
        }

        if (encoding == encoding_latin1)
            return convert_buffer_output_generic(r_u8, data, length, utf8_decoder(), latin1_writer());

        assert(false && "Invalid encoding"); // unreachable
        return 0;
    }
#endif

    class xml_buffered_writer
    {
        xml_buffered_writer(const xml_buffered_writer&);
        xml_buffered_writer& operator=(const xml_buffered_writer&);

    public:
        xml_buffered_writer(xml_writer& writer_, xml_encoding user_encoding): writer(writer_), bufsize(0), encoding(get_write_encoding(user_encoding))
        {
            PUGI__STATIC_ASSERT(bufcapacity >= 8);
        }

        size_t flush()
        {
            flush(buffer, bufsize);
            bufsize = 0;
            return 0;
        }

        void flush(const char_t* data, size_t size)
        {
            if (size == 0) return;

            // fast path, just write data
            if (encoding == get_write_native_encoding())
                writer.write(data, size * sizeof(char_t));
            else
            {
                // convert chunk
                size_t result = convert_buffer_output(scratch.data_char, scratch.data_u8, scratch.data_u16, scratch.data_u32, data, size, encoding);
                assert(result <= sizeof(scratch));

                // write data
                writer.write(scratch.data_u8, result);
            }
        }

        void write_direct(const char_t* data, size_t length)
        {
            // flush the remaining buffer contents
            flush();

            // handle large chunks
            if (length > bufcapacity)
            {
                if (encoding == get_write_native_encoding())
                {
                    // fast path, can just write data chunk
                    writer.write(data, length * sizeof(char_t));
                    return;
                }

                // need to convert in suitable chunks
                while (length > bufcapacity)
                {
                    // get chunk size by selecting such number of characters that are guaranteed to fit into scratch buffer
                    // and form a complete codepoint sequence (i.e. discard start of last codepoint if necessary)
                    size_t chunk_size = get_valid_length(data, bufcapacity);
                    assert(chunk_size);

                    // convert chunk and write
                    flush(data, chunk_size);

                    // iterate
                    data += chunk_size;
                    length -= chunk_size;
                }

                // small tail is copied below
                bufsize = 0;
            }

            memcpy(buffer + bufsize, data, length * sizeof(char_t));
            bufsize += length;
        }

        void write_buffer(const char_t* data, size_t length)
        {
            size_t offset = bufsize;

            if (offset + length <= bufcapacity)
            {
                memcpy(buffer + offset, data, length * sizeof(char_t));
                bufsize = offset + length;
            }
            else
            {
                write_direct(data, length);
            }
        }

        void write_string(const char_t* data)
        {
            // write the part of the string that fits in the buffer
            size_t offset = bufsize;

            while (*data && offset < bufcapacity)
                buffer[offset++] = *data++;

            // write the rest
            if (offset < bufcapacity)
            {
                bufsize = offset;
            }
            else
            {
                // backtrack a bit if we have split the codepoint
                size_t length = offset - bufsize;
                size_t extra = length - get_valid_length(data - length, length);

                bufsize = offset - extra;

                write_direct(data - extra, strlength(data) + extra);
            }
        }

        void write(char_t d0)
        {
            size_t offset = bufsize;
            if (offset > bufcapacity - 1) offset = flush();

            buffer[offset + 0] = d0;
            bufsize = offset + 1;
        }

        void write(char_t d0, char_t d1)
        {
            size_t offset = bufsize;
            if (offset > bufcapacity - 2) offset = flush();

            buffer[offset + 0] = d0;
            buffer[offset + 1] = d1;
            bufsize = offset + 2;
        }

        void write(char_t d0, char_t d1, char_t d2)
        {
            size_t offset = bufsize;
            if (offset > bufcapacity - 3) offset = flush();

            buffer[offset + 0] = d0;
            buffer[offset + 1] = d1;
            buffer[offset + 2] = d2;
            bufsize = offset + 3;
        }

        void write(char_t d0, char_t d1, char_t d2, char_t d3)
        {
            size_t offset = bufsize;
            if (offset > bufcapacity - 4) offset = flush();

            buffer[offset + 0] = d0;
            buffer[offset + 1] = d1;
            buffer[offset + 2] = d2;
            buffer[offset + 3] = d3;
            bufsize = offset + 4;
        }

        void write(char_t d0, char_t d1, char_t d2, char_t d3, char_t d4)
        {
            size_t offset = bufsize;
            if (offset > bufcapacity - 5) offset = flush();

            buffer[offset + 0] = d0;
            buffer[offset + 1] = d1;
            buffer[offset + 2] = d2;
            buffer[offset + 3] = d3;
            buffer[offset + 4] = d4;
            bufsize = offset + 5;
        }

        void write(char_t d0, char_t d1, char_t d2, char_t d3, char_t d4, char_t d5)
        {
            size_t offset = bufsize;
            if (offset > bufcapacity - 6) offset = flush();

            buffer[offset + 0] = d0;
            buffer[offset + 1] = d1;
            buffer[offset + 2] = d2;
            buffer[offset + 3] = d3;
            buffer[offset + 4] = d4;
            buffer[offset + 5] = d5;
            bufsize = offset + 6;
        }

        // utf8 maximum expansion: x4 (-> utf32)
        // utf16 maximum expansion: x2 (-> utf32)
        // utf32 maximum expansion: x1
        enum
        {
            bufcapacitybytes =
            #ifdef PUGIXML_MEMORY_OUTPUT_STACK
                PUGIXML_MEMORY_OUTPUT_STACK
            #else
                10240
            #endif
            ,
            bufcapacity = bufcapacitybytes / (sizeof(char_t) + 4)
        };

        char_t buffer[bufcapacity];

        union
        {
            uint8_t data_u8[4 * bufcapacity];
            uint16_t data_u16[2 * bufcapacity];
            uint32_t data_u32[bufcapacity];
            char_t data_char[bufcapacity];
        } scratch;

        xml_writer& writer;
        size_t bufsize;
        xml_encoding encoding;
    };

    PUGI__FN void text_output_escaped(xml_buffered_writer& writer, const char_t* s, chartypex_t type, unsigned int flags)
    {
        while (*s)
        {
            const char_t* prev = s;

            // While *s is a usual symbol
            PUGI__SCANWHILE_UNROLL(!PUGI__IS_CHARTYPEX(ss, type));

            writer.write_buffer(prev, static_cast<size_t>(s - prev));

            switch (*s)
            {
                case 0: break;
                case '&':
                    writer.write('&', 'a', 'm', 'p', ';');
                    ++s;
                    break;
                case '<':
                    writer.write('&', 'l', 't', ';');
                    ++s;
                    break;
                case '>':
                    writer.write('&', 'g', 't', ';');
                    ++s;
                    break;
                case '"':
                    if (flags & format_attribute_single_quote)
                        writer.write('"');
                    else
                        writer.write('&', 'q', 'u', 'o', 't', ';');
                    ++s;
                    break;
                case '\'':
                    if (flags & format_attribute_single_quote)
                        writer.write('&', 'a', 'p', 'o', 's', ';');
                    else
                        writer.write('\'');
                    ++s;
                    break;
                default: // s is not a usual symbol
                {
                    unsigned int ch = static_cast<unsigned int>(*s++);
                    assert(ch < 32);

                    if (!(flags & format_skip_control_chars))
                        writer.write('&', '#', static_cast<char_t>((ch / 10) + '0'), static_cast<char_t>((ch % 10) + '0'), ';');
                }
            }
        }
    }

    PUGI__FN void text_output(xml_buffered_writer& writer, const char_t* s, chartypex_t type, unsigned int flags)
    {
        if (flags & format_no_escapes)
            writer.write_string(s);
        else
            text_output_escaped(writer, s, type, flags);
    }

    PUGI__FN void text_output_cdata(xml_buffered_writer& writer, const char_t* s)
    {
        do
        {
            writer.write('<', '!', '[', 'C', 'D');
            writer.write('A', 'T', 'A', '[');

            const char_t* prev = s;

            // look for ]]> sequence - we can't output it as is since it terminates CDATA
            while (*s && !(s[0] == ']' && s[1] == ']' && s[2] == '>')) ++s;

            // skip ]] if we stopped at ]]>, > will go to the next CDATA section
            if (*s) s += 2;

            writer.write_buffer(prev, static_cast<size_t>(s - prev));

            writer.write(']', ']', '>');
        }
        while (*s);
    }

    PUGI__FN void text_output_indent(xml_buffered_writer& writer, const char_t* indent, size_t indent_length, unsigned int depth)
    {
        switch (indent_length)
        {
        case 1:
        {
            for (unsigned int i = 0; i < depth; ++i)
                writer.write(indent[0]);
            break;
        }

        case 2:
        {
            for (unsigned int i = 0; i < depth; ++i)
                writer.write(indent[0], indent[1]);
            break;
        }

        case 3:
        {
            for (unsigned int i = 0; i < depth; ++i)
                writer.write(indent[0], indent[1], indent[2]);
            break;
        }

        case 4:
        {
            for (unsigned int i = 0; i < depth; ++i)
                writer.write(indent[0], indent[1], indent[2], indent[3]);
            break;
        }

        default:
        {
            for (unsigned int i = 0; i < depth; ++i)
                writer.write_buffer(indent, indent_length);
        }
        }
    }

    PUGI__FN void node_output_comment(xml_buffered_writer& writer, const char_t* s)
    {
        writer.write('<', '!', '-', '-');

        while (*s)
        {
            const char_t* prev = s;

            // look for -\0 or -- sequence - we can't output it since -- is illegal in comment body
            while (*s && !(s[0] == '-' && (s[1] == '-' || s[1] == 0))) ++s;

            writer.write_buffer(prev, static_cast<size_t>(s - prev));

            if (*s)
            {
                assert(*s == '-');

                writer.write('-', ' ');
                ++s;
            }
        }

        writer.write('-', '-', '>');
    }

    PUGI__FN void node_output_pi_value(xml_buffered_writer& writer, const char_t* s)
    {
        while (*s)
        {
            const char_t* prev = s;

            // look for ?> sequence - we can't output it since ?> terminates PI
            while (*s && !(s[0] == '?' && s[1] == '>')) ++s;

            writer.write_buffer(prev, static_cast<size_t>(s - prev));

            if (*s)
            {
                assert(s[0] == '?' && s[1] == '>');

                writer.write('?', ' ', '>');
                s += 2;
            }
        }
    }

    PUGI__FN void node_output_attributes(xml_buffered_writer& writer, xml_node_struct* node, const char_t* indent, size_t indent_length, unsigned int flags, unsigned int depth)
    {
        const char_t* default_name = PUGIXML_TEXT(":anonymous");
        const char_t enquotation_char = (flags & format_attribute_single_quote) ? '\'' : '"';

        for (xml_attribute_struct* a = node->first_attribute; a; a = a->next_attribute)
        {
            if ((flags & (format_indent_attributes | format_raw)) == format_indent_attributes)
            {
                writer.write('\n');

                text_output_indent(writer, indent, indent_length, depth + 1);
            }
            else
            {
                writer.write(' ');
            }

            writer.write_string(a->name ? a->name + 0 : default_name);
            writer.write('=', enquotation_char);

            if (a->value)
                text_output(writer, a->value, ctx_special_attr, flags);

            writer.write(enquotation_char);
        }
    }

    PUGI__FN bool node_output_start(xml_buffered_writer& writer, xml_node_struct* node, const char_t* indent, size_t indent_length, unsigned int flags, unsigned int depth)
    {
        const char_t* default_name = PUGIXML_TEXT(":anonymous");
        const char_t* name = node->name ? node->name + 0 : default_name;

        writer.write('<');
        writer.write_string(name);

        if (node->first_attribute)
            node_output_attributes(writer, node, indent, indent_length, flags, depth);

        // element nodes can have value if parse_embed_pcdata was used
        if (!node->value)
        {
            if (!node->first_child)
            {
                if (flags & format_no_empty_element_tags)
                {
                    writer.write('>', '<', '/');
                    writer.write_string(name);
                    writer.write('>');

                    return false;
                }
                else
                {
                    if ((flags & format_raw) == 0)
                        writer.write(' ');

                    writer.write('/', '>');

                    return false;
                }
            }
            else
            {
                writer.write('>');

                return true;
            }
        }
        else
        {
            writer.write('>');

            text_output(writer, node->value, ctx_special_pcdata, flags);

            if (!node->first_child)
            {
                writer.write('<', '/');
                writer.write_string(name);
                writer.write('>');

                return false;
            }
            else
            {
                return true;
            }
        }
    }

    PUGI__FN void node_output_end(xml_buffered_writer& writer, xml_node_struct* node)
    {
        const char_t* default_name = PUGIXML_TEXT(":anonymous");
        const char_t* name = node->name ? node->name + 0 : default_name;

        writer.write('<', '/');
        writer.write_string(name);
        writer.write('>');
    }

    PUGI__FN void node_output_simple(xml_buffered_writer& writer, xml_node_struct* node, unsigned int flags)
    {
        const char_t* default_name = PUGIXML_TEXT(":anonymous");

        switch (PUGI__NODETYPE(node))
        {
            case node_pcdata:
                text_output(writer, node->value ? node->value + 0 : PUGIXML_TEXT(""), ctx_special_pcdata, flags);
                break;

            case node_cdata:
                text_output_cdata(writer, node->value ? node->value + 0 : PUGIXML_TEXT(""));
                break;

            case node_comment:
                node_output_comment(writer, node->value ? node->value + 0 : PUGIXML_TEXT(""));
                break;

            case node_pi:
                writer.write('<', '?');
                writer.write_string(node->name ? node->name + 0 : default_name);

                if (node->value)
                {
                    writer.write(' ');
                    node_output_pi_value(writer, node->value);
                }

                writer.write('?', '>');
                break;

            case node_declaration:
                writer.write('<', '?');
                writer.write_string(node->name ? node->name + 0 : default_name);
                node_output_attributes(writer, node, PUGIXML_TEXT(""), 0, flags | format_raw, 0);
                writer.write('?', '>');
                break;

            case node_doctype:
                writer.write('<', '!', 'D', 'O', 'C');
                writer.write('T', 'Y', 'P', 'E');

                if (node->value)
                {
                    writer.write(' ');
                    writer.write_string(node->value);
                }

                writer.write('>');
                break;

            default:
                assert(false && "Invalid node type"); // unreachable
        }
    }

    enum indent_flags_t
    {
        indent_newline = 1,
        indent_indent = 2
    };

    PUGI__FN void node_output(xml_buffered_writer& writer, xml_node_struct* root, const char_t* indent, unsigned int flags, unsigned int depth)
    {
        size_t indent_length = ((flags & (format_indent | format_indent_attributes)) && (flags & format_raw) == 0) ? strlength(indent) : 0;
        unsigned int indent_flags = indent_indent;

        xml_node_struct* node = root;

        do
        {
            assert(node);

            // begin writing current node
            if (PUGI__NODETYPE(node) == node_pcdata || PUGI__NODETYPE(node) == node_cdata)
            {
                node_output_simple(writer, node, flags);

                indent_flags = 0;
            }
            else
            {
                if ((indent_flags & indent_newline) && (flags & format_raw) == 0)
                    writer.write('\n');

                if ((indent_flags & indent_indent) && indent_length)
                    text_output_indent(writer, indent, indent_length, depth);

                if (PUGI__NODETYPE(node) == node_element)
                {
                    indent_flags = indent_newline | indent_indent;

                    if (node_output_start(writer, node, indent, indent_length, flags, depth))
                    {
                        // element nodes can have value if parse_embed_pcdata was used
                        if (node->value)
                            indent_flags = 0;

                        node = node->first_child;
                        depth++;
                        continue;
                    }
                }
                else if (PUGI__NODETYPE(node) == node_document)
                {
                    indent_flags = indent_indent;

                    if (node->first_child)
                    {
                        node = node->first_child;
                        continue;
                    }
                }
                else
                {
                    node_output_simple(writer, node, flags);

                    indent_flags = indent_newline | indent_indent;
                }
            }

            // continue to the next node
            while (node != root)
            {
                if (node->next_sibling)
                {
                    node = node->next_sibling;
                    break;
                }

                node = node->parent;

                // write closing node
                if (PUGI__NODETYPE(node) == node_element)
                {
                    depth--;

                    if ((indent_flags & indent_newline) && (flags & format_raw) == 0)
                        writer.write('\n');

                    if ((indent_flags & indent_indent) && indent_length)
                        text_output_indent(writer, indent, indent_length, depth);

                    node_output_end(writer, node);

                    indent_flags = indent_newline | indent_indent;
                }
            }
        }
        while (node != root);

        if ((indent_flags & indent_newline) && (flags & format_raw) == 0)
            writer.write('\n');
    }

    PUGI__FN bool has_declaration(xml_node_struct* node)
    {
        for (xml_node_struct* child = node->first_child; child; child = child->next_sibling)
        {
            xml_node_type type = PUGI__NODETYPE(child);

            if (type == node_declaration) return true;
            if (type == node_element) return false;
        }

        return false;
    }

    PUGI__FN bool is_attribute_of(xml_attribute_struct* attr, xml_node_struct* node)
    {
        for (xml_attribute_struct* a = node->first_attribute; a; a = a->next_attribute)
            if (a == attr)
                return true;

        return false;
    }

    PUGI__FN bool allow_insert_attribute(xml_node_type parent)
    {
        return parent == node_element || parent == node_declaration;
    }

    PUGI__FN bool allow_insert_child(xml_node_type parent, xml_node_type child)
    {
        if (parent != node_document && parent != node_element) return false;
        if (child == node_document || child == node_null) return false;
        if (parent != node_document && (child == node_declaration || child == node_doctype)) return false;

        return true;
    }

    PUGI__FN bool allow_move(xml_node parent, xml_node child)
    {
        // check that child can be a child of parent
        if (!allow_insert_child(parent.type(), child.type()))
            return false;

        // check that node is not moved between documents
        if (parent.root() != child.root())
            return false;

        // check that new parent is not in the child subtree
        xml_node cur = parent;

        while (cur)
        {
            if (cur == child)
                return false;

            cur = cur.parent();
        }

        return true;
    }

    template <typename String, typename Header>
    PUGI__FN void node_copy_string(String& dest, Header& header, uintptr_t header_mask, char_t* source, Header& source_header, xml_allocator* alloc)
    {
        assert(!dest && (header & header_mask) == 0);

        if (source)
        {
            if (alloc && (source_header & header_mask) == 0)
            {
                dest = source;

                // since strcpy_insitu can reuse document buffer memory we need to mark both source and dest as shared
                header |= xml_memory_page_contents_shared_mask;
                source_header |= xml_memory_page_contents_shared_mask;
            }
            else
                strcpy_insitu(dest, header, header_mask, source, strlength(source));
        }
    }

    PUGI__FN void node_copy_contents(xml_node_struct* dn, xml_node_struct* sn, xml_allocator* shared_alloc)
    {
        node_copy_string(dn->name, dn->header, xml_memory_page_name_allocated_mask, sn->name, sn->header, shared_alloc);
        node_copy_string(dn->value, dn->header, xml_memory_page_value_allocated_mask, sn->value, sn->header, shared_alloc);

        for (xml_attribute_struct* sa = sn->first_attribute; sa; sa = sa->next_attribute)
        {
            xml_attribute_struct* da = append_new_attribute(dn, get_allocator(dn));

            if (da)
            {
                node_copy_string(da->name, da->header, xml_memory_page_name_allocated_mask, sa->name, sa->header, shared_alloc);
                node_copy_string(da->value, da->header, xml_memory_page_value_allocated_mask, sa->value, sa->header, shared_alloc);
            }
        }
    }

    PUGI__FN void node_copy_tree(xml_node_struct* dn, xml_node_struct* sn)
    {
        xml_allocator& alloc = get_allocator(dn);
        xml_allocator* shared_alloc = (&alloc == &get_allocator(sn)) ? &alloc : 0;

        node_copy_contents(dn, sn, shared_alloc);

        xml_node_struct* dit = dn;
        xml_node_struct* sit = sn->first_child;

        while (sit && sit != sn)
        {
            // loop invariant: dit is inside the subtree rooted at dn
            assert(dit);

            // when a tree is copied into one of the descendants, we need to skip that subtree to avoid an infinite loop
            if (sit != dn)
            {
                xml_node_struct* copy = append_new_node(dit, alloc, PUGI__NODETYPE(sit));

                if (copy)
                {
                    node_copy_contents(copy, sit, shared_alloc);

                    if (sit->first_child)
                    {
                        dit = copy;
                        sit = sit->first_child;
                        continue;
                    }
                }
            }

            // continue to the next node
            do
            {
                if (sit->next_sibling)
                {
                    sit = sit->next_sibling;
                    break;
                }

                sit = sit->parent;
                dit = dit->parent;

                // loop invariant: dit is inside the subtree rooted at dn while sit is inside sn
                assert(sit == sn || dit);
            }
            while (sit != sn);
        }

        assert(!sit || dit == dn->parent);
    }

    PUGI__FN void node_copy_attribute(xml_attribute_struct* da, xml_attribute_struct* sa)
    {
        xml_allocator& alloc = get_allocator(da);
        xml_allocator* shared_alloc = (&alloc == &get_allocator(sa)) ? &alloc : 0;

        node_copy_string(da->name, da->header, xml_memory_page_name_allocated_mask, sa->name, sa->header, shared_alloc);
        node_copy_string(da->value, da->header, xml_memory_page_value_allocated_mask, sa->value, sa->header, shared_alloc);
    }

    inline bool is_text_node(xml_node_struct* node)
    {
        xml_node_type type = PUGI__NODETYPE(node);

        return type == node_pcdata || type == node_cdata;
    }

    // get value with conversion functions
    template <typename U> PUGI__FN PUGI__UNSIGNED_OVERFLOW U string_to_integer(const char_t* value, U minv, U maxv)
    {
        U result = 0;
        const char_t* s = value;

        while (PUGI__IS_CHARTYPE(*s, ct_space))
            s++;

        bool negative = (*s == '-');

        s += (*s == '+' || *s == '-');

        bool overflow = false;

        if (s[0] == '0' && (s[1] | ' ') == 'x')
        {
            s += 2;

            // since overflow detection relies on length of the sequence skip leading zeros
            while (*s == '0')
                s++;

            const char_t* start = s;

            for (;;)
            {
                if (static_cast<unsigned>(*s - '0') < 10)
                    result = result * 16 + (*s - '0');
                else if (static_cast<unsigned>((*s | ' ') - 'a') < 6)
                    result = result * 16 + ((*s | ' ') - 'a' + 10);
                else
                    break;

                s++;
            }

            size_t digits = static_cast<size_t>(s - start);

            overflow = digits > sizeof(U) * 2;
        }
        else
        {
            // since overflow detection relies on length of the sequence skip leading zeros
            while (*s == '0')
                s++;

            const char_t* start = s;

            for (;;)
            {
                if (static_cast<unsigned>(*s - '0') < 10)
                    result = result * 10 + (*s - '0');
                else
                    break;

                s++;
            }

            size_t digits = static_cast<size_t>(s - start);

            PUGI__STATIC_ASSERT(sizeof(U) == 8 || sizeof(U) == 4 || sizeof(U) == 2);

            const size_t max_digits10 = sizeof(U) == 8 ? 20 : sizeof(U) == 4 ? 10 : 5;
            const char_t max_lead = sizeof(U) == 8 ? '1' : sizeof(U) == 4 ? '4' : '6';
            const size_t high_bit = sizeof(U) * 8 - 1;

            overflow = digits >= max_digits10 && !(digits == max_digits10 && (*start < max_lead || (*start == max_lead && result >> high_bit)));
        }

        if (negative)
        {
            // Workaround for crayc++ CC-3059: Expected no overflow in routine.
        #ifdef _CRAYC
            return (overflow || result > ~minv + 1) ? minv : ~result + 1;
        #else
            return (overflow || result > 0 - minv) ? minv : 0 - result;
        #endif
        }
        else
            return (overflow || result > maxv) ? maxv : result;
    }

    PUGI__FN int get_value_int(const char_t* value)
    {
        return string_to_integer<unsigned int>(value, static_cast<unsigned int>(INT_MIN), INT_MAX);
    }

    PUGI__FN unsigned int get_value_uint(const char_t* value)
    {
        return string_to_integer<unsigned int>(value, 0, UINT_MAX);
    }

    PUGI__FN double get_value_double(const char_t* value)
    {
    #ifdef PUGIXML_WCHAR_MODE
        return wcstod(value, 0);
    #else
        return strtod(value, 0);
    #endif
    }

    PUGI__FN float get_value_float(const char_t* value)
    {
    #ifdef PUGIXML_WCHAR_MODE
        return static_cast<float>(wcstod(value, 0));
    #else
        return static_cast<float>(strtod(value, 0));
    #endif
    }

    PUGI__FN bool get_value_bool(const char_t* value)
    {
        // only look at first char
        char_t first = *value;

        // 1*, t* (true), T* (True), y* (yes), Y* (YES)
        return (first == '1' || first == 't' || first == 'T' || first == 'y' || first == 'Y');
    }

#ifdef PUGIXML_HAS_LONG_LONG
    PUGI__FN long long get_value_llong(const char_t* value)
    {
        return string_to_integer<unsigned long long>(value, static_cast<unsigned long long>(LLONG_MIN), LLONG_MAX);
    }

    PUGI__FN unsigned long long get_value_ullong(const char_t* value)
    {
        return string_to_integer<unsigned long long>(value, 0, ULLONG_MAX);
    }
#endif

    template <typename U> PUGI__FN PUGI__UNSIGNED_OVERFLOW char_t* integer_to_string(char_t* begin, char_t* end, U value, bool negative)
    {
        char_t* result = end - 1;
        U rest = negative ? 0 - value : value;

        do
        {
            *result-- = static_cast<char_t>('0' + (rest % 10));
            rest /= 10;
        }
        while (rest);

        assert(result >= begin);
        (void)begin;

        *result = '-';

        return result + !negative;
    }

    // set value with conversion functions
    template <typename String, typename Header>
    PUGI__FN bool set_value_ascii(String& dest, Header& header, uintptr_t header_mask, char* buf)
    {
    #ifdef PUGIXML_WCHAR_MODE
        char_t wbuf[128];
        assert(strlen(buf) < sizeof(wbuf) / sizeof(wbuf[0]));

        size_t offset = 0;
        for (; buf[offset]; ++offset) wbuf[offset] = buf[offset];

        return strcpy_insitu(dest, header, header_mask, wbuf, offset);
    #else
        return strcpy_insitu(dest, header, header_mask, buf, strlen(buf));
    #endif
    }

    template <typename U, typename String, typename Header>
    PUGI__FN bool set_value_integer(String& dest, Header& header, uintptr_t header_mask, U value, bool negative)
    {
        char_t buf[64];
        char_t* end = buf + sizeof(buf) / sizeof(buf[0]);
        char_t* begin = integer_to_string(buf, end, value, negative);

        return strcpy_insitu(dest, header, header_mask, begin, end - begin);
    }

    template <typename String, typename Header>
    PUGI__FN bool set_value_convert(String& dest, Header& header, uintptr_t header_mask, float value, int precision)
    {
        char buf[128];
        PUGI__SNPRINTF(buf, "%.*g", precision, double(value));

        return set_value_ascii(dest, header, header_mask, buf);
    }

    template <typename String, typename Header>
    PUGI__FN bool set_value_convert(String& dest, Header& header, uintptr_t header_mask, double value, int precision)
    {
        char buf[128];
        PUGI__SNPRINTF(buf, "%.*g", precision, value);

        return set_value_ascii(dest, header, header_mask, buf);
    }

    template <typename String, typename Header>
    PUGI__FN bool set_value_bool(String& dest, Header& header, uintptr_t header_mask, bool value)
    {
        return strcpy_insitu(dest, header, header_mask, value ? PUGIXML_TEXT("true") : PUGIXML_TEXT("false"), value ? 4 : 5);
    }

    PUGI__FN xml_parse_result load_buffer_impl(xml_document_struct* doc, xml_node_struct* root, void* contents, size_t size, unsigned int options, xml_encoding encoding, bool is_mutable, bool own, char_t** out_buffer)
    {
        // check input buffer
        if (!contents && size) return make_parse_result(status_io_error);

        // get actual encoding
        xml_encoding buffer_encoding = impl::get_buffer_encoding(encoding, contents, size);

        // get private buffer
        char_t* buffer = 0;
        size_t length = 0;

        // coverity[var_deref_model]
        if (!impl::convert_buffer(buffer, length, buffer_encoding, contents, size, is_mutable)) return impl::make_parse_result(status_out_of_memory);

        // delete original buffer if we performed a conversion
        if (own && buffer != contents && contents) impl::xml_memory::deallocate(contents);

        // grab onto buffer if it's our buffer, user is responsible for deallocating contents himself
        if (own || buffer != contents) *out_buffer = buffer;

        // store buffer for offset_debug
        doc->buffer = buffer;

        // parse
        xml_parse_result res = impl::xml_parser::parse(buffer, length, doc, root, options);

        // remember encoding
        res.encoding = buffer_encoding;

        return res;
    }

    // we need to get length of entire file to load it in memory; the only (relatively) sane way to do it is via seek/tell trick
    PUGI__FN xml_parse_status get_file_size(FILE* file, size_t& out_result)
    {
    #if defined(PUGI__MSVC_CRT_VERSION) && PUGI__MSVC_CRT_VERSION >= 1400
        // there are 64-bit versions of fseek/ftell, let's use them
        typedef __int64 length_type;

        _fseeki64(file, 0, SEEK_END);
        length_type length = _ftelli64(file);
        _fseeki64(file, 0, SEEK_SET);
    #elif defined(__MINGW32__) && !defined(__NO_MINGW_LFS) && (!defined(__STRICT_ANSI__) || defined(__MINGW64_VERSION_MAJOR))
        // there are 64-bit versions of fseek/ftell, let's use them
        typedef off64_t length_type;

        fseeko64(file, 0, SEEK_END);
        length_type length = ftello64(file);
        fseeko64(file, 0, SEEK_SET);
    #else
        // if this is a 32-bit OS, long is enough; if this is a unix system, long is 64-bit, which is enough; otherwise we can't do anything anyway.
        typedef long length_type;

        fseek(file, 0, SEEK_END);
        length_type length = ftell(file);
        fseek(file, 0, SEEK_SET);
    #endif

        // check for I/O errors
        if (length < 0) return status_io_error;

        // check for overflow
        size_t result = static_cast<size_t>(length);

        if (static_cast<length_type>(result) != length) return status_out_of_memory;

        // finalize
        out_result = result;

        return status_ok;
    }

    // This function assumes that buffer has extra sizeof(char_t) writable bytes after size
    PUGI__FN size_t zero_terminate_buffer(void* buffer, size_t size, xml_encoding encoding)
    {
        // We only need to zero-terminate if encoding conversion does not do it for us
    #ifdef PUGIXML_WCHAR_MODE
        xml_encoding wchar_encoding = get_wchar_encoding();

        if (encoding == wchar_encoding || need_endian_swap_utf(encoding, wchar_encoding))
        {
            size_t length = size / sizeof(char_t);

            static_cast<char_t*>(buffer)[length] = 0;
            return (length + 1) * sizeof(char_t);
        }
    #else
        if (encoding == encoding_utf8)
        {
            static_cast<char*>(buffer)[size] = 0;
            return size + 1;
        }
    #endif

        return size;
    }

    PUGI__FN xml_parse_result load_file_impl(xml_document_struct* doc, FILE* file, unsigned int options, xml_encoding encoding, char_t** out_buffer)
    {
        if (!file) return make_parse_result(status_file_not_found);

        // get file size (can result in I/O errors)
        size_t size = 0;
        xml_parse_status size_status = get_file_size(file, size);
        if (size_status != status_ok) return make_parse_result(size_status);

        size_t max_suffix_size = sizeof(char_t);

        // allocate buffer for the whole file
        char* contents = static_cast<char*>(xml_memory::allocate(size + max_suffix_size));
        if (!contents) return make_parse_result(status_out_of_memory);

        // read file in memory
        size_t read_size = fread(contents, 1, size, file);

        if (read_size != size)
        {
            xml_memory::deallocate(contents);
            return make_parse_result(status_io_error);
        }

        xml_encoding real_encoding = get_buffer_encoding(encoding, contents, size);

        return load_buffer_impl(doc, doc, contents, zero_terminate_buffer(contents, size, real_encoding), options, real_encoding, true, true, out_buffer);
    }

    PUGI__FN void close_file(FILE* file)
    {
        fclose(file);
    }

#ifndef PUGIXML_NO_STL
    template <typename T> struct xml_stream_chunk
    {
        static xml_stream_chunk* create()
        {
            void* memory = xml_memory::allocate(sizeof(xml_stream_chunk));
            if (!memory) return 0;

            return new (memory) xml_stream_chunk();
        }

        static void destroy(xml_stream_chunk* chunk)
        {
            // free chunk chain
            while (chunk)
            {
                xml_stream_chunk* next_ = chunk->next;

                xml_memory::deallocate(chunk);

                chunk = next_;
            }
        }

        xml_stream_chunk(): next(0), size(0)
        {
        }

        xml_stream_chunk* next;
        size_t size;

        T data[xml_memory_page_size / sizeof(T)];
    };

    template <typename T> PUGI__FN xml_parse_status load_stream_data_noseek(std::basic_istream<T>& stream, void** out_buffer, size_t* out_size)
    {
        auto_deleter<xml_stream_chunk<T> > chunks(0, xml_stream_chunk<T>::destroy);

        // read file to a chunk list
        size_t total = 0;
        xml_stream_chunk<T>* last = 0;

        while (!stream.eof())
        {
            // allocate new chunk
            xml_stream_chunk<T>* chunk = xml_stream_chunk<T>::create();
            if (!chunk) return status_out_of_memory;

            // append chunk to list
            if (last) last = last->next = chunk;
            else chunks.data = last = chunk;

            // read data to chunk
            stream.read(chunk->data, static_cast<std::streamsize>(sizeof(chunk->data) / sizeof(T)));
            chunk->size = static_cast<size_t>(stream.gcount()) * sizeof(T);

            // read may set failbit | eofbit in case gcount() is less than read length, so check for other I/O errors
            if (stream.bad() || (!stream.eof() && stream.fail())) return status_io_error;

            // guard against huge files (chunk size is small enough to make this overflow check work)
            if (total + chunk->size < total) return status_out_of_memory;
            total += chunk->size;
        }

        size_t max_suffix_size = sizeof(char_t);

        // copy chunk list to a contiguous buffer
        char* buffer = static_cast<char*>(xml_memory::allocate(total + max_suffix_size));
        if (!buffer) return status_out_of_memory;

        char* write = buffer;

        for (xml_stream_chunk<T>* chunk = chunks.data; chunk; chunk = chunk->next)
        {
            assert(write + chunk->size <= buffer + total);
            memcpy(write, chunk->data, chunk->size);
            write += chunk->size;
        }

        assert(write == buffer + total);

        // return buffer
        *out_buffer = buffer;
        *out_size = total;

        return status_ok;
    }

    template <typename T> PUGI__FN xml_parse_status load_stream_data_seek(std::basic_istream<T>& stream, void** out_buffer, size_t* out_size)
    {
        // get length of remaining data in stream
        typename std::basic_istream<T>::pos_type pos = stream.tellg();
        stream.seekg(0, std::ios::end);
        std::streamoff length = stream.tellg() - pos;
        stream.seekg(pos);

        if (stream.fail() || pos < 0) return status_io_error;

        // guard against huge files
        size_t read_length = static_cast<size_t>(length);

        if (static_cast<std::streamsize>(read_length) != length || length < 0) return status_out_of_memory;

        size_t max_suffix_size = sizeof(char_t);

        // read stream data into memory (guard against stream exceptions with buffer holder)
        auto_deleter<void> buffer(xml_memory::allocate(read_length * sizeof(T) + max_suffix_size), xml_memory::deallocate);
        if (!buffer.data) return status_out_of_memory;

        stream.read(static_cast<T*>(buffer.data), static_cast<std::streamsize>(read_length));

        // read may set failbit | eofbit in case gcount() is less than read_length (i.e. line ending conversion), so check for other I/O errors
        if (stream.bad() || (!stream.eof() && stream.fail())) return status_io_error;

        // return buffer
        size_t actual_length = static_cast<size_t>(stream.gcount());
        assert(actual_length <= read_length);

        *out_buffer = buffer.release();
        *out_size = actual_length * sizeof(T);

        return status_ok;
    }

    template <typename T> PUGI__FN xml_parse_result load_stream_impl(xml_document_struct* doc, std::basic_istream<T>& stream, unsigned int options, xml_encoding encoding, char_t** out_buffer)
    {
        void* buffer = 0;
        size_t size = 0;
        xml_parse_status status = status_ok;

        // if stream has an error bit set, bail out (otherwise tellg() can fail and we'll clear error bits)
        if (stream.fail()) return make_parse_result(status_io_error);

        // load stream to memory (using seek-based implementation if possible, since it's faster and takes less memory)
        if (stream.tellg() < 0)
        {
            stream.clear(); // clear error flags that could be set by a failing tellg
            status = load_stream_data_noseek(stream, &buffer, &size);
        }
        else
            status = load_stream_data_seek(stream, &buffer, &size);

        if (status != status_ok) return make_parse_result(status);

        xml_encoding real_encoding = get_buffer_encoding(encoding, buffer, size);

        return load_buffer_impl(doc, doc, buffer, zero_terminate_buffer(buffer, size, real_encoding), options, real_encoding, true, true, out_buffer);
    }
#endif

#if defined(PUGI__MSVC_CRT_VERSION) || defined(__BORLANDC__) || (defined(__MINGW32__) && (!defined(__STRICT_ANSI__) || defined(__MINGW64_VERSION_MAJOR)))
    PUGI__FN FILE* open_file_wide(const wchar_t* path, const wchar_t* mode)
    {
#if defined(PUGI__MSVC_CRT_VERSION) && PUGI__MSVC_CRT_VERSION >= 1400
        FILE* file = 0;
        return _wfopen_s(&file, path, mode) == 0 ? file : 0;
#else
        return _wfopen(path, mode);
#endif
    }
#else
    PUGI__FN char* convert_path_heap(const wchar_t* str)
    {
        assert(str);

        // first pass: get length in utf8 characters
        size_t length = strlength_wide(str);
        size_t size = as_utf8_begin(str, length);

        // allocate resulting string
        char* result = static_cast<char*>(xml_memory::allocate(size + 1));
        if (!result) return 0;

        // second pass: convert to utf8
        as_utf8_end(result, size, str, length);

        // zero-terminate
        result[size] = 0;

        return result;
    }

    PUGI__FN FILE* open_file_wide(const wchar_t* path, const wchar_t* mode)
    {
        // there is no standard function to open wide paths, so our best bet is to try utf8 path
        char* path_utf8 = convert_path_heap(path);
        if (!path_utf8) return 0;

        // convert mode to ASCII (we mirror _wfopen interface)
        char mode_ascii[4] = {0};
        for (size_t i = 0; mode[i]; ++i) mode_ascii[i] = static_cast<char>(mode[i]);

        // try to open the utf8 path
        FILE* result = fopen(path_utf8, mode_ascii);

        // free dummy buffer
        xml_memory::deallocate(path_utf8);

        return result;
    }
#endif

    PUGI__FN FILE* open_file(const char* path, const char* mode)
    {
#if defined(PUGI__MSVC_CRT_VERSION) && PUGI__MSVC_CRT_VERSION >= 1400
        FILE* file = 0;
        return fopen_s(&file, path, mode) == 0 ? file : 0;
#else
        return fopen(path, mode);
#endif
    }

    PUGI__FN bool save_file_impl(const xml_document& doc, FILE* file, const char_t* indent, unsigned int flags, xml_encoding encoding)
    {
        if (!file) return false;

        xml_writer_file writer(file);
        doc.save(writer, indent, flags, encoding);

        return ferror(file) == 0;
    }

    struct name_null_sentry
    {
        xml_node_struct* node;
        char_t* name;

        name_null_sentry(xml_node_struct* node_): node(node_), name(node_->name)
        {
            node->name = 0;
        }

        ~name_null_sentry()
        {
            node->name = name;
        }
    };
PUGI__NS_END

namespace pugi
{
    PUGI__FN xml_writer_file::xml_writer_file(void* file_): file(file_)
    {
    }

    PUGI__FN void xml_writer_file::write(const void* data, size_t size)
    {
        size_t result = fwrite(data, 1, size, static_cast<FILE*>(file));
        (void)!result; // unfortunately we can't do proper error handling here
    }

#ifndef PUGIXML_NO_STL
    PUGI__FN xml_writer_stream::xml_writer_stream(std::basic_ostream<char, std::char_traits<char> >& stream): narrow_stream(&stream), wide_stream(0)
    {
    }

    PUGI__FN xml_writer_stream::xml_writer_stream(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& stream): narrow_stream(0), wide_stream(&stream)
    {
    }

    PUGI__FN void xml_writer_stream::write(const void* data, size_t size)
    {
        if (narrow_stream)
        {
            assert(!wide_stream);
            narrow_stream->write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
        }
        else
        {
            assert(wide_stream);
            assert(size % sizeof(wchar_t) == 0);

            wide_stream->write(reinterpret_cast<const wchar_t*>(data), static_cast<std::streamsize>(size / sizeof(wchar_t)));
        }
    }
#endif

    PUGI__FN xml_tree_walker::xml_tree_walker(): _depth(0)
    {
    }

    PUGI__FN xml_tree_walker::~xml_tree_walker()
    {
    }

    PUGI__FN int xml_tree_walker::depth() const
    {
        return _depth;
    }

    PUGI__FN bool xml_tree_walker::begin(xml_node&)
    {
        return true;
    }

    PUGI__FN bool xml_tree_walker::end(xml_node&)
    {
        return true;
    }

    PUGI__FN xml_attribute::xml_attribute(): _attr(0)
    {
    }

    PUGI__FN xml_attribute::xml_attribute(xml_attribute_struct* attr): _attr(attr)
    {
    }

    PUGI__FN static void unspecified_bool_xml_attribute(xml_attribute***)
    {
    }

    PUGI__FN xml_attribute::operator xml_attribute::unspecified_bool_type() const
    {
        return _attr ? unspecified_bool_xml_attribute : 0;
    }

    PUGI__FN bool xml_attribute::operator!() const
    {
        return !_attr;
    }

    PUGI__FN bool xml_attribute::operator==(const xml_attribute& r) const
    {
        return (_attr == r._attr);
    }

    PUGI__FN bool xml_attribute::operator!=(const xml_attribute& r) const
    {
        return (_attr != r._attr);
    }

    PUGI__FN bool xml_attribute::operator<(const xml_attribute& r) const
    {
        return (_attr < r._attr);
    }

    PUGI__FN bool xml_attribute::operator>(const xml_attribute& r) const
    {
        return (_attr > r._attr);
    }

    PUGI__FN bool xml_attribute::operator<=(const xml_attribute& r) const
    {
        return (_attr <= r._attr);
    }

    PUGI__FN bool xml_attribute::operator>=(const xml_attribute& r) const
    {
        return (_attr >= r._attr);
    }

    PUGI__FN xml_attribute xml_attribute::next_attribute() const
    {
        return _attr ? xml_attribute(_attr->next_attribute) : xml_attribute();
    }

    PUGI__FN xml_attribute xml_attribute::previous_attribute() const
    {
        return _attr && _attr->prev_attribute_c->next_attribute ? xml_attribute(_attr->prev_attribute_c) : xml_attribute();
    }

    PUGI__FN const char_t* xml_attribute::as_string(const char_t* def) const
    {
        return (_attr && _attr->value) ? _attr->value + 0 : def;
    }

    PUGI__FN int xml_attribute::as_int(int def) const
    {
        return (_attr && _attr->value) ? impl::get_value_int(_attr->value) : def;
    }

    PUGI__FN unsigned int xml_attribute::as_uint(unsigned int def) const
    {
        return (_attr && _attr->value) ? impl::get_value_uint(_attr->value) : def;
    }

    PUGI__FN double xml_attribute::as_double(double def) const
    {
        return (_attr && _attr->value) ? impl::get_value_double(_attr->value) : def;
    }

    PUGI__FN float xml_attribute::as_float(float def) const
    {
        return (_attr && _attr->value) ? impl::get_value_float(_attr->value) : def;
    }

    PUGI__FN bool xml_attribute::as_bool(bool def) const
    {
        return (_attr && _attr->value) ? impl::get_value_bool(_attr->value) : def;
    }

#ifdef PUGIXML_HAS_LONG_LONG
    PUGI__FN long long xml_attribute::as_llong(long long def) const
    {
        return (_attr && _attr->value) ? impl::get_value_llong(_attr->value) : def;
    }

    PUGI__FN unsigned long long xml_attribute::as_ullong(unsigned long long def) const
    {
        return (_attr && _attr->value) ? impl::get_value_ullong(_attr->value) : def;
    }
#endif

    PUGI__FN bool xml_attribute::empty() const
    {
        return !_attr;
    }

    PUGI__FN const char_t* xml_attribute::name() const
    {
        return (_attr && _attr->name) ? _attr->name + 0 : PUGIXML_TEXT("");
    }

    PUGI__FN const char_t* xml_attribute::value() const
    {
        return (_attr && _attr->value) ? _attr->value + 0 : PUGIXML_TEXT("");
    }

    PUGI__FN size_t xml_attribute::hash_value() const
    {
        return static_cast<size_t>(reinterpret_cast<uintptr_t>(_attr) / sizeof(xml_attribute_struct));
    }

    PUGI__FN xml_attribute_struct* xml_attribute::internal_object() const
    {
        return _attr;
    }

    PUGI__FN xml_attribute& xml_attribute::operator=(const char_t* rhs)
    {
        set_value(rhs);
        return *this;
    }

    PUGI__FN xml_attribute& xml_attribute::operator=(int rhs)
    {
        set_value(rhs);
        return *this;
    }

    PUGI__FN xml_attribute& xml_attribute::operator=(unsigned int rhs)
    {
        set_value(rhs);
        return *this;
    }

    PUGI__FN xml_attribute& xml_attribute::operator=(long rhs)
    {
        set_value(rhs);
        return *this;
    }

    PUGI__FN xml_attribute& xml_attribute::operator=(unsigned long rhs)
    {
        set_value(rhs);
        return *this;
    }

    PUGI__FN xml_attribute& xml_attribute::operator=(double rhs)
    {
        set_value(rhs);
        return *this;
    }

    PUGI__FN xml_attribute& xml_attribute::operator=(float rhs)
    {
        set_value(rhs);
        return *this;
    }

    PUGI__FN xml_attribute& xml_attribute::operator=(bool rhs)
    {
        set_value(rhs);
        return *this;
    }

#ifdef PUGIXML_HAS_LONG_LONG
    PUGI__FN xml_attribute& xml_attribute::operator=(long long rhs)
    {
        set_value(rhs);
        return *this;
    }

    PUGI__FN xml_attribute& xml_attribute::operator=(unsigned long long rhs)
    {
        set_value(rhs);
        return *this;
    }
#endif

    PUGI__FN bool xml_attribute::set_name(const char_t* rhs)
    {
        if (!_attr) return false;

        return impl::strcpy_insitu(_attr->name, _attr->header, impl::xml_memory_page_name_allocated_mask, rhs, impl::strlength(rhs));
    }

    PUGI__FN bool xml_attribute::set_value(const char_t* rhs)
    {
        if (!_attr) return false;

        return impl::strcpy_insitu(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs, impl::strlength(rhs));
    }

    PUGI__FN bool xml_attribute::set_value(int rhs)
    {
        if (!_attr) return false;

        return impl::set_value_integer<unsigned int>(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs, rhs < 0);
    }

    PUGI__FN bool xml_attribute::set_value(unsigned int rhs)
    {
        if (!_attr) return false;

        return impl::set_value_integer<unsigned int>(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs, false);
    }

    PUGI__FN bool xml_attribute::set_value(long rhs)
    {
        if (!_attr) return false;

        return impl::set_value_integer<unsigned long>(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs, rhs < 0);
    }

    PUGI__FN bool xml_attribute::set_value(unsigned long rhs)
    {
        if (!_attr) return false;

        return impl::set_value_integer<unsigned long>(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs, false);
    }

    PUGI__FN bool xml_attribute::set_value(double rhs)
    {
        if (!_attr) return false;

        return impl::set_value_convert(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs, default_double_precision);
    }

    PUGI__FN bool xml_attribute::set_value(double rhs, int precision)
    {
        if (!_attr) return false;

        return impl::set_value_convert(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs, precision);
    }

    PUGI__FN bool xml_attribute::set_value(float rhs)
    {
        if (!_attr) return false;

        return impl::set_value_convert(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs, default_float_precision);
    }

    PUGI__FN bool xml_attribute::set_value(float rhs, int precision)
    {
        if (!_attr) return false;

        return impl::set_value_convert(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs, precision);
    }

    PUGI__FN bool xml_attribute::set_value(bool rhs)
    {
        if (!_attr) return false;

        return impl::set_value_bool(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs);
    }

#ifdef PUGIXML_HAS_LONG_LONG
    PUGI__FN bool xml_attribute::set_value(long long rhs)
    {
        if (!_attr) return false;

        return impl::set_value_integer<unsigned long long>(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs, rhs < 0);
    }

    PUGI__FN bool xml_attribute::set_value(unsigned long long rhs)
    {
        if (!_attr) return false;

        return impl::set_value_integer<unsigned long long>(_attr->value, _attr->header, impl::xml_memory_page_value_allocated_mask, rhs, false);
    }
#endif

#ifdef __BORLANDC__
    PUGI__FN bool operator&&(const xml_attribute& lhs, bool rhs)
    {
        return (bool)lhs && rhs;
    }

    PUGI__FN bool operator||(const xml_attribute& lhs, bool rhs)
    {
        return (bool)lhs || rhs;
    }
#endif

    PUGI__FN xml_node::xml_node(): _root(0)
    {
    }

    PUGI__FN xml_node::xml_node(xml_node_struct* p): _root(p)
    {
    }

    PUGI__FN static void unspecified_bool_xml_node(xml_node***)
    {
    }

    PUGI__FN xml_node::operator xml_node::unspecified_bool_type() const
    {
        return _root ? unspecified_bool_xml_node : 0;
    }

    PUGI__FN bool xml_node::operator!() const
    {
        return !_root;
    }

    PUGI__FN xml_node::iterator xml_node::begin() const
    {
        return iterator(_root ? _root->first_child + 0 : 0, _root);
    }

    PUGI__FN xml_node::iterator xml_node::end() const
    {
        return iterator(0, _root);
    }

    PUGI__FN xml_node::attribute_iterator xml_node::attributes_begin() const
    {
        return attribute_iterator(_root ? _root->first_attribute + 0 : 0, _root);
    }

    PUGI__FN xml_node::attribute_iterator xml_node::attributes_end() const
    {
        return attribute_iterator(0, _root);
    }

    PUGI__FN xml_object_range<xml_node_iterator> xml_node::children() const
    {
        return xml_object_range<xml_node_iterator>(begin(), end());
    }

    PUGI__FN xml_object_range<xml_named_node_iterator> xml_node::children(const char_t* name_) const
    {
        return xml_object_range<xml_named_node_iterator>(xml_named_node_iterator(child(name_)._root, _root, name_), xml_named_node_iterator(0, _root, name_));
    }

    PUGI__FN xml_object_range<xml_attribute_iterator> xml_node::attributes() const
    {
        return xml_object_range<xml_attribute_iterator>(attributes_begin(), attributes_end());
    }

    PUGI__FN bool xml_node::operator==(const xml_node& r) const
    {
        return (_root == r._root);
    }

    PUGI__FN bool xml_node::operator!=(const xml_node& r) const
    {
        return (_root != r._root);
    }

    PUGI__FN bool xml_node::operator<(const xml_node& r) const
    {
        return (_root < r._root);
    }

    PUGI__FN bool xml_node::operator>(const xml_node& r) const
    {
        return (_root > r._root);
    }

    PUGI__FN bool xml_node::operator<=(const xml_node& r) const
    {
        return (_root <= r._root);
    }

    PUGI__FN bool xml_node::operator>=(const xml_node& r) const
    {
        return (_root >= r._root);
    }

    PUGI__FN bool xml_node::empty() const
    {
        return !_root;
    }

    PUGI__FN const char_t* xml_node::name() const
    {
        return (_root && _root->name) ? _root->name + 0 : PUGIXML_TEXT("");
    }

    PUGI__FN xml_node_type xml_node::type() const
    {
        return _root ? PUGI__NODETYPE(_root) : node_null;
    }

    PUGI__FN const char_t* xml_node::value() const
    {
        return (_root && _root->value) ? _root->value + 0 : PUGIXML_TEXT("");
    }

    PUGI__FN xml_node xml_node::child(const char_t* name_) const
    {
        if (!_root) return xml_node();

        for (xml_node_struct* i = _root->first_child; i; i = i->next_sibling)
            if (i->name && impl::strequal(name_, i->name)) return xml_node(i);

        return xml_node();
    }

    PUGI__FN xml_attribute xml_node::attribute(const char_t* name_) const
    {
        if (!_root) return xml_attribute();

        for (xml_attribute_struct* i = _root->first_attribute; i; i = i->next_attribute)
            if (i->name && impl::strequal(name_, i->name))
                return xml_attribute(i);

        return xml_attribute();
    }

    PUGI__FN xml_node xml_node::next_sibling(const char_t* name_) const
    {
        if (!_root) return xml_node();

        for (xml_node_struct* i = _root->next_sibling; i; i = i->next_sibling)
            if (i->name && impl::strequal(name_, i->name)) return xml_node(i);

        return xml_node();
    }

    PUGI__FN xml_node xml_node::next_sibling() const
    {
        return _root ? xml_node(_root->next_sibling) : xml_node();
    }

    PUGI__FN xml_node xml_node::previous_sibling(const char_t* name_) const
    {
        if (!_root) return xml_node();

        for (xml_node_struct* i = _root->prev_sibling_c; i->next_sibling; i = i->prev_sibling_c)
            if (i->name && impl::strequal(name_, i->name)) return xml_node(i);

        return xml_node();
    }

    PUGI__FN xml_attribute xml_node::attribute(const char_t* name_, xml_attribute& hint_) const
    {
        xml_attribute_struct* hint = hint_._attr;

        // if hint is not an attribute of node, behavior is not defined
        assert(!hint || (_root && impl::is_attribute_of(hint, _root)));

        if (!_root) return xml_attribute();

        // optimistically search from hint up until the end
        for (xml_attribute_struct* i = hint; i; i = i->next_attribute)
            if (i->name && impl::strequal(name_, i->name))
            {
                // update hint to maximize efficiency of searching for consecutive attributes
                hint_._attr = i->next_attribute;

                return xml_attribute(i);
            }

        // wrap around and search from the first attribute until the hint
        // 'j' null pointer check is technically redundant, but it prevents a crash in case the assertion above fails
        for (xml_attribute_struct* j = _root->first_attribute; j && j != hint; j = j->next_attribute)
            if (j->name && impl::strequal(name_, j->name))
            {
                // update hint to maximize efficiency of searching for consecutive attributes
                hint_._attr = j->next_attribute;

                return xml_attribute(j);
            }

        return xml_attribute();
    }

    PUGI__FN xml_node xml_node::previous_sibling() const
    {
        if (!_root) return xml_node();

        if (_root->prev_sibling_c->next_sibling) return xml_node(_root->prev_sibling_c);
        else return xml_node();
    }

    PUGI__FN xml_node xml_node::parent() const
    {
        return _root ? xml_node(_root->parent) : xml_node();
    }

    PUGI__FN xml_node xml_node::root() const
    {
        return _root ? xml_node(&impl::get_document(_root)) : xml_node();
    }

    PUGI__FN xml_text xml_node::text() const
    {
        return xml_text(_root);
    }

    PUGI__FN const char_t* xml_node::child_value() const
    {
        if (!_root) return PUGIXML_TEXT("");

        // element nodes can have value if parse_embed_pcdata was used
        if (PUGI__NODETYPE(_root) == node_element && _root->value)
            return _root->value;

        for (xml_node_struct* i = _root->first_child; i; i = i->next_sibling)
            if (impl::is_text_node(i) && i->value)
                return i->value;

        return PUGIXML_TEXT("");
    }

    PUGI__FN const char_t* xml_node::child_value(const char_t* name_) const
    {
        return child(name_).child_value();
    }

    PUGI__FN xml_attribute xml_node::first_attribute() const
    {
        return _root ? xml_attribute(_root->first_attribute) : xml_attribute();
    }

    PUGI__FN xml_attribute xml_node::last_attribute() const
    {
        return _root && _root->first_attribute ? xml_attribute(_root->first_attribute->prev_attribute_c) : xml_attribute();
    }

    PUGI__FN xml_node xml_node::first_child() const
    {
        return _root ? xml_node(_root->first_child) : xml_node();
    }

    PUGI__FN xml_node xml_node::last_child() const
    {
        return _root && _root->first_child ? xml_node(_root->first_child->prev_sibling_c) : xml_node();
    }

    PUGI__FN bool xml_node::set_name(const char_t* rhs)
    {
        xml_node_type type_ = _root ? PUGI__NODETYPE(_root) : node_null;

        if (type_ != node_element && type_ != node_pi && type_ != node_declaration)
            return false;

        return impl::strcpy_insitu(_root->name, _root->header, impl::xml_memory_page_name_allocated_mask, rhs, impl::strlength(rhs));
    }

    PUGI__FN bool xml_node::set_value(const char_t* rhs)
    {
        xml_node_type type_ = _root ? PUGI__NODETYPE(_root) : node_null;

        if (type_ != node_pcdata && type_ != node_cdata && type_ != node_comment && type_ != node_pi && type_ != node_doctype)
            return false;

        return impl::strcpy_insitu(_root->value, _root->header, impl::xml_memory_page_value_allocated_mask, rhs, impl::strlength(rhs));
    }

    PUGI__FN xml_attribute xml_node::append_attribute(const char_t* name_)
    {
        if (!impl::allow_insert_attribute(type())) return xml_attribute();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_attribute();

        xml_attribute a(impl::allocate_attribute(alloc));
        if (!a) return xml_attribute();

        impl::append_attribute(a._attr, _root);

        a.set_name(name_);

        return a;
    }

    PUGI__FN xml_attribute xml_node::prepend_attribute(const char_t* name_)
    {
        if (!impl::allow_insert_attribute(type())) return xml_attribute();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_attribute();

        xml_attribute a(impl::allocate_attribute(alloc));
        if (!a) return xml_attribute();

        impl::prepend_attribute(a._attr, _root);

        a.set_name(name_);

        return a;
    }

    PUGI__FN xml_attribute xml_node::insert_attribute_after(const char_t* name_, const xml_attribute& attr)
    {
        if (!impl::allow_insert_attribute(type())) return xml_attribute();
        if (!attr || !impl::is_attribute_of(attr._attr, _root)) return xml_attribute();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_attribute();

        xml_attribute a(impl::allocate_attribute(alloc));
        if (!a) return xml_attribute();

        impl::insert_attribute_after(a._attr, attr._attr, _root);

        a.set_name(name_);

        return a;
    }

    PUGI__FN xml_attribute xml_node::insert_attribute_before(const char_t* name_, const xml_attribute& attr)
    {
        if (!impl::allow_insert_attribute(type())) return xml_attribute();
        if (!attr || !impl::is_attribute_of(attr._attr, _root)) return xml_attribute();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_attribute();

        xml_attribute a(impl::allocate_attribute(alloc));
        if (!a) return xml_attribute();

        impl::insert_attribute_before(a._attr, attr._attr, _root);

        a.set_name(name_);

        return a;
    }

    PUGI__FN xml_attribute xml_node::append_copy(const xml_attribute& proto)
    {
        if (!proto) return xml_attribute();
        if (!impl::allow_insert_attribute(type())) return xml_attribute();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_attribute();

        xml_attribute a(impl::allocate_attribute(alloc));
        if (!a) return xml_attribute();

        impl::append_attribute(a._attr, _root);
        impl::node_copy_attribute(a._attr, proto._attr);

        return a;
    }

    PUGI__FN xml_attribute xml_node::prepend_copy(const xml_attribute& proto)
    {
        if (!proto) return xml_attribute();
        if (!impl::allow_insert_attribute(type())) return xml_attribute();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_attribute();

        xml_attribute a(impl::allocate_attribute(alloc));
        if (!a) return xml_attribute();

        impl::prepend_attribute(a._attr, _root);
        impl::node_copy_attribute(a._attr, proto._attr);

        return a;
    }

    PUGI__FN xml_attribute xml_node::insert_copy_after(const xml_attribute& proto, const xml_attribute& attr)
    {
        if (!proto) return xml_attribute();
        if (!impl::allow_insert_attribute(type())) return xml_attribute();
        if (!attr || !impl::is_attribute_of(attr._attr, _root)) return xml_attribute();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_attribute();

        xml_attribute a(impl::allocate_attribute(alloc));
        if (!a) return xml_attribute();

        impl::insert_attribute_after(a._attr, attr._attr, _root);
        impl::node_copy_attribute(a._attr, proto._attr);

        return a;
    }

    PUGI__FN xml_attribute xml_node::insert_copy_before(const xml_attribute& proto, const xml_attribute& attr)
    {
        if (!proto) return xml_attribute();
        if (!impl::allow_insert_attribute(type())) return xml_attribute();
        if (!attr || !impl::is_attribute_of(attr._attr, _root)) return xml_attribute();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_attribute();

        xml_attribute a(impl::allocate_attribute(alloc));
        if (!a) return xml_attribute();

        impl::insert_attribute_before(a._attr, attr._attr, _root);
        impl::node_copy_attribute(a._attr, proto._attr);

        return a;
    }

    PUGI__FN xml_node xml_node::append_child(xml_node_type type_)
    {
        if (!impl::allow_insert_child(type(), type_)) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        xml_node n(impl::allocate_node(alloc, type_));
        if (!n) return xml_node();

        impl::append_node(n._root, _root);

        if (type_ == node_declaration) n.set_name(PUGIXML_TEXT("xml"));

        return n;
    }

    PUGI__FN xml_node xml_node::prepend_child(xml_node_type type_)
    {
        if (!impl::allow_insert_child(type(), type_)) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        xml_node n(impl::allocate_node(alloc, type_));
        if (!n) return xml_node();

        impl::prepend_node(n._root, _root);

        if (type_ == node_declaration) n.set_name(PUGIXML_TEXT("xml"));

        return n;
    }

    PUGI__FN xml_node xml_node::insert_child_before(xml_node_type type_, const xml_node& node)
    {
        if (!impl::allow_insert_child(type(), type_)) return xml_node();
        if (!node._root || node._root->parent != _root) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        xml_node n(impl::allocate_node(alloc, type_));
        if (!n) return xml_node();

        impl::insert_node_before(n._root, node._root);

        if (type_ == node_declaration) n.set_name(PUGIXML_TEXT("xml"));

        return n;
    }

    PUGI__FN xml_node xml_node::insert_child_after(xml_node_type type_, const xml_node& node)
    {
        if (!impl::allow_insert_child(type(), type_)) return xml_node();
        if (!node._root || node._root->parent != _root) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        xml_node n(impl::allocate_node(alloc, type_));
        if (!n) return xml_node();

        impl::insert_node_after(n._root, node._root);

        if (type_ == node_declaration) n.set_name(PUGIXML_TEXT("xml"));

        return n;
    }

    PUGI__FN xml_node xml_node::append_child(const char_t* name_)
    {
        xml_node result = append_child(node_element);

        result.set_name(name_);

        return result;
    }

    PUGI__FN xml_node xml_node::prepend_child(const char_t* name_)
    {
        xml_node result = prepend_child(node_element);

        result.set_name(name_);

        return result;
    }

    PUGI__FN xml_node xml_node::insert_child_after(const char_t* name_, const xml_node& node)
    {
        xml_node result = insert_child_after(node_element, node);

        result.set_name(name_);

        return result;
    }

    PUGI__FN xml_node xml_node::insert_child_before(const char_t* name_, const xml_node& node)
    {
        xml_node result = insert_child_before(node_element, node);

        result.set_name(name_);

        return result;
    }

    PUGI__FN xml_node xml_node::append_copy(const xml_node& proto)
    {
        xml_node_type type_ = proto.type();
        if (!impl::allow_insert_child(type(), type_)) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        xml_node n(impl::allocate_node(alloc, type_));
        if (!n) return xml_node();

        impl::append_node(n._root, _root);
        impl::node_copy_tree(n._root, proto._root);

        return n;
    }

    PUGI__FN xml_node xml_node::prepend_copy(const xml_node& proto)
    {
        xml_node_type type_ = proto.type();
        if (!impl::allow_insert_child(type(), type_)) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        xml_node n(impl::allocate_node(alloc, type_));
        if (!n) return xml_node();

        impl::prepend_node(n._root, _root);
        impl::node_copy_tree(n._root, proto._root);

        return n;
    }

    PUGI__FN xml_node xml_node::insert_copy_after(const xml_node& proto, const xml_node& node)
    {
        xml_node_type type_ = proto.type();
        if (!impl::allow_insert_child(type(), type_)) return xml_node();
        if (!node._root || node._root->parent != _root) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        xml_node n(impl::allocate_node(alloc, type_));
        if (!n) return xml_node();

        impl::insert_node_after(n._root, node._root);
        impl::node_copy_tree(n._root, proto._root);

        return n;
    }

    PUGI__FN xml_node xml_node::insert_copy_before(const xml_node& proto, const xml_node& node)
    {
        xml_node_type type_ = proto.type();
        if (!impl::allow_insert_child(type(), type_)) return xml_node();
        if (!node._root || node._root->parent != _root) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        xml_node n(impl::allocate_node(alloc, type_));
        if (!n) return xml_node();

        impl::insert_node_before(n._root, node._root);
        impl::node_copy_tree(n._root, proto._root);

        return n;
    }

    PUGI__FN xml_node xml_node::append_move(const xml_node& moved)
    {
        if (!impl::allow_move(*this, moved)) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        // disable document_buffer_order optimization since moving nodes around changes document order without changing buffer pointers
        impl::get_document(_root).header |= impl::xml_memory_page_contents_shared_mask;

        impl::remove_node(moved._root);
        impl::append_node(moved._root, _root);

        return moved;
    }

    PUGI__FN xml_node xml_node::prepend_move(const xml_node& moved)
    {
        if (!impl::allow_move(*this, moved)) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        // disable document_buffer_order optimization since moving nodes around changes document order without changing buffer pointers
        impl::get_document(_root).header |= impl::xml_memory_page_contents_shared_mask;

        impl::remove_node(moved._root);
        impl::prepend_node(moved._root, _root);

        return moved;
    }

    PUGI__FN xml_node xml_node::insert_move_after(const xml_node& moved, const xml_node& node)
    {
        if (!impl::allow_move(*this, moved)) return xml_node();
        if (!node._root || node._root->parent != _root) return xml_node();
        if (moved._root == node._root) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        // disable document_buffer_order optimization since moving nodes around changes document order without changing buffer pointers
        impl::get_document(_root).header |= impl::xml_memory_page_contents_shared_mask;

        impl::remove_node(moved._root);
        impl::insert_node_after(moved._root, node._root);

        return moved;
    }

    PUGI__FN xml_node xml_node::insert_move_before(const xml_node& moved, const xml_node& node)
    {
        if (!impl::allow_move(*this, moved)) return xml_node();
        if (!node._root || node._root->parent != _root) return xml_node();
        if (moved._root == node._root) return xml_node();

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return xml_node();

        // disable document_buffer_order optimization since moving nodes around changes document order without changing buffer pointers
        impl::get_document(_root).header |= impl::xml_memory_page_contents_shared_mask;

        impl::remove_node(moved._root);
        impl::insert_node_before(moved._root, node._root);

        return moved;
    }

    PUGI__FN bool xml_node::remove_attribute(const char_t* name_)
    {
        return remove_attribute(attribute(name_));
    }

    PUGI__FN bool xml_node::remove_attribute(const xml_attribute& a)
    {
        if (!_root || !a._attr) return false;
        if (!impl::is_attribute_of(a._attr, _root)) return false;

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return false;

        impl::remove_attribute(a._attr, _root);
        impl::destroy_attribute(a._attr, alloc);

        return true;
    }

    PUGI__FN bool xml_node::remove_attributes()
    {
        if (!_root) return false;

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return false;

        for (xml_attribute_struct* attr = _root->first_attribute; attr; )
        {
            xml_attribute_struct* next = attr->next_attribute;

            impl::destroy_attribute(attr, alloc);

            attr = next;
        }

        _root->first_attribute = 0;

        return true;
    }

    PUGI__FN bool xml_node::remove_child(const char_t* name_)
    {
        return remove_child(child(name_));
    }

    PUGI__FN bool xml_node::remove_child(const xml_node& n)
    {
        if (!_root || !n._root || n._root->parent != _root) return false;

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return false;

        impl::remove_node(n._root);
        impl::destroy_node(n._root, alloc);

        return true;
    }

    PUGI__FN bool xml_node::remove_children()
    {
        if (!_root) return false;

        impl::xml_allocator& alloc = impl::get_allocator(_root);
        if (!alloc.reserve()) return false;

        for (xml_node_struct* cur = _root->first_child; cur; )
        {
            xml_node_struct* next = cur->next_sibling;

            impl::destroy_node(cur, alloc);

            cur = next;
        }

        _root->first_child = 0;

        return true;
    }

    PUGI__FN xml_parse_result xml_node::append_buffer(const void* contents, size_t size, unsigned int options, xml_encoding encoding)
    {
        // append_buffer is only valid for elements/documents
        if (!impl::allow_insert_child(type(), node_element)) return impl::make_parse_result(status_append_invalid_root);

        // get document node
        impl::xml_document_struct* doc = &impl::get_document(_root);

        // disable document_buffer_order optimization since in a document with multiple buffers comparing buffer pointers does not make sense
        doc->header |= impl::xml_memory_page_contents_shared_mask;

        // get extra buffer element (we'll store the document fragment buffer there so that we can deallocate it later)
        impl::xml_memory_page* page = 0;
        impl::xml_extra_buffer* extra = static_cast<impl::xml_extra_buffer*>(doc->allocate_memory(sizeof(impl::xml_extra_buffer) + sizeof(void*), page));
        (void)page;

        if (!extra) return impl::make_parse_result(status_out_of_memory);

    #ifdef PUGIXML_COMPACT
        // align the memory block to a pointer boundary; this is required for compact mode where memory allocations are only 4b aligned
        // note that this requires up to sizeof(void*)-1 additional memory, which the allocation above takes into account
        extra = reinterpret_cast<impl::xml_extra_buffer*>((reinterpret_cast<uintptr_t>(extra) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1));
    #endif

        // add extra buffer to the list
        extra->buffer = 0;
        extra->next = doc->extra_buffers;
        doc->extra_buffers = extra;

        // name of the root has to be NULL before parsing - otherwise closing node mismatches will not be detected at the top level
        impl::name_null_sentry sentry(_root);

        return impl::load_buffer_impl(doc, _root, const_cast<void*>(contents), size, options, encoding, false, false, &extra->buffer);
    }

    PUGI__FN xml_node xml_node::find_child_by_attribute(const char_t* name_, const char_t* attr_name, const char_t* attr_value) const
    {
        if (!_root) return xml_node();

        for (xml_node_struct* i = _root->first_child; i; i = i->next_sibling)
            if (i->name && impl::strequal(name_, i->name))
            {
                for (xml_attribute_struct* a = i->first_attribute; a; a = a->next_attribute)
                    if (a->name && impl::strequal(attr_name, a->name) && impl::strequal(attr_value, a->value ? a->value + 0 : PUGIXML_TEXT("")))
                        return xml_node(i);
            }

        return xml_node();
    }

    PUGI__FN xml_node xml_node::find_child_by_attribute(const char_t* attr_name, const char_t* attr_value) const
    {
        if (!_root) return xml_node();

        for (xml_node_struct* i = _root->first_child; i; i = i->next_sibling)
            for (xml_attribute_struct* a = i->first_attribute; a; a = a->next_attribute)
                if (a->name && impl::strequal(attr_name, a->name) && impl::strequal(attr_value, a->value ? a->value + 0 : PUGIXML_TEXT("")))
                    return xml_node(i);

        return xml_node();
    }

#ifndef PUGIXML_NO_STL
    PUGI__FN string_t xml_node::path(char_t delimiter) const
    {
        if (!_root) return string_t();

        size_t offset = 0;

        for (xml_node_struct* i = _root; i; i = i->parent)
        {
            offset += (i != _root);
            offset += i->name ? impl::strlength(i->name) : 0;
        }

        string_t result;
        result.resize(offset);

        for (xml_node_struct* j = _root; j; j = j->parent)
        {
            if (j != _root)
                result[--offset] = delimiter;

            if (j->name)
            {
                size_t length = impl::strlength(j->name);

                offset -= length;
                memcpy(&result[offset], j->name, length * sizeof(char_t));
            }
        }

        assert(offset == 0);

        return result;
    }
#endif

    PUGI__FN xml_node xml_node::first_element_by_path(const char_t* path_, char_t delimiter) const
    {
        xml_node context = path_[0] == delimiter ? root() : *this;

        if (!context._root) return xml_node();

        const char_t* path_segment = path_;

        while (*path_segment == delimiter) ++path_segment;

        const char_t* path_segment_end = path_segment;

        while (*path_segment_end && *path_segment_end != delimiter) ++path_segment_end;

        if (path_segment == path_segment_end) return context;

        const char_t* next_segment = path_segment_end;

        while (*next_segment == delimiter) ++next_segment;

        if (*path_segment == '.' && path_segment + 1 == path_segment_end)
            return context.first_element_by_path(next_segment, delimiter);
        else if (*path_segment == '.' && *(path_segment+1) == '.' && path_segment + 2 == path_segment_end)
            return context.parent().first_element_by_path(next_segment, delimiter);
        else
        {
            for (xml_node_struct* j = context._root->first_child; j; j = j->next_sibling)
            {
                if (j->name && impl::strequalrange(j->name, path_segment, static_cast<size_t>(path_segment_end - path_segment)))
                {
                    xml_node subsearch = xml_node(j).first_element_by_path(next_segment, delimiter);

                    if (subsearch) return subsearch;
                }
            }

            return xml_node();
        }
    }

    PUGI__FN bool xml_node::traverse(xml_tree_walker& walker)
    {
        walker._depth = -1;

        xml_node arg_begin(_root);
        if (!walker.begin(arg_begin)) return false;

        xml_node_struct* cur = _root ? _root->first_child + 0 : 0;

        if (cur)
        {
            ++walker._depth;

            do
            {
                xml_node arg_for_each(cur);
                if (!walker.for_each(arg_for_each))
                    return false;

                if (cur->first_child)
                {
                    ++walker._depth;
                    cur = cur->first_child;
                }
                else if (cur->next_sibling)
                    cur = cur->next_sibling;
                else
                {
                    while (!cur->next_sibling && cur != _root && cur->parent)
                    {
                        --walker._depth;
                        cur = cur->parent;
                    }

                    if (cur != _root)
                        cur = cur->next_sibling;
                }
            }
            while (cur && cur != _root);
        }

        assert(walker._depth == -1);

        xml_node arg_end(_root);
        return walker.end(arg_end);
    }

    PUGI__FN size_t xml_node::hash_value() const
    {
        return static_cast<size_t>(reinterpret_cast<uintptr_t>(_root) / sizeof(xml_node_struct));
    }

    PUGI__FN xml_node_struct* xml_node::internal_object() const
    {
        return _root;
    }

    PUGI__FN void xml_node::print(xml_writer& writer, const char_t* indent, unsigned int flags, xml_encoding encoding, unsigned int depth) const
    {
        if (!_root) return;

        impl::xml_buffered_writer buffered_writer(writer, encoding);

        impl::node_output(buffered_writer, _root, indent, flags, depth);

        buffered_writer.flush();
    }

#ifndef PUGIXML_NO_STL
    PUGI__FN void xml_node::print(std::basic_ostream<char, std::char_traits<char> >& stream, const char_t* indent, unsigned int flags, xml_encoding encoding, unsigned int depth) const
    {
        xml_writer_stream writer(stream);

        print(writer, indent, flags, encoding, depth);
    }

    PUGI__FN void xml_node::print(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& stream, const char_t* indent, unsigned int flags, unsigned int depth) const
    {
        xml_writer_stream writer(stream);

        print(writer, indent, flags, encoding_wchar, depth);
    }
#endif

    PUGI__FN ptrdiff_t xml_node::offset_debug() const
    {
        if (!_root) return -1;

        impl::xml_document_struct& doc = impl::get_document(_root);

        // we can determine the offset reliably only if there is exactly once parse buffer
        if (!doc.buffer || doc.extra_buffers) return -1;

        switch (type())
        {
        case node_document:
            return 0;

        case node_element:
        case node_declaration:
        case node_pi:
            return _root->name && (_root->header & impl::xml_memory_page_name_allocated_or_shared_mask) == 0 ? _root->name - doc.buffer : -1;

        case node_pcdata:
        case node_cdata:
        case node_comment:
        case node_doctype:
            return _root->value && (_root->header & impl::xml_memory_page_value_allocated_or_shared_mask) == 0 ? _root->value - doc.buffer : -1;

        default:
            assert(false && "Invalid node type"); // unreachable
            return -1;
        }
    }

#ifdef __BORLANDC__
    PUGI__FN bool operator&&(const xml_node& lhs, bool rhs)
    {
        return (bool)lhs && rhs;
    }

    PUGI__FN bool operator||(const xml_node& lhs, bool rhs)
    {
        return (bool)lhs || rhs;
    }
#endif

    PUGI__FN xml_text::xml_text(xml_node_struct* root): _root(root)
    {
    }

    PUGI__FN xml_node_struct* xml_text::_data() const
    {
        if (!_root || impl::is_text_node(_root)) return _root;

        // element nodes can have value if parse_embed_pcdata was used
        if (PUGI__NODETYPE(_root) == node_element && _root->value)
            return _root;

        for (xml_node_struct* node = _root->first_child; node; node = node->next_sibling)
            if (impl::is_text_node(node))
                return node;

        return 0;
    }

    PUGI__FN xml_node_struct* xml_text::_data_new()
    {
        xml_node_struct* d = _data();
        if (d) return d;

        return xml_node(_root).append_child(node_pcdata).internal_object();
    }

    PUGI__FN xml_text::xml_text(): _root(0)
    {
    }

    PUGI__FN static void unspecified_bool_xml_text(xml_text***)
    {
    }

    PUGI__FN xml_text::operator xml_text::unspecified_bool_type() const
    {
        return _data() ? unspecified_bool_xml_text : 0;
    }

    PUGI__FN bool xml_text::operator!() const
    {
        return !_data();
    }

    PUGI__FN bool xml_text::empty() const
    {
        return _data() == 0;
    }

    PUGI__FN const char_t* xml_text::get() const
    {
        xml_node_struct* d = _data();

        return (d && d->value) ? d->value + 0 : PUGIXML_TEXT("");
    }

    PUGI__FN const char_t* xml_text::as_string(const char_t* def) const
    {
        xml_node_struct* d = _data();

        return (d && d->value) ? d->value + 0 : def;
    }

    PUGI__FN int xml_text::as_int(int def) const
    {
        xml_node_struct* d = _data();

        return (d && d->value) ? impl::get_value_int(d->value) : def;
    }

    PUGI__FN unsigned int xml_text::as_uint(unsigned int def) const
    {
        xml_node_struct* d = _data();

        return (d && d->value) ? impl::get_value_uint(d->value) : def;
    }

    PUGI__FN double xml_text::as_double(double def) const
    {
        xml_node_struct* d = _data();

        return (d && d->value) ? impl::get_value_double(d->value) : def;
    }

    PUGI__FN float xml_text::as_float(float def) const
    {
        xml_node_struct* d = _data();

        return (d && d->value) ? impl::get_value_float(d->value) : def;
    }

    PUGI__FN bool xml_text::as_bool(bool def) const
    {
        xml_node_struct* d = _data();

        return (d && d->value) ? impl::get_value_bool(d->value) : def;
    }

#ifdef PUGIXML_HAS_LONG_LONG
    PUGI__FN long long xml_text::as_llong(long long def) const
    {
        xml_node_struct* d = _data();

        return (d && d->value) ? impl::get_value_llong(d->value) : def;
    }

    PUGI__FN unsigned long long xml_text::as_ullong(unsigned long long def) const
    {
        xml_node_struct* d = _data();

        return (d && d->value) ? impl::get_value_ullong(d->value) : def;
    }
#endif

    PUGI__FN bool xml_text::set(const char_t* rhs)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::strcpy_insitu(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs, impl::strlength(rhs)) : false;
    }

    PUGI__FN bool xml_text::set(int rhs)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::set_value_integer<unsigned int>(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs, rhs < 0) : false;
    }

    PUGI__FN bool xml_text::set(unsigned int rhs)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::set_value_integer<unsigned int>(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs, false) : false;
    }

    PUGI__FN bool xml_text::set(long rhs)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::set_value_integer<unsigned long>(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs, rhs < 0) : false;
    }

    PUGI__FN bool xml_text::set(unsigned long rhs)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::set_value_integer<unsigned long>(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs, false) : false;
    }

    PUGI__FN bool xml_text::set(float rhs)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::set_value_convert(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs, default_float_precision) : false;
    }

    PUGI__FN bool xml_text::set(float rhs, int precision)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::set_value_convert(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs, precision) : false;
    }

    PUGI__FN bool xml_text::set(double rhs)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::set_value_convert(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs, default_double_precision) : false;
    }

    PUGI__FN bool xml_text::set(double rhs, int precision)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::set_value_convert(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs, precision) : false;
    }

    PUGI__FN bool xml_text::set(bool rhs)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::set_value_bool(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs) : false;
    }

#ifdef PUGIXML_HAS_LONG_LONG
    PUGI__FN bool xml_text::set(long long rhs)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::set_value_integer<unsigned long long>(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs, rhs < 0) : false;
    }

    PUGI__FN bool xml_text::set(unsigned long long rhs)
    {
        xml_node_struct* dn = _data_new();

        return dn ? impl::set_value_integer<unsigned long long>(dn->value, dn->header, impl::xml_memory_page_value_allocated_mask, rhs, false) : false;
    }
#endif

    PUGI__FN xml_text& xml_text::operator=(const char_t* rhs)
    {
        set(rhs);
        return *this;
    }

    PUGI__FN xml_text& xml_text::operator=(int rhs)
    {
        set(rhs);
        return *this;
    }

    PUGI__FN xml_text& xml_text::operator=(unsigned int rhs)
    {
        set(rhs);
        return *this;
    }

    PUGI__FN xml_text& xml_text::operator=(long rhs)
    {
        set(rhs);
        return *this;
    }

    PUGI__FN xml_text& xml_text::operator=(unsigned long rhs)
    {
        set(rhs);
        return *this;
    }

    PUGI__FN xml_text& xml_text::operator=(double rhs)
    {
        set(rhs);
        return *this;
    }

    PUGI__FN xml_text& xml_text::operator=(float rhs)
    {
        set(rhs);
        return *this;
    }

    PUGI__FN xml_text& xml_text::operator=(bool rhs)
    {
        set(rhs);
        return *this;
    }

#ifdef PUGIXML_HAS_LONG_LONG
    PUGI__FN xml_text& xml_text::operator=(long long rhs)
    {
        set(rhs);
        return *this;
    }

    PUGI__FN xml_text& xml_text::operator=(unsigned long long rhs)
    {
        set(rhs);
        return *this;
    }
#endif

    PUGI__FN xml_node xml_text::data() const
    {
        return xml_node(_data());
    }

#ifdef __BORLANDC__
    PUGI__FN bool operator&&(const xml_text& lhs, bool rhs)
    {
        return (bool)lhs && rhs;
    }

    PUGI__FN bool operator||(const xml_text& lhs, bool rhs)
    {
        return (bool)lhs || rhs;
    }
#endif

    PUGI__FN xml_node_iterator::xml_node_iterator()
    {
    }

    PUGI__FN xml_node_iterator::xml_node_iterator(const xml_node& node): _wrap(node), _parent(node.parent())
    {
    }

    PUGI__FN xml_node_iterator::xml_node_iterator(xml_node_struct* ref, xml_node_struct* parent): _wrap(ref), _parent(parent)
    {
    }

    PUGI__FN bool xml_node_iterator::operator==(const xml_node_iterator& rhs) const
    {
        return _wrap._root == rhs._wrap._root && _parent._root == rhs._parent._root;
    }

    PUGI__FN bool xml_node_iterator::operator!=(const xml_node_iterator& rhs) const
    {
        return _wrap._root != rhs._wrap._root || _parent._root != rhs._parent._root;
    }

    PUGI__FN xml_node& xml_node_iterator::operator*() const
    {
        assert(_wrap._root);
        return _wrap;
    }

    PUGI__FN xml_node* xml_node_iterator::operator->() const
    {
        assert(_wrap._root);
        return const_cast<xml_node*>(&_wrap); // BCC5 workaround
    }

    PUGI__FN xml_node_iterator& xml_node_iterator::operator++()
    {
        assert(_wrap._root);
        _wrap._root = _wrap._root->next_sibling;
        return *this;
    }

    PUGI__FN xml_node_iterator xml_node_iterator::operator++(int)
    {
        xml_node_iterator temp = *this;
        ++*this;
        return temp;
    }

    PUGI__FN xml_node_iterator& xml_node_iterator::operator--()
    {
        _wrap = _wrap._root ? _wrap.previous_sibling() : _parent.last_child();
        return *this;
    }

    PUGI__FN xml_node_iterator xml_node_iterator::operator--(int)
    {
        xml_node_iterator temp = *this;
        --*this;
        return temp;
    }

    PUGI__FN xml_attribute_iterator::xml_attribute_iterator()
    {
    }

    PUGI__FN xml_attribute_iterator::xml_attribute_iterator(const xml_attribute& attr, const xml_node& parent): _wrap(attr), _parent(parent)
    {
    }

    PUGI__FN xml_attribute_iterator::xml_attribute_iterator(xml_attribute_struct* ref, xml_node_struct* parent): _wrap(ref), _parent(parent)
    {
    }

    PUGI__FN bool xml_attribute_iterator::operator==(const xml_attribute_iterator& rhs) const
    {
        return _wrap._attr == rhs._wrap._attr && _parent._root == rhs._parent._root;
    }

    PUGI__FN bool xml_attribute_iterator::operator!=(const xml_attribute_iterator& rhs) const
    {
        return _wrap._attr != rhs._wrap._attr || _parent._root != rhs._parent._root;
    }

    PUGI__FN xml_attribute& xml_attribute_iterator::operator*() const
    {
        assert(_wrap._attr);
        return _wrap;
    }

    PUGI__FN xml_attribute* xml_attribute_iterator::operator->() const
    {
        assert(_wrap._attr);
        return const_cast<xml_attribute*>(&_wrap); // BCC5 workaround
    }

    PUGI__FN xml_attribute_iterator& xml_attribute_iterator::operator++()
    {
        assert(_wrap._attr);
        _wrap._attr = _wrap._attr->next_attribute;
        return *this;
    }

    PUGI__FN xml_attribute_iterator xml_attribute_iterator::operator++(int)
    {
        xml_attribute_iterator temp = *this;
        ++*this;
        return temp;
    }

    PUGI__FN xml_attribute_iterator& xml_attribute_iterator::operator--()
    {
        _wrap = _wrap._attr ? _wrap.previous_attribute() : _parent.last_attribute();
        return *this;
    }

    PUGI__FN xml_attribute_iterator xml_attribute_iterator::operator--(int)
    {
        xml_attribute_iterator temp = *this;
        --*this;
        return temp;
    }

    PUGI__FN xml_named_node_iterator::xml_named_node_iterator(): _name(0)
    {
    }

    PUGI__FN xml_named_node_iterator::xml_named_node_iterator(const xml_node& node, const char_t* name): _wrap(node), _parent(node.parent()), _name(name)
    {
    }

    PUGI__FN xml_named_node_iterator::xml_named_node_iterator(xml_node_struct* ref, xml_node_struct* parent, const char_t* name): _wrap(ref), _parent(parent), _name(name)
    {
    }

    PUGI__FN bool xml_named_node_iterator::operator==(const xml_named_node_iterator& rhs) const
    {
        return _wrap._root == rhs._wrap._root && _parent._root == rhs._parent._root;
    }

    PUGI__FN bool xml_named_node_iterator::operator!=(const xml_named_node_iterator& rhs) const
    {
        return _wrap._root != rhs._wrap._root || _parent._root != rhs._parent._root;
    }

    PUGI__FN xml_node& xml_named_node_iterator::operator*() const
    {
        assert(_wrap._root);
        return _wrap;
    }

    PUGI__FN xml_node* xml_named_node_iterator::operator->() const
    {
        assert(_wrap._root);
        return const_cast<xml_node*>(&_wrap); // BCC5 workaround
    }

    PUGI__FN xml_named_node_iterator& xml_named_node_iterator::operator++()
    {
        assert(_wrap._root);
        _wrap = _wrap.next_sibling(_name);
        return *this;
    }

    PUGI__FN xml_named_node_iterator xml_named_node_iterator::operator++(int)
    {
        xml_named_node_iterator temp = *this;
        ++*this;
        return temp;
    }

    PUGI__FN xml_named_node_iterator& xml_named_node_iterator::operator--()
    {
        if (_wrap._root)
            _wrap = _wrap.previous_sibling(_name);
        else
        {
            _wrap = _parent.last_child();

            if (!impl::strequal(_wrap.name(), _name))
                _wrap = _wrap.previous_sibling(_name);
        }

        return *this;
    }

    PUGI__FN xml_named_node_iterator xml_named_node_iterator::operator--(int)
    {
        xml_named_node_iterator temp = *this;
        --*this;
        return temp;
    }

    PUGI__FN xml_parse_result::xml_parse_result(): status(status_internal_error), offset(0), encoding(encoding_auto)
    {
    }

    PUGI__FN xml_parse_result::operator bool() const
    {
        return status == status_ok;
    }

    PUGI__FN const char* xml_parse_result::description() const
    {
        switch (status)
        {
        case status_ok: return "No error";

        case status_file_not_found: return "File was not found";
        case status_io_error: return "Error reading from file/stream";
        case status_out_of_memory: return "Could not allocate memory";
        case status_internal_error: return "Internal error occurred";

        case status_unrecognized_tag: return "Could not determine tag type";

        case status_bad_pi: return "Error parsing document declaration/processing instruction";
        case status_bad_comment: return "Error parsing comment";
        case status_bad_cdata: return "Error parsing CDATA section";
        case status_bad_doctype: return "Error parsing document type declaration";
        case status_bad_pcdata: return "Error parsing PCDATA section";
        case status_bad_start_element: return "Error parsing start element tag";
        case status_bad_attribute: return "Error parsing element attribute";
        case status_bad_end_element: return "Error parsing end element tag";
        case status_end_element_mismatch: return "Start-end tags mismatch";

        case status_append_invalid_root: return "Unable to append nodes: root is not an element or document";

        case status_no_document_element: return "No document element found";

        default: return "Unknown error";
        }
    }

    PUGI__FN xml_document::xml_document(): _buffer(0)
    {
        _create();
    }

    PUGI__FN xml_document::~xml_document()
    {
        _destroy();
    }

#ifdef PUGIXML_HAS_MOVE
    PUGI__FN xml_document::xml_document(xml_document&& rhs) PUGIXML_NOEXCEPT_IF_NOT_COMPACT: _buffer(0)
    {
        _create();
        _move(rhs);
    }

    PUGI__FN xml_document& xml_document::operator=(xml_document&& rhs) PUGIXML_NOEXCEPT_IF_NOT_COMPACT
    {
        if (this == &rhs) return *this;

        _destroy();
        _create();
        _move(rhs);

        return *this;
    }
#endif

    PUGI__FN void xml_document::reset()
    {
        _destroy();
        _create();
    }

    PUGI__FN void xml_document::reset(const xml_document& proto)
    {
        reset();

        impl::node_copy_tree(_root, proto._root);
    }

    PUGI__FN void xml_document::_create()
    {
        assert(!_root);

    #ifdef PUGIXML_COMPACT
        // space for page marker for the first page (uint32_t), rounded up to pointer size; assumes pointers are at least 32-bit
        const size_t page_offset = sizeof(void*);
    #else
        const size_t page_offset = 0;
    #endif

        // initialize sentinel page
        PUGI__STATIC_ASSERT(sizeof(impl::xml_memory_page) + sizeof(impl::xml_document_struct) + page_offset <= sizeof(_memory));

        // prepare page structure
        impl::xml_memory_page* page = impl::xml_memory_page::construct(_memory);
        assert(page);

        page->busy_size = impl::xml_memory_page_size;

        // setup first page marker
    #ifdef PUGIXML_COMPACT
        // round-trip through void* to avoid 'cast increases required alignment of target type' warning
        page->compact_page_marker = reinterpret_cast<uint32_t*>(static_cast<void*>(reinterpret_cast<char*>(page) + sizeof(impl::xml_memory_page)));
        *page->compact_page_marker = sizeof(impl::xml_memory_page);
    #endif

        // allocate new root
        _root = new (reinterpret_cast<char*>(page) + sizeof(impl::xml_memory_page) + page_offset) impl::xml_document_struct(page);
        _root->prev_sibling_c = _root;

        // setup sentinel page
        page->allocator = static_cast<impl::xml_document_struct*>(_root);

        // setup hash table pointer in allocator
    #ifdef PUGIXML_COMPACT
        page->allocator->_hash = &static_cast<impl::xml_document_struct*>(_root)->hash;
    #endif

        // verify the document allocation
        assert(reinterpret_cast<char*>(_root) + sizeof(impl::xml_document_struct) <= _memory + sizeof(_memory));
    }

    PUGI__FN void xml_document::_destroy()
    {
        assert(_root);

        // destroy static storage
        if (_buffer)
        {
            impl::xml_memory::deallocate(_buffer);
            _buffer = 0;
        }

        // destroy extra buffers (note: no need to destroy linked list nodes, they're allocated using document allocator)
        for (impl::xml_extra_buffer* extra = static_cast<impl::xml_document_struct*>(_root)->extra_buffers; extra; extra = extra->next)
        {
            if (extra->buffer) impl::xml_memory::deallocate(extra->buffer);
        }

        // destroy dynamic storage, leave sentinel page (it's in static memory)
        impl::xml_memory_page* root_page = PUGI__GETPAGE(_root);
        assert(root_page && !root_page->prev);
        assert(reinterpret_cast<char*>(root_page) >= _memory && reinterpret_cast<char*>(root_page) < _memory + sizeof(_memory));

        for (impl::xml_memory_page* page = root_page->next; page; )
        {
            impl::xml_memory_page* next = page->next;

            impl::xml_allocator::deallocate_page(page);

            page = next;
        }

    #ifdef PUGIXML_COMPACT
        // destroy hash table
        static_cast<impl::xml_document_struct*>(_root)->hash.clear();
    #endif

        _root = 0;
    }

#ifdef PUGIXML_HAS_MOVE
    PUGI__FN void xml_document::_move(xml_document& rhs) PUGIXML_NOEXCEPT_IF_NOT_COMPACT
    {
        impl::xml_document_struct* doc = static_cast<impl::xml_document_struct*>(_root);
        impl::xml_document_struct* other = static_cast<impl::xml_document_struct*>(rhs._root);

        // save first child pointer for later; this needs hash access
        xml_node_struct* other_first_child = other->first_child;

    #ifdef PUGIXML_COMPACT
        // reserve space for the hash table up front; this is the only operation that can fail
        // if it does, we have no choice but to throw (if we have exceptions)
        if (other_first_child)
        {
            size_t other_children = 0;
            for (xml_node_struct* node = other_first_child; node; node = node->next_sibling)
                other_children++;

            // in compact mode, each pointer assignment could result in a hash table request
            // during move, we have to relocate document first_child and parents of all children
            // normally there's just one child and its parent has a pointerless encoding but
            // we assume the worst here
            if (!other->_hash->reserve(other_children + 1))
            {
            #ifdef PUGIXML_NO_EXCEPTIONS
                return;
            #else
                throw std::bad_alloc();
            #endif
            }
        }
    #endif

        // move allocation state
        // note that other->_root may point to the embedded document page, in which case we should keep original (empty) state
        if (other->_root != PUGI__GETPAGE(other))
        {
            doc->_root = other->_root;
            doc->_busy_size = other->_busy_size;
        }

        // move buffer state
        doc->buffer = other->buffer;
        doc->extra_buffers = other->extra_buffers;
        _buffer = rhs._buffer;

    #ifdef PUGIXML_COMPACT
        // move compact hash; note that the hash table can have pointers to other but they will be "inactive", similarly to nodes removed with remove_child
        doc->hash = other->hash;
        doc->_hash = &doc->hash;

        // make sure we don't access other hash up until the end when we reinitialize other document
        other->_hash = 0;
    #endif

        // move page structure
        impl::xml_memory_page* doc_page = PUGI__GETPAGE(doc);
        assert(doc_page && !doc_page->prev && !doc_page->next);

        impl::xml_memory_page* other_page = PUGI__GETPAGE(other);
        assert(other_page && !other_page->prev);

        // relink pages since root page is embedded into xml_document
        if (impl::xml_memory_page* page = other_page->next)
        {
            assert(page->prev == other_page);

            page->prev = doc_page;

            doc_page->next = page;
            other_page->next = 0;
        }

        // make sure pages point to the correct document state
        for (impl::xml_memory_page* page = doc_page->next; page; page = page->next)
        {
            assert(page->allocator == other);

            page->allocator = doc;

        #ifdef PUGIXML_COMPACT
            // this automatically migrates most children between documents and prevents ->parent assignment from allocating
            if (page->compact_shared_parent == other)
                page->compact_shared_parent = doc;
        #endif
        }

        // move tree structure
        assert(!doc->first_child);

        doc->first_child = other_first_child;

        for (xml_node_struct* node = other_first_child; node; node = node->next_sibling)
        {
        #ifdef PUGIXML_COMPACT
            // most children will have migrated when we reassigned compact_shared_parent
            assert(node->parent == other || node->parent == doc);

            node->parent = doc;
        #else
            assert(node->parent == other);
            node->parent = doc;
        #endif
        }

        // reset other document
        new (other) impl::xml_document_struct(PUGI__GETPAGE(other));
        rhs._buffer = 0;
    }
#endif

#ifndef PUGIXML_NO_STL
    PUGI__FN xml_parse_result xml_document::load(std::basic_istream<char, std::char_traits<char> >& stream, unsigned int options, xml_encoding encoding)
    {
        reset();

        return impl::load_stream_impl(static_cast<impl::xml_document_struct*>(_root), stream, options, encoding, &_buffer);
    }

    PUGI__FN xml_parse_result xml_document::load(std::basic_istream<wchar_t, std::char_traits<wchar_t> >& stream, unsigned int options)
    {
        reset();

        return impl::load_stream_impl(static_cast<impl::xml_document_struct*>(_root), stream, options, encoding_wchar, &_buffer);
    }
#endif

    PUGI__FN xml_parse_result xml_document::load_string(const char_t* contents, unsigned int options)
    {
        // Force native encoding (skip autodetection)
    #ifdef PUGIXML_WCHAR_MODE
        xml_encoding encoding = encoding_wchar;
    #else
        xml_encoding encoding = encoding_utf8;
    #endif

        return load_buffer(contents, impl::strlength(contents) * sizeof(char_t), options, encoding);
    }

    PUGI__FN xml_parse_result xml_document::load(const char_t* contents, unsigned int options)
    {
        return load_string(contents, options);
    }

    PUGI__FN xml_parse_result xml_document::load_file(const char* path_, unsigned int options, xml_encoding encoding)
    {
        reset();

        using impl::auto_deleter; // MSVC7 workaround
        auto_deleter<FILE> file(impl::open_file(path_, "rb"), impl::close_file);

        return impl::load_file_impl(static_cast<impl::xml_document_struct*>(_root), file.data, options, encoding, &_buffer);
    }

    PUGI__FN xml_parse_result xml_document::load_file(const wchar_t* path_, unsigned int options, xml_encoding encoding)
    {
        reset();

        using impl::auto_deleter; // MSVC7 workaround
        auto_deleter<FILE> file(impl::open_file_wide(path_, L"rb"), impl::close_file);

        return impl::load_file_impl(static_cast<impl::xml_document_struct*>(_root), file.data, options, encoding, &_buffer);
    }

    PUGI__FN xml_parse_result xml_document::load_buffer(const void* contents, size_t size, unsigned int options, xml_encoding encoding)
    {
        reset();

        return impl::load_buffer_impl(static_cast<impl::xml_document_struct*>(_root), _root, const_cast<void*>(contents), size, options, encoding, false, false, &_buffer);
    }

    PUGI__FN xml_parse_result xml_document::load_buffer_inplace(void* contents, size_t size, unsigned int options, xml_encoding encoding)
    {
        reset();

        return impl::load_buffer_impl(static_cast<impl::xml_document_struct*>(_root), _root, contents, size, options, encoding, true, false, &_buffer);
    }

    PUGI__FN xml_parse_result xml_document::load_buffer_inplace_own(void* contents, size_t size, unsigned int options, xml_encoding encoding)
    {
        reset();

        return impl::load_buffer_impl(static_cast<impl::xml_document_struct*>(_root), _root, contents, size, options, encoding, true, true, &_buffer);
    }

    PUGI__FN void xml_document::save(xml_writer& writer, const char_t* indent, unsigned int flags, xml_encoding encoding) const
    {
        impl::xml_buffered_writer buffered_writer(writer, encoding);

        if ((flags & format_write_bom) && encoding != encoding_latin1)
        {
            // BOM always represents the codepoint U+FEFF, so just write it in native encoding
        #ifdef PUGIXML_WCHAR_MODE
            unsigned int bom = 0xfeff;
            buffered_writer.write(static_cast<wchar_t>(bom));
        #else
            buffered_writer.write('\xef', '\xbb', '\xbf');
        #endif
        }

        if (!(flags & format_no_declaration) && !impl::has_declaration(_root))
        {
            buffered_writer.write_string(PUGIXML_TEXT("<?xml version=\"1.0\""));
            if (encoding == encoding_latin1) buffered_writer.write_string(PUGIXML_TEXT(" encoding=\"ISO-8859-1\""));
            buffered_writer.write('?', '>');
            if (!(flags & format_raw)) buffered_writer.write('\n');
        }

        impl::node_output(buffered_writer, _root, indent, flags, 0);

        buffered_writer.flush();
    }

#ifndef PUGIXML_NO_STL
    PUGI__FN void xml_document::save(std::basic_ostream<char, std::char_traits<char> >& stream, const char_t* indent, unsigned int flags, xml_encoding encoding) const
    {
        xml_writer_stream writer(stream);

        save(writer, indent, flags, encoding);
    }

    PUGI__FN void xml_document::save(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& stream, const char_t* indent, unsigned int flags) const
    {
        xml_writer_stream writer(stream);

        save(writer, indent, flags, encoding_wchar);
    }
#endif

    PUGI__FN bool xml_document::save_file(const char* path_, const char_t* indent, unsigned int flags, xml_encoding encoding) const
    {
        using impl::auto_deleter; // MSVC7 workaround
        auto_deleter<FILE> file(impl::open_file(path_, (flags & format_save_file_text) ? "w" : "wb"), impl::close_file);

        return impl::save_file_impl(*this, file.data, indent, flags, encoding);
    }

    PUGI__FN bool xml_document::save_file(const wchar_t* path_, const char_t* indent, unsigned int flags, xml_encoding encoding) const
    {
        using impl::auto_deleter; // MSVC7 workaround
        auto_deleter<FILE> file(impl::open_file_wide(path_, (flags & format_save_file_text) ? L"w" : L"wb"), impl::close_file);

        return impl::save_file_impl(*this, file.data, indent, flags, encoding);
    }

    PUGI__FN xml_node xml_document::document_element() const
    {
        assert(_root);

        for (xml_node_struct* i = _root->first_child; i; i = i->next_sibling)
            if (PUGI__NODETYPE(i) == node_element)
                return xml_node(i);

        return xml_node();
    }

#ifndef PUGIXML_NO_STL
    PUGI__FN std::string PUGIXML_FUNCTION as_utf8(const wchar_t* str)
    {
        assert(str);

        return impl::as_utf8_impl(str, impl::strlength_wide(str));
    }

    PUGI__FN std::string PUGIXML_FUNCTION as_utf8(const std::basic_string<wchar_t>& str)
    {
        return impl::as_utf8_impl(str.c_str(), str.size());
    }

    PUGI__FN std::basic_string<wchar_t> PUGIXML_FUNCTION as_wide(const char* str)
    {
        assert(str);

        return impl::as_wide_impl(str, strlen(str));
    }

    PUGI__FN std::basic_string<wchar_t> PUGIXML_FUNCTION as_wide(const std::string& str)
    {
        return impl::as_wide_impl(str.c_str(), str.size());
    }
#endif

    PUGI__FN void PUGIXML_FUNCTION set_memory_management_functions(allocation_function allocate, deallocation_function deallocate)
    {
        impl::xml_memory::allocate = allocate;
        impl::xml_memory::deallocate = deallocate;
    }

    PUGI__FN allocation_function PUGIXML_FUNCTION get_memory_allocation_function()
    {
        return impl::xml_memory::allocate;
    }

    PUGI__FN deallocation_function PUGIXML_FUNCTION get_memory_deallocation_function()
    {
        return impl::xml_memory::deallocate;
    }
}

#if !defined(PUGIXML_NO_STL) && (defined(_MSC_VER) || defined(__ICC))
namespace std
{
    // Workarounds for (non-standard) iterator category detection for older versions (MSVC7/IC8 and earlier)
    PUGI__FN std::bidirectional_iterator_tag _Iter_cat(const pugi::xml_node_iterator&)
    {
        return std::bidirectional_iterator_tag();
    }

    PUGI__FN std::bidirectional_iterator_tag _Iter_cat(const pugi::xml_attribute_iterator&)
    {
        return std::bidirectional_iterator_tag();
    }

    PUGI__FN std::bidirectional_iterator_tag _Iter_cat(const pugi::xml_named_node_iterator&)
    {
        return std::bidirectional_iterator_tag();
    }
}
#endif

#if !defined(PUGIXML_NO_STL) && defined(__SUNPRO_CC)
namespace std
{
    // Workarounds for (non-standard) iterator category detection
    PUGI__FN std::bidirectional_iterator_tag __iterator_category(const pugi::xml_node_iterator&)
    {
        return std::bidirectional_iterator_tag();
    }

    PUGI__FN std::bidirectional_iterator_tag __iterator_category(const pugi::xml_attribute_iterator&)
    {
        return std::bidirectional_iterator_tag();
    }

    PUGI__FN std::bidirectional_iterator_tag __iterator_category(const pugi::xml_named_node_iterator&)
    {
        return std::bidirectional_iterator_tag();
    }
}
#endif

#ifndef PUGIXML_NO_XPATH
// STL replacements
PUGI__NS_BEGIN
    struct equal_to
    {
        template <typename T> bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs == rhs;
        }
    };

    struct not_equal_to
    {
        template <typename T> bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs != rhs;
        }
    };

    struct less
    {
        template <typename T> bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs < rhs;
        }
    };

    struct less_equal
    {
        template <typename T> bool operator()(const T& lhs, const T& rhs) const
        {
            return lhs <= rhs;
        }
    };

    template <typename T> inline void swap(T& lhs, T& rhs)
    {
        T temp = lhs;
        lhs = rhs;
        rhs = temp;
    }

    template <typename I, typename Pred> PUGI__FN I min_element(I begin, I end, const Pred& pred)
    {
        I result = begin;

        for (I it = begin + 1; it != end; ++it)
            if (pred(*it, *result))
                result = it;

        return result;
    }

    template <typename I> PUGI__FN void reverse(I begin, I end)
    {
        while (end - begin > 1)
            swap(*begin++, *--end);
    }

    template <typename I> PUGI__FN I unique(I begin, I end)
    {
        // fast skip head
        while (end - begin > 1 && *begin != *(begin + 1))
            begin++;

        if (begin == end)
            return begin;

        // last written element
        I write = begin++;

        // merge unique elements
        while (begin != end)
        {
            if (*begin != *write)
                *++write = *begin++;
            else
                begin++;
        }

        // past-the-end (write points to live element)
        return write + 1;
    }

    template <typename T, typename Pred> PUGI__FN void insertion_sort(T* begin, T* end, const Pred& pred)
    {
        if (begin == end)
            return;

        for (T* it = begin + 1; it != end; ++it)
        {
            T val = *it;
            T* hole = it;

            // move hole backwards
            while (hole > begin && pred(val, *(hole - 1)))
            {
                *hole = *(hole - 1);
                hole--;
            }

            // fill hole with element
            *hole = val;
        }
    }

    template <typename I, typename Pred> inline I median3(I first, I middle, I last, const Pred& pred)
    {
        if (pred(*middle, *first))
            swap(middle, first);
        if (pred(*last, *middle))
            swap(last, middle);
        if (pred(*middle, *first))
            swap(middle, first);

        return middle;
    }

    template <typename T, typename Pred> PUGI__FN void partition3(T* begin, T* end, T pivot, const Pred& pred, T** out_eqbeg, T** out_eqend)
    {
        // invariant: array is split into 4 groups: = < ? > (each variable denotes the boundary between the groups)
        T* eq = begin;
        T* lt = begin;
        T* gt = end;

        while (lt < gt)
        {
            if (pred(*lt, pivot))
                lt++;
            else if (*lt == pivot)
                swap(*eq++, *lt++);
            else
                swap(*lt, *--gt);
        }

        // we now have just 4 groups: = < >; move equal elements to the middle
        T* eqbeg = gt;

        for (T* it = begin; it != eq; ++it)
            swap(*it, *--eqbeg);

        *out_eqbeg = eqbeg;
        *out_eqend = gt;
    }

    template <typename I, typename Pred> PUGI__FN void sort(I begin, I end, const Pred& pred)
    {
        // sort large chunks
        while (end - begin > 16)
        {
            // find median element
            I middle = begin + (end - begin) / 2;
            I median = median3(begin, middle, end - 1, pred);

            // partition in three chunks (< = >)
            I eqbeg, eqend;
            partition3(begin, end, *median, pred, &eqbeg, &eqend);

            // loop on larger half
            if (eqbeg - begin > end - eqend)
            {
                sort(eqend, end, pred);
                end = eqbeg;
            }
            else
            {
                sort(begin, eqbeg, pred);
                begin = eqend;
            }
        }

        // insertion sort small chunk
        insertion_sort(begin, end, pred);
    }

    PUGI__FN bool hash_insert(const void** table, size_t size, const void* key)
    {
        assert(key);

        unsigned int h = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(key));

        // MurmurHash3 32-bit finalizer
        h ^= h >> 16;
        h *= 0x85ebca6bu;
        h ^= h >> 13;
        h *= 0xc2b2ae35u;
        h ^= h >> 16;

        size_t hashmod = size - 1;
        size_t bucket = h & hashmod;

        for (size_t probe = 0; probe <= hashmod; ++probe)
        {
            if (table[bucket] == 0)
            {
                table[bucket] = key;
                return true;
            }

            if (table[bucket] == key)
                return false;

            // hash collision, quadratic probing
            bucket = (bucket + probe + 1) & hashmod;
        }

        assert(false && "Hash table is full"); // unreachable
        return false;
    }
PUGI__NS_END

// Allocator used for AST and evaluation stacks
PUGI__NS_BEGIN
    static const size_t xpath_memory_page_size =
    #ifdef PUGIXML_MEMORY_XPATH_PAGE_SIZE
        PUGIXML_MEMORY_XPATH_PAGE_SIZE
    #else
        4096
    #endif
        ;

    static const uintptr_t xpath_memory_block_alignment = sizeof(double) > sizeof(void*) ? sizeof(double) : sizeof(void*);

    struct xpath_memory_block
    {
        xpath_memory_block* next;
        size_t capacity;

        union
        {
            char data[xpath_memory_page_size];
            double alignment;
        };
    };

    struct xpath_allocator
    {
        xpath_memory_block* _root;
        size_t _root_size;
        bool* _error;

        xpath_allocator(xpath_memory_block* root, bool* error = 0): _root(root), _root_size(0), _error(error)
        {
        }

        void* allocate(size_t size)
        {
            // round size up to block alignment boundary
            size = (size + xpath_memory_block_alignment - 1) & ~(xpath_memory_block_alignment - 1);

            if (_root_size + size <= _root->capacity)
            {
                void* buf = &_root->data[0] + _root_size;
                _root_size += size;
                return buf;
            }
            else
            {
                // make sure we have at least 1/4th of the page free after allocation to satisfy subsequent allocation requests
                size_t block_capacity_base = sizeof(_root->data);
                size_t block_capacity_req = size + block_capacity_base / 4;
                size_t block_capacity = (block_capacity_base > block_capacity_req) ? block_capacity_base : block_capacity_req;

                size_t block_size = block_capacity + offsetof(xpath_memory_block, data);

                xpath_memory_block* block = static_cast<xpath_memory_block*>(xml_memory::allocate(block_size));
                if (!block)
                {
                    if (_error) *_error = true;
                    return 0;
                }

                block->next = _root;
                block->capacity = block_capacity;

                _root = block;
                _root_size = size;

                return block->data;
            }
        }

        void* reallocate(void* ptr, size_t old_size, size_t new_size)
        {
            // round size up to block alignment boundary
            old_size = (old_size + xpath_memory_block_alignment - 1) & ~(xpath_memory_block_alignment - 1);
            new_size = (new_size + xpath_memory_block_alignment - 1) & ~(xpath_memory_block_alignment - 1);

            // we can only reallocate the last object
            assert(ptr == 0 || static_cast<char*>(ptr) + old_size == &_root->data[0] + _root_size);

            // try to reallocate the object inplace
            if (ptr && _root_size - old_size + new_size <= _root->capacity)
            {
                _root_size = _root_size - old_size + new_size;
                return ptr;
            }

            // allocate a new block
            void* result = allocate(new_size);
            if (!result) return 0;

            // we have a new block
            if (ptr)
            {
                // copy old data (we only support growing)
                assert(new_size >= old_size);
                memcpy(result, ptr, old_size);

                // free the previous page if it had no other objects
                assert(_root->data == result);
                assert(_root->next);

                if (_root->next->data == ptr)
                {
                    // deallocate the whole page, unless it was the first one
                    xpath_memory_block* next = _root->next->next;

                    if (next)
                    {
                        xml_memory::deallocate(_root->next);
                        _root->next = next;
                    }
                }
            }

            return result;
        }

        void revert(const xpath_allocator& state)
        {
            // free all new pages
            xpath_memory_block* cur = _root;

            while (cur != state._root)
            {
                xpath_memory_block* next = cur->next;

                xml_memory::deallocate(cur);

                cur = next;
            }

            // restore state
            _root = state._root;
            _root_size = state._root_size;
        }

        void release()
        {
            xpath_memory_block* cur = _root;
            assert(cur);

            while (cur->next)
            {
                xpath_memory_block* next = cur->next;

                xml_memory::deallocate(cur);

                cur = next;
            }
        }
    };

    struct xpath_allocator_capture
    {
        xpath_allocator_capture(xpath_allocator* alloc): _target(alloc), _state(*alloc)
        {
        }

        ~xpath_allocator_capture()
        {
            _target->revert(_state);
        }

        xpath_allocator* _target;
        xpath_allocator _state;
    };

    struct xpath_stack
    {
        xpath_allocator* result;
        xpath_allocator* temp;
    };

    struct xpath_stack_data
    {
        xpath_memory_block blocks[2];
        xpath_allocator result;
        xpath_allocator temp;
        xpath_stack stack;
        bool oom;

        xpath_stack_data(): result(blocks + 0, &oom), temp(blocks + 1, &oom), oom(false)
        {
            blocks[0].next = blocks[1].next = 0;
            blocks[0].capacity = blocks[1].capacity = sizeof(blocks[0].data);

            stack.result = &result;
            stack.temp = &temp;
        }

        ~xpath_stack_data()
        {
            result.release();
            temp.release();
        }
    };
PUGI__NS_END

// String class
PUGI__NS_BEGIN
    class xpath_string
    {
        const char_t* _buffer;
        bool _uses_heap;
        size_t _length_heap;

        static char_t* duplicate_string(const char_t* string, size_t length, xpath_allocator* alloc)
        {
            char_t* result = static_cast<char_t*>(alloc->allocate((length + 1) * sizeof(char_t)));
            if (!result) return 0;

            memcpy(result, string, length * sizeof(char_t));
            result[length] = 0;

            return result;
        }

        xpath_string(const char_t* buffer, bool uses_heap_, size_t length_heap): _buffer(buffer), _uses_heap(uses_heap_), _length_heap(length_heap)
        {
        }

    public:
        static xpath_string from_const(const char_t* str)
        {
            return xpath_string(str, false, 0);
        }

        static xpath_string from_heap_preallocated(const char_t* begin, const char_t* end)
        {
            assert(begin <= end && *end == 0);

            return xpath_string(begin, true, static_cast<size_t>(end - begin));
        }

        static xpath_string from_heap(const char_t* begin, const char_t* end, xpath_allocator* alloc)
        {
            assert(begin <= end);

            if (begin == end)
                return xpath_string();

            size_t length = static_cast<size_t>(end - begin);
            const char_t* data = duplicate_string(begin, length, alloc);

            return data ? xpath_string(data, true, length) : xpath_string();
        }

        xpath_string(): _buffer(PUGIXML_TEXT("")), _uses_heap(false), _length_heap(0)
        {
        }

        void append(const xpath_string& o, xpath_allocator* alloc)
        {
            // skip empty sources
            if (!*o._buffer) return;

            // fast append for constant empty target and constant source
            if (!*_buffer && !_uses_heap && !o._uses_heap)
            {
                _buffer = o._buffer;
            }
            else
            {
                // need to make heap copy
                size_t target_length = length();
                size_t source_length = o.length();
                size_t result_length = target_length + source_length;

                // allocate new buffer
                char_t* result = static_cast<char_t*>(alloc->reallocate(_uses_heap ? const_cast<char_t*>(_buffer) : 0, (target_length + 1) * sizeof(char_t), (result_length + 1) * sizeof(char_t)));
                if (!result) return;

                // append first string to the new buffer in case there was no reallocation
                if (!_uses_heap) memcpy(result, _buffer, target_length * sizeof(char_t));

                // append second string to the new buffer
                memcpy(result + target_length, o._buffer, source_length * sizeof(char_t));
                result[result_length] = 0;

                // finalize
                _buffer = result;
                _uses_heap = true;
                _length_heap = result_length;
            }
        }

        const char_t* c_str() const
        {
            return _buffer;
        }

        size_t length() const
        {
            return _uses_heap ? _length_heap : strlength(_buffer);
        }

        char_t* data(xpath_allocator* alloc)
        {
            // make private heap copy
            if (!_uses_heap)
            {
                size_t length_ = strlength(_buffer);
                const char_t* data_ = duplicate_string(_buffer, length_, alloc);

                if (!data_) return 0;

                _buffer = data_;
                _uses_heap = true;
                _length_heap = length_;
            }

            return const_cast<char_t*>(_buffer);
        }

        bool empty() const
        {
            return *_buffer == 0;
        }

        bool operator==(const xpath_string& o) const
        {
            return strequal(_buffer, o._buffer);
        }

        bool operator!=(const xpath_string& o) const
        {
            return !strequal(_buffer, o._buffer);
        }

        bool uses_heap() const
        {
            return _uses_heap;
        }
    };
PUGI__NS_END

PUGI__NS_BEGIN
    PUGI__FN bool starts_with(const char_t* string, const char_t* pattern)
    {
        while (*pattern && *string == *pattern)
        {
            string++;
            pattern++;
        }

        return *pattern == 0;
    }

    PUGI__FN const char_t* find_char(const char_t* s, char_t c)
    {
    #ifdef PUGIXML_WCHAR_MODE
        return wcschr(s, c);
    #else
        return strchr(s, c);
    #endif
    }

    PUGI__FN const char_t* find_substring(const char_t* s, const char_t* p)
    {
    #ifdef PUGIXML_WCHAR_MODE
        // MSVC6 wcsstr bug workaround (if s is empty it always returns 0)
        return (*p == 0) ? s : wcsstr(s, p);
    #else
        return strstr(s, p);
    #endif
    }

    // Converts symbol to lower case, if it is an ASCII one
    PUGI__FN char_t tolower_ascii(char_t ch)
    {
        return static_cast<unsigned int>(ch - 'A') < 26 ? static_cast<char_t>(ch | ' ') : ch;
    }

    PUGI__FN xpath_string string_value(const xpath_node& na, xpath_allocator* alloc)
    {
        if (na.attribute())
            return xpath_string::from_const(na.attribute().value());
        else
        {
            xml_node n = na.node();

            switch (n.type())
            {
            case node_pcdata:
            case node_cdata:
            case node_comment:
            case node_pi:
                return xpath_string::from_const(n.value());

            case node_document:
            case node_element:
            {
                xpath_string result;

                // element nodes can have value if parse_embed_pcdata was used
                if (n.value()[0])
                    result.append(xpath_string::from_const(n.value()), alloc);

                xml_node cur = n.first_child();

                while (cur && cur != n)
                {
                    if (cur.type() == node_pcdata || cur.type() == node_cdata)
                        result.append(xpath_string::from_const(cur.value()), alloc);

                    if (cur.first_child())
                        cur = cur.first_child();
                    else if (cur.next_sibling())
                        cur = cur.next_sibling();
                    else
                    {
                        while (!cur.next_sibling() && cur != n)
                            cur = cur.parent();

                        if (cur != n) cur = cur.next_sibling();
                    }
                }

                return result;
            }

            default:
                return xpath_string();
            }
        }
    }

    PUGI__FN bool node_is_before_sibling(xml_node_struct* ln, xml_node_struct* rn)
    {
        assert(ln->parent == rn->parent);

        // there is no common ancestor (the shared parent is null), nodes are from different documents
        if (!ln->parent) return ln < rn;

        // determine sibling order
        xml_node_struct* ls = ln;
        xml_node_struct* rs = rn;

        while (ls && rs)
        {
            if (ls == rn) return true;
            if (rs == ln) return false;

            ls = ls->next_sibling;
            rs = rs->next_sibling;
        }

        // if rn sibling chain ended ln must be before rn
        return !rs;
    }

    PUGI__FN bool node_is_before(xml_node_struct* ln, xml_node_struct* rn)
    {
        // find common ancestor at the same depth, if any
        xml_node_struct* lp = ln;
        xml_node_struct* rp = rn;

        while (lp && rp && lp->parent != rp->parent)
        {
            lp = lp->parent;
            rp = rp->parent;
        }

        // parents are the same!
        if (lp && rp) return node_is_before_sibling(lp, rp);

        // nodes are at different depths, need to normalize heights
        bool left_higher = !lp;

        while (lp)
        {
            lp = lp->parent;
            ln = ln->parent;
        }

        while (rp)
        {
            rp = rp->parent;
            rn = rn->parent;
        }

        // one node is the ancestor of the other
        if (ln == rn) return left_higher;

        // find common ancestor... again
        while (ln->parent != rn->parent)
        {
            ln = ln->parent;
            rn = rn->parent;
        }

        return node_is_before_sibling(ln, rn);
    }

    PUGI__FN bool node_is_ancestor(xml_node_struct* parent, xml_node_struct* node)
    {
        while (node && node != parent) node = node->parent;

        return parent && node == parent;
    }

    PUGI__FN const void* document_buffer_order(const xpath_node& xnode)
    {
        xml_node_struct* node = xnode.node().internal_object();

        if (node)
        {
            if ((get_document(node).header & xml_memory_page_contents_shared_mask) == 0)
            {
                if (node->name && (node->header & impl::xml_memory_page_name_allocated_or_shared_mask) == 0) return node->name;
                if (node->value && (node->header & impl::xml_memory_page_value_allocated_or_shared_mask) == 0) return node->value;
            }

            return 0;
        }

        xml_attribute_struct* attr = xnode.attribute().internal_object();

        if (attr)
        {
            if ((get_document(attr).header & xml_memory_page_contents_shared_mask) == 0)
            {
                if ((attr->header & impl::xml_memory_page_name_allocated_or_shared_mask) == 0) return attr->name;
                if ((attr->header & impl::xml_memory_page_value_allocated_or_shared_mask) == 0) return attr->value;
            }

            return 0;
        }

        return 0;
    }

    struct document_order_comparator
    {
        bool operator()(const xpath_node& lhs, const xpath_node& rhs) const
        {
            // optimized document order based check
            const void* lo = document_buffer_order(lhs);
            const void* ro = document_buffer_order(rhs);

            if (lo && ro) return lo < ro;

            // slow comparison
            xml_node ln = lhs.node(), rn = rhs.node();

            // compare attributes
            if (lhs.attribute() && rhs.attribute())
            {
                // shared parent
                if (lhs.parent() == rhs.parent())
                {
                    // determine sibling order
                    for (xml_attribute a = lhs.attribute(); a; a = a.next_attribute())
                        if (a == rhs.attribute())
                            return true;

                    return false;
                }

                // compare attribute parents
                ln = lhs.parent();
                rn = rhs.parent();
            }
            else if (lhs.attribute())
            {
                // attributes go after the parent element
                if (lhs.parent() == rhs.node()) return false;

                ln = lhs.parent();
            }
            else if (rhs.attribute())
            {
                // attributes go after the parent element
                if (rhs.parent() == lhs.node()) return true;

                rn = rhs.parent();
            }

            if (ln == rn) return false;

            if (!ln || !rn) return ln < rn;

            return node_is_before(ln.internal_object(), rn.internal_object());
        }
    };

    PUGI__FN double gen_nan()
    {
    #if defined(__STDC_IEC_559__) || ((FLT_RADIX - 0 == 2) && (FLT_MAX_EXP - 0 == 128) && (FLT_MANT_DIG - 0 == 24))
        PUGI__STATIC_ASSERT(sizeof(float) == sizeof(uint32_t));
        typedef uint32_t UI; // BCC5 workaround
        union { float f; UI i; } u;
        u.i = 0x7fc00000;
        return double(u.f);
    #else
        // fallback
        const volatile double zero = 0.0;
        return zero / zero;
    #endif
    }

    PUGI__FN bool is_nan(double value)
    {
    #if defined(PUGI__MSVC_CRT_VERSION) || defined(__BORLANDC__)
        return !!_isnan(value);
    #elif defined(fpclassify) && defined(FP_NAN)
        return fpclassify(value) == FP_NAN;
    #else
        // fallback
        const volatile double v = value;
        return v != v;
    #endif
    }

    PUGI__FN const char_t* convert_number_to_string_special(double value)
    {
    #if defined(PUGI__MSVC_CRT_VERSION) || defined(__BORLANDC__)
        if (_finite(value)) return (value == 0) ? PUGIXML_TEXT("0") : 0;
        if (_isnan(value)) return PUGIXML_TEXT("NaN");
        return value > 0 ? PUGIXML_TEXT("Infinity") : PUGIXML_TEXT("-Infinity");
    #elif defined(fpclassify) && defined(FP_NAN) && defined(FP_INFINITE) && defined(FP_ZERO)
        switch (fpclassify(value))
        {
        case FP_NAN:
            return PUGIXML_TEXT("NaN");

        case FP_INFINITE:
            return value > 0 ? PUGIXML_TEXT("Infinity") : PUGIXML_TEXT("-Infinity");

        case FP_ZERO:
            return PUGIXML_TEXT("0");

        default:
            return 0;
        }
    #else
        // fallback
        const volatile double v = value;

        if (v == 0) return PUGIXML_TEXT("0");
        if (v != v) return PUGIXML_TEXT("NaN");
        if (v * 2 == v) return value > 0 ? PUGIXML_TEXT("Infinity") : PUGIXML_TEXT("-Infinity");
        return 0;
    #endif
    }

    PUGI__FN bool convert_number_to_boolean(double value)
    {
        return (value != 0 && !is_nan(value));
    }

    PUGI__FN void truncate_zeros(char* begin, char* end)
    {
        while (begin != end && end[-1] == '0') end--;

        *end = 0;
    }

    // gets mantissa digits in the form of 0.xxxxx with 0. implied and the exponent
#if defined(PUGI__MSVC_CRT_VERSION) && PUGI__MSVC_CRT_VERSION >= 1400
    PUGI__FN void convert_number_to_mantissa_exponent(double value, char (&buffer)[32], char** out_mantissa, int* out_exponent)
    {
        // get base values
        int sign, exponent;
        _ecvt_s(buffer, sizeof(buffer), value, DBL_DIG + 1, &exponent, &sign);

        // truncate redundant zeros
        truncate_zeros(buffer, buffer + strlen(buffer));

        // fill results
        *out_mantissa = buffer;
        *out_exponent = exponent;
    }
#else
    PUGI__FN void convert_number_to_mantissa_exponent(double value, char (&buffer)[32], char** out_mantissa, int* out_exponent)
    {
        // get a scientific notation value with IEEE DBL_DIG decimals
        PUGI__SNPRINTF(buffer, "%.*e", DBL_DIG, value);

        // get the exponent (possibly negative)
        char* exponent_string = strchr(buffer, 'e');
        assert(exponent_string);

        int exponent = atoi(exponent_string + 1);

        // extract mantissa string: skip sign
        char* mantissa = buffer[0] == '-' ? buffer + 1 : buffer;
        assert(mantissa[0] != '0' && mantissa[1] == '.');

        // divide mantissa by 10 to eliminate integer part
        mantissa[1] = mantissa[0];
        mantissa++;
        exponent++;

        // remove extra mantissa digits and zero-terminate mantissa
        truncate_zeros(mantissa, exponent_string);

        // fill results
        *out_mantissa = mantissa;
        *out_exponent = exponent;
    }
#endif

    PUGI__FN xpath_string convert_number_to_string(double value, xpath_allocator* alloc)
    {
        // try special number conversion
        const char_t* special = convert_number_to_string_special(value);
        if (special) return xpath_string::from_const(special);

        // get mantissa + exponent form
        char mantissa_buffer[32];

        char* mantissa;
        int exponent;
        convert_number_to_mantissa_exponent(value, mantissa_buffer, &mantissa, &exponent);

        // allocate a buffer of suitable length for the number
        size_t result_size = strlen(mantissa_buffer) + (exponent > 0 ? exponent : -exponent) + 4;
        char_t* result = static_cast<char_t*>(alloc->allocate(sizeof(char_t) * result_size));
        if (!result) return xpath_string();

        // make the number!
        char_t* s = result;

        // sign
        if (value < 0) *s++ = '-';

        // integer part
        if (exponent <= 0)
        {
            *s++ = '0';
        }
        else
        {
            while (exponent > 0)
            {
                assert(*mantissa == 0 || static_cast<unsigned int>(*mantissa - '0') <= 9);
                *s++ = *mantissa ? *mantissa++ : '0';
                exponent--;
            }
        }

        // fractional part
        if (*mantissa)
        {
            // decimal point
            *s++ = '.';

            // extra zeroes from negative exponent
            while (exponent < 0)
            {
                *s++ = '0';
                exponent++;
            }

            // extra mantissa digits
            while (*mantissa)
            {
                assert(static_cast<unsigned int>(*mantissa - '0') <= 9);
                *s++ = *mantissa++;
            }
        }

        // zero-terminate
        assert(s < result + result_size);
        *s = 0;

        return xpath_string::from_heap_preallocated(result, s);
    }

    PUGI__FN bool check_string_to_number_format(const char_t* string)
    {
        // parse leading whitespace
        while (PUGI__IS_CHARTYPE(*string, ct_space)) ++string;

        // parse sign
        if (*string == '-') ++string;

        if (!*string) return false;

        // if there is no integer part, there should be a decimal part with at least one digit
        if (!PUGI__IS_CHARTYPEX(string[0], ctx_digit) && (string[0] != '.' || !PUGI__IS_CHARTYPEX(string[1], ctx_digit))) return false;

        // parse integer part
        while (PUGI__IS_CHARTYPEX(*string, ctx_digit)) ++string;

        // parse decimal part
        if (*string == '.')
        {
            ++string;

            while (PUGI__IS_CHARTYPEX(*string, ctx_digit)) ++string;
        }

        // parse trailing whitespace
        while (PUGI__IS_CHARTYPE(*string, ct_space)) ++string;

        return *string == 0;
    }

    PUGI__FN double convert_string_to_number(const char_t* string)
    {
        // check string format
        if (!check_string_to_number_format(string)) return gen_nan();

        // parse string
    #ifdef PUGIXML_WCHAR_MODE
        return wcstod(string, 0);
    #else
        return strtod(string, 0);
    #endif
    }

    PUGI__FN bool convert_string_to_number_scratch(char_t (&buffer)[32], const char_t* begin, const char_t* end, double* out_result)
    {
        size_t length = static_cast<size_t>(end - begin);
        char_t* scratch = buffer;

        if (length >= sizeof(buffer) / sizeof(buffer[0]))
        {
            // need to make dummy on-heap copy
            scratch = static_cast<char_t*>(xml_memory::allocate((length + 1) * sizeof(char_t)));
            if (!scratch) return false;
        }

        // copy string to zero-terminated buffer and perform conversion
        memcpy(scratch, begin, length * sizeof(char_t));
        scratch[length] = 0;

        *out_result = convert_string_to_number(scratch);

        // free dummy buffer
        if (scratch != buffer) xml_memory::deallocate(scratch);

        return true;
    }

    PUGI__FN double round_nearest(double value)
    {
        return floor(value + 0.5);
    }

    PUGI__FN double round_nearest_nzero(double value)
    {
        // same as round_nearest, but returns -0 for [-0.5, -0]
        // ceil is used to differentiate between +0 and -0 (we return -0 for [-0.5, -0] and +0 for +0)
        return (value >= -0.5 && value <= 0) ? ceil(value) : floor(value + 0.5);
    }

    PUGI__FN const char_t* qualified_name(const xpath_node& node)
    {
        return node.attribute() ? node.attribute().name() : node.node().name();
    }

    PUGI__FN const char_t* local_name(const xpath_node& node)
    {
        const char_t* name = qualified_name(node);
        const char_t* p = find_char(name, ':');

        return p ? p + 1 : name;
    }

    struct namespace_uri_predicate
    {
        const char_t* prefix;
        size_t prefix_length;

        namespace_uri_predicate(const char_t* name)
        {
            const char_t* pos = find_char(name, ':');

            prefix = pos ? name : 0;
            prefix_length = pos ? static_cast<size_t>(pos - name) : 0;
        }

        bool operator()(xml_attribute a) const
        {
            const char_t* name = a.name();

            if (!starts_with(name, PUGIXML_TEXT("xmlns"))) return false;

            return prefix ? name[5] == ':' && strequalrange(name + 6, prefix, prefix_length) : name[5] == 0;
        }
    };

    PUGI__FN const char_t* namespace_uri(xml_node node)
    {
        namespace_uri_predicate pred = node.name();

        xml_node p = node;

        while (p)
        {
            xml_attribute a = p.find_attribute(pred);

            if (a) return a.value();

            p = p.parent();
        }

        return PUGIXML_TEXT("");
    }

    PUGI__FN const char_t* namespace_uri(xml_attribute attr, xml_node parent)
    {
        namespace_uri_predicate pred = attr.name();

        // Default namespace does not apply to attributes
        if (!pred.prefix) return PUGIXML_TEXT("");

        xml_node p = parent;

        while (p)
        {
            xml_attribute a = p.find_attribute(pred);

            if (a) return a.value();

            p = p.parent();
        }

        return PUGIXML_TEXT("");
    }

    PUGI__FN const char_t* namespace_uri(const xpath_node& node)
    {
        return node.attribute() ? namespace_uri(node.attribute(), node.parent()) : namespace_uri(node.node());
    }

    PUGI__FN char_t* normalize_space(char_t* buffer)
    {
        char_t* write = buffer;

        for (char_t* it = buffer; *it; )
        {
            char_t ch = *it++;

            if (PUGI__IS_CHARTYPE(ch, ct_space))
            {
                // replace whitespace sequence with single space
                while (PUGI__IS_CHARTYPE(*it, ct_space)) it++;

                // avoid leading spaces
                if (write != buffer) *write++ = ' ';
            }
            else *write++ = ch;
        }

        // remove trailing space
        if (write != buffer && PUGI__IS_CHARTYPE(write[-1], ct_space)) write--;

        // zero-terminate
        *write = 0;

        return write;
    }

    PUGI__FN char_t* translate(char_t* buffer, const char_t* from, const char_t* to, size_t to_length)
    {
        char_t* write = buffer;

        while (*buffer)
        {
            PUGI__DMC_VOLATILE char_t ch = *buffer++;

            const char_t* pos = find_char(from, ch);

            if (!pos)
                *write++ = ch; // do not process
            else if (static_cast<size_t>(pos - from) < to_length)
                *write++ = to[pos - from]; // replace
        }

        // zero-terminate
        *write = 0;

        return write;
    }

    PUGI__FN unsigned char* translate_table_generate(xpath_allocator* alloc, const char_t* from, const char_t* to)
    {
        unsigned char table[128] = {0};

        while (*from)
        {
            unsigned int fc = static_cast<unsigned int>(*from);
            unsigned int tc = static_cast<unsigned int>(*to);

            if (fc >= 128 || tc >= 128)
                return 0;

            // code=128 means "skip character"
            if (!table[fc])
                table[fc] = static_cast<unsigned char>(tc ? tc : 128);

            from++;
            if (tc) to++;
        }

        for (int i = 0; i < 128; ++i)
            if (!table[i])
                table[i] = static_cast<unsigned char>(i);

        void* result = alloc->allocate(sizeof(table));
        if (!result) return 0;

        memcpy(result, table, sizeof(table));

        return static_cast<unsigned char*>(result);
    }

    PUGI__FN char_t* translate_table(char_t* buffer, const unsigned char* table)
    {
        char_t* write = buffer;

        while (*buffer)
        {
            char_t ch = *buffer++;
            unsigned int index = static_cast<unsigned int>(ch);

            if (index < 128)
            {
                unsigned char code = table[index];

                // code=128 means "skip character" (table size is 128 so 128 can be a special value)
                // this code skips these characters without extra branches
                *write = static_cast<char_t>(code);
                write += 1 - (code >> 7);
            }
            else
            {
                *write++ = ch;
            }
        }

        // zero-terminate
        *write = 0;

        return write;
    }

    inline bool is_xpath_attribute(const char_t* name)
    {
        return !(starts_with(name, PUGIXML_TEXT("xmlns")) && (name[5] == 0 || name[5] == ':'));
    }

    struct xpath_variable_boolean: xpath_variable
    {
        xpath_variable_boolean(): xpath_variable(xpath_type_boolean), value(false)
        {
        }

        bool value;
        char_t name[1];
    };

    struct xpath_variable_number: xpath_variable
    {
        xpath_variable_number(): xpath_variable(xpath_type_number), value(0)
        {
        }

        double value;
        char_t name[1];
    };

    struct xpath_variable_string: xpath_variable
    {
        xpath_variable_string(): xpath_variable(xpath_type_string), value(0)
        {
        }

        ~xpath_variable_string()
        {
            if (value) xml_memory::deallocate(value);
        }

        char_t* value;
        char_t name[1];
    };

    struct xpath_variable_node_set: xpath_variable
    {
        xpath_variable_node_set(): xpath_variable(xpath_type_node_set)
        {
        }

        xpath_node_set value;
        char_t name[1];
    };

    static const xpath_node_set dummy_node_set;

    PUGI__FN PUGI__UNSIGNED_OVERFLOW unsigned int hash_string(const char_t* str)
    {
        // Jenkins one-at-a-time hash (http://en.wikipedia.org/wiki/Jenkins_hash_function#one-at-a-time)
        unsigned int result = 0;

        while (*str)
        {
            result += static_cast<unsigned int>(*str++);
            result += result << 10;
            result ^= result >> 6;
        }

        result += result << 3;
        result ^= result >> 11;
        result += result << 15;

        return result;
    }

    template <typename T> PUGI__FN T* new_xpath_variable(const char_t* name)
    {
        size_t length = strlength(name);
        if (length == 0) return 0; // empty variable names are invalid

        // $$ we can't use offsetof(T, name) because T is non-POD, so we just allocate additional length characters
        void* memory = xml_memory::allocate(sizeof(T) + length * sizeof(char_t));
        if (!memory) return 0;

        T* result = new (memory) T();

        memcpy(result->name, name, (length + 1) * sizeof(char_t));

        return result;
    }

    PUGI__FN xpath_variable* new_xpath_variable(xpath_value_type type, const char_t* name)
    {
        switch (type)
        {
        case xpath_type_node_set:
            return new_xpath_variable<xpath_variable_node_set>(name);

        case xpath_type_number:
            return new_xpath_variable<xpath_variable_number>(name);

        case xpath_type_string:
            return new_xpath_variable<xpath_variable_string>(name);

        case xpath_type_boolean:
            return new_xpath_variable<xpath_variable_boolean>(name);

        default:
            return 0;
        }
    }

    template <typename T> PUGI__FN void delete_xpath_variable(T* var)
    {
        var->~T();
        xml_memory::deallocate(var);
    }

    PUGI__FN void delete_xpath_variable(xpath_value_type type, xpath_variable* var)
    {
        switch (type)
        {
        case xpath_type_node_set:
            delete_xpath_variable(static_cast<xpath_variable_node_set*>(var));
            break;

        case xpath_type_number:
            delete_xpath_variable(static_cast<xpath_variable_number*>(var));
            break;

        case xpath_type_string:
            delete_xpath_variable(static_cast<xpath_variable_string*>(var));
            break;

        case xpath_type_boolean:
            delete_xpath_variable(static_cast<xpath_variable_boolean*>(var));
            break;

        default:
            assert(false && "Invalid variable type"); // unreachable
        }
    }

    PUGI__FN bool copy_xpath_variable(xpath_variable* lhs, const xpath_variable* rhs)
    {
        switch (rhs->type())
        {
        case xpath_type_node_set:
            return lhs->set(static_cast<const xpath_variable_node_set*>(rhs)->value);

        case xpath_type_number:
            return lhs->set(static_cast<const xpath_variable_number*>(rhs)->value);

        case xpath_type_string:
            return lhs->set(static_cast<const xpath_variable_string*>(rhs)->value);

        case xpath_type_boolean:
            return lhs->set(static_cast<const xpath_variable_boolean*>(rhs)->value);

        default:
            assert(false && "Invalid variable type"); // unreachable
            return false;
        }
    }

    PUGI__FN bool get_variable_scratch(char_t (&buffer)[32], xpath_variable_set* set, const char_t* begin, const char_t* end, xpath_variable** out_result)
    {
        size_t length = static_cast<size_t>(end - begin);
        char_t* scratch = buffer;

        if (length >= sizeof(buffer) / sizeof(buffer[0]))
        {
            // need to make dummy on-heap copy
            scratch = static_cast<char_t*>(xml_memory::allocate((length + 1) * sizeof(char_t)));
            if (!scratch) return false;
        }

        // copy string to zero-terminated buffer and perform lookup
        memcpy(scratch, begin, length * sizeof(char_t));
        scratch[length] = 0;

        *out_result = set->get(scratch);

        // free dummy buffer
        if (scratch != buffer) xml_memory::deallocate(scratch);

        return true;
    }
PUGI__NS_END

// Internal node set class
PUGI__NS_BEGIN
    PUGI__FN xpath_node_set::type_t xpath_get_order(const xpath_node* begin, const xpath_node* end)
    {
        if (end - begin < 2)
            return xpath_node_set::type_sorted;

        document_order_comparator cmp;

        bool first = cmp(begin[0], begin[1]);

        for (const xpath_node* it = begin + 1; it + 1 < end; ++it)
            if (cmp(it[0], it[1]) != first)
                return xpath_node_set::type_unsorted;

        return first ? xpath_node_set::type_sorted : xpath_node_set::type_sorted_reverse;
    }

    PUGI__FN xpath_node_set::type_t xpath_sort(xpath_node* begin, xpath_node* end, xpath_node_set::type_t type, bool rev)
    {
        xpath_node_set::type_t order = rev ? xpath_node_set::type_sorted_reverse : xpath_node_set::type_sorted;

        if (type == xpath_node_set::type_unsorted)
        {
            xpath_node_set::type_t sorted = xpath_get_order(begin, end);

            if (sorted == xpath_node_set::type_unsorted)
            {
                sort(begin, end, document_order_comparator());

                type = xpath_node_set::type_sorted;
            }
            else
                type = sorted;
        }

        if (type != order) reverse(begin, end);

        return order;
    }

    PUGI__FN xpath_node xpath_first(const xpath_node* begin, const xpath_node* end, xpath_node_set::type_t type)
    {
        if (begin == end) return xpath_node();

        switch (type)
        {
        case xpath_node_set::type_sorted:
            return *begin;

        case xpath_node_set::type_sorted_reverse:
            return *(end - 1);

        case xpath_node_set::type_unsorted:
            return *min_element(begin, end, document_order_comparator());

        default:
            assert(false && "Invalid node set type"); // unreachable
            return xpath_node();
        }
    }

    class xpath_node_set_raw
    {
        xpath_node_set::type_t _type;

        xpath_node* _begin;
        xpath_node* _end;
        xpath_node* _eos;

    public:
        xpath_node_set_raw(): _type(xpath_node_set::type_unsorted), _begin(0), _end(0), _eos(0)
        {
        }

        xpath_node* begin() const
        {
            return _begin;
        }

        xpath_node* end() const
        {
            return _end;
        }

        bool empty() const
        {
            return _begin == _end;
        }

        size_t size() const
        {
            return static_cast<size_t>(_end - _begin);
        }

        xpath_node first() const
        {
            return xpath_first(_begin, _end, _type);
        }

        void push_back_grow(const xpath_node& node, xpath_allocator* alloc);

        void push_back(const xpath_node& node, xpath_allocator* alloc)
        {
            if (_end != _eos)
                *_end++ = node;
            else
                push_back_grow(node, alloc);
        }

        void append(const xpath_node* begin_, const xpath_node* end_, xpath_allocator* alloc)
        {
            if (begin_ == end_) return;

            size_t size_ = static_cast<size_t>(_end - _begin);
            size_t capacity = static_cast<size_t>(_eos - _begin);
            size_t count = static_cast<size_t>(end_ - begin_);

            if (size_ + count > capacity)
            {
                // reallocate the old array or allocate a new one
                xpath_node* data = static_cast<xpath_node*>(alloc->reallocate(_begin, capacity * sizeof(xpath_node), (size_ + count) * sizeof(xpath_node)));
                if (!data) return;

                // finalize
                _begin = data;
                _end = data + size_;
                _eos = data + size_ + count;
            }

            memcpy(_end, begin_, count * sizeof(xpath_node));
            _end += count;
        }

        void sort_do()
        {
            _type = xpath_sort(_begin, _end, _type, false);
        }

        void truncate(xpath_node* pos)
        {
            assert(_begin <= pos && pos <= _end);

            _end = pos;
        }

        void remove_duplicates(xpath_allocator* alloc)
        {
            if (_type == xpath_node_set::type_unsorted && _end - _begin > 2)
            {
                xpath_allocator_capture cr(alloc);

                size_t size_ = static_cast<size_t>(_end - _begin);

                size_t hash_size = 1;
                while (hash_size < size_ + size_ / 2) hash_size *= 2;

                const void** hash_data = static_cast<const void**>(alloc->allocate(hash_size * sizeof(void**)));
                if (!hash_data) return;

                memset(hash_data, 0, hash_size * sizeof(const void**));

                xpath_node* write = _begin;

                for (xpath_node* it = _begin; it != _end; ++it)
                {
                    const void* attr = it->attribute().internal_object();
                    const void* node = it->node().internal_object();
                    const void* key = attr ? attr : node;

                    if (key && hash_insert(hash_data, hash_size, key))
                    {
                        *write++ = *it;
                    }
                }

                _end = write;
            }
            else
            {
                _end = unique(_begin, _end);
            }
        }

        xpath_node_set::type_t type() const
        {
            return _type;
        }

        void set_type(xpath_node_set::type_t value)
        {
            _type = value;
        }
    };

    PUGI__FN_NO_INLINE void xpath_node_set_raw::push_back_grow(const xpath_node& node, xpath_allocator* alloc)
    {
        size_t capacity = static_cast<size_t>(_eos - _begin);

        // get new capacity (1.5x rule)
        size_t new_capacity = capacity + capacity / 2 + 1;

        // reallocate the old array or allocate a new one
        xpath_node* data = static_cast<xpath_node*>(alloc->reallocate(_begin, capacity * sizeof(xpath_node), new_capacity * sizeof(xpath_node)));
        if (!data) return;

        // finalize
        _begin = data;
        _end = data + capacity;
        _eos = data + new_capacity;

        // push
        *_end++ = node;
    }
PUGI__NS_END

PUGI__NS_BEGIN
    struct xpath_context
    {
        xpath_node n;
        size_t position, size;

        xpath_context(const xpath_node& n_, size_t position_, size_t size_): n(n_), position(position_), size(size_)
        {
        }
    };

    enum lexeme_t
    {
        lex_none = 0,
        lex_equal,
        lex_not_equal,
        lex_less,
        lex_greater,
        lex_less_or_equal,
        lex_greater_or_equal,
        lex_plus,
        lex_minus,
        lex_multiply,
        lex_union,
        lex_var_ref,
        lex_open_brace,
        lex_close_brace,
        lex_quoted_string,
        lex_number,
        lex_slash,
        lex_double_slash,
        lex_open_square_brace,
        lex_close_square_brace,
        lex_string,
        lex_comma,
        lex_axis_attribute,
        lex_dot,
        lex_double_dot,
        lex_double_colon,
        lex_eof
    };

    struct xpath_lexer_string
    {
        const char_t* begin;
        const char_t* end;

        xpath_lexer_string(): begin(0), end(0)
        {
        }

        bool operator==(const char_t* other) const
        {
            size_t length = static_cast<size_t>(end - begin);

            return strequalrange(other, begin, length);
        }
    };

    class xpath_lexer
    {
        const char_t* _cur;
        const char_t* _cur_lexeme_pos;
        xpath_lexer_string _cur_lexeme_contents;

        lexeme_t _cur_lexeme;

    public:
        explicit xpath_lexer(const char_t* query): _cur(query)
        {
            next();
        }

        const char_t* state() const
        {
            return _cur;
        }

        void next()
        {
            const char_t* cur = _cur;

            while (PUGI__IS_CHARTYPE(*cur, ct_space)) ++cur;

            // save lexeme position for error reporting
            _cur_lexeme_pos = cur;

            switch (*cur)
            {
            case 0:
                _cur_lexeme = lex_eof;
                break;

            case '>':
                if (*(cur+1) == '=')
                {
                    cur += 2;
                    _cur_lexeme = lex_greater_or_equal;
                }
                else
                {
                    cur += 1;
                    _cur_lexeme = lex_greater;
                }
                break;

            case '<':
                if (*(cur+1) == '=')
                {
                    cur += 2;
                    _cur_lexeme = lex_less_or_equal;
                }
                else
                {
                    cur += 1;
                    _cur_lexeme = lex_less;
                }
                break;

            case '!':
                if (*(cur+1) == '=')
                {
                    cur += 2;
                    _cur_lexeme = lex_not_equal;
                }
                else
                {
                    _cur_lexeme = lex_none;
                }
                break;

            case '=':
                cur += 1;
                _cur_lexeme = lex_equal;

                break;

            case '+':
                cur += 1;
                _cur_lexeme = lex_plus;

                break;

            case '-':
                cur += 1;
                _cur_lexeme = lex_minus;

                break;

            case '*':
                cur += 1;
                _cur_lexeme = lex_multiply;

                break;

            case '|':
                cur += 1;
                _cur_lexeme = lex_union;

                break;

            case '$':
                cur += 1;

                if (PUGI__IS_CHARTYPEX(*cur, ctx_start_symbol))
                {
                    _cur_lexeme_contents.begin = cur;

                    while (PUGI__IS_CHARTYPEX(*cur, ctx_symbol)) cur++;

                    if (cur[0] == ':' && PUGI__IS_CHARTYPEX(cur[1], ctx_symbol)) // qname
                    {
                        cur++; // :

                        while (PUGI__IS_CHARTYPEX(*cur, ctx_symbol)) cur++;
                    }

                    _cur_lexeme_contents.end = cur;

                    _cur_lexeme = lex_var_ref;
                }
                else
                {
                    _cur_lexeme = lex_none;
                }

                break;

            case '(':
                cur += 1;
                _cur_lexeme = lex_open_brace;

                break;

            case ')':
                cur += 1;
                _cur_lexeme = lex_close_brace;

                break;

            case '[':
                cur += 1;
                _cur_lexeme = lex_open_square_brace;

                break;

            case ']':
                cur += 1;
                _cur_lexeme = lex_close_square_brace;

                break;

            case ',':
                cur += 1;
                _cur_lexeme = lex_comma;

                break;

            case '/':
                if (*(cur+1) == '/')
                {
                    cur += 2;
                    _cur_lexeme = lex_double_slash;
                }
                else
                {
                    cur += 1;
                    _cur_lexeme = lex_slash;
                }
                break;

            case '.':
                if (*(cur+1) == '.')
                {
                    cur += 2;
                    _cur_lexeme = lex_double_dot;
                }
                else if (PUGI__IS_CHARTYPEX(*(cur+1), ctx_digit))
                {
                    _cur_lexeme_contents.begin = cur; // .

                    ++cur;

                    while (PUGI__IS_CHARTYPEX(*cur, ctx_digit)) cur++;

                    _cur_lexeme_contents.end = cur;

                    _cur_lexeme = lex_number;
                }
                else
                {
                    cur += 1;
                    _cur_lexeme = lex_dot;
                }
                break;

            case '@':
                cur += 1;
                _cur_lexeme = lex_axis_attribute;

                break;

            case '"':
            case '\'':
            {
                char_t terminator = *cur;

                ++cur;

                _cur_lexeme_contents.begin = cur;
                while (*cur && *cur != terminator) cur++;
                _cur_lexeme_contents.end = cur;

                if (!*cur)
                    _cur_lexeme = lex_none;
                else
                {
                    cur += 1;
                    _cur_lexeme = lex_quoted_string;
                }

                break;
            }

            case ':':
                if (*(cur+1) == ':')
                {
                    cur += 2;
                    _cur_lexeme = lex_double_colon;
                }
                else
                {
                    _cur_lexeme = lex_none;
                }
                break;

            default:
                if (PUGI__IS_CHARTYPEX(*cur, ctx_digit))
                {
                    _cur_lexeme_contents.begin = cur;

                    while (PUGI__IS_CHARTYPEX(*cur, ctx_digit)) cur++;

                    if (*cur == '.')
                    {
                        cur++;

                        while (PUGI__IS_CHARTYPEX(*cur, ctx_digit)) cur++;
                    }

                    _cur_lexeme_contents.end = cur;

                    _cur_lexeme = lex_number;
                }
                else if (PUGI__IS_CHARTYPEX(*cur, ctx_start_symbol))
                {
                    _cur_lexeme_contents.begin = cur;

                    while (PUGI__IS_CHARTYPEX(*cur, ctx_symbol)) cur++;

                    if (cur[0] == ':')
                    {
                        if (cur[1] == '*') // namespace test ncname:*
                        {
                            cur += 2; // :*
                        }
                        else if (PUGI__IS_CHARTYPEX(cur[1], ctx_symbol)) // namespace test qname
                        {
                            cur++; // :

                            while (PUGI__IS_CHARTYPEX(*cur, ctx_symbol)) cur++;
                        }
                    }

                    _cur_lexeme_contents.end = cur;

                    _cur_lexeme = lex_string;
                }
                else
                {
                    _cur_lexeme = lex_none;
                }
            }

            _cur = cur;
        }

        lexeme_t current() const
        {
            return _cur_lexeme;
        }

        const char_t* current_pos() const
        {
            return _cur_lexeme_pos;
        }

        const xpath_lexer_string& contents() const
        {
            assert(_cur_lexeme == lex_var_ref || _cur_lexeme == lex_number || _cur_lexeme == lex_string || _cur_lexeme == lex_quoted_string);

            return _cur_lexeme_contents;
        }
    };

    enum ast_type_t
    {
        ast_unknown,
        ast_op_or,                      // left or right
        ast_op_and,                     // left and right
        ast_op_equal,                   // left = right
        ast_op_not_equal,               // left != right
        ast_op_less,                    // left < right
        ast_op_greater,                 // left > right
        ast_op_less_or_equal,           // left <= right
        ast_op_greater_or_equal,        // left >= right
        ast_op_add,                     // left + right
        ast_op_subtract,                // left - right
        ast_op_multiply,                // left * right
        ast_op_divide,                  // left / right
        ast_op_mod,                     // left % right
        ast_op_negate,                  // left - right
        ast_op_union,                   // left | right
        ast_predicate,                  // apply predicate to set; next points to next predicate
        ast_filter,                     // select * from left where right
        ast_string_constant,            // string constant
        ast_number_constant,            // number constant
        ast_variable,                   // variable
        ast_func_last,                  // last()
        ast_func_position,              // position()
        ast_func_count,                 // count(left)
        ast_func_id,                    // id(left)
        ast_func_local_name_0,          // local-name()
        ast_func_local_name_1,          // local-name(left)
        ast_func_namespace_uri_0,       // namespace-uri()
        ast_func_namespace_uri_1,       // namespace-uri(left)
        ast_func_name_0,                // name()
        ast_func_name_1,                // name(left)
        ast_func_string_0,              // string()
        ast_func_string_1,              // string(left)
        ast_func_concat,                // concat(left, right, siblings)
        ast_func_starts_with,           // starts_with(left, right)
        ast_func_contains,              // contains(left, right)
        ast_func_substring_before,      // substring-before(left, right)
        ast_func_substring_after,       // substring-after(left, right)
        ast_func_substring_2,           // substring(left, right)
        ast_func_substring_3,           // substring(left, right, third)
        ast_func_string_length_0,       // string-length()
        ast_func_string_length_1,       // string-length(left)
        ast_func_normalize_space_0,     // normalize-space()
        ast_func_normalize_space_1,     // normalize-space(left)
        ast_func_translate,             // translate(left, right, third)
        ast_func_boolean,               // boolean(left)
        ast_func_not,                   // not(left)
        ast_func_true,                  // true()
        ast_func_false,                 // false()
        ast_func_lang,                  // lang(left)
        ast_func_number_0,              // number()
        ast_func_number_1,              // number(left)
        ast_func_sum,                   // sum(left)
        ast_func_floor,                 // floor(left)
        ast_func_ceiling,               // ceiling(left)
        ast_func_round,                 // round(left)
        ast_step,                       // process set left with step
        ast_step_root,                  // select root node

        ast_opt_translate_table,        // translate(left, right, third) where right/third are constants
        ast_opt_compare_attribute       // @name = 'string'
    };

    enum axis_t
    {
        axis_ancestor,
        axis_ancestor_or_self,
        axis_attribute,
        axis_child,
        axis_descendant,
        axis_descendant_or_self,
        axis_following,
        axis_following_sibling,
        axis_namespace,
        axis_parent,
        axis_preceding,
        axis_preceding_sibling,
        axis_self
    };

    enum nodetest_t
    {
        nodetest_none,
        nodetest_name,
        nodetest_type_node,
        nodetest_type_comment,
        nodetest_type_pi,
        nodetest_type_text,
        nodetest_pi,
        nodetest_all,
        nodetest_all_in_namespace
    };

    enum predicate_t
    {
        predicate_default,
        predicate_posinv,
        predicate_constant,
        predicate_constant_one
    };

    enum nodeset_eval_t
    {
        nodeset_eval_all,
        nodeset_eval_any,
        nodeset_eval_first
    };

    template <axis_t N> struct axis_to_type
    {
        static const axis_t axis;
    };

    template <axis_t N> const axis_t axis_to_type<N>::axis = N;

    class xpath_ast_node
    {
    private:
        // node type
        char _type;
        char _rettype;

        // for ast_step
        char _axis;

        // for ast_step/ast_predicate/ast_filter
        char _test;

        // tree node structure
        xpath_ast_node* _left;
        xpath_ast_node* _right;
        xpath_ast_node* _next;

        union
        {
            // value for ast_string_constant
            const char_t* string;
            // value for ast_number_constant
            double number;
            // variable for ast_variable
            xpath_variable* variable;
            // node test for ast_step (node name/namespace/node type/pi target)
            const char_t* nodetest;
            // table for ast_opt_translate_table
            const unsigned char* table;
        } _data;

        xpath_ast_node(const xpath_ast_node&);
        xpath_ast_node& operator=(const xpath_ast_node&);

        template <class Comp> static bool compare_eq(xpath_ast_node* lhs, xpath_ast_node* rhs, const xpath_context& c, const xpath_stack& stack, const Comp& comp)
        {
            xpath_value_type lt = lhs->rettype(), rt = rhs->rettype();

            if (lt != xpath_type_node_set && rt != xpath_type_node_set)
            {
                if (lt == xpath_type_boolean || rt == xpath_type_boolean)
                    return comp(lhs->eval_boolean(c, stack), rhs->eval_boolean(c, stack));
                else if (lt == xpath_type_number || rt == xpath_type_number)
                    return comp(lhs->eval_number(c, stack), rhs->eval_number(c, stack));
                else if (lt == xpath_type_string || rt == xpath_type_string)
                {
                    xpath_allocator_capture cr(stack.result);

                    xpath_string ls = lhs->eval_string(c, stack);
                    xpath_string rs = rhs->eval_string(c, stack);

                    return comp(ls, rs);
                }
            }
            else if (lt == xpath_type_node_set && rt == xpath_type_node_set)
            {
                xpath_allocator_capture cr(stack.result);

                xpath_node_set_raw ls = lhs->eval_node_set(c, stack, nodeset_eval_all);
                xpath_node_set_raw rs = rhs->eval_node_set(c, stack, nodeset_eval_all);

                for (const xpath_node* li = ls.begin(); li != ls.end(); ++li)
                    for (const xpath_node* ri = rs.begin(); ri != rs.end(); ++ri)
                    {
                        xpath_allocator_capture cri(stack.result);

                        if (comp(string_value(*li, stack.result), string_value(*ri, stack.result)))
                            return true;
                    }

                return false;
            }
            else
            {
                if (lt == xpath_type_node_set)
                {
                    swap(lhs, rhs);
                    swap(lt, rt);
                }

                if (lt == xpath_type_boolean)
                    return comp(lhs->eval_boolean(c, stack), rhs->eval_boolean(c, stack));
                else if (lt == xpath_type_number)
                {
                    xpath_allocator_capture cr(stack.result);

                    double l = lhs->eval_number(c, stack);
                    xpath_node_set_raw rs = rhs->eval_node_set(c, stack, nodeset_eval_all);

                    for (const xpath_node* ri = rs.begin(); ri != rs.end(); ++ri)
                    {
                        xpath_allocator_capture cri(stack.result);

                        if (comp(l, convert_string_to_number(string_value(*ri, stack.result).c_str())))
                            return true;
                    }

                    return false;
                }
                else if (lt == xpath_type_string)
                {
                    xpath_allocator_capture cr(stack.result);

                    xpath_string l = lhs->eval_string(c, stack);
                    xpath_node_set_raw rs = rhs->eval_node_set(c, stack, nodeset_eval_all);

                    for (const xpath_node* ri = rs.begin(); ri != rs.end(); ++ri)
                    {
                        xpath_allocator_capture cri(stack.result);

                        if (comp(l, string_value(*ri, stack.result)))
                            return true;
                    }

                    return false;
                }
            }

            assert(false && "Wrong types"); // unreachable
            return false;
        }

        static bool eval_once(xpath_node_set::type_t type, nodeset_eval_t eval)
        {
            return type == xpath_node_set::type_sorted ? eval != nodeset_eval_all : eval == nodeset_eval_any;
        }

        template <class Comp> static bool compare_rel(xpath_ast_node* lhs, xpath_ast_node* rhs, const xpath_context& c, const xpath_stack& stack, const Comp& comp)
        {
            xpath_value_type lt = lhs->rettype(), rt = rhs->rettype();

            if (lt != xpath_type_node_set && rt != xpath_type_node_set)
                return comp(lhs->eval_number(c, stack), rhs->eval_number(c, stack));
            else if (lt == xpath_type_node_set && rt == xpath_type_node_set)
            {
                xpath_allocator_capture cr(stack.result);

                xpath_node_set_raw ls = lhs->eval_node_set(c, stack, nodeset_eval_all);
                xpath_node_set_raw rs = rhs->eval_node_set(c, stack, nodeset_eval_all);

                for (const xpath_node* li = ls.begin(); li != ls.end(); ++li)
                {
                    xpath_allocator_capture cri(stack.result);

                    double l = convert_string_to_number(string_value(*li, stack.result).c_str());

                    for (const xpath_node* ri = rs.begin(); ri != rs.end(); ++ri)
                    {
                        xpath_allocator_capture crii(stack.result);

                        if (comp(l, convert_string_to_number(string_value(*ri, stack.result).c_str())))
                            return true;
                    }
                }

                return false;
            }
            else if (lt != xpath_type_node_set && rt == xpath_type_node_set)
            {
                xpath_allocator_capture cr(stack.result);

                double l = lhs->eval_number(c, stack);
                xpath_node_set_raw rs = rhs->eval_node_set(c, stack, nodeset_eval_all);

                for (const xpath_node* ri = rs.begin(); ri != rs.end(); ++ri)
                {
                    xpath_allocator_capture cri(stack.result);

                    if (comp(l, convert_string_to_number(string_value(*ri, stack.result).c_str())))
                        return true;
                }

                return false;
            }
            else if (lt == xpath_type_node_set && rt != xpath_type_node_set)
            {
                xpath_allocator_capture cr(stack.result);

                xpath_node_set_raw ls = lhs->eval_node_set(c, stack, nodeset_eval_all);
                double r = rhs->eval_number(c, stack);

                for (const xpath_node* li = ls.begin(); li != ls.end(); ++li)
                {
                    xpath_allocator_capture cri(stack.result);

                    if (comp(convert_string_to_number(string_value(*li, stack.result).c_str()), r))
                        return true;
                }

                return false;
            }
            else
            {
                assert(false && "Wrong types"); // unreachable
                return false;
            }
        }

        static void apply_predicate_boolean(xpath_node_set_raw& ns, size_t first, xpath_ast_node* expr, const xpath_stack& stack, bool once)
        {
            assert(ns.size() >= first);
            assert(expr->rettype() != xpath_type_number);

            size_t i = 1;
            size_t size = ns.size() - first;

            xpath_node* last = ns.begin() + first;

            // remove_if... or well, sort of
            for (xpath_node* it = last; it != ns.end(); ++it, ++i)
            {
                xpath_context c(*it, i, size);

                if (expr->eval_boolean(c, stack))
                {
                    *last++ = *it;

                    if (once) break;
                }
            }

            ns.truncate(last);
        }

        static void apply_predicate_number(xpath_node_set_raw& ns, size_t first, xpath_ast_node* expr, const xpath_stack& stack, bool once)
        {
            assert(ns.size() >= first);
            assert(expr->rettype() == xpath_type_number);

            size_t i = 1;
            size_t size = ns.size() - first;

            xpath_node* last = ns.begin() + first;

            // remove_if... or well, sort of
            for (xpath_node* it = last; it != ns.end(); ++it, ++i)
            {
                xpath_context c(*it, i, size);

                if (expr->eval_number(c, stack) == static_cast<double>(i))
                {
                    *last++ = *it;

                    if (once) break;
                }
            }

            ns.truncate(last);
        }

        static void apply_predicate_number_const(xpath_node_set_raw& ns, size_t first, xpath_ast_node* expr, const xpath_stack& stack)
        {
            assert(ns.size() >= first);
            assert(expr->rettype() == xpath_type_number);

            size_t size = ns.size() - first;

            xpath_node* last = ns.begin() + first;

            xpath_context c(xpath_node(), 1, size);

            double er = expr->eval_number(c, stack);

            if (er >= 1.0 && er <= static_cast<double>(size))
            {
                size_t eri = static_cast<size_t>(er);

                if (er == static_cast<double>(eri))
                {
                    xpath_node r = last[eri - 1];

                    *last++ = r;
                }
            }

            ns.truncate(last);
        }

        void apply_predicate(xpath_node_set_raw& ns, size_t first, const xpath_stack& stack, bool once)
        {
            if (ns.size() == first) return;

            assert(_type == ast_filter || _type == ast_predicate);

            if (_test == predicate_constant || _test == predicate_constant_one)
                apply_predicate_number_const(ns, first, _right, stack);
            else if (_right->rettype() == xpath_type_number)
                apply_predicate_number(ns, first, _right, stack, once);
            else
                apply_predicate_boolean(ns, first, _right, stack, once);
        }

        void apply_predicates(xpath_node_set_raw& ns, size_t first, const xpath_stack& stack, nodeset_eval_t eval)
        {
            if (ns.size() == first) return;

            bool last_once = eval_once(ns.type(), eval);

            for (xpath_ast_node* pred = _right; pred; pred = pred->_next)
                pred->apply_predicate(ns, first, stack, !pred->_next && last_once);
        }

        bool step_push(xpath_node_set_raw& ns, xml_attribute_struct* a, xml_node_struct* parent, xpath_allocator* alloc)
        {
            assert(a);

            const char_t* name = a->name ? a->name + 0 : PUGIXML_TEXT("");

            switch (_test)
            {
            case nodetest_name:
                if (strequal(name, _data.nodetest) && is_xpath_attribute(name))
                {
                    ns.push_back(xpath_node(xml_attribute(a), xml_node(parent)), alloc);
                    return true;
                }
                break;

            case nodetest_type_node:
            case nodetest_all:
                if (is_xpath_attribute(name))
                {
                    ns.push_back(xpath_node(xml_attribute(a), xml_node(parent)), alloc);
                    return true;
                }
                break;

            case nodetest_all_in_namespace:
                if (starts_with(name, _data.nodetest) && is_xpath_attribute(name))
                {
                    ns.push_back(xpath_node(xml_attribute(a), xml_node(parent)), alloc);
                    return true;
                }
                break;

            default:
                ;
            }

            return false;
        }

        bool step_push(xpath_node_set_raw& ns, xml_node_struct* n, xpath_allocator* alloc)
        {
            assert(n);

            xml_node_type type = PUGI__NODETYPE(n);

            switch (_test)
            {
            case nodetest_name:
                if (type == node_element && n->name && strequal(n->name, _data.nodetest))
                {
                    ns.push_back(xml_node(n), alloc);
                    return true;
                }
                break;

            case nodetest_type_node:
                ns.push_back(xml_node(n), alloc);
                return true;

            case nodetest_type_comment:
                if (type == node_comment)
                {
                    ns.push_back(xml_node(n), alloc);
                    return true;
                }
                break;

            case nodetest_type_text:
                if (type == node_pcdata || type == node_cdata)
                {
                    ns.push_back(xml_node(n), alloc);
                    return true;
                }
                break;

            case nodetest_type_pi:
                if (type == node_pi)
                {
                    ns.push_back(xml_node(n), alloc);
                    return true;
                }
                break;

            case nodetest_pi:
                if (type == node_pi && n->name && strequal(n->name, _data.nodetest))
                {
                    ns.push_back(xml_node(n), alloc);
                    return true;
                }
                break;

            case nodetest_all:
                if (type == node_element)
                {
                    ns.push_back(xml_node(n), alloc);
                    return true;
                }
                break;

            case nodetest_all_in_namespace:
                if (type == node_element && n->name && starts_with(n->name, _data.nodetest))
                {
                    ns.push_back(xml_node(n), alloc);
                    return true;
                }
                break;

            default:
                assert(false && "Unknown axis"); // unreachable
            }

            return false;
        }

        template <class T> void step_fill(xpath_node_set_raw& ns, xml_node_struct* n, xpath_allocator* alloc, bool once, T)
        {
            const axis_t axis = T::axis;

            switch (axis)
            {
            case axis_attribute:
            {
                for (xml_attribute_struct* a = n->first_attribute; a; a = a->next_attribute)
                    if (step_push(ns, a, n, alloc) & once)
                        return;

                break;
            }

            case axis_child:
            {
                for (xml_node_struct* c = n->first_child; c; c = c->next_sibling)
                    if (step_push(ns, c, alloc) & once)
                        return;

                break;
            }

            case axis_descendant:
            case axis_descendant_or_self:
            {
                if (axis == axis_descendant_or_self)
                    if (step_push(ns, n, alloc) & once)
                        return;

                xml_node_struct* cur = n->first_child;

                while (cur)
                {
                    if (step_push(ns, cur, alloc) & once)
                        return;

                    if (cur->first_child)
                        cur = cur->first_child;
                    else
                    {
                        while (!cur->next_sibling)
                        {
                            cur = cur->parent;

                            if (cur == n) return;
                        }

                        cur = cur->next_sibling;
                    }
                }

                break;
            }

            case axis_following_sibling:
            {
                for (xml_node_struct* c = n->next_sibling; c; c = c->next_sibling)
                    if (step_push(ns, c, alloc) & once)
                        return;

                break;
            }

            case axis_preceding_sibling:
            {
                for (xml_node_struct* c = n->prev_sibling_c; c->next_sibling; c = c->prev_sibling_c)
                    if (step_push(ns, c, alloc) & once)
                        return;

                break;
            }

            case axis_following:
            {
                xml_node_struct* cur = n;

                // exit from this node so that we don't include descendants
                while (!cur->next_sibling)
                {
                    cur = cur->parent;

                    if (!cur) return;
                }

                cur = cur->next_sibling;

                while (cur)
                {
                    if (step_push(ns, cur, alloc) & once)
                        return;

                    if (cur->first_child)
                        cur = cur->first_child;
                    else
                    {
                        while (!cur->next_sibling)
                        {
                            cur = cur->parent;

                            if (!cur) return;
                        }

                        cur = cur->next_sibling;
                    }
                }

                break;
            }

            case axis_preceding:
            {
                xml_node_struct* cur = n;

                // exit from this node so that we don't include descendants
                while (!cur->prev_sibling_c->next_sibling)
                {
                    cur = cur->parent;

                    if (!cur) return;
                }

                cur = cur->prev_sibling_c;

                while (cur)
                {
                    if (cur->first_child)
                        cur = cur->first_child->prev_sibling_c;
                    else
                    {
                        // leaf node, can't be ancestor
                        if (step_push(ns, cur, alloc) & once)
                            return;

                        while (!cur->prev_sibling_c->next_sibling)
                        {
                            cur = cur->parent;

                            if (!cur) return;

                            if (!node_is_ancestor(cur, n))
                                if (step_push(ns, cur, alloc) & once)
                                    return;
                        }

                        cur = cur->prev_sibling_c;
                    }
                }

                break;
            }

            case axis_ancestor:
            case axis_ancestor_or_self:
            {
                if (axis == axis_ancestor_or_self)
                    if (step_push(ns, n, alloc) & once)
                        return;

                xml_node_struct* cur = n->parent;

                while (cur)
                {
                    if (step_push(ns, cur, alloc) & once)
                        return;

                    cur = cur->parent;
                }

                break;
            }

            case axis_self:
            {
                step_push(ns, n, alloc);

                break;
            }

            case axis_parent:
            {
                if (n->parent)
                    step_push(ns, n->parent, alloc);

                break;
            }

            default:
                assert(false && "Unimplemented axis"); // unreachable
            }
        }

        template <class T> void step_fill(xpath_node_set_raw& ns, xml_attribute_struct* a, xml_node_struct* p, xpath_allocator* alloc, bool once, T v)
        {
            const axis_t axis = T::axis;

            switch (axis)
            {
            case axis_ancestor:
            case axis_ancestor_or_self:
            {
                if (axis == axis_ancestor_or_self && _test == nodetest_type_node) // reject attributes based on principal node type test
                    if (step_push(ns, a, p, alloc) & once)
                        return;

                xml_node_struct* cur = p;

                while (cur)
                {
                    if (step_push(ns, cur, alloc) & once)
                        return;

                    cur = cur->parent;
                }

                break;
            }

            case axis_descendant_or_self:
            case axis_self:
            {
                if (_test == nodetest_type_node) // reject attributes based on principal node type test
                    step_push(ns, a, p, alloc);

                break;
            }

            case axis_following:
            {
                xml_node_struct* cur = p;

                while (cur)
                {
                    if (cur->first_child)
                        cur = cur->first_child;
                    else
                    {
                        while (!cur->next_sibling)
                        {
                            cur = cur->parent;

                            if (!cur) return;
                        }

                        cur = cur->next_sibling;
                    }

                    if (step_push(ns, cur, alloc) & once)
                        return;
                }

                break;
            }

            case axis_parent:
            {
                step_push(ns, p, alloc);

                break;
            }

            case axis_preceding:
            {
                // preceding:: axis does not include attribute nodes and attribute ancestors (they are the same as parent's ancestors), so we can reuse node preceding
                step_fill(ns, p, alloc, once, v);
                break;
            }

            default:
                assert(false && "Unimplemented axis"); // unreachable
            }
        }

        template <class T> void step_fill(xpath_node_set_raw& ns, const xpath_node& xn, xpath_allocator* alloc, bool once, T v)
        {
            const axis_t axis = T::axis;
            const bool axis_has_attributes = (axis == axis_ancestor || axis == axis_ancestor_or_self || axis == axis_descendant_or_self || axis == axis_following || axis == axis_parent || axis == axis_preceding || axis == axis_self);

            if (xn.node())
                step_fill(ns, xn.node().internal_object(), alloc, once, v);
            else if (axis_has_attributes && xn.attribute() && xn.parent())
                step_fill(ns, xn.attribute().internal_object(), xn.parent().internal_object(), alloc, once, v);
        }

        template <class T> xpath_node_set_raw step_do(const xpath_context& c, const xpath_stack& stack, nodeset_eval_t eval, T v)
        {
            const axis_t axis = T::axis;
            const bool axis_reverse = (axis == axis_ancestor || axis == axis_ancestor_or_self || axis == axis_preceding || axis == axis_preceding_sibling);
            const xpath_node_set::type_t axis_type = axis_reverse ? xpath_node_set::type_sorted_reverse : xpath_node_set::type_sorted;

            bool once =
                (axis == axis_attribute && _test == nodetest_name) ||
                (!_right && eval_once(axis_type, eval)) ||
                // coverity[mixed_enums]
                (_right && !_right->_next && _right->_test == predicate_constant_one);

            xpath_node_set_raw ns;
            ns.set_type(axis_type);

            if (_left)
            {
                xpath_node_set_raw s = _left->eval_node_set(c, stack, nodeset_eval_all);

                // self axis preserves the original order
                if (axis == axis_self) ns.set_type(s.type());

                for (const xpath_node* it = s.begin(); it != s.end(); ++it)
                {
                    size_t size = ns.size();

                    // in general, all axes generate elements in a particular order, but there is no order guarantee if axis is applied to two nodes
                    if (axis != axis_self && size != 0) ns.set_type(xpath_node_set::type_unsorted);

                    step_fill(ns, *it, stack.result, once, v);
                    if (_right) apply_predicates(ns, size, stack, eval);
                }
            }
            else
            {
                step_fill(ns, c.n, stack.result, once, v);
                if (_right) apply_predicates(ns, 0, stack, eval);
            }

            // child, attribute and self axes always generate unique set of nodes
            // for other axis, if the set stayed sorted, it stayed unique because the traversal algorithms do not visit the same node twice
            if (axis != axis_child && axis != axis_attribute && axis != axis_self && ns.type() == xpath_node_set::type_unsorted)
                ns.remove_duplicates(stack.temp);

            return ns;
        }

    public:
        xpath_ast_node(ast_type_t type, xpath_value_type rettype_, const char_t* value):
            _type(static_cast<char>(type)), _rettype(static_cast<char>(rettype_)), _axis(0), _test(0), _left(0), _right(0), _next(0)
        {
            assert(type == ast_string_constant);
            _data.string = value;
        }

        xpath_ast_node(ast_type_t type, xpath_value_type rettype_, double value):
            _type(static_cast<char>(type)), _rettype(static_cast<char>(rettype_)), _axis(0), _test(0), _left(0), _right(0), _next(0)
        {
            assert(type == ast_number_constant);
            _data.number = value;
        }

        xpath_ast_node(ast_type_t type, xpath_value_type rettype_, xpath_variable* value):
            _type(static_cast<char>(type)), _rettype(static_cast<char>(rettype_)), _axis(0), _test(0), _left(0), _right(0), _next(0)
        {
            assert(type == ast_variable);
            _data.variable = value;
        }

        xpath_ast_node(ast_type_t type, xpath_value_type rettype_, xpath_ast_node* left = 0, xpath_ast_node* right = 0):
            _type(static_cast<char>(type)), _rettype(static_cast<char>(rettype_)), _axis(0), _test(0), _left(left), _right(right), _next(0)
        {
        }

        xpath_ast_node(ast_type_t type, xpath_ast_node* left, axis_t axis, nodetest_t test, const char_t* contents):
            _type(static_cast<char>(type)), _rettype(xpath_type_node_set), _axis(static_cast<char>(axis)), _test(static_cast<char>(test)), _left(left), _right(0), _next(0)
        {
            assert(type == ast_step);
            _data.nodetest = contents;
        }

        xpath_ast_node(ast_type_t type, xpath_ast_node* left, xpath_ast_node* right, predicate_t test):
            _type(static_cast<char>(type)), _rettype(xpath_type_node_set), _axis(0), _test(static_cast<char>(test)), _left(left), _right(right), _next(0)
        {
            assert(type == ast_filter || type == ast_predicate);
        }

        void set_next(xpath_ast_node* value)
        {
            _next = value;
        }

        void set_right(xpath_ast_node* value)
        {
            _right = value;
        }

        bool eval_boolean(const xpath_context& c, const xpath_stack& stack)
        {
            switch (_type)
            {
            case ast_op_or:
                return _left->eval_boolean(c, stack) || _right->eval_boolean(c, stack);

            case ast_op_and:
                return _left->eval_boolean(c, stack) && _right->eval_boolean(c, stack);

            case ast_op_equal:
                return compare_eq(_left, _right, c, stack, equal_to());

            case ast_op_not_equal:
                return compare_eq(_left, _right, c, stack, not_equal_to());

            case ast_op_less:
                return compare_rel(_left, _right, c, stack, less());

            case ast_op_greater:
                return compare_rel(_right, _left, c, stack, less());

            case ast_op_less_or_equal:
                return compare_rel(_left, _right, c, stack, less_equal());

            case ast_op_greater_or_equal:
                return compare_rel(_right, _left, c, stack, less_equal());

            case ast_func_starts_with:
            {
                xpath_allocator_capture cr(stack.result);

                xpath_string lr = _left->eval_string(c, stack);
                xpath_string rr = _right->eval_string(c, stack);

                return starts_with(lr.c_str(), rr.c_str());
            }

            case ast_func_contains:
            {
                xpath_allocator_capture cr(stack.result);

                xpath_string lr = _left->eval_string(c, stack);
                xpath_string rr = _right->eval_string(c, stack);

                return find_substring(lr.c_str(), rr.c_str()) != 0;
            }

            case ast_func_boolean:
                return _left->eval_boolean(c, stack);

            case ast_func_not:
                return !_left->eval_boolean(c, stack);

            case ast_func_true:
                return true;

            case ast_func_false:
                return false;

            case ast_func_lang:
            {
                if (c.n.attribute()) return false;

                xpath_allocator_capture cr(stack.result);

                xpath_string lang = _left->eval_string(c, stack);

                for (xml_node n = c.n.node(); n; n = n.parent())
                {
                    xml_attribute a = n.attribute(PUGIXML_TEXT("xml:lang"));

                    if (a)
                    {
                        const char_t* value = a.value();

                        // strnicmp / strncasecmp is not portable
                        for (const char_t* lit = lang.c_str(); *lit; ++lit)
                        {
                            if (tolower_ascii(*lit) != tolower_ascii(*value)) return false;
                            ++value;
                        }

                        return *value == 0 || *value == '-';
                    }
                }

                return false;
            }

            case ast_opt_compare_attribute:
            {
                const char_t* value = (_right->_type == ast_string_constant) ? _right->_data.string : _right->_data.variable->get_string();

                xml_attribute attr = c.n.node().attribute(_left->_data.nodetest);

                return attr && strequal(attr.value(), value) && is_xpath_attribute(attr.name());
            }

            case ast_variable:
            {
                assert(_rettype == _data.variable->type());

                if (_rettype == xpath_type_boolean)
                    return _data.variable->get_boolean();

                // variable needs to be converted to the correct type, this is handled by the fallthrough block below
                break;
            }

            default:
                ;
            }

            // none of the ast types that return the value directly matched, we need to perform type conversion
            switch (_rettype)
            {
            case xpath_type_number:
                return convert_number_to_boolean(eval_number(c, stack));

            case xpath_type_string:
            {
                xpath_allocator_capture cr(stack.result);

                return !eval_string(c, stack).empty();
            }

            case xpath_type_node_set:
            {
                xpath_allocator_capture cr(stack.result);

                return !eval_node_set(c, stack, nodeset_eval_any).empty();
            }

            default:
                assert(false && "Wrong expression for return type boolean"); // unreachable
                return false;
            }
        }

        double eval_number(const xpath_context& c, const xpath_stack& stack)
        {
            switch (_type)
            {
            case ast_op_add:
                return _left->eval_number(c, stack) + _right->eval_number(c, stack);

            case ast_op_subtract:
                return _left->eval_number(c, stack) - _right->eval_number(c, stack);

            case ast_op_multiply:
                return _left->eval_number(c, stack) * _right->eval_number(c, stack);

            case ast_op_divide:
                return _left->eval_number(c, stack) / _right->eval_number(c, stack);

            case ast_op_mod:
                return fmod(_left->eval_number(c, stack), _right->eval_number(c, stack));

            case ast_op_negate:
                return -_left->eval_number(c, stack);

            case ast_number_constant:
                return _data.number;

            case ast_func_last:
                return static_cast<double>(c.size);

            case ast_func_position:
                return static_cast<double>(c.position);

            case ast_func_count:
            {
                xpath_allocator_capture cr(stack.result);

                return static_cast<double>(_left->eval_node_set(c, stack, nodeset_eval_all).size());
            }

            case ast_func_string_length_0:
            {
                xpath_allocator_capture cr(stack.result);

                return static_cast<double>(string_value(c.n, stack.result).length());
            }

            case ast_func_string_length_1:
            {
                xpath_allocator_capture cr(stack.result);

                return static_cast<double>(_left->eval_string(c, stack).length());
            }

            case ast_func_number_0:
            {
                xpath_allocator_capture cr(stack.result);

                return convert_string_to_number(string_value(c.n, stack.result).c_str());
            }

            case ast_func_number_1:
                return _left->eval_number(c, stack);

            case ast_func_sum:
            {
                xpath_allocator_capture cr(stack.result);

                double r = 0;

                xpath_node_set_raw ns = _left->eval_node_set(c, stack, nodeset_eval_all);

                for (const xpath_node* it = ns.begin(); it != ns.end(); ++it)
                {
                    xpath_allocator_capture cri(stack.result);

                    r += convert_string_to_number(string_value(*it, stack.result).c_str());
                }

                return r;
            }

            case ast_func_floor:
            {
                double r = _left->eval_number(c, stack);

                return r == r ? floor(r) : r;
            }

            case ast_func_ceiling:
            {
                double r = _left->eval_number(c, stack);

                return r == r ? ceil(r) : r;
            }

            case ast_func_round:
                return round_nearest_nzero(_left->eval_number(c, stack));

            case ast_variable:
            {
                assert(_rettype == _data.variable->type());

                if (_rettype == xpath_type_number)
                    return _data.variable->get_number();

                // variable needs to be converted to the correct type, this is handled by the fallthrough block below
                break;
            }

            default:
                ;
            }

            // none of the ast types that return the value directly matched, we need to perform type conversion
            switch (_rettype)
            {
            case xpath_type_boolean:
                return eval_boolean(c, stack) ? 1 : 0;

            case xpath_type_string:
            {
                xpath_allocator_capture cr(stack.result);

                return convert_string_to_number(eval_string(c, stack).c_str());
            }

            case xpath_type_node_set:
            {
                xpath_allocator_capture cr(stack.result);

                return convert_string_to_number(eval_string(c, stack).c_str());
            }

            default:
                assert(false && "Wrong expression for return type number"); // unreachable
                return 0;
            }
        }

        xpath_string eval_string_concat(const xpath_context& c, const xpath_stack& stack)
        {
            assert(_type == ast_func_concat);

            xpath_allocator_capture ct(stack.temp);

            // count the string number
            size_t count = 1;
            for (xpath_ast_node* nc = _right; nc; nc = nc->_next) count++;

            // allocate a buffer for temporary string objects
            xpath_string* buffer = static_cast<xpath_string*>(stack.temp->allocate(count * sizeof(xpath_string)));
            if (!buffer) return xpath_string();

            // evaluate all strings to temporary stack
            xpath_stack swapped_stack = {stack.temp, stack.result};

            buffer[0] = _left->eval_string(c, swapped_stack);

            size_t pos = 1;
            for (xpath_ast_node* n = _right; n; n = n->_next, ++pos) buffer[pos] = n->eval_string(c, swapped_stack);
            assert(pos == count);

            // get total length
            size_t length = 0;
            for (size_t i = 0; i < count; ++i) length += buffer[i].length();

            // create final string
            char_t* result = static_cast<char_t*>(stack.result->allocate((length + 1) * sizeof(char_t)));
            if (!result) return xpath_string();

            char_t* ri = result;

            for (size_t j = 0; j < count; ++j)
                for (const char_t* bi = buffer[j].c_str(); *bi; ++bi)
                    *ri++ = *bi;

            *ri = 0;

            return xpath_string::from_heap_preallocated(result, ri);
        }

        xpath_string eval_string(const xpath_context& c, const xpath_stack& stack)
        {
            switch (_type)
            {
            case ast_string_constant:
                return xpath_string::from_const(_data.string);

            case ast_func_local_name_0:
            {
                xpath_node na = c.n;

                return xpath_string::from_const(local_name(na));
            }

            case ast_func_local_name_1:
            {
                xpath_allocator_capture cr(stack.result);

                xpath_node_set_raw ns = _left->eval_node_set(c, stack, nodeset_eval_first);
                xpath_node na = ns.first();

                return xpath_string::from_const(local_name(na));
            }

            case ast_func_name_0:
            {
                xpath_node na = c.n;

                return xpath_string::from_const(qualified_name(na));
            }

            case ast_func_name_1:
            {
                xpath_allocator_capture cr(stack.result);

                xpath_node_set_raw ns = _left->eval_node_set(c, stack, nodeset_eval_first);
                xpath_node na = ns.first();

                return xpath_string::from_const(qualified_name(na));
            }

            case ast_func_namespace_uri_0:
            {
                xpath_node na = c.n;

                return xpath_string::from_const(namespace_uri(na));
            }

            case ast_func_namespace_uri_1:
            {
                xpath_allocator_capture cr(stack.result);

                xpath_node_set_raw ns = _left->eval_node_set(c, stack, nodeset_eval_first);
                xpath_node na = ns.first();

                return xpath_string::from_const(namespace_uri(na));
            }

            case ast_func_string_0:
                return string_value(c.n, stack.result);

            case ast_func_string_1:
                return _left->eval_string(c, stack);

            case ast_func_concat:
                return eval_string_concat(c, stack);

            case ast_func_substring_before:
            {
                xpath_allocator_capture cr(stack.temp);

                xpath_stack swapped_stack = {stack.temp, stack.result};

                xpath_string s = _left->eval_string(c, swapped_stack);
                xpath_string p = _right->eval_string(c, swapped_stack);

                const char_t* pos = find_substring(s.c_str(), p.c_str());

                return pos ? xpath_string::from_heap(s.c_str(), pos, stack.result) : xpath_string();
            }

            case ast_func_substring_after:
            {
                xpath_allocator_capture cr(stack.temp);

                xpath_stack swapped_stack = {stack.temp, stack.result};

                xpath_string s = _left->eval_string(c, swapped_stack);
                xpath_string p = _right->eval_string(c, swapped_stack);

                const char_t* pos = find_substring(s.c_str(), p.c_str());
                if (!pos) return xpath_string();

                const char_t* rbegin = pos + p.length();
                const char_t* rend = s.c_str() + s.length();

                return s.uses_heap() ? xpath_string::from_heap(rbegin, rend, stack.result) : xpath_string::from_const(rbegin);
            }

            case ast_func_substring_2:
            {
                xpath_allocator_capture cr(stack.temp);

                xpath_stack swapped_stack = {stack.temp, stack.result};

                xpath_string s = _left->eval_string(c, swapped_stack);
                size_t s_length = s.length();

                double first = round_nearest(_right->eval_number(c, stack));

                if (is_nan(first)) return xpath_string(); // NaN
                else if (first >= static_cast<double>(s_length + 1)) return xpath_string();

                size_t pos = first < 1 ? 1 : static_cast<size_t>(first);
                assert(1 <= pos && pos <= s_length + 1);

                const char_t* rbegin = s.c_str() + (pos - 1);
                const char_t* rend = s.c_str() + s.length();

                return s.uses_heap() ? xpath_string::from_heap(rbegin, rend, stack.result) : xpath_string::from_const(rbegin);
            }

            case ast_func_substring_3:
            {
                xpath_allocator_capture cr(stack.temp);

                xpath_stack swapped_stack = {stack.temp, stack.result};

                xpath_string s = _left->eval_string(c, swapped_stack);
                size_t s_length = s.length();

                double first = round_nearest(_right->eval_number(c, stack));
                double last = first + round_nearest(_right->_next->eval_number(c, stack));

                if (is_nan(first) || is_nan(last)) return xpath_string();
                else if (first >= static_cast<double>(s_length + 1)) return xpath_string();
                else if (first >= last) return xpath_string();
                else if (last < 1) return xpath_string();

                size_t pos = first < 1 ? 1 : static_cast<size_t>(first);
                size_t end = last >= static_cast<double>(s_length + 1) ? s_length + 1 : static_cast<size_t>(last);

                assert(1 <= pos && pos <= end && end <= s_length + 1);
                const char_t* rbegin = s.c_str() + (pos - 1);
                const char_t* rend = s.c_str() + (end - 1);

                return (end == s_length + 1 && !s.uses_heap()) ? xpath_string::from_const(rbegin) : xpath_string::from_heap(rbegin, rend, stack.result);
            }

            case ast_func_normalize_space_0:
            {
                xpath_string s = string_value(c.n, stack.result);

                char_t* begin = s.data(stack.result);
                if (!begin) return xpath_string();

                char_t* end = normalize_space(begin);

                return xpath_string::from_heap_preallocated(begin, end);
            }

            case ast_func_normalize_space_1:
            {
                xpath_string s = _left->eval_string(c, stack);

                char_t* begin = s.data(stack.result);
                if (!begin) return xpath_string();

                char_t* end = normalize_space(begin);

                return xpath_string::from_heap_preallocated(begin, end);
            }

            case ast_func_translate:
            {
                xpath_allocator_capture cr(stack.temp);

                xpath_stack swapped_stack = {stack.temp, stack.result};

                xpath_string s = _left->eval_string(c, stack);
                xpath_string from = _right->eval_string(c, swapped_stack);
                xpath_string to = _right->_next->eval_string(c, swapped_stack);

                char_t* begin = s.data(stack.result);
                if (!begin) return xpath_string();

                char_t* end = translate(begin, from.c_str(), to.c_str(), to.length());

                return xpath_string::from_heap_preallocated(begin, end);
            }

            case ast_opt_translate_table:
            {
                xpath_string s = _left->eval_string(c, stack);

                char_t* begin = s.data(stack.result);
                if (!begin) return xpath_string();

                char_t* end = translate_table(begin, _data.table);

                return xpath_string::from_heap_preallocated(begin, end);
            }

            case ast_variable:
            {
                assert(_rettype == _data.variable->type());

                if (_rettype == xpath_type_string)
                    return xpath_string::from_const(_data.variable->get_string());

                // variable needs to be converted to the correct type, this is handled by the fallthrough block below
                break;
            }

            default:
                ;
            }

            // none of the ast types that return the value directly matched, we need to perform type conversion
            switch (_rettype)
            {
            case xpath_type_boolean:
                return xpath_string::from_const(eval_boolean(c, stack) ? PUGIXML_TEXT("true") : PUGIXML_TEXT("false"));

            case xpath_type_number:
                return convert_number_to_string(eval_number(c, stack), stack.result);

            case xpath_type_node_set:
            {
                xpath_allocator_capture cr(stack.temp);

                xpath_stack swapped_stack = {stack.temp, stack.result};

                xpath_node_set_raw ns = eval_node_set(c, swapped_stack, nodeset_eval_first);
                return ns.empty() ? xpath_string() : string_value(ns.first(), stack.result);
            }

            default:
                assert(false && "Wrong expression for return type string"); // unreachable
                return xpath_string();
            }
        }

        xpath_node_set_raw eval_node_set(const xpath_context& c, const xpath_stack& stack, nodeset_eval_t eval)
        {
            switch (_type)
            {
            case ast_op_union:
            {
                xpath_allocator_capture cr(stack.temp);

                xpath_stack swapped_stack = {stack.temp, stack.result};

                xpath_node_set_raw ls = _left->eval_node_set(c, stack, eval);
                xpath_node_set_raw rs = _right->eval_node_set(c, swapped_stack, eval);

                // we can optimize merging two sorted sets, but this is a very rare operation, so don't bother
                ls.set_type(xpath_node_set::type_unsorted);

                ls.append(rs.begin(), rs.end(), stack.result);
                ls.remove_duplicates(stack.temp);

                return ls;
            }

            case ast_filter:
            {
                xpath_node_set_raw set = _left->eval_node_set(c, stack, _test == predicate_constant_one ? nodeset_eval_first : nodeset_eval_all);

                // either expression is a number or it contains position() call; sort by document order
                if (_test != predicate_posinv) set.sort_do();

                bool once = eval_once(set.type(), eval);

                apply_predicate(set, 0, stack, once);

                return set;
            }

            case ast_func_id:
                return xpath_node_set_raw();

            case ast_step:
            {
                switch (_axis)
                {
                case axis_ancestor:
                    return step_do(c, stack, eval, axis_to_type<axis_ancestor>());

                case axis_ancestor_or_self:
                    return step_do(c, stack, eval, axis_to_type<axis_ancestor_or_self>());

                case axis_attribute:
                    return step_do(c, stack, eval, axis_to_type<axis_attribute>());

                case axis_child:
                    return step_do(c, stack, eval, axis_to_type<axis_child>());

                case axis_descendant:
                    return step_do(c, stack, eval, axis_to_type<axis_descendant>());

                case axis_descendant_or_self:
                    return step_do(c, stack, eval, axis_to_type<axis_descendant_or_self>());

                case axis_following:
                    return step_do(c, stack, eval, axis_to_type<axis_following>());

                case axis_following_sibling:
                    return step_do(c, stack, eval, axis_to_type<axis_following_sibling>());

                case axis_namespace:
                    // namespaced axis is not supported
                    return xpath_node_set_raw();

                case axis_parent:
                    return step_do(c, stack, eval, axis_to_type<axis_parent>());

                case axis_preceding:
                    return step_do(c, stack, eval, axis_to_type<axis_preceding>());

                case axis_preceding_sibling:
                    return step_do(c, stack, eval, axis_to_type<axis_preceding_sibling>());

                case axis_self:
                    return step_do(c, stack, eval, axis_to_type<axis_self>());

                default:
                    assert(false && "Unknown axis"); // unreachable
                    return xpath_node_set_raw();
                }
            }

            case ast_step_root:
            {
                assert(!_right); // root step can't have any predicates

                xpath_node_set_raw ns;

                ns.set_type(xpath_node_set::type_sorted);

                if (c.n.node()) ns.push_back(c.n.node().root(), stack.result);
                else if (c.n.attribute()) ns.push_back(c.n.parent().root(), stack.result);

                return ns;
            }

            case ast_variable:
            {
                assert(_rettype == _data.variable->type());

                if (_rettype == xpath_type_node_set)
                {
                    const xpath_node_set& s = _data.variable->get_node_set();

                    xpath_node_set_raw ns;

                    ns.set_type(s.type());
                    ns.append(s.begin(), s.end(), stack.result);

                    return ns;
                }

                // variable needs to be converted to the correct type, this is handled by the fallthrough block below
                break;
            }

            default:
                ;
            }

            // none of the ast types that return the value directly matched, but conversions to node set are invalid
            assert(false && "Wrong expression for return type node set"); // unreachable
            return xpath_node_set_raw();
        }

        void optimize(xpath_allocator* alloc)
        {
            if (_left)
                _left->optimize(alloc);

            if (_right)
                _right->optimize(alloc);

            if (_next)
                _next->optimize(alloc);

            // coverity[var_deref_model]
            optimize_self(alloc);
        }

        void optimize_self(xpath_allocator* alloc)
        {
            // Rewrite [position()=expr] with [expr]
            // Note that this step has to go before classification to recognize [position()=1]
            if ((_type == ast_filter || _type == ast_predicate) &&
                _right && // workaround for clang static analyzer (_right is never null for ast_filter/ast_predicate)
                _right->_type == ast_op_equal && _right->_left->_type == ast_func_position && _right->_right->_rettype == xpath_type_number)
            {
                _right = _right->_right;
            }

            // Classify filter/predicate ops to perform various optimizations during evaluation
            if ((_type == ast_filter || _type == ast_predicate) && _right) // workaround for clang static analyzer (_right is never null for ast_filter/ast_predicate)
            {
                assert(_test == predicate_default);

                if (_right->_type == ast_number_constant && _right->_data.number == 1.0)
                    _test = predicate_constant_one;
                else if (_right->_rettype == xpath_type_number && (_right->_type == ast_number_constant || _right->_type == ast_variable || _right->_type == ast_func_last))
                    _test = predicate_constant;
                else if (_right->_rettype != xpath_type_number && _right->is_posinv_expr())
                    _test = predicate_posinv;
            }

            // Rewrite descendant-or-self::node()/child::foo with descendant::foo
            // The former is a full form of //foo, the latter is much faster since it executes the node test immediately
            // Do a similar kind of rewrite for self/descendant/descendant-or-self axes
            // Note that we only rewrite positionally invariant steps (//foo[1] != /descendant::foo[1])
            if (_type == ast_step && (_axis == axis_child || _axis == axis_self || _axis == axis_descendant || _axis == axis_descendant_or_self) &&
                _left && _left->_type == ast_step && _left->_axis == axis_descendant_or_self && _left->_test == nodetest_type_node && !_left->_right &&
                is_posinv_step())
            {
                if (_axis == axis_child || _axis == axis_descendant)
                    _axis = axis_descendant;
                else
                    _axis = axis_descendant_or_self;

                _left = _left->_left;
            }

            // Use optimized lookup table implementation for translate() with constant arguments
            if (_type == ast_func_translate &&
                _right && // workaround for clang static analyzer (_right is never null for ast_func_translate)
                _right->_type == ast_string_constant && _right->_next->_type == ast_string_constant)
            {
                unsigned char* table = translate_table_generate(alloc, _right->_data.string, _right->_next->_data.string);

                if (table)
                {
                    _type = ast_opt_translate_table;
                    _data.table = table;
                }
            }

            // Use optimized path for @attr = 'value' or @attr = $value
            if (_type == ast_op_equal &&
                _left && _right && // workaround for clang static analyzer and Coverity (_left and _right are never null for ast_op_equal)
                // coverity[mixed_enums]
                _left->_type == ast_step && _left->_axis == axis_attribute && _left->_test == nodetest_name && !_left->_left && !_left->_right &&
                (_right->_type == ast_string_constant || (_right->_type == ast_variable && _right->_rettype == xpath_type_string)))
            {
                _type = ast_opt_compare_attribute;
            }
        }

        bool is_posinv_expr() const
        {
            switch (_type)
            {
            case ast_func_position:
            case ast_func_last:
                return false;

            case ast_string_constant:
            case ast_number_constant:
            case ast_variable:
                return true;

            case ast_step:
            case ast_step_root:
                return true;

            case ast_predicate:
            case ast_filter:
                return true;

            default:
                if (_left && !_left->is_posinv_expr()) return false;

                for (xpath_ast_node* n = _right; n; n = n->_next)
                    if (!n->is_posinv_expr()) return false;

                return true;
            }
        }

        bool is_posinv_step() const
        {
            assert(_type == ast_step);

            for (xpath_ast_node* n = _right; n; n = n->_next)
            {
                assert(n->_type == ast_predicate);

                if (n->_test != predicate_posinv)
                    return false;
            }

            return true;
        }

        xpath_value_type rettype() const
        {
            return static_cast<xpath_value_type>(_rettype);
        }
    };

    static const size_t xpath_ast_depth_limit =
    #ifdef PUGIXML_XPATH_DEPTH_LIMIT
        PUGIXML_XPATH_DEPTH_LIMIT
    #else
        1024
    #endif
        ;

    struct xpath_parser
    {
        xpath_allocator* _alloc;
        xpath_lexer _lexer;

        const char_t* _query;
        xpath_variable_set* _variables;

        xpath_parse_result* _result;

        char_t _scratch[32];

        size_t _depth;

        xpath_ast_node* error(const char* message)
        {
            _result->error = message;
            _result->offset = _lexer.current_pos() - _query;

            return 0;
        }

        xpath_ast_node* error_oom()
        {
            assert(_alloc->_error);
            *_alloc->_error = true;

            return 0;
        }

        xpath_ast_node* error_rec()
        {
            return error("Exceeded maximum allowed query depth");
        }

        void* alloc_node()
        {
            return _alloc->allocate(sizeof(xpath_ast_node));
        }

        xpath_ast_node* alloc_node(ast_type_t type, xpath_value_type rettype, const char_t* value)
        {
            void* memory = alloc_node();
            return memory ? new (memory) xpath_ast_node(type, rettype, value) : 0;
        }

        xpath_ast_node* alloc_node(ast_type_t type, xpath_value_type rettype, double value)
        {
            void* memory = alloc_node();
            return memory ? new (memory) xpath_ast_node(type, rettype, value) : 0;
        }

        xpath_ast_node* alloc_node(ast_type_t type, xpath_value_type rettype, xpath_variable* value)
        {
            void* memory = alloc_node();
            return memory ? new (memory) xpath_ast_node(type, rettype, value) : 0;
        }

        xpath_ast_node* alloc_node(ast_type_t type, xpath_value_type rettype, xpath_ast_node* left = 0, xpath_ast_node* right = 0)
        {
            void* memory = alloc_node();
            return memory ? new (memory) xpath_ast_node(type, rettype, left, right) : 0;
        }

        xpath_ast_node* alloc_node(ast_type_t type, xpath_ast_node* left, axis_t axis, nodetest_t test, const char_t* contents)
        {
            void* memory = alloc_node();
            return memory ? new (memory) xpath_ast_node(type, left, axis, test, contents) : 0;
        }

        xpath_ast_node* alloc_node(ast_type_t type, xpath_ast_node* left, xpath_ast_node* right, predicate_t test)
        {
            void* memory = alloc_node();
            return memory ? new (memory) xpath_ast_node(type, left, right, test) : 0;
        }

        const char_t* alloc_string(const xpath_lexer_string& value)
        {
            if (!value.begin)
                return PUGIXML_TEXT("");

            size_t length = static_cast<size_t>(value.end - value.begin);

            char_t* c = static_cast<char_t*>(_alloc->allocate((length + 1) * sizeof(char_t)));
            if (!c) return 0;

            memcpy(c, value.begin, length * sizeof(char_t));
            c[length] = 0;

            return c;
        }

        xpath_ast_node* parse_function(const xpath_lexer_string& name, size_t argc, xpath_ast_node* args[2])
        {
            switch (name.begin[0])
            {
            case 'b':
                if (name == PUGIXML_TEXT("boolean") && argc == 1)
                    return alloc_node(ast_func_boolean, xpath_type_boolean, args[0]);

                break;

            case 'c':
                if (name == PUGIXML_TEXT("count") && argc == 1)
                {
                    if (args[0]->rettype() != xpath_type_node_set) return error("Function has to be applied to node set");
                    return alloc_node(ast_func_count, xpath_type_number, args[0]);
                }
                else if (name == PUGIXML_TEXT("contains") && argc == 2)
                    return alloc_node(ast_func_contains, xpath_type_boolean, args[0], args[1]);
                else if (name == PUGIXML_TEXT("concat") && argc >= 2)
                    return alloc_node(ast_func_concat, xpath_type_string, args[0], args[1]);
                else if (name == PUGIXML_TEXT("ceiling") && argc == 1)
                    return alloc_node(ast_func_ceiling, xpath_type_number, args[0]);

                break;

            case 'f':
                if (name == PUGIXML_TEXT("false") && argc == 0)
                    return alloc_node(ast_func_false, xpath_type_boolean);
                else if (name == PUGIXML_TEXT("floor") && argc == 1)
                    return alloc_node(ast_func_floor, xpath_type_number, args[0]);

                break;

            case 'i':
                if (name == PUGIXML_TEXT("id") && argc == 1)
                    return alloc_node(ast_func_id, xpath_type_node_set, args[0]);

                break;

            case 'l':
                if (name == PUGIXML_TEXT("last") && argc == 0)
                    return alloc_node(ast_func_last, xpath_type_number);
                else if (name == PUGIXML_TEXT("lang") && argc == 1)
                    return alloc_node(ast_func_lang, xpath_type_boolean, args[0]);
                else if (name == PUGIXML_TEXT("local-name") && argc <= 1)
                {
                    if (argc == 1 && args[0]->rettype() != xpath_type_node_set) return error("Function has to be applied to node set");
                    return alloc_node(argc == 0 ? ast_func_local_name_0 : ast_func_local_name_1, xpath_type_string, args[0]);
                }

                break;

            case 'n':
                if (name == PUGIXML_TEXT("name") && argc <= 1)
                {
                    if (argc == 1 && args[0]->rettype() != xpath_type_node_set) return error("Function has to be applied to node set");
                    return alloc_node(argc == 0 ? ast_func_name_0 : ast_func_name_1, xpath_type_string, args[0]);
                }
                else if (name == PUGIXML_TEXT("namespace-uri") && argc <= 1)
                {
                    if (argc == 1 && args[0]->rettype() != xpath_type_node_set) return error("Function has to be applied to node set");
                    return alloc_node(argc == 0 ? ast_func_namespace_uri_0 : ast_func_namespace_uri_1, xpath_type_string, args[0]);
                }
                else if (name == PUGIXML_TEXT("normalize-space") && argc <= 1)
                    return alloc_node(argc == 0 ? ast_func_normalize_space_0 : ast_func_normalize_space_1, xpath_type_string, args[0], args[1]);
                else if (name == PUGIXML_TEXT("not") && argc == 1)
                    return alloc_node(ast_func_not, xpath_type_boolean, args[0]);
                else if (name == PUGIXML_TEXT("number") && argc <= 1)
                    return alloc_node(argc == 0 ? ast_func_number_0 : ast_func_number_1, xpath_type_number, args[0]);

                break;

            case 'p':
                if (name == PUGIXML_TEXT("position") && argc == 0)
                    return alloc_node(ast_func_position, xpath_type_number);

                break;

            case 'r':
                if (name == PUGIXML_TEXT("round") && argc == 1)
                    return alloc_node(ast_func_round, xpath_type_number, args[0]);

                break;

            case 's':
                if (name == PUGIXML_TEXT("string") && argc <= 1)
                    return alloc_node(argc == 0 ? ast_func_string_0 : ast_func_string_1, xpath_type_string, args[0]);
                else if (name == PUGIXML_TEXT("string-length") && argc <= 1)
                    return alloc_node(argc == 0 ? ast_func_string_length_0 : ast_func_string_length_1, xpath_type_number, args[0]);
                else if (name == PUGIXML_TEXT("starts-with") && argc == 2)
                    return alloc_node(ast_func_starts_with, xpath_type_boolean, args[0], args[1]);
                else if (name == PUGIXML_TEXT("substring-before") && argc == 2)
                    return alloc_node(ast_func_substring_before, xpath_type_string, args[0], args[1]);
                else if (name == PUGIXML_TEXT("substring-after") && argc == 2)
                    return alloc_node(ast_func_substring_after, xpath_type_string, args[0], args[1]);
                else if (name == PUGIXML_TEXT("substring") && (argc == 2 || argc == 3))
                    return alloc_node(argc == 2 ? ast_func_substring_2 : ast_func_substring_3, xpath_type_string, args[0], args[1]);
                else if (name == PUGIXML_TEXT("sum") && argc == 1)
                {
                    if (args[0]->rettype() != xpath_type_node_set) return error("Function has to be applied to node set");
                    return alloc_node(ast_func_sum, xpath_type_number, args[0]);
                }

                break;

            case 't':
                if (name == PUGIXML_TEXT("translate") && argc == 3)
                    return alloc_node(ast_func_translate, xpath_type_string, args[0], args[1]);
                else if (name == PUGIXML_TEXT("true") && argc == 0)
                    return alloc_node(ast_func_true, xpath_type_boolean);

                break;

            default:
                break;
            }

            return error("Unrecognized function or wrong parameter count");
        }

        axis_t parse_axis_name(const xpath_lexer_string& name, bool& specified)
        {
            specified = true;

            switch (name.begin[0])
            {
            case 'a':
                if (name == PUGIXML_TEXT("ancestor"))
                    return axis_ancestor;
                else if (name == PUGIXML_TEXT("ancestor-or-self"))
                    return axis_ancestor_or_self;
                else if (name == PUGIXML_TEXT("attribute"))
                    return axis_attribute;

                break;

            case 'c':
                if (name == PUGIXML_TEXT("child"))
                    return axis_child;

                break;

            case 'd':
                if (name == PUGIXML_TEXT("descendant"))
                    return axis_descendant;
                else if (name == PUGIXML_TEXT("descendant-or-self"))
                    return axis_descendant_or_self;

                break;

            case 'f':
                if (name == PUGIXML_TEXT("following"))
                    return axis_following;
                else if (name == PUGIXML_TEXT("following-sibling"))
                    return axis_following_sibling;

                break;

            case 'n':
                if (name == PUGIXML_TEXT("namespace"))
                    return axis_namespace;

                break;

            case 'p':
                if (name == PUGIXML_TEXT("parent"))
                    return axis_parent;
                else if (name == PUGIXML_TEXT("preceding"))
                    return axis_preceding;
                else if (name == PUGIXML_TEXT("preceding-sibling"))
                    return axis_preceding_sibling;

                break;

            case 's':
                if (name == PUGIXML_TEXT("self"))
                    return axis_self;

                break;

            default:
                break;
            }

            specified = false;
            return axis_child;
        }

        nodetest_t parse_node_test_type(const xpath_lexer_string& name)
        {
            switch (name.begin[0])
            {
            case 'c':
                if (name == PUGIXML_TEXT("comment"))
                    return nodetest_type_comment;

                break;

            case 'n':
                if (name == PUGIXML_TEXT("node"))
                    return nodetest_type_node;

                break;

            case 'p':
                if (name == PUGIXML_TEXT("processing-instruction"))
                    return nodetest_type_pi;

                break;

            case 't':
                if (name == PUGIXML_TEXT("text"))
                    return nodetest_type_text;

                break;

            default:
                break;
            }

            return nodetest_none;
        }

        // PrimaryExpr ::= VariableReference | '(' Expr ')' | Literal | Number | FunctionCall
        xpath_ast_node* parse_primary_expression()
        {
            switch (_lexer.current())
            {
            case lex_var_ref:
            {
                xpath_lexer_string name = _lexer.contents();

                if (!_variables)
                    return error("Unknown variable: variable set is not provided");

                xpath_variable* var = 0;
                if (!get_variable_scratch(_scratch, _variables, name.begin, name.end, &var))
                    return error_oom();

                if (!var)
                    return error("Unknown variable: variable set does not contain the given name");

                _lexer.next();

                return alloc_node(ast_variable, var->type(), var);
            }

            case lex_open_brace:
            {
                _lexer.next();

                xpath_ast_node* n = parse_expression();
                if (!n) return 0;

                if (_lexer.current() != lex_close_brace)
                    return error("Expected ')' to match an opening '('");

                _lexer.next();

                return n;
            }

            case lex_quoted_string:
            {
                const char_t* value = alloc_string(_lexer.contents());
                if (!value) return 0;

                _lexer.next();

                return alloc_node(ast_string_constant, xpath_type_string, value);
            }

            case lex_number:
            {
                double value = 0;

                if (!convert_string_to_number_scratch(_scratch, _lexer.contents().begin, _lexer.contents().end, &value))
                    return error_oom();

                _lexer.next();

                return alloc_node(ast_number_constant, xpath_type_number, value);
            }

            case lex_string:
            {
                xpath_ast_node* args[2] = {0};
                size_t argc = 0;

                xpath_lexer_string function = _lexer.contents();
                _lexer.next();

                xpath_ast_node* last_arg = 0;

                if (_lexer.current() != lex_open_brace)
                    return error("Unrecognized function call");
                _lexer.next();

                size_t old_depth = _depth;

                while (_lexer.current() != lex_close_brace)
                {
                    if (argc > 0)
                    {
                        if (_lexer.current() != lex_comma)
                            return error("No comma between function arguments");
                        _lexer.next();
                    }

                    if (++_depth > xpath_ast_depth_limit)
                        return error_rec();

                    xpath_ast_node* n = parse_expression();
                    if (!n) return 0;

                    if (argc < 2) args[argc] = n;
                    else last_arg->set_next(n);

                    argc++;
                    last_arg = n;
                }

                _lexer.next();

                _depth = old_depth;

                return parse_function(function, argc, args);
            }

            default:
                return error("Unrecognizable primary expression");
            }
        }

        // FilterExpr ::= PrimaryExpr | FilterExpr Predicate
        // Predicate ::= '[' PredicateExpr ']'
        // PredicateExpr ::= Expr
        xpath_ast_node* parse_filter_expression()
        {
            xpath_ast_node* n = parse_primary_expression();
            if (!n) return 0;

            size_t old_depth = _depth;

            while (_lexer.current() == lex_open_square_brace)
            {
                _lexer.next();

                if (++_depth > xpath_ast_depth_limit)
                    return error_rec();

                if (n->rettype() != xpath_type_node_set)
                    return error("Predicate has to be applied to node set");

                xpath_ast_node* expr = parse_expression();
                if (!expr) return 0;

                n = alloc_node(ast_filter, n, expr, predicate_default);
                if (!n) return 0;

                if (_lexer.current() != lex_close_square_brace)
                    return error("Expected ']' to match an opening '['");

                _lexer.next();
            }

            _depth = old_depth;

            return n;
        }

        // Step ::= AxisSpecifier NodeTest Predicate* | AbbreviatedStep
        // AxisSpecifier ::= AxisName '::' | '@'?
        // NodeTest ::= NameTest | NodeType '(' ')' | 'processing-instruction' '(' Literal ')'
        // NameTest ::= '*' | NCName ':' '*' | QName
        // AbbreviatedStep ::= '.' | '..'
        xpath_ast_node* parse_step(xpath_ast_node* set)
        {
            if (set && set->rettype() != xpath_type_node_set)
                return error("Step has to be applied to node set");

            bool axis_specified = false;
            axis_t axis = axis_child; // implied child axis

            if (_lexer.current() == lex_axis_attribute)
            {
                axis = axis_attribute;
                axis_specified = true;

                _lexer.next();
            }
            else if (_lexer.current() == lex_dot)
            {
                _lexer.next();

                if (_lexer.current() == lex_open_square_brace)
                    return error("Predicates are not allowed after an abbreviated step");

                return alloc_node(ast_step, set, axis_self, nodetest_type_node, 0);
            }
            else if (_lexer.current() == lex_double_dot)
            {
                _lexer.next();

                if (_lexer.current() == lex_open_square_brace)
                    return error("Predicates are not allowed after an abbreviated step");

                return alloc_node(ast_step, set, axis_parent, nodetest_type_node, 0);
            }

            nodetest_t nt_type = nodetest_none;
            xpath_lexer_string nt_name;

            if (_lexer.current() == lex_string)
            {
                // node name test
                nt_name = _lexer.contents();
                _lexer.next();

                // was it an axis name?
                if (_lexer.current() == lex_double_colon)
                {
                    // parse axis name
                    if (axis_specified)
                        return error("Two axis specifiers in one step");

                    axis = parse_axis_name(nt_name, axis_specified);

                    if (!axis_specified)
                        return error("Unknown axis");

                    // read actual node test
                    _lexer.next();

                    if (_lexer.current() == lex_multiply)
                    {
                        nt_type = nodetest_all;
                        nt_name = xpath_lexer_string();
                        _lexer.next();
                    }
                    else if (_lexer.current() == lex_string)
                    {
                        nt_name = _lexer.contents();
                        _lexer.next();
                    }
                    else
                    {
                        return error("Unrecognized node test");
                    }
                }

                if (nt_type == nodetest_none)
                {
                    // node type test or processing-instruction
                    if (_lexer.current() == lex_open_brace)
                    {
                        _lexer.next();

                        if (_lexer.current() == lex_close_brace)
                        {
                            _lexer.next();

                            nt_type = parse_node_test_type(nt_name);

                            if (nt_type == nodetest_none)
                                return error("Unrecognized node type");

                            nt_name = xpath_lexer_string();
                        }
                        else if (nt_name == PUGIXML_TEXT("processing-instruction"))
                        {
                            if (_lexer.current() != lex_quoted_string)
                                return error("Only literals are allowed as arguments to processing-instruction()");

                            nt_type = nodetest_pi;
                            nt_name = _lexer.contents();
                            _lexer.next();

                            if (_lexer.current() != lex_close_brace)
                                return error("Unmatched brace near processing-instruction()");
                            _lexer.next();
                        }
                        else
                        {
                            return error("Unmatched brace near node type test");
                        }
                    }
                    // QName or NCName:*
                    else
                    {
                        if (nt_name.end - nt_name.begin > 2 && nt_name.end[-2] == ':' && nt_name.end[-1] == '*') // NCName:*
                        {
                            nt_name.end--; // erase *

                            nt_type = nodetest_all_in_namespace;
                        }
                        else
                        {
                            nt_type = nodetest_name;
                        }
                    }
                }
            }
            else if (_lexer.current() == lex_multiply)
            {
                nt_type = nodetest_all;
                _lexer.next();
            }
            else
            {
                return error("Unrecognized node test");
            }

            const char_t* nt_name_copy = alloc_string(nt_name);
            if (!nt_name_copy) return 0;

            xpath_ast_node* n = alloc_node(ast_step, set, axis, nt_type, nt_name_copy);
            if (!n) return 0;

            size_t old_depth = _depth;

            xpath_ast_node* last = 0;

            while (_lexer.current() == lex_open_square_brace)
            {
                _lexer.next();

                if (++_depth > xpath_ast_depth_limit)
                    return error_rec();

                xpath_ast_node* expr = parse_expression();
                if (!expr) return 0;

                xpath_ast_node* pred = alloc_node(ast_predicate, 0, expr, predicate_default);
                if (!pred) return 0;

                if (_lexer.current() != lex_close_square_brace)
                    return error("Expected ']' to match an opening '['");
                _lexer.next();

                if (last) last->set_next(pred);
                else n->set_right(pred);

                last = pred;
            }

            _depth = old_depth;

            return n;
        }

        // RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
        xpath_ast_node* parse_relative_location_path(xpath_ast_node* set)
        {
            xpath_ast_node* n = parse_step(set);
            if (!n) return 0;

            size_t old_depth = _depth;

            while (_lexer.current() == lex_slash || _lexer.current() == lex_double_slash)
            {
                lexeme_t l = _lexer.current();
                _lexer.next();

                if (l == lex_double_slash)
                {
                    n = alloc_node(ast_step, n, axis_descendant_or_self, nodetest_type_node, 0);
                    if (!n) return 0;

                    ++_depth;
                }

                if (++_depth > xpath_ast_depth_limit)
                    return error_rec();

                n = parse_step(n);
                if (!n) return 0;
            }

            _depth = old_depth;

            return n;
        }

        // LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
        // AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
        xpath_ast_node* parse_location_path()
        {
            if (_lexer.current() == lex_slash)
            {
                _lexer.next();

                xpath_ast_node* n = alloc_node(ast_step_root, xpath_type_node_set);
                if (!n) return 0;

                // relative location path can start from axis_attribute, dot, double_dot, multiply and string lexemes; any other lexeme means standalone root path
                lexeme_t l = _lexer.current();

                if (l == lex_string || l == lex_axis_attribute || l == lex_dot || l == lex_double_dot || l == lex_multiply)
                    return parse_relative_location_path(n);
                else
                    return n;
            }
            else if (_lexer.current() == lex_double_slash)
            {
                _lexer.next();

                xpath_ast_node* n = alloc_node(ast_step_root, xpath_type_node_set);
                if (!n) return 0;

                n = alloc_node(ast_step, n, axis_descendant_or_self, nodetest_type_node, 0);
                if (!n) return 0;

                return parse_relative_location_path(n);
            }

            // else clause moved outside of if because of bogus warning 'control may reach end of non-void function being inlined' in gcc 4.0.1
            return parse_relative_location_path(0);
        }

        // PathExpr ::= LocationPath
        //              | FilterExpr
        //              | FilterExpr '/' RelativeLocationPath
        //              | FilterExpr '//' RelativeLocationPath
        // UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
        // UnaryExpr ::= UnionExpr | '-' UnaryExpr
        xpath_ast_node* parse_path_or_unary_expression()
        {
            // Clarification.
            // PathExpr begins with either LocationPath or FilterExpr.
            // FilterExpr begins with PrimaryExpr
            // PrimaryExpr begins with '$' in case of it being a variable reference,
            // '(' in case of it being an expression, string literal, number constant or
            // function call.
            if (_lexer.current() == lex_var_ref || _lexer.current() == lex_open_brace ||
                _lexer.current() == lex_quoted_string || _lexer.current() == lex_number ||
                _lexer.current() == lex_string)
            {
                if (_lexer.current() == lex_string)
                {
                    // This is either a function call, or not - if not, we shall proceed with location path
                    const char_t* state = _lexer.state();

                    while (PUGI__IS_CHARTYPE(*state, ct_space)) ++state;

                    if (*state != '(')
                        return parse_location_path();

                    // This looks like a function call; however this still can be a node-test. Check it.
                    if (parse_node_test_type(_lexer.contents()) != nodetest_none)
                        return parse_location_path();
                }

                xpath_ast_node* n = parse_filter_expression();
                if (!n) return 0;

                if (_lexer.current() == lex_slash || _lexer.current() == lex_double_slash)
                {
                    lexeme_t l = _lexer.current();
                    _lexer.next();

                    if (l == lex_double_slash)
                    {
                        if (n->rettype() != xpath_type_node_set)
                            return error("Step has to be applied to node set");

                        n = alloc_node(ast_step, n, axis_descendant_or_self, nodetest_type_node, 0);
                        if (!n) return 0;
                    }

                    // select from location path
                    return parse_relative_location_path(n);
                }

                return n;
            }
            else if (_lexer.current() == lex_minus)
            {
                _lexer.next();

                // precedence 7+ - only parses union expressions
                xpath_ast_node* n = parse_expression(7);
                if (!n) return 0;

                return alloc_node(ast_op_negate, xpath_type_number, n);
            }
            else
            {
                return parse_location_path();
            }
        }

        struct binary_op_t
        {
            ast_type_t asttype;
            xpath_value_type rettype;
            int precedence;

            binary_op_t(): asttype(ast_unknown), rettype(xpath_type_none), precedence(0)
            {
            }

            binary_op_t(ast_type_t asttype_, xpath_value_type rettype_, int precedence_): asttype(asttype_), rettype(rettype_), precedence(precedence_)
            {
            }

            static binary_op_t parse(xpath_lexer& lexer)
            {
                switch (lexer.current())
                {
                case lex_string:
                    if (lexer.contents() == PUGIXML_TEXT("or"))
                        return binary_op_t(ast_op_or, xpath_type_boolean, 1);
                    else if (lexer.contents() == PUGIXML_TEXT("and"))
                        return binary_op_t(ast_op_and, xpath_type_boolean, 2);
                    else if (lexer.contents() == PUGIXML_TEXT("div"))
                        return binary_op_t(ast_op_divide, xpath_type_number, 6);
                    else if (lexer.contents() == PUGIXML_TEXT("mod"))
                        return binary_op_t(ast_op_mod, xpath_type_number, 6);
                    else
                        return binary_op_t();

                case lex_equal:
                    return binary_op_t(ast_op_equal, xpath_type_boolean, 3);

                case lex_not_equal:
                    return binary_op_t(ast_op_not_equal, xpath_type_boolean, 3);

                case lex_less:
                    return binary_op_t(ast_op_less, xpath_type_boolean, 4);

                case lex_greater:
                    return binary_op_t(ast_op_greater, xpath_type_boolean, 4);

                case lex_less_or_equal:
                    return binary_op_t(ast_op_less_or_equal, xpath_type_boolean, 4);

                case lex_greater_or_equal:
                    return binary_op_t(ast_op_greater_or_equal, xpath_type_boolean, 4);

                case lex_plus:
                    return binary_op_t(ast_op_add, xpath_type_number, 5);

                case lex_minus:
                    return binary_op_t(ast_op_subtract, xpath_type_number, 5);

                case lex_multiply:
                    return binary_op_t(ast_op_multiply, xpath_type_number, 6);

                case lex_union:
                    return binary_op_t(ast_op_union, xpath_type_node_set, 7);

                default:
                    return binary_op_t();
                }
            }
        };

        xpath_ast_node* parse_expression_rec(xpath_ast_node* lhs, int limit)
        {
            binary_op_t op = binary_op_t::parse(_lexer);

            while (op.asttype != ast_unknown && op.precedence >= limit)
            {
                _lexer.next();

                if (++_depth > xpath_ast_depth_limit)
                    return error_rec();

                xpath_ast_node* rhs = parse_path_or_unary_expression();
                if (!rhs) return 0;

                binary_op_t nextop = binary_op_t::parse(_lexer);

                while (nextop.asttype != ast_unknown && nextop.precedence > op.precedence)
                {
                    rhs = parse_expression_rec(rhs, nextop.precedence);
                    if (!rhs) return 0;

                    nextop = binary_op_t::parse(_lexer);
                }

                if (op.asttype == ast_op_union && (lhs->rettype() != xpath_type_node_set || rhs->rettype() != xpath_type_node_set))
                    return error("Union operator has to be applied to node sets");

                lhs = alloc_node(op.asttype, op.rettype, lhs, rhs);
                if (!lhs) return 0;

                op = binary_op_t::parse(_lexer);
            }

            return lhs;
        }

        // Expr ::= OrExpr
        // OrExpr ::= AndExpr | OrExpr 'or' AndExpr
        // AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
        // EqualityExpr ::= RelationalExpr
        //                  | EqualityExpr '=' RelationalExpr
        //                  | EqualityExpr '!=' RelationalExpr
        // RelationalExpr ::= AdditiveExpr
        //                    | RelationalExpr '<' AdditiveExpr
        //                    | RelationalExpr '>' AdditiveExpr
        //                    | RelationalExpr '<=' AdditiveExpr
        //                    | RelationalExpr '>=' AdditiveExpr
        // AdditiveExpr ::= MultiplicativeExpr
        //                  | AdditiveExpr '+' MultiplicativeExpr
        //                  | AdditiveExpr '-' MultiplicativeExpr
        // MultiplicativeExpr ::= UnaryExpr
        //                        | MultiplicativeExpr '*' UnaryExpr
        //                        | MultiplicativeExpr 'div' UnaryExpr
        //                        | MultiplicativeExpr 'mod' UnaryExpr
        xpath_ast_node* parse_expression(int limit = 0)
        {
            size_t old_depth = _depth;

            if (++_depth > xpath_ast_depth_limit)
                return error_rec();

            xpath_ast_node* n = parse_path_or_unary_expression();
            if (!n) return 0;

            n = parse_expression_rec(n, limit);

            _depth = old_depth;

            return n;
        }

        xpath_parser(const char_t* query, xpath_variable_set* variables, xpath_allocator* alloc, xpath_parse_result* result): _alloc(alloc), _lexer(query), _query(query), _variables(variables), _result(result), _depth(0)
        {
        }

        xpath_ast_node* parse()
        {
            xpath_ast_node* n = parse_expression();
            if (!n) return 0;

            assert(_depth == 0);

            // check if there are unparsed tokens left
            if (_lexer.current() != lex_eof)
                return error("Incorrect query");

            return n;
        }

        static xpath_ast_node* parse(const char_t* query, xpath_variable_set* variables, xpath_allocator* alloc, xpath_parse_result* result)
        {
            xpath_parser parser(query, variables, alloc, result);

            return parser.parse();
        }
    };

    struct xpath_query_impl
    {
        static xpath_query_impl* create()
        {
            void* memory = xml_memory::allocate(sizeof(xpath_query_impl));
            if (!memory) return 0;

            return new (memory) xpath_query_impl();
        }

        static void destroy(xpath_query_impl* impl)
        {
            // free all allocated pages
            impl->alloc.release();

            // free allocator memory (with the first page)
            xml_memory::deallocate(impl);
        }

        xpath_query_impl(): root(0), alloc(&block, &oom), oom(false)
        {
            block.next = 0;
            block.capacity = sizeof(block.data);
        }

        xpath_ast_node* root;
        xpath_allocator alloc;
        xpath_memory_block block;
        bool oom;
    };

    PUGI__FN impl::xpath_ast_node* evaluate_node_set_prepare(xpath_query_impl* impl)
    {
        if (!impl) return 0;

        if (impl->root->rettype() != xpath_type_node_set)
        {
        #ifdef PUGIXML_NO_EXCEPTIONS
            return 0;
        #else
            xpath_parse_result res;
            res.error = "Expression does not evaluate to node set";

            throw xpath_exception(res);
        #endif
        }

        return impl->root;
    }
PUGI__NS_END

namespace pugi
{
#ifndef PUGIXML_NO_EXCEPTIONS
    PUGI__FN xpath_exception::xpath_exception(const xpath_parse_result& result_): _result(result_)
    {
        assert(_result.error);
    }

    PUGI__FN const char* xpath_exception::what() const throw()
    {
        return _result.error;
    }

    PUGI__FN const xpath_parse_result& xpath_exception::result() const
    {
        return _result;
    }
#endif

    PUGI__FN xpath_node::xpath_node()
    {
    }

    PUGI__FN xpath_node::xpath_node(const xml_node& node_): _node(node_)
    {
    }

    PUGI__FN xpath_node::xpath_node(const xml_attribute& attribute_, const xml_node& parent_): _node(attribute_ ? parent_ : xml_node()), _attribute(attribute_)
    {
    }

    PUGI__FN xml_node xpath_node::node() const
    {
        return _attribute ? xml_node() : _node;
    }

    PUGI__FN xml_attribute xpath_node::attribute() const
    {
        return _attribute;
    }

    PUGI__FN xml_node xpath_node::parent() const
    {
        return _attribute ? _node : _node.parent();
    }

    PUGI__FN static void unspecified_bool_xpath_node(xpath_node***)
    {
    }

    PUGI__FN xpath_node::operator xpath_node::unspecified_bool_type() const
    {
        return (_node || _attribute) ? unspecified_bool_xpath_node : 0;
    }

    PUGI__FN bool xpath_node::operator!() const
    {
        return !(_node || _attribute);
    }

    PUGI__FN bool xpath_node::operator==(const xpath_node& n) const
    {
        return _node == n._node && _attribute == n._attribute;
    }

    PUGI__FN bool xpath_node::operator!=(const xpath_node& n) const
    {
        return _node != n._node || _attribute != n._attribute;
    }

#ifdef __BORLANDC__
    PUGI__FN bool operator&&(const xpath_node& lhs, bool rhs)
    {
        return (bool)lhs && rhs;
    }

    PUGI__FN bool operator||(const xpath_node& lhs, bool rhs)
    {
        return (bool)lhs || rhs;
    }
#endif

    PUGI__FN void xpath_node_set::_assign(const_iterator begin_, const_iterator end_, type_t type_)
    {
        assert(begin_ <= end_);

        size_t size_ = static_cast<size_t>(end_ - begin_);

        // use internal buffer for 0 or 1 elements, heap buffer otherwise
        xpath_node* storage = (size_ <= 1) ? _storage : static_cast<xpath_node*>(impl::xml_memory::allocate(size_ * sizeof(xpath_node)));

        if (!storage)
        {
        #ifdef PUGIXML_NO_EXCEPTIONS
            return;
        #else
            throw std::bad_alloc();
        #endif
        }

        // deallocate old buffer
        if (_begin != _storage)
            impl::xml_memory::deallocate(_begin);

        // size check is necessary because for begin_ = end_ = nullptr, memcpy is UB
        if (size_)
            memcpy(storage, begin_, size_ * sizeof(xpath_node));

        _begin = storage;
        _end = storage + size_;
        _type = type_;
    }

#ifdef PUGIXML_HAS_MOVE
    PUGI__FN void xpath_node_set::_move(xpath_node_set& rhs) PUGIXML_NOEXCEPT
    {
        _type = rhs._type;
        _storage[0] = rhs._storage[0];
        _begin = (rhs._begin == rhs._storage) ? _storage : rhs._begin;
        _end = _begin + (rhs._end - rhs._begin);

        rhs._type = type_unsorted;
        rhs._begin = rhs._storage;
        rhs._end = rhs._storage;
    }
#endif

    PUGI__FN xpath_node_set::xpath_node_set(): _type(type_unsorted), _begin(_storage), _end(_storage)
    {
    }

    PUGI__FN xpath_node_set::xpath_node_set(const_iterator begin_, const_iterator end_, type_t type_): _type(type_unsorted), _begin(_storage), _end(_storage)
    {
        _assign(begin_, end_, type_);
    }

    PUGI__FN xpath_node_set::~xpath_node_set()
    {
        if (_begin != _storage)
            impl::xml_memory::deallocate(_begin);
    }

    PUGI__FN xpath_node_set::xpath_node_set(const xpath_node_set& ns): _type(type_unsorted), _begin(_storage), _end(_storage)
    {
        _assign(ns._begin, ns._end, ns._type);
    }

    PUGI__FN xpath_node_set& xpath_node_set::operator=(const xpath_node_set& ns)
    {
        if (this == &ns) return *this;

        _assign(ns._begin, ns._end, ns._type);

        return *this;
    }

#ifdef PUGIXML_HAS_MOVE
    PUGI__FN xpath_node_set::xpath_node_set(xpath_node_set&& rhs) PUGIXML_NOEXCEPT: _type(type_unsorted), _begin(_storage), _end(_storage)
    {
        _move(rhs);
    }

    PUGI__FN xpath_node_set& xpath_node_set::operator=(xpath_node_set&& rhs) PUGIXML_NOEXCEPT
    {
        if (this == &rhs) return *this;

        if (_begin != _storage)
            impl::xml_memory::deallocate(_begin);

        _move(rhs);

        return *this;
    }
#endif

    PUGI__FN xpath_node_set::type_t xpath_node_set::type() const
    {
        return _type;
    }

    PUGI__FN size_t xpath_node_set::size() const
    {
        return _end - _begin;
    }

    PUGI__FN bool xpath_node_set::empty() const
    {
        return _begin == _end;
    }

    PUGI__FN const xpath_node& xpath_node_set::operator[](size_t index) const
    {
        assert(index < size());
        return _begin[index];
    }

    PUGI__FN xpath_node_set::const_iterator xpath_node_set::begin() const
    {
        return _begin;
    }

    PUGI__FN xpath_node_set::const_iterator xpath_node_set::end() const
    {
        return _end;
    }

    PUGI__FN void xpath_node_set::sort(bool reverse)
    {
        _type = impl::xpath_sort(_begin, _end, _type, reverse);
    }

    PUGI__FN xpath_node xpath_node_set::first() const
    {
        return impl::xpath_first(_begin, _end, _type);
    }

    PUGI__FN xpath_parse_result::xpath_parse_result(): error("Internal error"), offset(0)
    {
    }

    PUGI__FN xpath_parse_result::operator bool() const
    {
        return error == 0;
    }

    PUGI__FN const char* xpath_parse_result::description() const
    {
        return error ? error : "No error";
    }

    PUGI__FN xpath_variable::xpath_variable(xpath_value_type type_): _type(type_), _next(0)
    {
    }

    PUGI__FN const char_t* xpath_variable::name() const
    {
        switch (_type)
        {
        case xpath_type_node_set:
            return static_cast<const impl::xpath_variable_node_set*>(this)->name;

        case xpath_type_number:
            return static_cast<const impl::xpath_variable_number*>(this)->name;

        case xpath_type_string:
            return static_cast<const impl::xpath_variable_string*>(this)->name;

        case xpath_type_boolean:
            return static_cast<const impl::xpath_variable_boolean*>(this)->name;

        default:
            assert(false && "Invalid variable type"); // unreachable
            return 0;
        }
    }

    PUGI__FN xpath_value_type xpath_variable::type() const
    {
        return _type;
    }

    PUGI__FN bool xpath_variable::get_boolean() const
    {
        return (_type == xpath_type_boolean) ? static_cast<const impl::xpath_variable_boolean*>(this)->value : false;
    }

    PUGI__FN double xpath_variable::get_number() const
    {
        return (_type == xpath_type_number) ? static_cast<const impl::xpath_variable_number*>(this)->value : impl::gen_nan();
    }

    PUGI__FN const char_t* xpath_variable::get_string() const
    {
        const char_t* value = (_type == xpath_type_string) ? static_cast<const impl::xpath_variable_string*>(this)->value : 0;
        return value ? value : PUGIXML_TEXT("");
    }

    PUGI__FN const xpath_node_set& xpath_variable::get_node_set() const
    {
        return (_type == xpath_type_node_set) ? static_cast<const impl::xpath_variable_node_set*>(this)->value : impl::dummy_node_set;
    }

    PUGI__FN bool xpath_variable::set(bool value)
    {
        if (_type != xpath_type_boolean) return false;

        static_cast<impl::xpath_variable_boolean*>(this)->value = value;
        return true;
    }

    PUGI__FN bool xpath_variable::set(double value)
    {
        if (_type != xpath_type_number) return false;

        static_cast<impl::xpath_variable_number*>(this)->value = value;
        return true;
    }

    PUGI__FN bool xpath_variable::set(const char_t* value)
    {
        if (_type != xpath_type_string) return false;

        impl::xpath_variable_string* var = static_cast<impl::xpath_variable_string*>(this);

        // duplicate string
        size_t size = (impl::strlength(value) + 1) * sizeof(char_t);

        char_t* copy = static_cast<char_t*>(impl::xml_memory::allocate(size));
        if (!copy) return false;

        memcpy(copy, value, size);

        // replace old string
        if (var->value) impl::xml_memory::deallocate(var->value);
        var->value = copy;

        return true;
    }

    PUGI__FN bool xpath_variable::set(const xpath_node_set& value)
    {
        if (_type != xpath_type_node_set) return false;

        static_cast<impl::xpath_variable_node_set*>(this)->value = value;
        return true;
    }

    PUGI__FN xpath_variable_set::xpath_variable_set()
    {
        for (size_t i = 0; i < sizeof(_data) / sizeof(_data[0]); ++i)
            _data[i] = 0;
    }

    PUGI__FN xpath_variable_set::~xpath_variable_set()
    {
        for (size_t i = 0; i < sizeof(_data) / sizeof(_data[0]); ++i)
            _destroy(_data[i]);
    }

    PUGI__FN xpath_variable_set::xpath_variable_set(const xpath_variable_set& rhs)
    {
        for (size_t i = 0; i < sizeof(_data) / sizeof(_data[0]); ++i)
            _data[i] = 0;

        _assign(rhs);
    }

    PUGI__FN xpath_variable_set& xpath_variable_set::operator=(const xpath_variable_set& rhs)
    {
        if (this == &rhs) return *this;

        _assign(rhs);

        return *this;
    }

#ifdef PUGIXML_HAS_MOVE
    PUGI__FN xpath_variable_set::xpath_variable_set(xpath_variable_set&& rhs) PUGIXML_NOEXCEPT
    {
        for (size_t i = 0; i < sizeof(_data) / sizeof(_data[0]); ++i)
        {
            _data[i] = rhs._data[i];
            rhs._data[i] = 0;
        }
    }

    PUGI__FN xpath_variable_set& xpath_variable_set::operator=(xpath_variable_set&& rhs) PUGIXML_NOEXCEPT
    {
        for (size_t i = 0; i < sizeof(_data) / sizeof(_data[0]); ++i)
        {
            _destroy(_data[i]);

            _data[i] = rhs._data[i];
            rhs._data[i] = 0;
        }

        return *this;
    }
#endif

    PUGI__FN void xpath_variable_set::_assign(const xpath_variable_set& rhs)
    {
        xpath_variable_set temp;

        for (size_t i = 0; i < sizeof(_data) / sizeof(_data[0]); ++i)
            if (rhs._data[i] && !_clone(rhs._data[i], &temp._data[i]))
                return;

        _swap(temp);
    }

    PUGI__FN void xpath_variable_set::_swap(xpath_variable_set& rhs)
    {
        for (size_t i = 0; i < sizeof(_data) / sizeof(_data[0]); ++i)
        {
            xpath_variable* chain = _data[i];

            _data[i] = rhs._data[i];
            rhs._data[i] = chain;
        }
    }

    PUGI__FN xpath_variable* xpath_variable_set::_find(const char_t* name) const
    {
        const size_t hash_size = sizeof(_data) / sizeof(_data[0]);
        size_t hash = impl::hash_string(name) % hash_size;

        // look for existing variable
        for (xpath_variable* var = _data[hash]; var; var = var->_next)
            if (impl::strequal(var->name(), name))
                return var;

        return 0;
    }

    PUGI__FN bool xpath_variable_set::_clone(xpath_variable* var, xpath_variable** out_result)
    {
        xpath_variable* last = 0;

        while (var)
        {
            // allocate storage for new variable
            xpath_variable* nvar = impl::new_xpath_variable(var->_type, var->name());
            if (!nvar) return false;

            // link the variable to the result immediately to handle failures gracefully
            if (last)
                last->_next = nvar;
            else
                *out_result = nvar;

            last = nvar;

            // copy the value; this can fail due to out-of-memory conditions
            if (!impl::copy_xpath_variable(nvar, var)) return false;

            var = var->_next;
        }

        return true;
    }

    PUGI__FN void xpath_variable_set::_destroy(xpath_variable* var)
    {
        while (var)
        {
            xpath_variable* next = var->_next;

            impl::delete_xpath_variable(var->_type, var);

            var = next;
        }
    }

    PUGI__FN xpath_variable* xpath_variable_set::add(const char_t* name, xpath_value_type type)
    {
        const size_t hash_size = sizeof(_data) / sizeof(_data[0]);
        size_t hash = impl::hash_string(name) % hash_size;

        // look for existing variable
        for (xpath_variable* var = _data[hash]; var; var = var->_next)
            if (impl::strequal(var->name(), name))
                return var->type() == type ? var : 0;

        // add new variable
        xpath_variable* result = impl::new_xpath_variable(type, name);

        if (result)
        {
            result->_next = _data[hash];

            _data[hash] = result;
        }

        return result;
    }

    PUGI__FN bool xpath_variable_set::set(const char_t* name, bool value)
    {
        xpath_variable* var = add(name, xpath_type_boolean);
        return var ? var->set(value) : false;
    }

    PUGI__FN bool xpath_variable_set::set(const char_t* name, double value)
    {
        xpath_variable* var = add(name, xpath_type_number);
        return var ? var->set(value) : false;
    }

    PUGI__FN bool xpath_variable_set::set(const char_t* name, const char_t* value)
    {
        xpath_variable* var = add(name, xpath_type_string);
        return var ? var->set(value) : false;
    }

    PUGI__FN bool xpath_variable_set::set(const char_t* name, const xpath_node_set& value)
    {
        xpath_variable* var = add(name, xpath_type_node_set);
        return var ? var->set(value) : false;
    }

    PUGI__FN xpath_variable* xpath_variable_set::get(const char_t* name)
    {
        return _find(name);
    }

    PUGI__FN const xpath_variable* xpath_variable_set::get(const char_t* name) const
    {
        return _find(name);
    }

    PUGI__FN xpath_query::xpath_query(const char_t* query, xpath_variable_set* variables): _impl(0)
    {
        impl::xpath_query_impl* qimpl = impl::xpath_query_impl::create();

        if (!qimpl)
        {
        #ifdef PUGIXML_NO_EXCEPTIONS
            _result.error = "Out of memory";
        #else
            throw std::bad_alloc();
        #endif
        }
        else
        {
            using impl::auto_deleter; // MSVC7 workaround
            auto_deleter<impl::xpath_query_impl> impl(qimpl, impl::xpath_query_impl::destroy);

            qimpl->root = impl::xpath_parser::parse(query, variables, &qimpl->alloc, &_result);

            if (qimpl->root)
            {
                qimpl->root->optimize(&qimpl->alloc);

                _impl = impl.release();
                _result.error = 0;
            }
            else
            {
            #ifdef PUGIXML_NO_EXCEPTIONS
                if (qimpl->oom) _result.error = "Out of memory";
            #else
                if (qimpl->oom) throw std::bad_alloc();
                throw xpath_exception(_result);
            #endif
            }
        }
    }

    PUGI__FN xpath_query::xpath_query(): _impl(0)
    {
    }

    PUGI__FN xpath_query::~xpath_query()
    {
        if (_impl)
            impl::xpath_query_impl::destroy(static_cast<impl::xpath_query_impl*>(_impl));
    }

#ifdef PUGIXML_HAS_MOVE
    PUGI__FN xpath_query::xpath_query(xpath_query&& rhs) PUGIXML_NOEXCEPT
    {
        _impl = rhs._impl;
        _result = rhs._result;
        rhs._impl = 0;
        rhs._result = xpath_parse_result();
    }

    PUGI__FN xpath_query& xpath_query::operator=(xpath_query&& rhs) PUGIXML_NOEXCEPT
    {
        if (this == &rhs) return *this;

        if (_impl)
            impl::xpath_query_impl::destroy(static_cast<impl::xpath_query_impl*>(_impl));

        _impl = rhs._impl;
        _result = rhs._result;
        rhs._impl = 0;
        rhs._result = xpath_parse_result();

        return *this;
    }
#endif

    PUGI__FN xpath_value_type xpath_query::return_type() const
    {
        if (!_impl) return xpath_type_none;

        return static_cast<impl::xpath_query_impl*>(_impl)->root->rettype();
    }

    PUGI__FN bool xpath_query::evaluate_boolean(const xpath_node& n) const
    {
        if (!_impl) return false;

        impl::xpath_context c(n, 1, 1);
        impl::xpath_stack_data sd;

        bool r = static_cast<impl::xpath_query_impl*>(_impl)->root->eval_boolean(c, sd.stack);

        if (sd.oom)
        {
        #ifdef PUGIXML_NO_EXCEPTIONS
            return false;
        #else
            throw std::bad_alloc();
        #endif
        }

        return r;
    }

    PUGI__FN double xpath_query::evaluate_number(const xpath_node& n) const
    {
        if (!_impl) return impl::gen_nan();

        impl::xpath_context c(n, 1, 1);
        impl::xpath_stack_data sd;

        double r = static_cast<impl::xpath_query_impl*>(_impl)->root->eval_number(c, sd.stack);

        if (sd.oom)
        {
        #ifdef PUGIXML_NO_EXCEPTIONS
            return impl::gen_nan();
        #else
            throw std::bad_alloc();
        #endif
        }

        return r;
    }

#ifndef PUGIXML_NO_STL
    PUGI__FN string_t xpath_query::evaluate_string(const xpath_node& n) const
    {
        if (!_impl) return string_t();

        impl::xpath_context c(n, 1, 1);
        impl::xpath_stack_data sd;

        impl::xpath_string r = static_cast<impl::xpath_query_impl*>(_impl)->root->eval_string(c, sd.stack);

        if (sd.oom)
        {
        #ifdef PUGIXML_NO_EXCEPTIONS
            return string_t();
        #else
            throw std::bad_alloc();
        #endif
        }

        return string_t(r.c_str(), r.length());
    }
#endif

    PUGI__FN size_t xpath_query::evaluate_string(char_t* buffer, size_t capacity, const xpath_node& n) const
    {
        impl::xpath_context c(n, 1, 1);
        impl::xpath_stack_data sd;

        impl::xpath_string r = _impl ? static_cast<impl::xpath_query_impl*>(_impl)->root->eval_string(c, sd.stack) : impl::xpath_string();

        if (sd.oom)
        {
        #ifdef PUGIXML_NO_EXCEPTIONS
            r = impl::xpath_string();
        #else
            throw std::bad_alloc();
        #endif
        }

        size_t full_size = r.length() + 1;

        if (capacity > 0)
        {
            size_t size = (full_size < capacity) ? full_size : capacity;
            assert(size > 0);

            memcpy(buffer, r.c_str(), (size - 1) * sizeof(char_t));
            buffer[size - 1] = 0;
        }

        return full_size;
    }

    PUGI__FN xpath_node_set xpath_query::evaluate_node_set(const xpath_node& n) const
    {
        impl::xpath_ast_node* root = impl::evaluate_node_set_prepare(static_cast<impl::xpath_query_impl*>(_impl));
        if (!root) return xpath_node_set();

        impl::xpath_context c(n, 1, 1);
        impl::xpath_stack_data sd;

        impl::xpath_node_set_raw r = root->eval_node_set(c, sd.stack, impl::nodeset_eval_all);

        if (sd.oom)
        {
        #ifdef PUGIXML_NO_EXCEPTIONS
            return xpath_node_set();
        #else
            throw std::bad_alloc();
        #endif
        }

        return xpath_node_set(r.begin(), r.end(), r.type());
    }

    PUGI__FN xpath_node xpath_query::evaluate_node(const xpath_node& n) const
    {
        impl::xpath_ast_node* root = impl::evaluate_node_set_prepare(static_cast<impl::xpath_query_impl*>(_impl));
        if (!root) return xpath_node();

        impl::xpath_context c(n, 1, 1);
        impl::xpath_stack_data sd;

        impl::xpath_node_set_raw r = root->eval_node_set(c, sd.stack, impl::nodeset_eval_first);

        if (sd.oom)
        {
        #ifdef PUGIXML_NO_EXCEPTIONS
            return xpath_node();
        #else
            throw std::bad_alloc();
        #endif
        }

        return r.first();
    }

    PUGI__FN const xpath_parse_result& xpath_query::result() const
    {
        return _result;
    }

    PUGI__FN static void unspecified_bool_xpath_query(xpath_query***)
    {
    }

    PUGI__FN xpath_query::operator xpath_query::unspecified_bool_type() const
    {
        return _impl ? unspecified_bool_xpath_query : 0;
    }

    PUGI__FN bool xpath_query::operator!() const
    {
        return !_impl;
    }

    PUGI__FN xpath_node xml_node::select_node(const char_t* query, xpath_variable_set* variables) const
    {
        xpath_query q(query, variables);
        return q.evaluate_node(*this);
    }

    PUGI__FN xpath_node xml_node::select_node(const xpath_query& query) const
    {
        return query.evaluate_node(*this);
    }

    PUGI__FN xpath_node_set xml_node::select_nodes(const char_t* query, xpath_variable_set* variables) const
    {
        xpath_query q(query, variables);
        return q.evaluate_node_set(*this);
    }

    PUGI__FN xpath_node_set xml_node::select_nodes(const xpath_query& query) const
    {
        return query.evaluate_node_set(*this);
    }

    PUGI__FN xpath_node xml_node::select_single_node(const char_t* query, xpath_variable_set* variables) const
    {
        xpath_query q(query, variables);
        return q.evaluate_node(*this);
    }

    PUGI__FN xpath_node xml_node::select_single_node(const xpath_query& query) const
    {
        return query.evaluate_node(*this);
    }
}

#endif

#ifdef __BORLANDC__
#   pragma option pop
#endif

// Intel C++ does not properly keep warning state for function templates,
// so popping warning state at the end of translation unit leads to warnings in the middle.
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#   pragma warning(pop)
#endif

#if defined(_MSC_VER) && defined(__c2__)
#   pragma clang diagnostic pop
#endif

// Undefine all local macros (makes sure we're not leaking macros in header-only mode)
#undef PUGI__NO_INLINE
#undef PUGI__UNLIKELY
#undef PUGI__STATIC_ASSERT
#undef PUGI__DMC_VOLATILE
#undef PUGI__UNSIGNED_OVERFLOW
#undef PUGI__MSVC_CRT_VERSION
#undef PUGI__SNPRINTF
#undef PUGI__NS_BEGIN
#undef PUGI__NS_END
#undef PUGI__FN
#undef PUGI__FN_NO_INLINE
#undef PUGI__GETHEADER_IMPL
#undef PUGI__GETPAGE_IMPL
#undef PUGI__GETPAGE
#undef PUGI__NODETYPE
#undef PUGI__IS_CHARTYPE_IMPL
#undef PUGI__IS_CHARTYPE
#undef PUGI__IS_CHARTYPEX
#undef PUGI__ENDSWITH
#undef PUGI__SKIPWS
#undef PUGI__OPTSET
#undef PUGI__PUSHNODE
#undef PUGI__POPNODE
#undef PUGI__SCANFOR
#undef PUGI__SCANWHILE
#undef PUGI__SCANWHILE_UNROLL
#undef PUGI__ENDSEG
#undef PUGI__THROW_ERROR
#undef PUGI__CHECK_ERROR

/**
 * Copyright (c) 2006-2022 Arseny Kapoulkine
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#endif

/**
 * Copyright (c) 2006-2022 Arseny Kapoulkine
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ---------------------------------------------------------------------------
// miniz

/* miniz.c v1.15 - public domain deflate/inflate, zlib-subset, ZIP reading/writing/appending, PNG writing
   See "unlicense" statement at the end of this file.
   Rich Geldreich <richgel99@gmail.com>, last updated Oct. 13, 2013
   Implements RFC 1950: http://www.ietf.org/rfc/rfc1950.txt and RFC 1951: http://www.ietf.org/rfc/rfc1951.txt

   Most API's defined in miniz.c are optional. For example, to disable the archive related functions just define
   MINIZ_NO_ARCHIVE_APIS, or to get rid of all stdio usage define MINIZ_NO_STDIO (see the list below for more macros).

   * Change History
     10/13/13 v1.15 r4 - Interim bugfix release while I work on the next major release with Zip64 support (almost there!):
       - Critical fix for the MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY bug (thanks kahmyong.moon@hp.com) which could cause locate files to not find files. This bug
        would only have occured in earlier versions if you explicitly used this flag, OR if you used mz_zip_extract_archive_file_to_heap() or mz_zip_add_mem_to_archive_file_in_place()
        (which used this flag). If you can't switch to v1.15 but want to fix this bug, just remove the uses of this flag from both helper funcs (and of course don't use the flag).
       - Bugfix in mz_zip_reader_extract_to_mem_no_alloc() from kymoon when pUser_read_buf is not NULL and compressed size is > uncompressed size
       - Fixing mz_zip_reader_extract_*() funcs so they don't try to extract compressed data from directory entries, to account for weird zipfiles which contain zero-size compressed data on dir entries.
         Hopefully this fix won't cause any issues on weird zip archives, because it assumes the low 16-bits of zip external attributes are DOS attributes (which I believe they always are in practice).
       - Fixing mz_zip_reader_is_file_a_directory() so it doesn't check the internal attributes, just the filename and external attributes
       - mz_zip_reader_init_file() - missing MZ_FCLOSE() call if the seek failed
       - Added cmake support for Linux builds which builds all the examples, tested with clang v3.3 and gcc v4.6.
       - Clang fix for tdefl_write_image_to_png_file_in_memory() from toffaletti
       - Merged MZ_FORCEINLINE fix from hdeanclark
       - Fix <time.h> include before config #ifdef, thanks emil.brink
       - Added tdefl_write_image_to_png_file_in_memory_ex(): supports Y flipping (super useful for OpenGL apps), and explicit control over the compression level (so you can
        set it to 1 for real-time compression).
       - Merged in some compiler fixes from paulharris's github repro.
       - Retested this build under Windows (VS 2010, including static analysis), tcc  0.9.26, gcc v4.6 and clang v3.3.
       - Added example6.c, which dumps an image of the mandelbrot set to a PNG file.
       - Modified example2 to help test the MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY flag more.
       - In r3: Bugfix to mz_zip_writer_add_file() found during merge: Fix possible src file fclose() leak if alignment bytes+local header file write faiiled
       - In r4: Minor bugfix to mz_zip_writer_add_from_zip_reader(): Was pushing the wrong central dir header offset, appears harmless in this release, but it became a problem in the zip64 branch
     5/20/12 v1.14 - MinGW32/64 GCC 4.6.1 compiler fixes: added MZ_FORCEINLINE, #include <time.h> (thanks fermtect).
     5/19/12 v1.13 - From jason@cornsyrup.org and kelwert@mtu.edu - Fix mz_crc32() so it doesn't compute the wrong CRC-32's when mz_ulong is 64-bit.
       - Temporarily/locally slammed in "typedef unsigned long mz_ulong" and re-ran a randomized regression test on ~500k files.
       - Eliminated a bunch of warnings when compiling with GCC 32-bit/64.
       - Ran all examples, miniz.c, and tinfl.c through MSVC 2008's /analyze (static analysis) option and fixed all warnings (except for the silly
        "Use of the comma-operator in a tested expression.." analysis warning, which I purposely use to work around a MSVC compiler warning).
       - Created 32-bit and 64-bit Codeblocks projects/workspace. Built and tested Linux executables. The codeblocks workspace is compatible with Linux+Win32/x64.
       - Added miniz_tester solution/project, which is a useful little app derived from LZHAM's tester app that I use as part of the regression test.
       - Ran miniz.c and tinfl.c through another series of regression testing on ~500,000 files and archives.
       - Modified example5.c so it purposely disables a bunch of high-level functionality (MINIZ_NO_STDIO, etc.). (Thanks to corysama for the MINIZ_NO_STDIO bug report.)
       - Fix ftell() usage in examples so they exit with an error on files which are too large (a limitation of the examples, not miniz itself).
     4/12/12 v1.12 - More comments, added low-level example5.c, fixed a couple minor level_and_flags issues in the archive API's.
      level_and_flags can now be set to MZ_DEFAULT_COMPRESSION. Thanks to Bruce Dawson <bruced@valvesoftware.com> for the feedback/bug report.
     5/28/11 v1.11 - Added statement from unlicense.org
     5/27/11 v1.10 - Substantial compressor optimizations:
      - Level 1 is now ~4x faster than before. The L1 compressor's throughput now varies between 70-110MB/sec. on a
      - Core i7 (actual throughput varies depending on the type of data, and x64 vs. x86).
      - Improved baseline L2-L9 compression perf. Also, greatly improved compression perf. issues on some file types.
      - Refactored the compression code for better readability and maintainability.
      - Added level 10 compression level (L10 has slightly better ratio than level 9, but could have a potentially large
       drop in throughput on some files).
     5/15/11 v1.09 - Initial stable release.

   * Low-level Deflate/Inflate implementation notes:

     Compression: Use the "tdefl" API's. The compressor supports raw, static, and dynamic blocks, lazy or
     greedy parsing, match length filtering, RLE-only, and Huffman-only streams. It performs and compresses
     approximately as well as zlib.

     Decompression: Use the "tinfl" API's. The entire decompressor is implemented as a single function
     coroutine: see tinfl_decompress(). It supports decompression into a 32KB (or larger power of 2) wrapping buffer, or into a memory
     block large enough to hold the entire file.

     The low-level tdefl/tinfl API's do not make any use of dynamic memory allocation.

   * zlib-style API notes:

     miniz.c implements a fairly large subset of zlib. There's enough functionality present for it to be a drop-in
     zlib replacement in many apps:
        The z_stream struct, optional memory allocation callbacks
        deflateInit/deflateInit2/deflate/deflateReset/deflateEnd/deflateBound
        inflateInit/inflateInit2/inflate/inflateEnd
        compress, compress2, compressBound, uncompress
        CRC-32, Adler-32 - Using modern, minimal code size, CPU cache friendly routines.
        Supports raw deflate streams or standard zlib streams with adler-32 checking.

     Limitations:
      The callback API's are not implemented yet. No support for gzip headers or zlib static dictionaries.
      I've tried to closely emulate zlib's various flavors of stream flushing and return status codes, but
      there are no guarantees that miniz.c pulls this off perfectly.

   * PNG writing: See the tdefl_write_image_to_png_file_in_memory() function, originally written by
     Alex Evans. Supports 1-4 bytes/pixel images.

   * ZIP archive API notes:

     The ZIP archive API's where designed with simplicity and efficiency in mind, with just enough abstraction to
     get the job done with minimal fuss. There are simple API's to retrieve file information, read files from
     existing archives, create new archives, append new files to existing archives, or clone archive data from
     one archive to another. It supports archives located in memory or the heap, on disk (using stdio.h),
     or you can specify custom file read/write callbacks.

     - Archive reading: Just call this function to read a single file from a disk archive:

      void *mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name,
        size_t *pSize, mz_uint zip_flags);

     For more complex cases, use the "mz_zip_reader" functions. Upon opening an archive, the entire central
     directory is located and read as-is into memory, and subsequent file access only occurs when reading individual files.

     - Archives file scanning: The simple way is to use this function to scan a loaded archive for a specific file:

     int mz_zip_reader_locate_file(mz_zip_archive *pZip, const char *pName, const char *pComment, mz_uint flags);

     The locate operation can optionally check file comments too, which (as one example) can be used to identify
     multiple versions of the same file in an archive. This function uses a simple linear search through the central
     directory, so it's not very fast.

     Alternately, you can iterate through all the files in an archive (using mz_zip_reader_get_num_files()) and
     retrieve detailed info on each file by calling mz_zip_reader_file_stat().

     - Archive creation: Use the "mz_zip_writer" functions. The ZIP writer immediately writes compressed file data
     to disk and builds an exact image of the central directory in memory. The central directory image is written
     all at once at the end of the archive file when the archive is finalized.

     The archive writer can optionally align each file's local header and file data to any power of 2 alignment,
     which can be useful when the archive will be read from optical media. Also, the writer supports placing
     arbitrary data blobs at the very beginning of ZIP archives. Archives written using either feature are still
     readable by any ZIP tool.

     - Archive appending: The simple way to add a single file to an archive is to call this function:

      mz_bool mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename, const char *pArchive_name,
        const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags);

     The archive will be created if it doesn't already exist, otherwise it'll be appended to.
     Note the appending is done in-place and is not an atomic operation, so if something goes wrong
     during the operation it's possible the archive could be left without a central directory (although the local
     file headers and file data will be fine, so the archive will be recoverable).

     For more complex archive modification scenarios:
     1. The safest way is to use a mz_zip_reader to read the existing archive, cloning only those bits you want to
     preserve into a new archive using using the mz_zip_writer_add_from_zip_reader() function (which compiles the
     compressed file data as-is). When you're done, delete the old archive and rename the newly written archive, and
     you're done. This is safe but requires a bunch of temporary disk space or heap memory.

     2. Or, you can convert an mz_zip_reader in-place to an mz_zip_writer using mz_zip_writer_init_from_reader(),
     append new files as needed, then finalize the archive which will write an updated central directory to the
     original archive. (This is basically what mz_zip_add_mem_to_archive_file_in_place() does.) There's a
     possibility that the archive's central directory could be lost with this method if anything goes wrong, though.

     - ZIP archive support limitations:
     No zip64 or spanning support. Extraction functions can only handle unencrypted, stored or deflated files.
     Requires streams capable of seeking.

   * This is a header file library, like stb_image.c. To get only a header file, either cut and paste the
     below header, or create miniz.h, #define MINIZ_HEADER_FILE_ONLY, and then include miniz.c from it.

   * Important: For best perf. be sure to customize the below macros for your target platform:
     #define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
     #define MINIZ_LITTLE_ENDIAN 1
     #define MINIZ_HAS_64BIT_REGISTERS 1

   * On platforms using glibc, Be sure to "#define _LARGEFILE64_SOURCE 1" before including miniz.c to ensure miniz
     uses the 64-bit variants: fopen64(), stat64(), etc. Otherwise you won't be able to process large files
     (i.e. 32-bit stat() fails for me on files > 0x7FFFFFFF bytes).
*/

#ifndef MINIZ_HEADER_INCLUDED
#define MINIZ_HEADER_INCLUDED

#include <stdlib.h>

// Defines to completely disable specific portions of miniz.c:
// If all macros here are defined the only functionality remaining will be CRC-32, adler-32, tinfl, and tdefl.

// Define MINIZ_NO_STDIO to disable all usage and any functions which rely on stdio for file I/O.
//#define MINIZ_NO_STDIO

// If MINIZ_NO_TIME is specified then the ZIP archive functions will not be able to get the current time, or
// get/set file times, and the C run-time funcs that get/set times won't be called.
// The current downside is the times written to your archives will be from 1979.
//#define MINIZ_NO_TIME

// Define MINIZ_NO_ARCHIVE_APIS to disable all ZIP archive API's.
//#define MINIZ_NO_ARCHIVE_APIS

// Define MINIZ_NO_ARCHIVE_APIS to disable all writing related ZIP archive API's.
//#define MINIZ_NO_ARCHIVE_WRITING_APIS

// Define MINIZ_NO_ZLIB_APIS to remove all ZLIB-style compression/decompression API's.
//#define MINIZ_NO_ZLIB_APIS

// Define MINIZ_NO_ZLIB_COMPATIBLE_NAME to disable zlib names, to prevent conflicts against stock zlib.
//#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES

// Define MINIZ_NO_MALLOC to disable all calls to malloc, free, and realloc.
// Note if MINIZ_NO_MALLOC is defined then the user must always provide custom user alloc/free/realloc
// callbacks to the zlib and archive API's, and a few stand-alone helper API's which don't provide custom user
// functions (such as tdefl_compress_mem_to_heap() and tinfl_decompress_mem_to_heap()) won't work.
//#define MINIZ_NO_MALLOC

#if defined(__TINYC__) && (defined(__linux) || defined(__linux__))
  // TODO: Work around "error: include file 'sys\utime.h' when compiling with tcc on Linux
  #define MINIZ_NO_TIME
#endif

#if !defined(MINIZ_NO_TIME) && !defined(MINIZ_NO_ARCHIVE_APIS)
  #include <time.h>
#endif

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
// MINIZ_X86_OR_X64_CPU is only used to help set the below macros.
#define MINIZ_X86_OR_X64_CPU 1
#endif

#if (__BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__) || MINIZ_X86_OR_X64_CPU
// Set MINIZ_LITTLE_ENDIAN to 1 if the processor is little endian.
#define MINIZ_LITTLE_ENDIAN 1
#endif

#if MINIZ_X86_OR_X64_CPU
// Set MINIZ_USE_UNALIGNED_LOADS_AND_STORES to 1 on CPU's that permit efficient integer loads and stores from unaligned addresses.
#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
#endif

#if defined(_M_X64) || defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__) || defined(__ia64__) || defined(__x86_64__)
// Set MINIZ_HAS_64BIT_REGISTERS to 1 if operations on 64-bit integers are reasonably fast (and don't involve compiler generated calls to helper functions).
#define MINIZ_HAS_64BIT_REGISTERS 1
#endif

#ifdef __APPLE__
#define ftello64 ftello
#define fseeko64 fseeko
#define fopen64 fopen
#define freopen64 freopen
//#define stat64 stat
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ------------------- zlib-style API Definitions.

// For more compatibility with zlib, miniz.c uses unsigned long for some parameters/struct members. Beware: mz_ulong can be either 32 or 64-bits!
typedef unsigned long mz_ulong;

// mz_free() internally uses the MZ_FREE() macro (which by default calls free() unless you've modified the MZ_MALLOC macro) to release a block allocated from the heap.
void mz_free(void *p);

#define MZ_ADLER32_INIT (1)
// mz_adler32() returns the initial adler-32 value to use when called with ptr==NULL.
mz_ulong mz_adler32(mz_ulong adler, const unsigned char *ptr, size_t buf_len);

#define MZ_CRC32_INIT (0)
// mz_crc32() returns the initial CRC-32 value to use when called with ptr==NULL.
mz_ulong mz_crc32(mz_ulong crc, const unsigned char *ptr, size_t buf_len);

// Compression strategies.
enum { MZ_DEFAULT_STRATEGY = 0, MZ_FILTERED = 1, MZ_HUFFMAN_ONLY = 2, MZ_RLE = 3, MZ_FIXED = 4 };

// Method
#define MZ_DEFLATED 8

#ifndef MINIZ_NO_ZLIB_APIS

// Heap allocation callbacks.
// Note that mz_alloc_func parameter types purpsosely differ from zlib's: items/size is size_t, not unsigned long.
typedef void *(*mz_alloc_func)(void *opaque, size_t items, size_t size);
typedef void (*mz_free_func)(void *opaque, void *address);
typedef void *(*mz_realloc_func)(void *opaque, void *address, size_t items, size_t size);

#define MZ_VERSION          "9.1.15"
#define MZ_VERNUM           0x91F0
#define MZ_VER_MAJOR        9
#define MZ_VER_MINOR        1
#define MZ_VER_REVISION     15
#define MZ_VER_SUBREVISION  0

// Flush values. For typical usage you only need MZ_NO_FLUSH and MZ_FINISH. The other values are for advanced use (refer to the zlib docs).
enum { MZ_NO_FLUSH = 0, MZ_PARTIAL_FLUSH = 1, MZ_SYNC_FLUSH = 2, MZ_FULL_FLUSH = 3, MZ_FINISH = 4, MZ_BLOCK = 5 };

// Return status codes. MZ_PARAM_ERROR is non-standard.
enum { MZ_OK = 0, MZ_STREAM_END = 1, MZ_NEED_DICT = 2, MZ_ERRNO = -1, MZ_STREAM_ERROR = -2, MZ_DATA_ERROR = -3, MZ_MEM_ERROR = -4, MZ_BUF_ERROR = -5, MZ_VERSION_ERROR = -6, MZ_PARAM_ERROR = -10000 };

// Compression levels: 0-9 are the standard zlib-style levels, 10 is best possible compression (not zlib compatible, and may be very slow), MZ_DEFAULT_COMPRESSION=MZ_DEFAULT_LEVEL.
enum { MZ_NO_COMPRESSION = 0, MZ_BEST_SPEED = 1, MZ_BEST_COMPRESSION = 9, MZ_UBER_COMPRESSION = 10, MZ_DEFAULT_LEVEL = 6, MZ_DEFAULT_COMPRESSION = -1 };

// Window bits
#define MZ_DEFAULT_WINDOW_BITS 15

struct mz_internal_state;

// Compression/decompression stream struct.
typedef struct mz_stream_s
{
  const unsigned char *next_in;     // pointer to next byte to read
  unsigned int avail_in;            // number of bytes available at next_in
  mz_ulong total_in;                // total number of bytes consumed so far

  unsigned char *next_out;          // pointer to next byte to write
  unsigned int avail_out;           // number of bytes that can be written to next_out
  mz_ulong total_out;               // total number of bytes produced so far

  char *msg;                        // error msg (unused)
  struct mz_internal_state *state;  // internal state, allocated by zalloc/zfree

  mz_alloc_func zalloc;             // optional heap allocation function (defaults to malloc)
  mz_free_func zfree;               // optional heap free function (defaults to free)
  void *opaque;                     // heap alloc function user pointer

  int data_type;                    // data_type (unused)
  mz_ulong adler;                   // adler32 of the source or uncompressed data
  mz_ulong reserved;                // not used
} mz_stream;

typedef mz_stream *mz_streamp;

// Returns the version string of miniz.c.
const char *mz_version(void);

// mz_deflateInit() initializes a compressor with default options:
// Parameters:
//  pStream must point to an initialized mz_stream struct.
//  level must be between [MZ_NO_COMPRESSION, MZ_BEST_COMPRESSION].
//  level 1 enables a specially optimized compression function that's been optimized purely for performance, not ratio.
//  (This special func. is currently only enabled when MINIZ_USE_UNALIGNED_LOADS_AND_STORES and MINIZ_LITTLE_ENDIAN are defined.)
// Return values:
//  MZ_OK on success.
//  MZ_STREAM_ERROR if the stream is bogus.
//  MZ_PARAM_ERROR if the input parameters are bogus.
//  MZ_MEM_ERROR on out of memory.
int mz_deflateInit(mz_streamp pStream, int level);

// mz_deflateInit2() is like mz_deflate(), except with more control:
// Additional parameters:
//   method must be MZ_DEFLATED
//   window_bits must be MZ_DEFAULT_WINDOW_BITS (to wrap the deflate stream with zlib header/adler-32 footer) or -MZ_DEFAULT_WINDOW_BITS (raw deflate/no header or footer)
//   mem_level must be between [1, 9] (it's checked but ignored by miniz.c)
int mz_deflateInit2(mz_streamp pStream, int level, int method, int window_bits, int mem_level, int strategy);

// Quickly resets a compressor without having to reallocate anything. Same as calling mz_deflateEnd() followed by mz_deflateInit()/mz_deflateInit2().
int mz_deflateReset(mz_streamp pStream);

// mz_deflate() compresses the input to output, consuming as much of the input and producing as much output as possible.
// Parameters:
//   pStream is the stream to read from and write to. You must initialize/update the next_in, avail_in, next_out, and avail_out members.
//   flush may be MZ_NO_FLUSH, MZ_PARTIAL_FLUSH/MZ_SYNC_FLUSH, MZ_FULL_FLUSH, or MZ_FINISH.
// Return values:
//   MZ_OK on success (when flushing, or if more input is needed but not available, and/or there's more output to be written but the output buffer is full).
//   MZ_STREAM_END if all input has been consumed and all output bytes have been written. Don't call mz_deflate() on the stream anymore.
//   MZ_STREAM_ERROR if the stream is bogus.
//   MZ_PARAM_ERROR if one of the parameters is invalid.
//   MZ_BUF_ERROR if no forward progress is possible because the input and/or output buffers are empty. (Fill up the input buffer or free up some output space and try again.)
int mz_deflate(mz_streamp pStream, int flush);

// mz_deflateEnd() deinitializes a compressor:
// Return values:
//  MZ_OK on success.
//  MZ_STREAM_ERROR if the stream is bogus.
int mz_deflateEnd(mz_streamp pStream);

// mz_deflateBound() returns a (very) conservative upper bound on the amount of data that could be generated by deflate(), assuming flush is set to only MZ_NO_FLUSH or MZ_FINISH.
mz_ulong mz_deflateBound(mz_streamp pStream, mz_ulong source_len);

// Single-call compression functions mz_compress() and mz_compress2():
// Returns MZ_OK on success, or one of the error codes from mz_deflate() on failure.
int mz_compress(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len);
int mz_compress2(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len, int level);

// mz_compressBound() returns a (very) conservative upper bound on the amount of data that could be generated by calling mz_compress().
mz_ulong mz_compressBound(mz_ulong source_len);

// Initializes a decompressor.
int mz_inflateInit(mz_streamp pStream);

// mz_inflateInit2() is like mz_inflateInit() with an additional option that controls the window size and whether or not the stream has been wrapped with a zlib header/footer:
// window_bits must be MZ_DEFAULT_WINDOW_BITS (to parse zlib header/footer) or -MZ_DEFAULT_WINDOW_BITS (raw deflate).
int mz_inflateInit2(mz_streamp pStream, int window_bits);

// Decompresses the input stream to the output, consuming only as much of the input as needed, and writing as much to the output as possible.
// Parameters:
//   pStream is the stream to read from and write to. You must initialize/update the next_in, avail_in, next_out, and avail_out members.
//   flush may be MZ_NO_FLUSH, MZ_SYNC_FLUSH, or MZ_FINISH.
//   On the first call, if flush is MZ_FINISH it's assumed the input and output buffers are both sized large enough to decompress the entire stream in a single call (this is slightly faster).
//   MZ_FINISH implies that there are no more source bytes available beside what's already in the input buffer, and that the output buffer is large enough to hold the rest of the decompressed data.
// Return values:
//   MZ_OK on success. Either more input is needed but not available, and/or there's more output to be written but the output buffer is full.
//   MZ_STREAM_END if all needed input has been consumed and all output bytes have been written. For zlib streams, the adler-32 of the decompressed data has also been verified.
//   MZ_STREAM_ERROR if the stream is bogus.
//   MZ_DATA_ERROR if the deflate stream is invalid.
//   MZ_PARAM_ERROR if one of the parameters is invalid.
//   MZ_BUF_ERROR if no forward progress is possible because the input buffer is empty but the inflater needs more input to continue, or if the output buffer is not large enough. Call mz_inflate() again
//   with more input data, or with more room in the output buffer (except when using single call decompression, described above).
int mz_inflate(mz_streamp pStream, int flush);

// Deinitializes a decompressor.
int mz_inflateEnd(mz_streamp pStream);

// Single-call decompression.
// Returns MZ_OK on success, or one of the error codes from mz_inflate() on failure.
int mz_uncompress(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len);

// Returns a string description of the specified error code, or NULL if the error code is invalid.
const char *mz_error(int err);

// Redefine zlib-compatible names to miniz equivalents, so miniz.c can be used as a drop-in replacement for the subset of zlib that miniz.c supports.
// Define MINIZ_NO_ZLIB_COMPATIBLE_NAMES to disable zlib-compatibility if you use zlib in the same project.
#ifndef MINIZ_NO_ZLIB_COMPATIBLE_NAMES
  typedef unsigned char Byte;
  typedef unsigned int uInt;
  typedef mz_ulong uLong;
  typedef Byte Bytef;
  typedef uInt uIntf;
  typedef char charf;
  typedef int intf;
  typedef void *voidpf;
  typedef uLong uLongf;
  typedef void *voidp;
  typedef void *const voidpc;
  #define Z_NULL                0
  #define Z_NO_FLUSH            MZ_NO_FLUSH
  #define Z_PARTIAL_FLUSH       MZ_PARTIAL_FLUSH
  #define Z_SYNC_FLUSH          MZ_SYNC_FLUSH
  #define Z_FULL_FLUSH          MZ_FULL_FLUSH
  #define Z_FINISH              MZ_FINISH
  #define Z_BLOCK               MZ_BLOCK
  #define Z_OK                  MZ_OK
  #define Z_STREAM_END          MZ_STREAM_END
  #define Z_NEED_DICT           MZ_NEED_DICT
  #define Z_ERRNO               MZ_ERRNO
  #define Z_STREAM_ERROR        MZ_STREAM_ERROR
  #define Z_DATA_ERROR          MZ_DATA_ERROR
  #define Z_MEM_ERROR           MZ_MEM_ERROR
  #define Z_BUF_ERROR           MZ_BUF_ERROR
  #define Z_VERSION_ERROR       MZ_VERSION_ERROR
  #define Z_PARAM_ERROR         MZ_PARAM_ERROR
  #define Z_NO_COMPRESSION      MZ_NO_COMPRESSION
  #define Z_BEST_SPEED          MZ_BEST_SPEED
  #define Z_BEST_COMPRESSION    MZ_BEST_COMPRESSION
  #define Z_DEFAULT_COMPRESSION MZ_DEFAULT_COMPRESSION
  #define Z_DEFAULT_STRATEGY    MZ_DEFAULT_STRATEGY
  #define Z_FILTERED            MZ_FILTERED
  #define Z_HUFFMAN_ONLY        MZ_HUFFMAN_ONLY
  #define Z_RLE                 MZ_RLE
  #define Z_FIXED               MZ_FIXED
  #define Z_DEFLATED            MZ_DEFLATED
  #define Z_DEFAULT_WINDOW_BITS MZ_DEFAULT_WINDOW_BITS
  #define alloc_func            mz_alloc_func
  #define free_func             mz_free_func
  #define internal_state        mz_internal_state
  #define z_stream              mz_stream
  #define deflateInit           mz_deflateInit
  #define deflateInit2          mz_deflateInit2
  #define deflateReset          mz_deflateReset
  #define deflate               mz_deflate
  #define deflateEnd            mz_deflateEnd
  #define deflateBound          mz_deflateBound
  #define compress              mz_compress
  #define compress2             mz_compress2
  #define compressBound         mz_compressBound
  #define inflateInit           mz_inflateInit
  #define inflateInit2          mz_inflateInit2
  #define inflate               mz_inflate
  #define inflateEnd            mz_inflateEnd
  #define uncompress            mz_uncompress
  #define crc32                 mz_crc32
  #define adler32               mz_adler32
  #define MAX_WBITS             15
  #define MAX_MEM_LEVEL         9
  #define zError                mz_error
  #define ZLIB_VERSION          MZ_VERSION
  #define ZLIB_VERNUM           MZ_VERNUM
  #define ZLIB_VER_MAJOR        MZ_VER_MAJOR
  #define ZLIB_VER_MINOR        MZ_VER_MINOR
  #define ZLIB_VER_REVISION     MZ_VER_REVISION
  #define ZLIB_VER_SUBREVISION  MZ_VER_SUBREVISION
  #define zlibVersion           mz_version
  #define zlib_version          mz_version()
#endif // #ifndef MINIZ_NO_ZLIB_COMPATIBLE_NAMES

#endif // MINIZ_NO_ZLIB_APIS

// ------------------- Types and macros

typedef unsigned char mz_uint8;
typedef signed short mz_int16;
typedef unsigned short mz_uint16;
typedef unsigned int mz_uint32;
typedef unsigned int mz_uint;
typedef long long mz_int64;
typedef unsigned long long mz_uint64;
typedef int mz_bool;

#define MZ_FALSE (0)
#define MZ_TRUE (1)

// An attempt to work around MSVC's spammy "warning C4127: conditional expression is constant" message.
#ifdef _MSC_VER
   #define MZ_MACRO_END while (0, 0)
#else
   #define MZ_MACRO_END while (0)
#endif

// ------------------- ZIP archive reading/writing

#ifndef MINIZ_NO_ARCHIVE_APIS

enum
{
  MZ_ZIP_MAX_IO_BUF_SIZE = 64*1024,
  MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE = 260,
  MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE = 256
};

typedef struct
{
  mz_uint32 m_file_index;
  mz_uint32 m_central_dir_ofs;
  mz_uint16 m_version_made_by;
  mz_uint16 m_version_needed;
  mz_uint16 m_bit_flag;
  mz_uint16 m_method;
#ifndef MINIZ_NO_TIME
  time_t m_time;
#endif
  mz_uint32 m_crc32;
  mz_uint64 m_comp_size;
  mz_uint64 m_uncomp_size;
  mz_uint16 m_internal_attr;
  mz_uint32 m_external_attr;
  mz_uint64 m_local_header_ofs;
  mz_uint32 m_comment_size;
  char m_filename[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
  char m_comment[MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE];
} mz_zip_archive_file_stat;

typedef size_t (*mz_file_read_func)(void *pOpaque, mz_uint64 file_ofs, void *pBuf, size_t n);
typedef size_t (*mz_file_write_func)(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n);

struct mz_zip_internal_state_tag;
typedef struct mz_zip_internal_state_tag mz_zip_internal_state;

typedef enum
{
  MZ_ZIP_MODE_INVALID = 0,
  MZ_ZIP_MODE_READING = 1,
  MZ_ZIP_MODE_WRITING = 2,
  MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED = 3
} mz_zip_mode;

typedef struct mz_zip_archive_tag
{
  mz_uint64 m_archive_size;
  mz_uint64 m_central_directory_file_ofs;
  mz_uint m_total_files;
  mz_zip_mode m_zip_mode;

  mz_uint m_file_offset_alignment;

  mz_alloc_func m_pAlloc;
  mz_free_func m_pFree;
  mz_realloc_func m_pRealloc;
  void *m_pAlloc_opaque;

  mz_file_read_func m_pRead;
  mz_file_write_func m_pWrite;
  void *m_pIO_opaque;

  mz_zip_internal_state *m_pState;

} mz_zip_archive;

typedef enum
{
  MZ_ZIP_FLAG_CASE_SENSITIVE                = 0x0100,
  MZ_ZIP_FLAG_IGNORE_PATH                   = 0x0200,
  MZ_ZIP_FLAG_COMPRESSED_DATA               = 0x0400,
  MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY = 0x0800
} mz_zip_flags;

// ZIP archive reading

// Inits a ZIP archive reader.
// These functions read and validate the archive's central directory.
mz_bool mz_zip_reader_init(mz_zip_archive *pZip, mz_uint64 size, mz_uint32 flags);
mz_bool mz_zip_reader_init_mem(mz_zip_archive *pZip, const void *pMem, size_t size, mz_uint32 flags);

#ifndef MINIZ_NO_STDIO
mz_bool mz_zip_reader_init_file(mz_zip_archive *pZip, const char *pFilename, mz_uint32 flags);
#endif

// Returns the total number of files in the archive.
mz_uint mz_zip_reader_get_num_files(mz_zip_archive *pZip);

// Returns detailed information about an archive file entry.
mz_bool mz_zip_reader_file_stat(mz_zip_archive *pZip, mz_uint file_index, mz_zip_archive_file_stat *pStat);

// Determines if an archive file entry is a directory entry.
mz_bool mz_zip_reader_is_file_a_directory(mz_zip_archive *pZip, mz_uint file_index);
mz_bool mz_zip_reader_is_file_encrypted(mz_zip_archive *pZip, mz_uint file_index);

// Retrieves the filename of an archive file entry.
// Returns the number of bytes written to pFilename, or if filename_buf_size is 0 this function returns the number of bytes needed to fully store the filename.
mz_uint mz_zip_reader_get_filename(mz_zip_archive *pZip, mz_uint file_index, char *pFilename, mz_uint filename_buf_size);

// Attempts to locates a file in the archive's central directory.
// Valid flags: MZ_ZIP_FLAG_CASE_SENSITIVE, MZ_ZIP_FLAG_IGNORE_PATH
// Returns -1 if the file cannot be found.
int mz_zip_reader_locate_file(mz_zip_archive *pZip, const char *pName, const char *pComment, mz_uint flags);

// Extracts a archive file to a memory buffer using no memory allocation.
mz_bool mz_zip_reader_extract_to_mem_no_alloc(mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size, mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);
mz_bool mz_zip_reader_extract_file_to_mem_no_alloc(mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);

// Extracts a archive file to a memory buffer.
mz_bool mz_zip_reader_extract_to_mem(mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size, mz_uint flags);
mz_bool mz_zip_reader_extract_file_to_mem(mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, mz_uint flags);

// Extracts a archive file to a dynamically allocated heap buffer.
void *mz_zip_reader_extract_to_heap(mz_zip_archive *pZip, mz_uint file_index, size_t *pSize, mz_uint flags);
void *mz_zip_reader_extract_file_to_heap(mz_zip_archive *pZip, const char *pFilename, size_t *pSize, mz_uint flags);

// Extracts a archive file using a callback function to output the file's data.
mz_bool mz_zip_reader_extract_to_callback(mz_zip_archive *pZip, mz_uint file_index, mz_file_write_func pCallback, void *pOpaque, mz_uint flags);
mz_bool mz_zip_reader_extract_file_to_callback(mz_zip_archive *pZip, const char *pFilename, mz_file_write_func pCallback, void *pOpaque, mz_uint flags);

#ifndef MINIZ_NO_STDIO
// Extracts a archive file to a disk file and sets its last accessed and modified times.
// This function only extracts files, not archive directory records.
mz_bool mz_zip_reader_extract_to_file(mz_zip_archive *pZip, mz_uint file_index, const char *pDst_filename, mz_uint flags);
mz_bool mz_zip_reader_extract_file_to_file(mz_zip_archive *pZip, const char *pArchive_filename, const char *pDst_filename, mz_uint flags);
#endif

// Ends archive reading, freeing all allocations, and closing the input archive file if mz_zip_reader_init_file() was used.
mz_bool mz_zip_reader_end(mz_zip_archive *pZip);

// ZIP archive writing

#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS

// Inits a ZIP archive writer.
mz_bool mz_zip_writer_init(mz_zip_archive *pZip, mz_uint64 existing_size);
mz_bool mz_zip_writer_init_heap(mz_zip_archive *pZip, size_t size_to_reserve_at_beginning, size_t initial_allocation_size);

#ifndef MINIZ_NO_STDIO
mz_bool mz_zip_writer_init_file(mz_zip_archive *pZip, const char *pFilename, mz_uint64 size_to_reserve_at_beginning);
#endif

// Converts a ZIP archive reader object into a writer object, to allow efficient in-place file appends to occur on an existing archive.
// For archives opened using mz_zip_reader_init_file, pFilename must be the archive's filename so it can be reopened for writing. If the file can't be reopened, mz_zip_reader_end() will be called.
// For archives opened using mz_zip_reader_init_mem, the memory block must be growable using the realloc callback (which defaults to realloc unless you've overridden it).
// Finally, for archives opened using mz_zip_reader_init, the mz_zip_archive's user provided m_pWrite function cannot be NULL.
// Note: In-place archive modification is not recommended unless you know what you're doing, because if execution stops or something goes wrong before
// the archive is finalized the file's central directory will be hosed.
mz_bool mz_zip_writer_init_from_reader(mz_zip_archive *pZip, const char *pFilename);

// Adds the contents of a memory buffer to an archive. These functions record the current local time into the archive.
// To add a directory entry, call this method with an archive name ending in a forwardslash with empty buffer.
// level_and_flags - compression level (0-10, see MZ_BEST_SPEED, MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more mz_zip_flags, or just set to MZ_DEFAULT_COMPRESSION.
mz_bool mz_zip_writer_add_mem(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, mz_uint level_and_flags);
mz_bool mz_zip_writer_add_mem_ex(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32);

#ifndef MINIZ_NO_STDIO
// Adds the contents of a disk file to an archive. This function also records the disk file's modified time into the archive.
// level_and_flags - compression level (0-10, see MZ_BEST_SPEED, MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more mz_zip_flags, or just set to MZ_DEFAULT_COMPRESSION.
mz_bool mz_zip_writer_add_file(mz_zip_archive *pZip, const char *pArchive_name, const char *pSrc_filename, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags);
#endif

// Adds a file to an archive by fully cloning the data from another archive.
// This function fully clones the source file's compressed data (no recompression), along with its full filename, extra data, and comment fields.
mz_bool mz_zip_writer_add_from_zip_reader(mz_zip_archive *pZip, mz_zip_archive *pSource_zip, mz_uint file_index);

// Finalizes the archive by writing the central directory records followed by the end of central directory record.
// After an archive is finalized, the only valid call on the mz_zip_archive struct is mz_zip_writer_end().
// An archive must be manually finalized by calling this function for it to be valid.
mz_bool mz_zip_writer_finalize_archive(mz_zip_archive *pZip);
mz_bool mz_zip_writer_finalize_heap_archive(mz_zip_archive *pZip, void **pBuf, size_t *pSize);

// Ends archive writing, freeing all allocations, and closing the output file if mz_zip_writer_init_file() was used.
// Note for the archive to be valid, it must have been finalized before ending.
mz_bool mz_zip_writer_end(mz_zip_archive *pZip);

// Misc. high-level helper functions:

// mz_zip_add_mem_to_archive_file_in_place() efficiently (but not atomically) appends a memory blob to a ZIP archive.
// level_and_flags - compression level (0-10, see MZ_BEST_SPEED, MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more mz_zip_flags, or just set to MZ_DEFAULT_COMPRESSION.
mz_bool mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags);

// Reads a single file from an archive into a heap block.
// Returns NULL on failure.
void *mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name, size_t *pSize, mz_uint zip_flags);

#endif // #ifndef MINIZ_NO_ARCHIVE_WRITING_APIS

#endif // #ifndef MINIZ_NO_ARCHIVE_APIS

// ------------------- Low-level Decompression API Definitions

// Decompression flags used by tinfl_decompress().
// TINFL_FLAG_PARSE_ZLIB_HEADER: If set, the input has a valid zlib header and ends with an adler32 checksum (it's a valid zlib stream). Otherwise, the input is a raw deflate stream.
// TINFL_FLAG_HAS_MORE_INPUT: If set, there are more input bytes available beyond the end of the supplied input buffer. If clear, the input buffer contains all remaining input.
// TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF: If set, the output buffer is large enough to hold the entire decompressed stream. If clear, the output buffer is at least the size of the dictionary (typically 32KB).
// TINFL_FLAG_COMPUTE_ADLER32: Force adler-32 checksum computation of the decompressed bytes.
enum
{
  TINFL_FLAG_PARSE_ZLIB_HEADER = 1,
  TINFL_FLAG_HAS_MORE_INPUT = 2,
  TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4,
  TINFL_FLAG_COMPUTE_ADLER32 = 8
};

// High level decompression functions:
// tinfl_decompress_mem_to_heap() decompresses a block in memory to a heap block allocated via malloc().
// On entry:
//  pSrc_buf, src_buf_len: Pointer and size of the Deflate or zlib source data to decompress.
// On return:
//  Function returns a pointer to the decompressed data, or NULL on failure.
//  *pOut_len will be set to the decompressed data's size, which could be larger than src_buf_len on uncompressible data.
//  The caller must call mz_free() on the returned block when it's no longer needed.
void *tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

// tinfl_decompress_mem_to_mem() decompresses a block in memory to another block in memory.
// Returns TINFL_DECOMPRESS_MEM_TO_MEM_FAILED on failure, or the number of bytes written on success.
#define TINFL_DECOMPRESS_MEM_TO_MEM_FAILED ((size_t)(-1))
size_t tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

// tinfl_decompress_mem_to_callback() decompresses a block in memory to an internal 32KB buffer, and a user provided callback function will be called to flush the buffer.
// Returns 1 on success or 0 on failure.
typedef int (*tinfl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);
int tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

struct tinfl_decompressor_tag; typedef struct tinfl_decompressor_tag tinfl_decompressor;

// Max size of LZ dictionary.
#define TINFL_LZ_DICT_SIZE 32768

// Return status.
typedef enum
{
  TINFL_STATUS_BAD_PARAM = -3,
  TINFL_STATUS_ADLER32_MISMATCH = -2,
  TINFL_STATUS_FAILED = -1,
  TINFL_STATUS_DONE = 0,
  TINFL_STATUS_NEEDS_MORE_INPUT = 1,
  TINFL_STATUS_HAS_MORE_OUTPUT = 2
} tinfl_status;

// Initializes the decompressor to its initial state.
#define tinfl_init(r) do { (r)->m_state = 0; } MZ_MACRO_END
#define tinfl_get_adler32(r) (r)->m_check_adler32

// Main low-level decompressor coroutine function. This is the only function actually needed for decompression. All the other functions are just high-level helpers for improved usability.
// This is a universal API, i.e. it can be used as a building block to build any desired higher level decompression API. In the limit case, it can be called once per every byte input or output.
tinfl_status tinfl_decompress(tinfl_decompressor *r, const mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mz_uint8 *pOut_buf_start, mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mz_uint32 decomp_flags);

// Internal/private bits follow.
enum
{
  TINFL_MAX_HUFF_TABLES = 3, TINFL_MAX_HUFF_SYMBOLS_0 = 288, TINFL_MAX_HUFF_SYMBOLS_1 = 32, TINFL_MAX_HUFF_SYMBOLS_2 = 19,
  TINFL_FAST_LOOKUP_BITS = 10, TINFL_FAST_LOOKUP_SIZE = 1 << TINFL_FAST_LOOKUP_BITS
};

typedef struct
{
  mz_uint8 m_code_size[TINFL_MAX_HUFF_SYMBOLS_0];
  mz_int16 m_look_up[TINFL_FAST_LOOKUP_SIZE], m_tree[TINFL_MAX_HUFF_SYMBOLS_0 * 2];
} tinfl_huff_table;

#if MINIZ_HAS_64BIT_REGISTERS
  #define TINFL_USE_64BIT_BITBUF 1
#endif

#if TINFL_USE_64BIT_BITBUF
  typedef mz_uint64 tinfl_bit_buf_t;
  #define TINFL_BITBUF_SIZE (64)
#else
  typedef mz_uint32 tinfl_bit_buf_t;
  #define TINFL_BITBUF_SIZE (32)
#endif

struct tinfl_decompressor_tag
{
  mz_uint32 m_state, m_num_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type, m_check_adler32, m_dist, m_counter, m_num_extra, m_table_sizes[TINFL_MAX_HUFF_TABLES];
  tinfl_bit_buf_t m_bit_buf;
  size_t m_dist_from_out_buf_start;
  tinfl_huff_table m_tables[TINFL_MAX_HUFF_TABLES];
  mz_uint8 m_raw_header[4], m_len_codes[TINFL_MAX_HUFF_SYMBOLS_0 + TINFL_MAX_HUFF_SYMBOLS_1 + 137];
};

// ------------------- Low-level Compression API Definitions

// Set TDEFL_LESS_MEMORY to 1 to use less memory (compression will be slightly slower, and raw/dynamic blocks will be output more frequently).
#define TDEFL_LESS_MEMORY 0

// tdefl_init() compression flags logically OR'd together (low 12 bits contain the max. number of probes per dictionary search):
// TDEFL_DEFAULT_MAX_PROBES: The compressor defaults to 128 dictionary probes per dictionary search. 0=Huffman only, 1=Huffman+LZ (fastest/crap compression), 4095=Huffman+LZ (slowest/best compression).
enum
{
  TDEFL_HUFFMAN_ONLY = 0, TDEFL_DEFAULT_MAX_PROBES = 128, TDEFL_MAX_PROBES_MASK = 0xFFF
};

// TDEFL_WRITE_ZLIB_HEADER: If set, the compressor outputs a zlib header before the deflate data, and the Adler-32 of the source data at the end. Otherwise, you'll get raw deflate data.
// TDEFL_COMPUTE_ADLER32: Always compute the adler-32 of the input data (even when not writing zlib headers).
// TDEFL_GREEDY_PARSING_FLAG: Set to use faster greedy parsing, instead of more efficient lazy parsing.
// TDEFL_NONDETERMINISTIC_PARSING_FLAG: Enable to decrease the compressor's initialization time to the minimum, but the output may vary from run to run given the same input (depending on the contents of memory).
// TDEFL_RLE_MATCHES: Only look for RLE matches (matches with a distance of 1)
// TDEFL_FILTER_MATCHES: Discards matches <= 5 chars if enabled.
// TDEFL_FORCE_ALL_STATIC_BLOCKS: Disable usage of optimized Huffman tables.
// TDEFL_FORCE_ALL_RAW_BLOCKS: Only use raw (uncompressed) deflate blocks.
// The low 12 bits are reserved to control the max # of hash probes per dictionary lookup (see TDEFL_MAX_PROBES_MASK).
enum
{
  TDEFL_WRITE_ZLIB_HEADER             = 0x01000,
  TDEFL_COMPUTE_ADLER32               = 0x02000,
  TDEFL_GREEDY_PARSING_FLAG           = 0x04000,
  TDEFL_NONDETERMINISTIC_PARSING_FLAG = 0x08000,
  TDEFL_RLE_MATCHES                   = 0x10000,
  TDEFL_FILTER_MATCHES                = 0x20000,
  TDEFL_FORCE_ALL_STATIC_BLOCKS       = 0x40000,
  TDEFL_FORCE_ALL_RAW_BLOCKS          = 0x80000
};

// High level compression functions:
// tdefl_compress_mem_to_heap() compresses a block in memory to a heap block allocated via malloc().
// On entry:
//  pSrc_buf, src_buf_len: Pointer and size of source block to compress.
//  flags: The max match finder probes (default is 128) logically OR'd against the above flags. Higher probes are slower but improve compression.
// On return:
//  Function returns a pointer to the compressed data, or NULL on failure.
//  *pOut_len will be set to the compressed data's size, which could be larger than src_buf_len on uncompressible data.
//  The caller must free() the returned block when it's no longer needed.
void *tdefl_compress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

// tdefl_compress_mem_to_mem() compresses a block in memory to another block in memory.
// Returns 0 on failure.
size_t tdefl_compress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

// Compresses an image to a compressed PNG file in memory.
// On entry:
//  pImage, w, h, and num_chans describe the image to compress. num_chans may be 1, 2, 3, or 4.
//  The image pitch in bytes per scanline will be w*num_chans. The leftmost pixel on the top scanline is stored first in memory.
//  level may range from [0,10], use MZ_NO_COMPRESSION, MZ_BEST_SPEED, MZ_BEST_COMPRESSION, etc. or a decent default is MZ_DEFAULT_LEVEL
//  If flip is true, the image will be flipped on the Y axis (useful for OpenGL apps).
// On return:
//  Function returns a pointer to the compressed data, or NULL on failure.
//  *pLen_out will be set to the size of the PNG image file.
//  The caller must mz_free() the returned heap block (which will typically be larger than *pLen_out) when it's no longer needed.
void *tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h, int num_chans, size_t *pLen_out, mz_uint level, mz_bool flip);
void *tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h, int num_chans, size_t *pLen_out);

// Output stream interface. The compressor uses this interface to write compressed data. It'll typically be called TDEFL_OUT_BUF_SIZE at a time.
typedef mz_bool (*tdefl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);

// tdefl_compress_mem_to_output() compresses a block to an output stream. The above helpers use this function internally.
mz_bool tdefl_compress_mem_to_output(const void *pBuf, size_t buf_len, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

enum { TDEFL_MAX_HUFF_TABLES = 3, TDEFL_MAX_HUFF_SYMBOLS_0 = 288, TDEFL_MAX_HUFF_SYMBOLS_1 = 32, TDEFL_MAX_HUFF_SYMBOLS_2 = 19, TDEFL_LZ_DICT_SIZE = 32768, TDEFL_LZ_DICT_SIZE_MASK = TDEFL_LZ_DICT_SIZE - 1, TDEFL_MIN_MATCH_LEN = 3, TDEFL_MAX_MATCH_LEN = 258 };

// TDEFL_OUT_BUF_SIZE MUST be large enough to hold a single entire compressed output block (using static/fixed Huffman codes).
#if TDEFL_LESS_MEMORY
enum { TDEFL_LZ_CODE_BUF_SIZE = 24 * 1024, TDEFL_OUT_BUF_SIZE = (TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10, TDEFL_MAX_HUFF_SYMBOLS = 288, TDEFL_LZ_HASH_BITS = 12, TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, TDEFL_LZ_HASH_SHIFT = (TDEFL_LZ_HASH_BITS + 2) / 3, TDEFL_LZ_HASH_SIZE = 1 << TDEFL_LZ_HASH_BITS };
#else
enum { TDEFL_LZ_CODE_BUF_SIZE = 64 * 1024, TDEFL_OUT_BUF_SIZE = (TDEFL_LZ_CODE_BUF_SIZE * 13 ) / 10, TDEFL_MAX_HUFF_SYMBOLS = 288, TDEFL_LZ_HASH_BITS = 15, TDEFL_LEVEL1_HASH_SIZE_MASK = 4095, TDEFL_LZ_HASH_SHIFT = (TDEFL_LZ_HASH_BITS + 2) / 3, TDEFL_LZ_HASH_SIZE = 1 << TDEFL_LZ_HASH_BITS };
#endif

// The low-level tdefl functions below may be used directly if the above helper functions aren't flexible enough. The low-level functions don't make any heap allocations, unlike the above helper functions.
typedef enum
{
  TDEFL_STATUS_BAD_PARAM = -2,
  TDEFL_STATUS_PUT_BUF_FAILED = -1,
  TDEFL_STATUS_OKAY = 0,
  TDEFL_STATUS_DONE = 1,
} tdefl_status;

// Must map to MZ_NO_FLUSH, MZ_SYNC_FLUSH, etc. enums
typedef enum
{
  TDEFL_NO_FLUSH = 0,
  TDEFL_SYNC_FLUSH = 2,
  TDEFL_FULL_FLUSH = 3,
  TDEFL_FINISH = 4
} tdefl_flush;

// tdefl's compression state structure.
typedef struct
{
  tdefl_put_buf_func_ptr m_pPut_buf_func;
  void *m_pPut_buf_user;
  mz_uint m_flags, m_max_probes[2];
  int m_greedy_parsing;
  mz_uint m_adler32, m_lookahead_pos, m_lookahead_size, m_dict_size;
  mz_uint8 *m_pLZ_code_buf, *m_pLZ_flags, *m_pOutput_buf, *m_pOutput_buf_end;
  mz_uint m_num_flags_left, m_total_lz_bytes, m_lz_code_buf_dict_pos, m_bits_in, m_bit_buffer;
  mz_uint m_saved_match_dist, m_saved_match_len, m_saved_lit, m_output_flush_ofs, m_output_flush_remaining, m_finished, m_block_index, m_wants_to_finish;
  tdefl_status m_prev_return_status;
  const void *m_pIn_buf;
  void *m_pOut_buf;
  size_t *m_pIn_buf_size, *m_pOut_buf_size;
  tdefl_flush m_flush;
  const mz_uint8 *m_pSrc;
  size_t m_src_buf_left, m_out_buf_ofs;
  mz_uint8 m_dict[TDEFL_LZ_DICT_SIZE + TDEFL_MAX_MATCH_LEN - 1];
  mz_uint16 m_huff_count[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mz_uint16 m_huff_codes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mz_uint8 m_huff_code_sizes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mz_uint8 m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE];
  mz_uint16 m_next[TDEFL_LZ_DICT_SIZE];
  mz_uint16 m_hash[TDEFL_LZ_HASH_SIZE];
  mz_uint8 m_output_buf[TDEFL_OUT_BUF_SIZE];
} tdefl_compressor;

// Initializes the compressor.
// There is no corresponding deinit() function because the tdefl API's do not dynamically allocate memory.
// pBut_buf_func: If NULL, output data will be supplied to the specified callback. In this case, the user should call the tdefl_compress_buffer() API for compression.
// If pBut_buf_func is NULL the user should always call the tdefl_compress() API.
// flags: See the above enums (TDEFL_HUFFMAN_ONLY, TDEFL_WRITE_ZLIB_HEADER, etc.)
tdefl_status tdefl_init(tdefl_compressor *d, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

// Compresses a block of data, consuming as much of the specified input buffer as possible, and writing as much compressed data to the specified output buffer as possible.
tdefl_status tdefl_compress(tdefl_compressor *d, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, tdefl_flush flush);

// tdefl_compress_buffer() is only usable when the tdefl_init() is called with a non-NULL tdefl_put_buf_func_ptr.
// tdefl_compress_buffer() always consumes the entire input buffer.
tdefl_status tdefl_compress_buffer(tdefl_compressor *d, const void *pIn_buf, size_t in_buf_size, tdefl_flush flush);

tdefl_status tdefl_get_prev_return_status(tdefl_compressor *d);
mz_uint32 tdefl_get_adler32(tdefl_compressor *d);

// Can't use tdefl_create_comp_flags_from_zip_params if MINIZ_NO_ZLIB_APIS isn't defined, because it uses some of its macros.
#ifndef MINIZ_NO_ZLIB_APIS
// Create tdefl_compress() flags given zlib-style compression parameters.
// level may range from [0,10] (where 10 is absolute max compression, but may be much slower on some files)
// window_bits may be -15 (raw deflate) or 15 (zlib)
// strategy may be either MZ_DEFAULT_STRATEGY, MZ_FILTERED, MZ_HUFFMAN_ONLY, MZ_RLE, or MZ_FIXED
mz_uint tdefl_create_comp_flags_from_zip_params(int level, int window_bits, int strategy);
#endif // #ifndef MINIZ_NO_ZLIB_APIS

#ifdef __cplusplus
}
#endif

#endif // MINIZ_HEADER_INCLUDED

// ------------------- End of Header: Implementation follows. (If you only want the header, define MINIZ_HEADER_FILE_ONLY.)

#ifndef MINIZ_HEADER_FILE_ONLY

typedef unsigned char mz_validate_uint16[sizeof(mz_uint16)==2 ? 1 : -1];
typedef unsigned char mz_validate_uint32[sizeof(mz_uint32)==4 ? 1 : -1];
typedef unsigned char mz_validate_uint64[sizeof(mz_uint64)==8 ? 1 : -1];

#include <string.h>
#include <assert.h>

#define MZ_ASSERT(x) assert(x)

#ifdef MINIZ_NO_MALLOC
  #define MZ_MALLOC(x) NULL
  #define MZ_FREE(x) (void)x, ((void)0)
  #define MZ_REALLOC(p, x) NULL
#else
  #define MZ_MALLOC(x) malloc(x)
  #define MZ_FREE(x) free(x)
  #define MZ_REALLOC(p, x) realloc(p, x)
#endif

#define MZ_MAX(a,b) (((a)>(b))?(a):(b))
#define MZ_MIN(a,b) (((a)<(b))?(a):(b))
#define MZ_CLEAR_OBJ(obj) memset(&(obj), 0, sizeof(obj))

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
  #define MZ_READ_LE16(p) *((const mz_uint16 *)(p))
  #define MZ_READ_LE32(p) *((const mz_uint32 *)(p))
#else
  #define MZ_READ_LE16(p) ((mz_uint32)(((const mz_uint8 *)(p))[0]) | ((mz_uint32)(((const mz_uint8 *)(p))[1]) << 8U))
  #define MZ_READ_LE32(p) ((mz_uint32)(((const mz_uint8 *)(p))[0]) | ((mz_uint32)(((const mz_uint8 *)(p))[1]) << 8U) | ((mz_uint32)(((const mz_uint8 *)(p))[2]) << 16U) | ((mz_uint32)(((const mz_uint8 *)(p))[3]) << 24U))
#endif

#ifdef _MSC_VER
  #define MZ_FORCEINLINE __forceinline
#elif defined(__GNUC__)
  #define MZ_FORCEINLINE inline __attribute__((__always_inline__))
#else
  #define MZ_FORCEINLINE inline
#endif

#ifdef __cplusplus
  extern "C" {
#endif

// ------------------- zlib-style API's

mz_ulong mz_adler32(mz_ulong adler, const unsigned char *ptr, size_t buf_len)
{
  mz_uint32 i, s1 = (mz_uint32)(adler & 0xffff), s2 = (mz_uint32)(adler >> 16); size_t block_len = buf_len % 5552;
  if (!ptr) return MZ_ADLER32_INIT;
  while (buf_len) {
    for (i = 0; i + 7 < block_len; i += 8, ptr += 8) {
      s1 += ptr[0], s2 += s1; s1 += ptr[1], s2 += s1; s1 += ptr[2], s2 += s1; s1 += ptr[3], s2 += s1;
      s1 += ptr[4], s2 += s1; s1 += ptr[5], s2 += s1; s1 += ptr[6], s2 += s1; s1 += ptr[7], s2 += s1;
    }
    for ( ; i < block_len; ++i) s1 += *ptr++, s2 += s1;
    s1 %= 65521U, s2 %= 65521U; buf_len -= block_len; block_len = 5552;
  }
  return (s2 << 16) + s1;
}

// Karl Malbrain's compact CRC-32. See "A compact CCITT crc16 and crc32 C implementation that balances processor cache usage against speed": http://www.geocities.com/malbrain/
mz_ulong mz_crc32(mz_ulong crc, const mz_uint8 *ptr, size_t buf_len)
{
  static const mz_uint32 s_crc32[16] = { 0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
  mz_uint32 crcu32 = (mz_uint32)crc;
  if (!ptr) return MZ_CRC32_INIT;
  crcu32 = ~crcu32; while (buf_len--) { mz_uint8 b = *ptr++; crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b & 0xF)]; crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b >> 4)]; }
  return ~crcu32;
}

void mz_free(void *p)
{
  MZ_FREE(p);
}

#ifndef MINIZ_NO_ZLIB_APIS

static void *def_alloc_func(void *opaque, size_t items, size_t size) { (void)opaque, (void)items, (void)size; return MZ_MALLOC(items * size); }
static void def_free_func(void *opaque, void *address) { (void)opaque, (void)address; MZ_FREE(address); }
static void *def_realloc_func(void *opaque, void *address, size_t items, size_t size) { (void)opaque, (void)address, (void)items, (void)size; return MZ_REALLOC(address, items * size); }

const char *mz_version(void)
{
  return MZ_VERSION;
}

int mz_deflateInit(mz_streamp pStream, int level)
{
  return mz_deflateInit2(pStream, level, MZ_DEFLATED, MZ_DEFAULT_WINDOW_BITS, 9, MZ_DEFAULT_STRATEGY);
}

int mz_deflateInit2(mz_streamp pStream, int level, int method, int window_bits, int mem_level, int strategy)
{
  tdefl_compressor *pComp;
  mz_uint comp_flags = TDEFL_COMPUTE_ADLER32 | tdefl_create_comp_flags_from_zip_params(level, window_bits, strategy);

  if (!pStream) return MZ_STREAM_ERROR;
  if ((method != MZ_DEFLATED) || ((mem_level < 1) || (mem_level > 9)) || ((window_bits != MZ_DEFAULT_WINDOW_BITS) && (-window_bits != MZ_DEFAULT_WINDOW_BITS))) return MZ_PARAM_ERROR;

  pStream->data_type = 0;
  pStream->adler = MZ_ADLER32_INIT;
  pStream->msg = NULL;
  pStream->reserved = 0;
  pStream->total_in = 0;
  pStream->total_out = 0;
  if (!pStream->zalloc) pStream->zalloc = def_alloc_func;
  if (!pStream->zfree) pStream->zfree = def_free_func;

  pComp = (tdefl_compressor *)pStream->zalloc(pStream->opaque, 1, sizeof(tdefl_compressor));
  if (!pComp)
    return MZ_MEM_ERROR;

  pStream->state = (struct mz_internal_state *)pComp;

  if (tdefl_init(pComp, NULL, NULL, comp_flags) != TDEFL_STATUS_OKAY)
  {
    mz_deflateEnd(pStream);
    return MZ_PARAM_ERROR;
  }

  return MZ_OK;
}

int mz_deflateReset(mz_streamp pStream)
{
  if ((!pStream) || (!pStream->state) || (!pStream->zalloc) || (!pStream->zfree)) return MZ_STREAM_ERROR;
  pStream->total_in = pStream->total_out = 0;
  tdefl_init((tdefl_compressor*)pStream->state, NULL, NULL, ((tdefl_compressor*)pStream->state)->m_flags);
  return MZ_OK;
}

int mz_deflate(mz_streamp pStream, int flush)
{
  size_t in_bytes, out_bytes;
  mz_ulong orig_total_in, orig_total_out;
  int mz_status = MZ_OK;

  if ((!pStream) || (!pStream->state) || (flush < 0) || (flush > MZ_FINISH) || (!pStream->next_out)) return MZ_STREAM_ERROR;
  if (!pStream->avail_out) return MZ_BUF_ERROR;

  if (flush == MZ_PARTIAL_FLUSH) flush = MZ_SYNC_FLUSH;

  if (((tdefl_compressor*)pStream->state)->m_prev_return_status == TDEFL_STATUS_DONE)
    return (flush == MZ_FINISH) ? MZ_STREAM_END : MZ_BUF_ERROR;

  orig_total_in = pStream->total_in; orig_total_out = pStream->total_out;
  for ( ; ; )
  {
    tdefl_status defl_status;
    in_bytes = pStream->avail_in; out_bytes = pStream->avail_out;

    defl_status = tdefl_compress((tdefl_compressor*)pStream->state, pStream->next_in, &in_bytes, pStream->next_out, &out_bytes, (tdefl_flush)flush);
    pStream->next_in += (mz_uint)in_bytes; pStream->avail_in -= (mz_uint)in_bytes;
    pStream->total_in += (mz_uint)in_bytes; pStream->adler = tdefl_get_adler32((tdefl_compressor*)pStream->state);

    pStream->next_out += (mz_uint)out_bytes; pStream->avail_out -= (mz_uint)out_bytes;
    pStream->total_out += (mz_uint)out_bytes;

    if (defl_status < 0)
    {
      mz_status = MZ_STREAM_ERROR;
      break;
    }
    else if (defl_status == TDEFL_STATUS_DONE)
    {
      mz_status = MZ_STREAM_END;
      break;
    }
    else if (!pStream->avail_out)
      break;
    else if ((!pStream->avail_in) && (flush != MZ_FINISH))
    {
      if ((flush) || (pStream->total_in != orig_total_in) || (pStream->total_out != orig_total_out))
        break;
      return MZ_BUF_ERROR; // Can't make forward progress without some input.
    }
  }
  return mz_status;
}

int mz_deflateEnd(mz_streamp pStream)
{
  if (!pStream) return MZ_STREAM_ERROR;
  if (pStream->state)
  {
    pStream->zfree(pStream->opaque, pStream->state);
    pStream->state = NULL;
  }
  return MZ_OK;
}

mz_ulong mz_deflateBound(mz_streamp pStream, mz_ulong source_len)
{
  (void)pStream;
  // This is really over conservative. (And lame, but it's actually pretty tricky to compute a true upper bound given the way tdefl's blocking works.)
  return MZ_MAX(128 + (source_len * 110) / 100, 128 + source_len + ((source_len / (31 * 1024)) + 1) * 5);
}

int mz_compress2(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len, int level)
{
  int status;
  mz_stream stream;
  memset(&stream, 0, sizeof(stream));

  // In case mz_ulong is 64-bits (argh I hate longs).
  if ((source_len | *pDest_len) > 0xFFFFFFFFU) return MZ_PARAM_ERROR;

  stream.next_in = pSource;
  stream.avail_in = (mz_uint32)source_len;
  stream.next_out = pDest;
  stream.avail_out = (mz_uint32)*pDest_len;

  status = mz_deflateInit(&stream, level);
  if (status != MZ_OK) return status;

  status = mz_deflate(&stream, MZ_FINISH);
  if (status != MZ_STREAM_END)
  {
    mz_deflateEnd(&stream);
    return (status == MZ_OK) ? MZ_BUF_ERROR : status;
  }

  *pDest_len = stream.total_out;
  return mz_deflateEnd(&stream);
}

int mz_compress(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len)
{
  return mz_compress2(pDest, pDest_len, pSource, source_len, MZ_DEFAULT_COMPRESSION);
}

mz_ulong mz_compressBound(mz_ulong source_len)
{
  return mz_deflateBound(NULL, source_len);
}

typedef struct
{
  tinfl_decompressor m_decomp;
  mz_uint m_dict_ofs, m_dict_avail, m_first_call, m_has_flushed; int m_window_bits;
  mz_uint8 m_dict[TINFL_LZ_DICT_SIZE];
  tinfl_status m_last_status;
} inflate_state;

int mz_inflateInit2(mz_streamp pStream, int window_bits)
{
  inflate_state *pDecomp;
  if (!pStream) return MZ_STREAM_ERROR;
  if ((window_bits != MZ_DEFAULT_WINDOW_BITS) && (-window_bits != MZ_DEFAULT_WINDOW_BITS)) return MZ_PARAM_ERROR;

  pStream->data_type = 0;
  pStream->adler = 0;
  pStream->msg = NULL;
  pStream->total_in = 0;
  pStream->total_out = 0;
  pStream->reserved = 0;
  if (!pStream->zalloc) pStream->zalloc = def_alloc_func;
  if (!pStream->zfree) pStream->zfree = def_free_func;

  pDecomp = (inflate_state*)pStream->zalloc(pStream->opaque, 1, sizeof(inflate_state));
  if (!pDecomp) return MZ_MEM_ERROR;

  pStream->state = (struct mz_internal_state *)pDecomp;

  tinfl_init(&pDecomp->m_decomp);
  pDecomp->m_dict_ofs = 0;
  pDecomp->m_dict_avail = 0;
  pDecomp->m_last_status = TINFL_STATUS_NEEDS_MORE_INPUT;
  pDecomp->m_first_call = 1;
  pDecomp->m_has_flushed = 0;
  pDecomp->m_window_bits = window_bits;

  return MZ_OK;
}

int mz_inflateInit(mz_streamp pStream)
{
   return mz_inflateInit2(pStream, MZ_DEFAULT_WINDOW_BITS);
}

int mz_inflate(mz_streamp pStream, int flush)
{
  inflate_state* pState;
  mz_uint n, first_call, decomp_flags = TINFL_FLAG_COMPUTE_ADLER32;
  size_t in_bytes, out_bytes, orig_avail_in;
  tinfl_status status;

  if ((!pStream) || (!pStream->state)) return MZ_STREAM_ERROR;
  if (flush == MZ_PARTIAL_FLUSH) flush = MZ_SYNC_FLUSH;
  if ((flush) && (flush != MZ_SYNC_FLUSH) && (flush != MZ_FINISH)) return MZ_STREAM_ERROR;

  pState = (inflate_state*)pStream->state;
  if (pState->m_window_bits > 0) decomp_flags |= TINFL_FLAG_PARSE_ZLIB_HEADER;
  orig_avail_in = pStream->avail_in;

  first_call = pState->m_first_call; pState->m_first_call = 0;
  if (pState->m_last_status < 0) return MZ_DATA_ERROR;

  if (pState->m_has_flushed && (flush != MZ_FINISH)) return MZ_STREAM_ERROR;
  pState->m_has_flushed |= (flush == MZ_FINISH);

  if ((flush == MZ_FINISH) && (first_call))
  {
    // MZ_FINISH on the first call implies that the input and output buffers are large enough to hold the entire compressed/decompressed file.
    decomp_flags |= TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF;
    in_bytes = pStream->avail_in; out_bytes = pStream->avail_out;
    status = tinfl_decompress(&pState->m_decomp, pStream->next_in, &in_bytes, pStream->next_out, pStream->next_out, &out_bytes, decomp_flags);
    pState->m_last_status = status;
    pStream->next_in += (mz_uint)in_bytes; pStream->avail_in -= (mz_uint)in_bytes; pStream->total_in += (mz_uint)in_bytes;
    pStream->adler = tinfl_get_adler32(&pState->m_decomp);
    pStream->next_out += (mz_uint)out_bytes; pStream->avail_out -= (mz_uint)out_bytes; pStream->total_out += (mz_uint)out_bytes;

    if (status < 0)
      return MZ_DATA_ERROR;
    else if (status != TINFL_STATUS_DONE)
    {
      pState->m_last_status = TINFL_STATUS_FAILED;
      return MZ_BUF_ERROR;
    }
    return MZ_STREAM_END;
  }
  // flush != MZ_FINISH then we must assume there's more input.
  if (flush != MZ_FINISH) decomp_flags |= TINFL_FLAG_HAS_MORE_INPUT;

  if (pState->m_dict_avail)
  {
    n = MZ_MIN(pState->m_dict_avail, pStream->avail_out);
    memcpy(pStream->next_out, pState->m_dict + pState->m_dict_ofs, n);
    pStream->next_out += n; pStream->avail_out -= n; pStream->total_out += n;
    pState->m_dict_avail -= n; pState->m_dict_ofs = (pState->m_dict_ofs + n) & (TINFL_LZ_DICT_SIZE - 1);
    return ((pState->m_last_status == TINFL_STATUS_DONE) && (!pState->m_dict_avail)) ? MZ_STREAM_END : MZ_OK;
  }

  for ( ; ; )
  {
    in_bytes = pStream->avail_in;
    out_bytes = TINFL_LZ_DICT_SIZE - pState->m_dict_ofs;

    status = tinfl_decompress(&pState->m_decomp, pStream->next_in, &in_bytes, pState->m_dict, pState->m_dict + pState->m_dict_ofs, &out_bytes, decomp_flags);
    pState->m_last_status = status;

    pStream->next_in += (mz_uint)in_bytes; pStream->avail_in -= (mz_uint)in_bytes;
    pStream->total_in += (mz_uint)in_bytes; pStream->adler = tinfl_get_adler32(&pState->m_decomp);

    pState->m_dict_avail = (mz_uint)out_bytes;

    n = MZ_MIN(pState->m_dict_avail, pStream->avail_out);
    memcpy(pStream->next_out, pState->m_dict + pState->m_dict_ofs, n);
    pStream->next_out += n; pStream->avail_out -= n; pStream->total_out += n;
    pState->m_dict_avail -= n; pState->m_dict_ofs = (pState->m_dict_ofs + n) & (TINFL_LZ_DICT_SIZE - 1);

    if (status < 0)
       return MZ_DATA_ERROR; // Stream is corrupted (there could be some uncompressed data left in the output dictionary - oh well).
    else if ((status == TINFL_STATUS_NEEDS_MORE_INPUT) && (!orig_avail_in))
      return MZ_BUF_ERROR; // Signal caller that we can't make forward progress without supplying more input or by setting flush to MZ_FINISH.
    else if (flush == MZ_FINISH)
    {
       // The output buffer MUST be large to hold the remaining uncompressed data when flush==MZ_FINISH.
       if (status == TINFL_STATUS_DONE)
          return pState->m_dict_avail ? MZ_BUF_ERROR : MZ_STREAM_END;
       // status here must be TINFL_STATUS_HAS_MORE_OUTPUT, which means there's at least 1 more byte on the way. If there's no more room left in the output buffer then something is wrong.
       else if (!pStream->avail_out)
          return MZ_BUF_ERROR;
    }
    else if ((status == TINFL_STATUS_DONE) || (!pStream->avail_in) || (!pStream->avail_out) || (pState->m_dict_avail))
      break;
  }

  return ((status == TINFL_STATUS_DONE) && (!pState->m_dict_avail)) ? MZ_STREAM_END : MZ_OK;
}

int mz_inflateEnd(mz_streamp pStream)
{
  if (!pStream)
    return MZ_STREAM_ERROR;
  if (pStream->state)
  {
    pStream->zfree(pStream->opaque, pStream->state);
    pStream->state = NULL;
  }
  return MZ_OK;
}

int mz_uncompress(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len)
{
  mz_stream stream;
  int status;
  memset(&stream, 0, sizeof(stream));

  // In case mz_ulong is 64-bits (argh I hate longs).
  if ((source_len | *pDest_len) > 0xFFFFFFFFU) return MZ_PARAM_ERROR;

  stream.next_in = pSource;
  stream.avail_in = (mz_uint32)source_len;
  stream.next_out = pDest;
  stream.avail_out = (mz_uint32)*pDest_len;

  status = mz_inflateInit(&stream);
  if (status != MZ_OK)
    return status;

  status = mz_inflate(&stream, MZ_FINISH);
  if (status != MZ_STREAM_END)
  {
    mz_inflateEnd(&stream);
    return ((status == MZ_BUF_ERROR) && (!stream.avail_in)) ? MZ_DATA_ERROR : status;
  }
  *pDest_len = stream.total_out;

  return mz_inflateEnd(&stream);
}

const char *mz_error(int err)
{
  static struct { int m_err; const char *m_pDesc; } s_error_descs[] =
  {
    { MZ_OK, "" }, { MZ_STREAM_END, "stream end" }, { MZ_NEED_DICT, "need dictionary" }, { MZ_ERRNO, "file error" }, { MZ_STREAM_ERROR, "stream error" },
    { MZ_DATA_ERROR, "data error" }, { MZ_MEM_ERROR, "out of memory" }, { MZ_BUF_ERROR, "buf error" }, { MZ_VERSION_ERROR, "version error" }, { MZ_PARAM_ERROR, "parameter error" }
  };
  mz_uint i; for (i = 0; i < sizeof(s_error_descs) / sizeof(s_error_descs[0]); ++i) if (s_error_descs[i].m_err == err) return s_error_descs[i].m_pDesc;
  return NULL;
}

#endif //MINIZ_NO_ZLIB_APIS

// ------------------- Low-level Decompression (completely independent from all compression API's)

#define TINFL_MEMCPY(d, s, l) memcpy(d, s, l)
#define TINFL_MEMSET(p, c, l) memset(p, c, l)

#define TINFL_CR_BEGIN switch(r->m_state) { case 0:
#define TINFL_CR_RETURN(state_index, result) do { status = result; r->m_state = state_index; goto common_exit; case state_index:; } MZ_MACRO_END
#define TINFL_CR_RETURN_FOREVER(state_index, result) do { for ( ; ; ) { TINFL_CR_RETURN(state_index, result); } } MZ_MACRO_END
#define TINFL_CR_FINISH }

// TODO: If the caller has indicated that there's no more input, and we attempt to read beyond the input buf, then something is wrong with the input because the inflator never
// reads ahead more than it needs to. Currently TINFL_GET_BYTE() pads the end of the stream with 0's in this scenario.
#define TINFL_GET_BYTE(state_index, c) do { \
  if (pIn_buf_cur >= pIn_buf_end) { \
    for ( ; ; ) { \
      if (decomp_flags & TINFL_FLAG_HAS_MORE_INPUT) { \
        TINFL_CR_RETURN(state_index, TINFL_STATUS_NEEDS_MORE_INPUT); \
        if (pIn_buf_cur < pIn_buf_end) { \
          c = *pIn_buf_cur++; \
          break; \
        } \
      } else { \
        c = 0; \
        break; \
      } \
    } \
  } else c = *pIn_buf_cur++; } MZ_MACRO_END

#define TINFL_NEED_BITS(state_index, n) do { mz_uint c; TINFL_GET_BYTE(state_index, c); bit_buf |= (((tinfl_bit_buf_t)c) << num_bits); num_bits += 8; } while (num_bits < (mz_uint)(n))
#define TINFL_SKIP_BITS(state_index, n) do { if (num_bits < (mz_uint)(n)) { TINFL_NEED_BITS(state_index, n); } bit_buf >>= (n); num_bits -= (n); } MZ_MACRO_END
#define TINFL_GET_BITS(state_index, b, n) do { if (num_bits < (mz_uint)(n)) { TINFL_NEED_BITS(state_index, n); } b = bit_buf & ((1 << (n)) - 1); bit_buf >>= (n); num_bits -= (n); } MZ_MACRO_END

// TINFL_HUFF_BITBUF_FILL() is only used rarely, when the number of bytes remaining in the input buffer falls below 2.
// It reads just enough bytes from the input stream that are needed to decode the next Huffman code (and absolutely no more). It works by trying to fully decode a
// Huffman code by using whatever bits are currently present in the bit buffer. If this fails, it reads another byte, and tries again until it succeeds or until the
// bit buffer contains >=15 bits (deflate's max. Huffman code size).
#define TINFL_HUFF_BITBUF_FILL(state_index, pHuff) \
  do { \
    temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]; \
    if (temp >= 0) { \
      code_len = temp >> 9; \
      if ((code_len) && (num_bits >= code_len)) \
      break; \
    } else if (num_bits > TINFL_FAST_LOOKUP_BITS) { \
       code_len = TINFL_FAST_LOOKUP_BITS; \
       do { \
          temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; \
       } while ((temp < 0) && (num_bits >= (code_len + 1))); if (temp >= 0) break; \
    } TINFL_GET_BYTE(state_index, c); bit_buf |= (((tinfl_bit_buf_t)c) << num_bits); num_bits += 8; \
  } while (num_bits < 15);

// TINFL_HUFF_DECODE() decodes the next Huffman coded symbol. It's more complex than you would initially expect because the zlib API expects the decompressor to never read
// beyond the final byte of the deflate stream. (In other words, when this macro wants to read another byte from the input, it REALLY needs another byte in order to fully
// decode the next Huffman code.) Handling this properly is particularly important on raw deflate (non-zlib) streams, which aren't followed by a byte aligned adler-32.
// The slow path is only executed at the very end of the input buffer.
#define TINFL_HUFF_DECODE(state_index, sym, pHuff) do { \
  int temp; mz_uint code_len, c; \
  if (num_bits < 15) { \
    if ((pIn_buf_end - pIn_buf_cur) < 2) { \
       TINFL_HUFF_BITBUF_FILL(state_index, pHuff); \
    } else { \
       bit_buf |= (((tinfl_bit_buf_t)pIn_buf_cur[0]) << num_bits) | (((tinfl_bit_buf_t)pIn_buf_cur[1]) << (num_bits + 8)); pIn_buf_cur += 2; num_bits += 16; \
    } \
  } \
  if ((temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0) \
    code_len = temp >> 9, temp &= 511; \
  else { \
    code_len = TINFL_FAST_LOOKUP_BITS; do { temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; } while (temp < 0); \
  } sym = temp; bit_buf >>= code_len; num_bits -= code_len; } MZ_MACRO_END

tinfl_status tinfl_decompress(tinfl_decompressor *r, const mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mz_uint8 *pOut_buf_start, mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mz_uint32 decomp_flags)
{
  static const int s_length_base[31] = { 3,4,5,6,7,8,9,10,11,13, 15,17,19,23,27,31,35,43,51,59, 67,83,99,115,131,163,195,227,258,0,0 };
  static const int s_length_extra[31]= { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };
  static const int s_dist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193, 257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
  static const int s_dist_extra[32] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
  static const mz_uint8 s_length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
  static const int s_min_table_sizes[3] = { 257, 1, 4 };

  tinfl_status status = TINFL_STATUS_FAILED; mz_uint32 num_bits, dist, counter, num_extra; tinfl_bit_buf_t bit_buf;
  const mz_uint8 *pIn_buf_cur = pIn_buf_next, *const pIn_buf_end = pIn_buf_next + *pIn_buf_size;
  mz_uint8 *pOut_buf_cur = pOut_buf_next, *const pOut_buf_end = pOut_buf_next + *pOut_buf_size;
  size_t out_buf_size_mask = (decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF) ? (size_t)-1 : ((pOut_buf_next - pOut_buf_start) + *pOut_buf_size) - 1, dist_from_out_buf_start;

  // Ensure the output buffer's size is a power of 2, unless the output buffer is large enough to hold the entire output file (in which case it doesn't matter).
  if (((out_buf_size_mask + 1) & out_buf_size_mask) || (pOut_buf_next < pOut_buf_start)) { *pIn_buf_size = *pOut_buf_size = 0; return TINFL_STATUS_BAD_PARAM; }

  num_bits = r->m_num_bits; bit_buf = r->m_bit_buf; dist = r->m_dist; counter = r->m_counter; num_extra = r->m_num_extra; dist_from_out_buf_start = r->m_dist_from_out_buf_start;
  TINFL_CR_BEGIN

  bit_buf = num_bits = dist = counter = num_extra = r->m_zhdr0 = r->m_zhdr1 = 0; r->m_z_adler32 = r->m_check_adler32 = 1;
  if (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
    TINFL_GET_BYTE(1, r->m_zhdr0); TINFL_GET_BYTE(2, r->m_zhdr1);
    counter = (((r->m_zhdr0 * 256 + r->m_zhdr1) % 31 != 0) || (r->m_zhdr1 & 32) || ((r->m_zhdr0 & 15) != 8));
    if (!(decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)) counter |= (((1U << (8U + (r->m_zhdr0 >> 4))) > 32768U) || ((out_buf_size_mask + 1) < (size_t)(1U << (8U + (r->m_zhdr0 >> 4)))));
    if (counter) { TINFL_CR_RETURN_FOREVER(36, TINFL_STATUS_FAILED); }
  }

  do
  {
    TINFL_GET_BITS(3, r->m_final, 3); r->m_type = r->m_final >> 1;
    if (r->m_type == 0)
    {
      TINFL_SKIP_BITS(5, num_bits & 7);
      for (counter = 0; counter < 4; ++counter) { if (num_bits) TINFL_GET_BITS(6, r->m_raw_header[counter], 8); else TINFL_GET_BYTE(7, r->m_raw_header[counter]); }
      if ((counter = (r->m_raw_header[0] | (r->m_raw_header[1] << 8))) != (mz_uint)(0xFFFF ^ (r->m_raw_header[2] | (r->m_raw_header[3] << 8)))) { TINFL_CR_RETURN_FOREVER(39, TINFL_STATUS_FAILED); }
      while ((counter) && (num_bits))
      {
        TINFL_GET_BITS(51, dist, 8);
        while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(52, TINFL_STATUS_HAS_MORE_OUTPUT); }
        *pOut_buf_cur++ = (mz_uint8)dist;
        counter--;
      }
      while (counter)
      {
        size_t n; while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(9, TINFL_STATUS_HAS_MORE_OUTPUT); }
        while (pIn_buf_cur >= pIn_buf_end)
        {
          if (decomp_flags & TINFL_FLAG_HAS_MORE_INPUT)
          {
            TINFL_CR_RETURN(38, TINFL_STATUS_NEEDS_MORE_INPUT);
          }
          else
          {
            TINFL_CR_RETURN_FOREVER(40, TINFL_STATUS_FAILED);
          }
        }
        n = MZ_MIN(MZ_MIN((size_t)(pOut_buf_end - pOut_buf_cur), (size_t)(pIn_buf_end - pIn_buf_cur)), counter);
        TINFL_MEMCPY(pOut_buf_cur, pIn_buf_cur, n); pIn_buf_cur += n; pOut_buf_cur += n; counter -= (mz_uint)n;
      }
    }
    else if (r->m_type == 3)
    {
      TINFL_CR_RETURN_FOREVER(10, TINFL_STATUS_FAILED);
    }
    else
    {
      if (r->m_type == 1)
      {
        mz_uint8 *p = r->m_tables[0].m_code_size; mz_uint i;
        r->m_table_sizes[0] = 288; r->m_table_sizes[1] = 32; TINFL_MEMSET(r->m_tables[1].m_code_size, 5, 32);
        for ( i = 0; i <= 143; ++i) *p++ = 8; for ( ; i <= 255; ++i) *p++ = 9; for ( ; i <= 279; ++i) *p++ = 7; for ( ; i <= 287; ++i) *p++ = 8;
      }
      else
      {
        for (counter = 0; counter < 3; counter++) { TINFL_GET_BITS(11, r->m_table_sizes[counter], "\05\05\04"[counter]); r->m_table_sizes[counter] += s_min_table_sizes[counter]; }
        MZ_CLEAR_OBJ(r->m_tables[2].m_code_size); for (counter = 0; counter < r->m_table_sizes[2]; counter++) { mz_uint s; TINFL_GET_BITS(14, s, 3); r->m_tables[2].m_code_size[s_length_dezigzag[counter]] = (mz_uint8)s; }
        r->m_table_sizes[2] = 19;
      }
      for ( ; (int)r->m_type >= 0; r->m_type--)
      {
        int tree_next, tree_cur; tinfl_huff_table *pTable;
        mz_uint i, j, used_syms, total, sym_index, next_code[17], total_syms[16]; pTable = &r->m_tables[r->m_type]; MZ_CLEAR_OBJ(total_syms); MZ_CLEAR_OBJ(pTable->m_look_up); MZ_CLEAR_OBJ(pTable->m_tree);
        for (i = 0; i < r->m_table_sizes[r->m_type]; ++i) total_syms[pTable->m_code_size[i]]++;
        used_syms = 0, total = 0; next_code[0] = next_code[1] = 0;
        for (i = 1; i <= 15; ++i) { used_syms += total_syms[i]; next_code[i + 1] = (total = ((total + total_syms[i]) << 1)); }
        if ((65536 != total) && (used_syms > 1))
        {
          TINFL_CR_RETURN_FOREVER(35, TINFL_STATUS_FAILED);
        }
        for (tree_next = -1, sym_index = 0; sym_index < r->m_table_sizes[r->m_type]; ++sym_index)
        {
          mz_uint rev_code = 0, l, cur_code, code_size = pTable->m_code_size[sym_index]; if (!code_size) continue;
          cur_code = next_code[code_size]++; for (l = code_size; l > 0; l--, cur_code >>= 1) rev_code = (rev_code << 1) | (cur_code & 1);
          if (code_size <= TINFL_FAST_LOOKUP_BITS) { mz_int16 k = (mz_int16)((code_size << 9) | sym_index); while (rev_code < TINFL_FAST_LOOKUP_SIZE) { pTable->m_look_up[rev_code] = k; rev_code += (1 << code_size); } continue; }
          if (0 == (tree_cur = pTable->m_look_up[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)])) { pTable->m_look_up[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)] = (mz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; }
          rev_code >>= (TINFL_FAST_LOOKUP_BITS - 1);
          for (j = code_size; j > (TINFL_FAST_LOOKUP_BITS + 1); j--)
          {
            tree_cur -= ((rev_code >>= 1) & 1);
            if (!pTable->m_tree[-tree_cur - 1]) { pTable->m_tree[-tree_cur - 1] = (mz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; } else tree_cur = pTable->m_tree[-tree_cur - 1];
          }
          tree_cur -= ((rev_code >>= 1) & 1); pTable->m_tree[-tree_cur - 1] = (mz_int16)sym_index;
        }
        if (r->m_type == 2)
        {
          for (counter = 0; counter < (r->m_table_sizes[0] + r->m_table_sizes[1]); )
          {
            mz_uint s; TINFL_HUFF_DECODE(16, dist, &r->m_tables[2]); if (dist < 16) { r->m_len_codes[counter++] = (mz_uint8)dist; continue; }
            if ((dist == 16) && (!counter))
            {
              TINFL_CR_RETURN_FOREVER(17, TINFL_STATUS_FAILED);
            }
            num_extra = "\02\03\07"[dist - 16]; TINFL_GET_BITS(18, s, num_extra); s += "\03\03\013"[dist - 16];
            TINFL_MEMSET(r->m_len_codes + counter, (dist == 16) ? r->m_len_codes[counter - 1] : 0, s); counter += s;
          }
          if ((r->m_table_sizes[0] + r->m_table_sizes[1]) != counter)
          {
            TINFL_CR_RETURN_FOREVER(21, TINFL_STATUS_FAILED);
          }
          TINFL_MEMCPY(r->m_tables[0].m_code_size, r->m_len_codes, r->m_table_sizes[0]); TINFL_MEMCPY(r->m_tables[1].m_code_size, r->m_len_codes + r->m_table_sizes[0], r->m_table_sizes[1]);
        }
      }
      for ( ; ; )
      {
        mz_uint8 *pSrc;
        for ( ; ; )
        {
          if (((pIn_buf_end - pIn_buf_cur) < 4) || ((pOut_buf_end - pOut_buf_cur) < 2))
          {
            TINFL_HUFF_DECODE(23, counter, &r->m_tables[0]);
            if (counter >= 256)
              break;
            while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(24, TINFL_STATUS_HAS_MORE_OUTPUT); }
            *pOut_buf_cur++ = (mz_uint8)counter;
          }
          else
          {
            int sym2; mz_uint code_len;
#if TINFL_USE_64BIT_BITBUF
            if (num_bits < 30) { bit_buf |= (((tinfl_bit_buf_t)MZ_READ_LE32(pIn_buf_cur)) << num_bits); pIn_buf_cur += 4; num_bits += 32; }
#else
            if (num_bits < 15) { bit_buf |= (((tinfl_bit_buf_t)MZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
            if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
              code_len = sym2 >> 9;
            else
            {
              code_len = TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
            }
            counter = sym2; bit_buf >>= code_len; num_bits -= code_len;
            if (counter & 256)
              break;

#if !TINFL_USE_64BIT_BITBUF
            if (num_bits < 15) { bit_buf |= (((tinfl_bit_buf_t)MZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
            if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
              code_len = sym2 >> 9;
            else
            {
              code_len = TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
            }
            bit_buf >>= code_len; num_bits -= code_len;

            pOut_buf_cur[0] = (mz_uint8)counter;
            if (sym2 & 256)
            {
              pOut_buf_cur++;
              counter = sym2;
              break;
            }
            pOut_buf_cur[1] = (mz_uint8)sym2;
            pOut_buf_cur += 2;
          }
        }
        if ((counter &= 511) == 256) break;

        num_extra = s_length_extra[counter - 257]; counter = s_length_base[counter - 257];
        if (num_extra) { mz_uint extra_bits; TINFL_GET_BITS(25, extra_bits, num_extra); counter += extra_bits; }

        TINFL_HUFF_DECODE(26, dist, &r->m_tables[1]);
        num_extra = s_dist_extra[dist]; dist = s_dist_base[dist];
        if (num_extra) { mz_uint extra_bits; TINFL_GET_BITS(27, extra_bits, num_extra); dist += extra_bits; }

        dist_from_out_buf_start = pOut_buf_cur - pOut_buf_start;
        if ((dist > dist_from_out_buf_start) && (decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF))
        {
          TINFL_CR_RETURN_FOREVER(37, TINFL_STATUS_FAILED);
        }

        pSrc = pOut_buf_start + ((dist_from_out_buf_start - dist) & out_buf_size_mask);

        if ((MZ_MAX(pOut_buf_cur, pSrc) + counter) > pOut_buf_end)
        {
          while (counter--)
          {
            while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(53, TINFL_STATUS_HAS_MORE_OUTPUT); }
            *pOut_buf_cur++ = pOut_buf_start[(dist_from_out_buf_start++ - dist) & out_buf_size_mask];
          }
          continue;
        }
#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES
        else if ((counter >= 9) && (counter <= dist))
        {
          const mz_uint8 *pSrc_end = pSrc + (counter & ~7);
          do
          {
            ((mz_uint32 *)pOut_buf_cur)[0] = ((const mz_uint32 *)pSrc)[0];
            ((mz_uint32 *)pOut_buf_cur)[1] = ((const mz_uint32 *)pSrc)[1];
            pOut_buf_cur += 8;
          } while ((pSrc += 8) < pSrc_end);
          if ((counter &= 7) < 3)
          {
            if (counter)
            {
              pOut_buf_cur[0] = pSrc[0];
              if (counter > 1)
                pOut_buf_cur[1] = pSrc[1];
              pOut_buf_cur += counter;
            }
            continue;
          }
        }
#endif
        do
        {
          pOut_buf_cur[0] = pSrc[0];
          pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur[2] = pSrc[2];
          pOut_buf_cur += 3; pSrc += 3;
        } while ((int)(counter -= 3) > 2);
        if ((int)counter > 0)
        {
          pOut_buf_cur[0] = pSrc[0];
          if ((int)counter > 1)
            pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur += counter;
        }
      }
    }
  } while (!(r->m_final & 1));
  if (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
    TINFL_SKIP_BITS(32, num_bits & 7); for (counter = 0; counter < 4; ++counter) { mz_uint s; if (num_bits) TINFL_GET_BITS(41, s, 8); else TINFL_GET_BYTE(42, s); r->m_z_adler32 = (r->m_z_adler32 << 8) | s; }
  }
  TINFL_CR_RETURN_FOREVER(34, TINFL_STATUS_DONE);
  TINFL_CR_FINISH

common_exit:
  r->m_num_bits = num_bits; r->m_bit_buf = bit_buf; r->m_dist = dist; r->m_counter = counter; r->m_num_extra = num_extra; r->m_dist_from_out_buf_start = dist_from_out_buf_start;
  *pIn_buf_size = pIn_buf_cur - pIn_buf_next; *pOut_buf_size = pOut_buf_cur - pOut_buf_next;
  if ((decomp_flags & (TINFL_FLAG_PARSE_ZLIB_HEADER | TINFL_FLAG_COMPUTE_ADLER32)) && (status >= 0))
  {
    const mz_uint8 *ptr = pOut_buf_next; size_t buf_len = *pOut_buf_size;
    mz_uint32 i, s1 = r->m_check_adler32 & 0xffff, s2 = r->m_check_adler32 >> 16; size_t block_len = buf_len % 5552;
    while (buf_len)
    {
      for (i = 0; i + 7 < block_len; i += 8, ptr += 8)
      {
        s1 += ptr[0], s2 += s1; s1 += ptr[1], s2 += s1; s1 += ptr[2], s2 += s1; s1 += ptr[3], s2 += s1;
        s1 += ptr[4], s2 += s1; s1 += ptr[5], s2 += s1; s1 += ptr[6], s2 += s1; s1 += ptr[7], s2 += s1;
      }
      for ( ; i < block_len; ++i) s1 += *ptr++, s2 += s1;
      s1 %= 65521U, s2 %= 65521U; buf_len -= block_len; block_len = 5552;
    }
    r->m_check_adler32 = (s2 << 16) + s1; if ((status == TINFL_STATUS_DONE) && (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER) && (r->m_check_adler32 != r->m_z_adler32)) status = TINFL_STATUS_ADLER32_MISMATCH;
  }
  return status;
}

// Higher level helper functions.
void *tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags)
{
  tinfl_decompressor decomp; void *pBuf = NULL, *pNew_buf; size_t src_buf_ofs = 0, out_buf_capacity = 0;
  *pOut_len = 0;
  tinfl_init(&decomp);
  for ( ; ; )
  {
    size_t src_buf_size = src_buf_len - src_buf_ofs, dst_buf_size = out_buf_capacity - *pOut_len, new_out_buf_capacity;
    tinfl_status status = tinfl_decompress(&decomp, (const mz_uint8*)pSrc_buf + src_buf_ofs, &src_buf_size, (mz_uint8*)pBuf, pBuf ? (mz_uint8*)pBuf + *pOut_len : NULL, &dst_buf_size,
      (flags & ~TINFL_FLAG_HAS_MORE_INPUT) | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
    if ((status < 0) || (status == TINFL_STATUS_NEEDS_MORE_INPUT))
    {
      MZ_FREE(pBuf); *pOut_len = 0; return NULL;
    }
    src_buf_ofs += src_buf_size;
    *pOut_len += dst_buf_size;
    if (status == TINFL_STATUS_DONE) break;
    new_out_buf_capacity = out_buf_capacity * 2; if (new_out_buf_capacity < 128) new_out_buf_capacity = 128;
    pNew_buf = MZ_REALLOC(pBuf, new_out_buf_capacity);
    if (!pNew_buf)
    {
      MZ_FREE(pBuf); *pOut_len = 0; return NULL;
    }
    pBuf = pNew_buf; out_buf_capacity = new_out_buf_capacity;
  }
  return pBuf;
}

size_t tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags)
{
  tinfl_decompressor decomp; tinfl_status status; tinfl_init(&decomp);
  status = tinfl_decompress(&decomp, (const mz_uint8*)pSrc_buf, &src_buf_len, (mz_uint8*)pOut_buf, (mz_uint8*)pOut_buf, &out_buf_len, (flags & ~TINFL_FLAG_HAS_MORE_INPUT) | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
  return (status != TINFL_STATUS_DONE) ? TINFL_DECOMPRESS_MEM_TO_MEM_FAILED : out_buf_len;
}

int tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  int result = 0;
  tinfl_decompressor decomp;
  mz_uint8 *pDict = (mz_uint8*)MZ_MALLOC(TINFL_LZ_DICT_SIZE); size_t in_buf_ofs = 0, dict_ofs = 0;
  if (!pDict)
    return TINFL_STATUS_FAILED;
  tinfl_init(&decomp);
  for ( ; ; )
  {
    size_t in_buf_size = *pIn_buf_size - in_buf_ofs, dst_buf_size = TINFL_LZ_DICT_SIZE - dict_ofs;
    tinfl_status status = tinfl_decompress(&decomp, (const mz_uint8*)pIn_buf + in_buf_ofs, &in_buf_size, pDict, pDict + dict_ofs, &dst_buf_size,
      (flags & ~(TINFL_FLAG_HAS_MORE_INPUT | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)));
    in_buf_ofs += in_buf_size;
    if ((dst_buf_size) && (!(*pPut_buf_func)(pDict + dict_ofs, (int)dst_buf_size, pPut_buf_user)))
      break;
    if (status != TINFL_STATUS_HAS_MORE_OUTPUT)
    {
      result = (status == TINFL_STATUS_DONE);
      break;
    }
    dict_ofs = (dict_ofs + dst_buf_size) & (TINFL_LZ_DICT_SIZE - 1);
  }
  MZ_FREE(pDict);
  *pIn_buf_size = in_buf_ofs;
  return result;
}

// ------------------- Low-level Compression (independent from all decompression API's)

// Purposely making these tables static for faster init and thread safety.
static const mz_uint16 s_tdefl_len_sym[256] = {
  257,258,259,260,261,262,263,264,265,265,266,266,267,267,268,268,269,269,269,269,270,270,270,270,271,271,271,271,272,272,272,272,
  273,273,273,273,273,273,273,273,274,274,274,274,274,274,274,274,275,275,275,275,275,275,275,275,276,276,276,276,276,276,276,276,
  277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,
  279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,280,280,280,280,280,280,280,280,280,280,280,280,280,280,280,280,
  281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,
  282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,
  283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,
  284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,285 };

static const mz_uint8 s_tdefl_len_extra[256] = {
  0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0 };

static const mz_uint8 s_tdefl_small_dist_sym[512] = {
  0,1,2,3,4,4,5,5,6,6,6,6,7,7,7,7,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,
  11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,
  13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,
  14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
  14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
  15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
  17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17 };

static const mz_uint8 s_tdefl_small_dist_extra[512] = {
  0,0,0,0,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7 };

static const mz_uint8 s_tdefl_large_dist_sym[128] = {
  0,0,18,19,20,20,21,21,22,22,22,22,23,23,23,23,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,26,26,26,26,
  26,26,26,26,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,
  28,28,28,28,28,28,28,28,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29,29 };

static const mz_uint8 s_tdefl_large_dist_extra[128] = {
  0,0,8,8,9,9,9,9,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
  12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
  13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13 };

// Radix sorts tdefl_sym_freq[] array by 16-bit key m_key. Returns ptr to sorted values.
typedef struct { mz_uint16 m_key, m_sym_index; } tdefl_sym_freq;
static tdefl_sym_freq* tdefl_radix_sort_syms(mz_uint num_syms, tdefl_sym_freq* pSyms0, tdefl_sym_freq* pSyms1)
{
  mz_uint32 total_passes = 2, pass_shift, pass, i, hist[256 * 2]; tdefl_sym_freq* pCur_syms = pSyms0, *pNew_syms = pSyms1; MZ_CLEAR_OBJ(hist);
  for (i = 0; i < num_syms; i++) { mz_uint freq = pSyms0[i].m_key; hist[freq & 0xFF]++; hist[256 + ((freq >> 8) & 0xFF)]++; }
  while ((total_passes > 1) && (num_syms == hist[(total_passes - 1) * 256])) total_passes--;
  for (pass_shift = 0, pass = 0; pass < total_passes; pass++, pass_shift += 8)
  {
    const mz_uint32* pHist = &hist[pass << 8];
    mz_uint offsets[256], cur_ofs = 0;
    for (i = 0; i < 256; i++) { offsets[i] = cur_ofs; cur_ofs += pHist[i]; }
    for (i = 0; i < num_syms; i++) pNew_syms[offsets[(pCur_syms[i].m_key >> pass_shift) & 0xFF]++] = pCur_syms[i];
    { tdefl_sym_freq* t = pCur_syms; pCur_syms = pNew_syms; pNew_syms = t; }
  }
  return pCur_syms;
}

// tdefl_calculate_minimum_redundancy() originally written by: Alistair Moffat, alistair@cs.mu.oz.au, Jyrki Katajainen, jyrki@diku.dk, November 1996.
static void tdefl_calculate_minimum_redundancy(tdefl_sym_freq *A, int n)
{
  int root, leaf, next, avbl, used, dpth;
  if (n==0) return; else if (n==1) { A[0].m_key = 1; return; }
  A[0].m_key += A[1].m_key; root = 0; leaf = 2;
  for (next=1; next < n-1; next++)
  {
    if (leaf>=n || A[root].m_key<A[leaf].m_key) { A[next].m_key = A[root].m_key; A[root++].m_key = (mz_uint16)next; } else A[next].m_key = A[leaf++].m_key;
    if (leaf>=n || (root<next && A[root].m_key<A[leaf].m_key)) { A[next].m_key = (mz_uint16)(A[next].m_key + A[root].m_key); A[root++].m_key = (mz_uint16)next; } else A[next].m_key = (mz_uint16)(A[next].m_key + A[leaf++].m_key);
  }
  A[n-2].m_key = 0; for (next=n-3; next>=0; next--) A[next].m_key = A[A[next].m_key].m_key+1;
  avbl = 1; used = dpth = 0; root = n-2; next = n-1;
  while (avbl>0)
  {
    while (root>=0 && (int)A[root].m_key==dpth) { used++; root--; }
    while (avbl>used) { A[next--].m_key = (mz_uint16)(dpth); avbl--; }
    avbl = 2*used; dpth++; used = 0;
  }
}

// Limits canonical Huffman code table's max code size.
enum { TDEFL_MAX_SUPPORTED_HUFF_CODESIZE = 32 };
static void tdefl_huffman_enforce_max_code_size(int *pNum_codes, int code_list_len, int max_code_size)
{
  int i; mz_uint32 total = 0; if (code_list_len <= 1) return;
  for (i = max_code_size + 1; i <= TDEFL_MAX_SUPPORTED_HUFF_CODESIZE; i++) pNum_codes[max_code_size] += pNum_codes[i];
  for (i = max_code_size; i > 0; i--) total += (((mz_uint32)pNum_codes[i]) << (max_code_size - i));
  while (total != (1UL << max_code_size))
  {
    pNum_codes[max_code_size]--;
    for (i = max_code_size - 1; i > 0; i--) if (pNum_codes[i]) { pNum_codes[i]--; pNum_codes[i + 1] += 2; break; }
    total--;
  }
}

static void tdefl_optimize_huffman_table(tdefl_compressor *d, int table_num, int table_len, int code_size_limit, int static_table)
{
  int i, j, l, num_codes[1 + TDEFL_MAX_SUPPORTED_HUFF_CODESIZE]; mz_uint next_code[TDEFL_MAX_SUPPORTED_HUFF_CODESIZE + 1]; MZ_CLEAR_OBJ(num_codes);
  if (static_table)
  {
    for (i = 0; i < table_len; i++) num_codes[d->m_huff_code_sizes[table_num][i]]++;
  }
  else
  {
    tdefl_sym_freq syms0[TDEFL_MAX_HUFF_SYMBOLS], syms1[TDEFL_MAX_HUFF_SYMBOLS], *pSyms;
    int num_used_syms = 0;
    const mz_uint16 *pSym_count = &d->m_huff_count[table_num][0];
    for (i = 0; i < table_len; i++) if (pSym_count[i]) { syms0[num_used_syms].m_key = (mz_uint16)pSym_count[i]; syms0[num_used_syms++].m_sym_index = (mz_uint16)i; }

    pSyms = tdefl_radix_sort_syms(num_used_syms, syms0, syms1); tdefl_calculate_minimum_redundancy(pSyms, num_used_syms);

    for (i = 0; i < num_used_syms; i++) num_codes[pSyms[i].m_key]++;

    tdefl_huffman_enforce_max_code_size(num_codes, num_used_syms, code_size_limit);

    MZ_CLEAR_OBJ(d->m_huff_code_sizes[table_num]); MZ_CLEAR_OBJ(d->m_huff_codes[table_num]);
    for (i = 1, j = num_used_syms; i <= code_size_limit; i++)
      for (l = num_codes[i]; l > 0; l--) d->m_huff_code_sizes[table_num][pSyms[--j].m_sym_index] = (mz_uint8)(i);
  }

  next_code[1] = 0; for (j = 0, i = 2; i <= code_size_limit; i++) next_code[i] = j = ((j + num_codes[i - 1]) << 1);

  for (i = 0; i < table_len; i++)
  {
    mz_uint rev_code = 0, code, code_size; if ((code_size = d->m_huff_code_sizes[table_num][i]) == 0) continue;
    code = next_code[code_size]++; for (l = code_size; l > 0; l--, code >>= 1) rev_code = (rev_code << 1) | (code & 1);
    d->m_huff_codes[table_num][i] = (mz_uint16)rev_code;
  }
}

#define TDEFL_PUT_BITS(b, l) do { \
  mz_uint bits = b; mz_uint len = l; MZ_ASSERT(bits <= ((1U << len) - 1U)); \
  d->m_bit_buffer |= (bits << d->m_bits_in); d->m_bits_in += len; \
  while (d->m_bits_in >= 8) { \
    if (d->m_pOutput_buf < d->m_pOutput_buf_end) \
      *d->m_pOutput_buf++ = (mz_uint8)(d->m_bit_buffer); \
      d->m_bit_buffer >>= 8; \
      d->m_bits_in -= 8; \
  } \
} MZ_MACRO_END

#define TDEFL_RLE_PREV_CODE_SIZE() { if (rle_repeat_count) { \
  if (rle_repeat_count < 3) { \
    d->m_huff_count[2][prev_code_size] = (mz_uint16)(d->m_huff_count[2][prev_code_size] + rle_repeat_count); \
    while (rle_repeat_count--) packed_code_sizes[num_packed_code_sizes++] = prev_code_size; \
  } else { \
    d->m_huff_count[2][16] = (mz_uint16)(d->m_huff_count[2][16] + 1); packed_code_sizes[num_packed_code_sizes++] = 16; packed_code_sizes[num_packed_code_sizes++] = (mz_uint8)(rle_repeat_count - 3); \
} rle_repeat_count = 0; } }

#define TDEFL_RLE_ZERO_CODE_SIZE() { if (rle_z_count) { \
  if (rle_z_count < 3) { \
    d->m_huff_count[2][0] = (mz_uint16)(d->m_huff_count[2][0] + rle_z_count); while (rle_z_count--) packed_code_sizes[num_packed_code_sizes++] = 0; \
  } else if (rle_z_count <= 10) { \
    d->m_huff_count[2][17] = (mz_uint16)(d->m_huff_count[2][17] + 1); packed_code_sizes[num_packed_code_sizes++] = 17; packed_code_sizes[num_packed_code_sizes++] = (mz_uint8)(rle_z_count - 3); \
  } else { \
    d->m_huff_count[2][18] = (mz_uint16)(d->m_huff_count[2][18] + 1); packed_code_sizes[num_packed_code_sizes++] = 18; packed_code_sizes[num_packed_code_sizes++] = (mz_uint8)(rle_z_count - 11); \
} rle_z_count = 0; } }

static mz_uint8 s_tdefl_packed_code_size_syms_swizzle[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

static void tdefl_start_dynamic_block(tdefl_compressor *d)
{
  int num_lit_codes, num_dist_codes, num_bit_lengths; mz_uint i, total_code_sizes_to_pack, num_packed_code_sizes, rle_z_count, rle_repeat_count, packed_code_sizes_index;
  mz_uint8 code_sizes_to_pack[TDEFL_MAX_HUFF_SYMBOLS_0 + TDEFL_MAX_HUFF_SYMBOLS_1], packed_code_sizes[TDEFL_MAX_HUFF_SYMBOLS_0 + TDEFL_MAX_HUFF_SYMBOLS_1], prev_code_size = 0xFF;

  d->m_huff_count[0][256] = 1;

  tdefl_optimize_huffman_table(d, 0, TDEFL_MAX_HUFF_SYMBOLS_0, 15, MZ_FALSE);
  tdefl_optimize_huffman_table(d, 1, TDEFL_MAX_HUFF_SYMBOLS_1, 15, MZ_FALSE);

  for (num_lit_codes = 286; num_lit_codes > 257; num_lit_codes--) if (d->m_huff_code_sizes[0][num_lit_codes - 1]) break;
  for (num_dist_codes = 30; num_dist_codes > 1; num_dist_codes--) if (d->m_huff_code_sizes[1][num_dist_codes - 1]) break;

  memcpy(code_sizes_to_pack, &d->m_huff_code_sizes[0][0], num_lit_codes);
  memcpy(code_sizes_to_pack + num_lit_codes, &d->m_huff_code_sizes[1][0], num_dist_codes);
  total_code_sizes_to_pack = num_lit_codes + num_dist_codes; num_packed_code_sizes = 0; rle_z_count = 0; rle_repeat_count = 0;

  memset(&d->m_huff_count[2][0], 0, sizeof(d->m_huff_count[2][0]) * TDEFL_MAX_HUFF_SYMBOLS_2);
  for (i = 0; i < total_code_sizes_to_pack; i++)
  {
    mz_uint8 code_size = code_sizes_to_pack[i];
    if (!code_size)
    {
      TDEFL_RLE_PREV_CODE_SIZE();
      if (++rle_z_count == 138) { TDEFL_RLE_ZERO_CODE_SIZE(); }
    }
    else
    {
      TDEFL_RLE_ZERO_CODE_SIZE();
      if (code_size != prev_code_size)
      {
        TDEFL_RLE_PREV_CODE_SIZE();
        d->m_huff_count[2][code_size] = (mz_uint16)(d->m_huff_count[2][code_size] + 1); packed_code_sizes[num_packed_code_sizes++] = code_size;
      }
      else if (++rle_repeat_count == 6)
      {
        TDEFL_RLE_PREV_CODE_SIZE();
      }
    }
    prev_code_size = code_size;
  }
  if (rle_repeat_count) { TDEFL_RLE_PREV_CODE_SIZE(); } else { TDEFL_RLE_ZERO_CODE_SIZE(); }

  tdefl_optimize_huffman_table(d, 2, TDEFL_MAX_HUFF_SYMBOLS_2, 7, MZ_FALSE);

  TDEFL_PUT_BITS(2, 2);

  TDEFL_PUT_BITS(num_lit_codes - 257, 5);
  TDEFL_PUT_BITS(num_dist_codes - 1, 5);

  for (num_bit_lengths = 18; num_bit_lengths >= 0; num_bit_lengths--) if (d->m_huff_code_sizes[2][s_tdefl_packed_code_size_syms_swizzle[num_bit_lengths]]) break;
  num_bit_lengths = MZ_MAX(4, (num_bit_lengths + 1)); TDEFL_PUT_BITS(num_bit_lengths - 4, 4);
  for (i = 0; (int)i < num_bit_lengths; i++) TDEFL_PUT_BITS(d->m_huff_code_sizes[2][s_tdefl_packed_code_size_syms_swizzle[i]], 3);

  for (packed_code_sizes_index = 0; packed_code_sizes_index < num_packed_code_sizes; )
  {
    mz_uint code = packed_code_sizes[packed_code_sizes_index++]; MZ_ASSERT(code < TDEFL_MAX_HUFF_SYMBOLS_2);
    TDEFL_PUT_BITS(d->m_huff_codes[2][code], d->m_huff_code_sizes[2][code]);
    if (code >= 16) TDEFL_PUT_BITS(packed_code_sizes[packed_code_sizes_index++], "\02\03\07"[code - 16]);
  }
}

static void tdefl_start_static_block(tdefl_compressor *d)
{
  mz_uint i;
  mz_uint8 *p = &d->m_huff_code_sizes[0][0];

  for (i = 0; i <= 143; ++i) *p++ = 8;
  for ( ; i <= 255; ++i) *p++ = 9;
  for ( ; i <= 279; ++i) *p++ = 7;
  for ( ; i <= 287; ++i) *p++ = 8;

  memset(d->m_huff_code_sizes[1], 5, 32);

  tdefl_optimize_huffman_table(d, 0, 288, 15, MZ_TRUE);
  tdefl_optimize_huffman_table(d, 1, 32, 15, MZ_TRUE);

  TDEFL_PUT_BITS(1, 2);
}

static const mz_uint mz_bitmasks[17] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN && MINIZ_HAS_64BIT_REGISTERS
static mz_bool tdefl_compress_lz_codes(tdefl_compressor *d)
{
  mz_uint flags;
  mz_uint8 *pLZ_codes;
  mz_uint8 *pOutput_buf = d->m_pOutput_buf;
  mz_uint8 *pLZ_code_buf_end = d->m_pLZ_code_buf;
  mz_uint64 bit_buffer = d->m_bit_buffer;
  mz_uint bits_in = d->m_bits_in;

#define TDEFL_PUT_BITS_FAST(b, l) { bit_buffer |= (((mz_uint64)(b)) << bits_in); bits_in += (l); }

  flags = 1;
  for (pLZ_codes = d->m_lz_code_buf; pLZ_codes < pLZ_code_buf_end; flags >>= 1)
  {
    if (flags == 1)
      flags = *pLZ_codes++ | 0x100;

    if (flags & 1)
    {
      mz_uint s0, s1, n0, n1, sym, num_extra_bits;
      mz_uint match_len = pLZ_codes[0], match_dist = *(const mz_uint16 *)(pLZ_codes + 1); pLZ_codes += 3;

      MZ_ASSERT(d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
      TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][s_tdefl_len_sym[match_len]], d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
      TDEFL_PUT_BITS_FAST(match_len & mz_bitmasks[s_tdefl_len_extra[match_len]], s_tdefl_len_extra[match_len]);

      // This sequence coaxes MSVC into using cmov's vs. jmp's.
      s0 = s_tdefl_small_dist_sym[match_dist & 511];
      n0 = s_tdefl_small_dist_extra[match_dist & 511];
      s1 = s_tdefl_large_dist_sym[match_dist >> 8];
      n1 = s_tdefl_large_dist_extra[match_dist >> 8];
      sym = (match_dist < 512) ? s0 : s1;
      num_extra_bits = (match_dist < 512) ? n0 : n1;

      MZ_ASSERT(d->m_huff_code_sizes[1][sym]);
      TDEFL_PUT_BITS_FAST(d->m_huff_codes[1][sym], d->m_huff_code_sizes[1][sym]);
      TDEFL_PUT_BITS_FAST(match_dist & mz_bitmasks[num_extra_bits], num_extra_bits);
    }
    else
    {
      mz_uint lit = *pLZ_codes++;
      MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
      TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);

      if (((flags & 2) == 0) && (pLZ_codes < pLZ_code_buf_end))
      {
        flags >>= 1;
        lit = *pLZ_codes++;
        MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
        TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);

        if (((flags & 2) == 0) && (pLZ_codes < pLZ_code_buf_end))
        {
          flags >>= 1;
          lit = *pLZ_codes++;
          MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
          TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);
        }
      }
    }

    if (pOutput_buf >= d->m_pOutput_buf_end)
      return MZ_FALSE;

    *(mz_uint64*)pOutput_buf = bit_buffer;
    pOutput_buf += (bits_in >> 3);
    bit_buffer >>= (bits_in & ~7);
    bits_in &= 7;
  }

#undef TDEFL_PUT_BITS_FAST

  d->m_pOutput_buf = pOutput_buf;
  d->m_bits_in = 0;
  d->m_bit_buffer = 0;

  while (bits_in)
  {
    mz_uint32 n = MZ_MIN(bits_in, 16);
    TDEFL_PUT_BITS((mz_uint)bit_buffer & mz_bitmasks[n], n);
    bit_buffer >>= n;
    bits_in -= n;
  }

  TDEFL_PUT_BITS(d->m_huff_codes[0][256], d->m_huff_code_sizes[0][256]);

  return (d->m_pOutput_buf < d->m_pOutput_buf_end);
}
#else
static mz_bool tdefl_compress_lz_codes(tdefl_compressor *d)
{
  mz_uint flags;
  mz_uint8 *pLZ_codes;

  flags = 1;
  for (pLZ_codes = d->m_lz_code_buf; pLZ_codes < d->m_pLZ_code_buf; flags >>= 1)
  {
    if (flags == 1)
      flags = *pLZ_codes++ | 0x100;
    if (flags & 1)
    {
      mz_uint sym, num_extra_bits;
      mz_uint match_len = pLZ_codes[0], match_dist = (pLZ_codes[1] | (pLZ_codes[2] << 8)); pLZ_codes += 3;

      MZ_ASSERT(d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
      TDEFL_PUT_BITS(d->m_huff_codes[0][s_tdefl_len_sym[match_len]], d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
      TDEFL_PUT_BITS(match_len & mz_bitmasks[s_tdefl_len_extra[match_len]], s_tdefl_len_extra[match_len]);

      if (match_dist < 512)
      {
        sym = s_tdefl_small_dist_sym[match_dist]; num_extra_bits = s_tdefl_small_dist_extra[match_dist];
      }
      else
      {
        sym = s_tdefl_large_dist_sym[match_dist >> 8]; num_extra_bits = s_tdefl_large_dist_extra[match_dist >> 8];
      }
      MZ_ASSERT(d->m_huff_code_sizes[1][sym]);
      TDEFL_PUT_BITS(d->m_huff_codes[1][sym], d->m_huff_code_sizes[1][sym]);
      TDEFL_PUT_BITS(match_dist & mz_bitmasks[num_extra_bits], num_extra_bits);
    }
    else
    {
      mz_uint lit = *pLZ_codes++;
      MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
      TDEFL_PUT_BITS(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);
    }
  }

  TDEFL_PUT_BITS(d->m_huff_codes[0][256], d->m_huff_code_sizes[0][256]);

  return (d->m_pOutput_buf < d->m_pOutput_buf_end);
}
#endif // MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN && MINIZ_HAS_64BIT_REGISTERS

static mz_bool tdefl_compress_block(tdefl_compressor *d, mz_bool static_block)
{
  if (static_block)
    tdefl_start_static_block(d);
  else
    tdefl_start_dynamic_block(d);
  return tdefl_compress_lz_codes(d);
}

static int tdefl_flush_block(tdefl_compressor *d, int flush)
{
  mz_uint saved_bit_buf, saved_bits_in;
  mz_uint8 *pSaved_output_buf;
  mz_bool comp_block_succeeded = MZ_FALSE;
  int n, use_raw_block = ((d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS) != 0) && (d->m_lookahead_pos - d->m_lz_code_buf_dict_pos) <= d->m_dict_size;
  mz_uint8 *pOutput_buf_start = ((d->m_pPut_buf_func == NULL) && ((*d->m_pOut_buf_size - d->m_out_buf_ofs) >= TDEFL_OUT_BUF_SIZE)) ? ((mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs) : d->m_output_buf;

  d->m_pOutput_buf = pOutput_buf_start;
  d->m_pOutput_buf_end = d->m_pOutput_buf + TDEFL_OUT_BUF_SIZE - 16;

  MZ_ASSERT(!d->m_output_flush_remaining);
  d->m_output_flush_ofs = 0;
  d->m_output_flush_remaining = 0;

  *d->m_pLZ_flags = (mz_uint8)(*d->m_pLZ_flags >> d->m_num_flags_left);
  d->m_pLZ_code_buf -= (d->m_num_flags_left == 8);

  if ((d->m_flags & TDEFL_WRITE_ZLIB_HEADER) && (!d->m_block_index))
  {
    TDEFL_PUT_BITS(0x78, 8); TDEFL_PUT_BITS(0x01, 8);
  }

  TDEFL_PUT_BITS(flush == TDEFL_FINISH, 1);

  pSaved_output_buf = d->m_pOutput_buf; saved_bit_buf = d->m_bit_buffer; saved_bits_in = d->m_bits_in;

  if (!use_raw_block)
    comp_block_succeeded = tdefl_compress_block(d, (d->m_flags & TDEFL_FORCE_ALL_STATIC_BLOCKS) || (d->m_total_lz_bytes < 48));

  // If the block gets expanded, forget the current contents of the output buffer and send a raw block instead.
  if ( ((use_raw_block) || ((d->m_total_lz_bytes) && ((d->m_pOutput_buf - pSaved_output_buf + 1U) >= d->m_total_lz_bytes))) &&
       ((d->m_lookahead_pos - d->m_lz_code_buf_dict_pos) <= d->m_dict_size) )
  {
    mz_uint i; d->m_pOutput_buf = pSaved_output_buf; d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
    TDEFL_PUT_BITS(0, 2);
    if (d->m_bits_in) { TDEFL_PUT_BITS(0, 8 - d->m_bits_in); }
    for (i = 2; i; --i, d->m_total_lz_bytes ^= 0xFFFF)
    {
      TDEFL_PUT_BITS(d->m_total_lz_bytes & 0xFFFF, 16);
    }
    for (i = 0; i < d->m_total_lz_bytes; ++i)
    {
      TDEFL_PUT_BITS(d->m_dict[(d->m_lz_code_buf_dict_pos + i) & TDEFL_LZ_DICT_SIZE_MASK], 8);
    }
  }
  // Check for the extremely unlikely (if not impossible) case of the compressed block not fitting into the output buffer when using dynamic codes.
  else if (!comp_block_succeeded)
  {
    d->m_pOutput_buf = pSaved_output_buf; d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
    tdefl_compress_block(d, MZ_TRUE);
  }

  if (flush)
  {
    if (flush == TDEFL_FINISH)
    {
      if (d->m_bits_in) { TDEFL_PUT_BITS(0, 8 - d->m_bits_in); }
      if (d->m_flags & TDEFL_WRITE_ZLIB_HEADER) { mz_uint i, a = d->m_adler32; for (i = 0; i < 4; i++) { TDEFL_PUT_BITS((a >> 24) & 0xFF, 8); a <<= 8; } }
    }
    else
    {
      mz_uint i, z = 0; TDEFL_PUT_BITS(0, 3); if (d->m_bits_in) { TDEFL_PUT_BITS(0, 8 - d->m_bits_in); } for (i = 2; i; --i, z ^= 0xFFFF) { TDEFL_PUT_BITS(z & 0xFFFF, 16); }
    }
  }

  MZ_ASSERT(d->m_pOutput_buf < d->m_pOutput_buf_end);

  memset(&d->m_huff_count[0][0], 0, sizeof(d->m_huff_count[0][0]) * TDEFL_MAX_HUFF_SYMBOLS_0);
  memset(&d->m_huff_count[1][0], 0, sizeof(d->m_huff_count[1][0]) * TDEFL_MAX_HUFF_SYMBOLS_1);

  d->m_pLZ_code_buf = d->m_lz_code_buf + 1; d->m_pLZ_flags = d->m_lz_code_buf; d->m_num_flags_left = 8; d->m_lz_code_buf_dict_pos += d->m_total_lz_bytes; d->m_total_lz_bytes = 0; d->m_block_index++;

  if ((n = (int)(d->m_pOutput_buf - pOutput_buf_start)) != 0)
  {
    if (d->m_pPut_buf_func)
    {
      *d->m_pIn_buf_size = d->m_pSrc - (const mz_uint8 *)d->m_pIn_buf;
      if (!(*d->m_pPut_buf_func)(d->m_output_buf, n, d->m_pPut_buf_user))
        return (d->m_prev_return_status = TDEFL_STATUS_PUT_BUF_FAILED);
    }
    else if (pOutput_buf_start == d->m_output_buf)
    {
      int bytes_to_copy = (int)MZ_MIN((size_t)n, (size_t)(*d->m_pOut_buf_size - d->m_out_buf_ofs));
      memcpy((mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs, d->m_output_buf, bytes_to_copy);
      d->m_out_buf_ofs += bytes_to_copy;
      if ((n -= bytes_to_copy) != 0)
      {
        d->m_output_flush_ofs = bytes_to_copy;
        d->m_output_flush_remaining = n;
      }
    }
    else
    {
      d->m_out_buf_ofs += n;
    }
  }

  return d->m_output_flush_remaining;
}

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES
#define TDEFL_READ_UNALIGNED_WORD(p) *(const mz_uint16*)(p)
static MZ_FORCEINLINE void tdefl_find_match(tdefl_compressor *d, mz_uint lookahead_pos, mz_uint max_dist, mz_uint max_match_len, mz_uint *pMatch_dist, mz_uint *pMatch_len)
{
  mz_uint dist, pos = lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK, match_len = *pMatch_len, probe_pos = pos, next_probe_pos, probe_len;
  mz_uint num_probes_left = d->m_max_probes[match_len >= 32];
  const mz_uint16 *s = (const mz_uint16*)(d->m_dict + pos), *p, *q;
  mz_uint16 c01 = TDEFL_READ_UNALIGNED_WORD(&d->m_dict[pos + match_len - 1]), s01 = TDEFL_READ_UNALIGNED_WORD(s);
  MZ_ASSERT(max_match_len <= TDEFL_MAX_MATCH_LEN); if (max_match_len <= match_len) return;
  for ( ; ; )
  {
    for ( ; ; )
    {
      if (--num_probes_left == 0) return;
      #define TDEFL_PROBE \
        next_probe_pos = d->m_next[probe_pos]; \
        if ((!next_probe_pos) || ((dist = (mz_uint16)(lookahead_pos - next_probe_pos)) > max_dist)) return; \
        probe_pos = next_probe_pos & TDEFL_LZ_DICT_SIZE_MASK; \
        if (TDEFL_READ_UNALIGNED_WORD(&d->m_dict[probe_pos + match_len - 1]) == c01) break;
      TDEFL_PROBE; TDEFL_PROBE; TDEFL_PROBE;
    }
    if (!dist) break; q = (const mz_uint16*)(d->m_dict + probe_pos); if (TDEFL_READ_UNALIGNED_WORD(q) != s01) continue; p = s; probe_len = 32;
    do { } while ( (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) &&
                   (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (--probe_len > 0) );
    if (!probe_len)
    {
      *pMatch_dist = dist; *pMatch_len = MZ_MIN(max_match_len, TDEFL_MAX_MATCH_LEN); break;
    }
    else if ((probe_len = ((mz_uint)(p - s) * 2) + (mz_uint)(*(const mz_uint8*)p == *(const mz_uint8*)q)) > match_len)
    {
      *pMatch_dist = dist; if ((*pMatch_len = match_len = MZ_MIN(max_match_len, probe_len)) == max_match_len) break;
      c01 = TDEFL_READ_UNALIGNED_WORD(&d->m_dict[pos + match_len - 1]);
    }
  }
}
#else
static MZ_FORCEINLINE void tdefl_find_match(tdefl_compressor *d, mz_uint lookahead_pos, mz_uint max_dist, mz_uint max_match_len, mz_uint *pMatch_dist, mz_uint *pMatch_len)
{
  mz_uint dist, pos = lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK, match_len = *pMatch_len, probe_pos = pos, next_probe_pos, probe_len;
  mz_uint num_probes_left = d->m_max_probes[match_len >= 32];
  const mz_uint8 *s = d->m_dict + pos, *p, *q;
  mz_uint8 c0 = d->m_dict[pos + match_len], c1 = d->m_dict[pos + match_len - 1];
  MZ_ASSERT(max_match_len <= TDEFL_MAX_MATCH_LEN); if (max_match_len <= match_len) return;
  for ( ; ; )
  {
    for ( ; ; )
    {
      if (--num_probes_left == 0) return;
      #define TDEFL_PROBE \
        next_probe_pos = d->m_next[probe_pos]; \
        if ((!next_probe_pos) || ((dist = (mz_uint16)(lookahead_pos - next_probe_pos)) > max_dist)) return; \
        probe_pos = next_probe_pos & TDEFL_LZ_DICT_SIZE_MASK; \
        if ((d->m_dict[probe_pos + match_len] == c0) && (d->m_dict[probe_pos + match_len - 1] == c1)) break;
      TDEFL_PROBE; TDEFL_PROBE; TDEFL_PROBE;
    }
    if (!dist) break; p = s; q = d->m_dict + probe_pos; for (probe_len = 0; probe_len < max_match_len; probe_len++) if (*p++ != *q++) break;
    if (probe_len > match_len)
    {
      *pMatch_dist = dist; if ((*pMatch_len = match_len = probe_len) == max_match_len) return;
      c0 = d->m_dict[pos + match_len]; c1 = d->m_dict[pos + match_len - 1];
    }
  }
}
#endif // #if MINIZ_USE_UNALIGNED_LOADS_AND_STORES

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
static mz_bool tdefl_compress_fast(tdefl_compressor *d)
{
  // Faster, minimally featured LZRW1-style match+parse loop with better register utilization. Intended for applications where raw throughput is valued more highly than ratio.
  mz_uint lookahead_pos = d->m_lookahead_pos, lookahead_size = d->m_lookahead_size, dict_size = d->m_dict_size, total_lz_bytes = d->m_total_lz_bytes, num_flags_left = d->m_num_flags_left;
  mz_uint8 *pLZ_code_buf = d->m_pLZ_code_buf, *pLZ_flags = d->m_pLZ_flags;
  mz_uint cur_pos = lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK;

  while ((d->m_src_buf_left) || ((d->m_flush) && (lookahead_size)))
  {
    const mz_uint TDEFL_COMP_FAST_LOOKAHEAD_SIZE = 4096;
    mz_uint dst_pos = (lookahead_pos + lookahead_size) & TDEFL_LZ_DICT_SIZE_MASK;
    mz_uint num_bytes_to_process = (mz_uint)MZ_MIN(d->m_src_buf_left, TDEFL_COMP_FAST_LOOKAHEAD_SIZE - lookahead_size);
    d->m_src_buf_left -= num_bytes_to_process;
    lookahead_size += num_bytes_to_process;

    while (num_bytes_to_process)
    {
      mz_uint32 n = MZ_MIN(TDEFL_LZ_DICT_SIZE - dst_pos, num_bytes_to_process);
      memcpy(d->m_dict + dst_pos, d->m_pSrc, n);
      if (dst_pos < (TDEFL_MAX_MATCH_LEN - 1))
        memcpy(d->m_dict + TDEFL_LZ_DICT_SIZE + dst_pos, d->m_pSrc, MZ_MIN(n, (TDEFL_MAX_MATCH_LEN - 1) - dst_pos));
      d->m_pSrc += n;
      dst_pos = (dst_pos + n) & TDEFL_LZ_DICT_SIZE_MASK;
      num_bytes_to_process -= n;
    }

    dict_size = MZ_MIN(TDEFL_LZ_DICT_SIZE - lookahead_size, dict_size);
    if ((!d->m_flush) && (lookahead_size < TDEFL_COMP_FAST_LOOKAHEAD_SIZE)) break;

    while (lookahead_size >= 4)
    {
      mz_uint cur_match_dist, cur_match_len = 1;
      mz_uint8 *pCur_dict = d->m_dict + cur_pos;
      mz_uint first_trigram = (*(const mz_uint32 *)pCur_dict) & 0xFFFFFF;
      mz_uint hash = (first_trigram ^ (first_trigram >> (24 - (TDEFL_LZ_HASH_BITS - 8)))) & TDEFL_LEVEL1_HASH_SIZE_MASK;
      mz_uint probe_pos = d->m_hash[hash];
      d->m_hash[hash] = (mz_uint16)lookahead_pos;

      if (((cur_match_dist = (mz_uint16)(lookahead_pos - probe_pos)) <= dict_size) && ((*(const mz_uint32 *)(d->m_dict + (probe_pos &= TDEFL_LZ_DICT_SIZE_MASK)) & 0xFFFFFF) == first_trigram))
      {
        const mz_uint16 *p = (const mz_uint16 *)pCur_dict;
        const mz_uint16 *q = (const mz_uint16 *)(d->m_dict + probe_pos);
        mz_uint32 probe_len = 32;
        do { } while ( (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) &&
          (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (TDEFL_READ_UNALIGNED_WORD(++p) == TDEFL_READ_UNALIGNED_WORD(++q)) && (--probe_len > 0) );
        cur_match_len = ((mz_uint)(p - (const mz_uint16 *)pCur_dict) * 2) + (mz_uint)(*(const mz_uint8 *)p == *(const mz_uint8 *)q);
        if (!probe_len)
          cur_match_len = cur_match_dist ? TDEFL_MAX_MATCH_LEN : 0;

        if ((cur_match_len < TDEFL_MIN_MATCH_LEN) || ((cur_match_len == TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 8U*1024U)))
        {
          cur_match_len = 1;
          *pLZ_code_buf++ = (mz_uint8)first_trigram;
          *pLZ_flags = (mz_uint8)(*pLZ_flags >> 1);
          d->m_huff_count[0][(mz_uint8)first_trigram]++;
        }
        else
        {
          mz_uint32 s0, s1;
          cur_match_len = MZ_MIN(cur_match_len, lookahead_size);

          MZ_ASSERT((cur_match_len >= TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 1) && (cur_match_dist <= TDEFL_LZ_DICT_SIZE));

          cur_match_dist--;

          pLZ_code_buf[0] = (mz_uint8)(cur_match_len - TDEFL_MIN_MATCH_LEN);
          *(mz_uint16 *)(&pLZ_code_buf[1]) = (mz_uint16)cur_match_dist;
          pLZ_code_buf += 3;
          *pLZ_flags = (mz_uint8)((*pLZ_flags >> 1) | 0x80);

          s0 = s_tdefl_small_dist_sym[cur_match_dist & 511];
          s1 = s_tdefl_large_dist_sym[cur_match_dist >> 8];
          d->m_huff_count[1][(cur_match_dist < 512) ? s0 : s1]++;

          d->m_huff_count[0][s_tdefl_len_sym[cur_match_len - TDEFL_MIN_MATCH_LEN]]++;
        }
      }
      else
      {
        *pLZ_code_buf++ = (mz_uint8)first_trigram;
        *pLZ_flags = (mz_uint8)(*pLZ_flags >> 1);
        d->m_huff_count[0][(mz_uint8)first_trigram]++;
      }

      if (--num_flags_left == 0) { num_flags_left = 8; pLZ_flags = pLZ_code_buf++; }

      total_lz_bytes += cur_match_len;
      lookahead_pos += cur_match_len;
      dict_size = MZ_MIN(dict_size + cur_match_len, TDEFL_LZ_DICT_SIZE);
      cur_pos = (cur_pos + cur_match_len) & TDEFL_LZ_DICT_SIZE_MASK;
      MZ_ASSERT(lookahead_size >= cur_match_len);
      lookahead_size -= cur_match_len;

      if (pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8])
      {
        int n;
        d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
        d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
        if ((n = tdefl_flush_block(d, 0)) != 0)
          return (n < 0) ? MZ_FALSE : MZ_TRUE;
        total_lz_bytes = d->m_total_lz_bytes; pLZ_code_buf = d->m_pLZ_code_buf; pLZ_flags = d->m_pLZ_flags; num_flags_left = d->m_num_flags_left;
      }
    }

    while (lookahead_size)
    {
      mz_uint8 lit = d->m_dict[cur_pos];

      total_lz_bytes++;
      *pLZ_code_buf++ = lit;
      *pLZ_flags = (mz_uint8)(*pLZ_flags >> 1);
      if (--num_flags_left == 0) { num_flags_left = 8; pLZ_flags = pLZ_code_buf++; }

      d->m_huff_count[0][lit]++;

      lookahead_pos++;
      dict_size = MZ_MIN(dict_size + 1, TDEFL_LZ_DICT_SIZE);
      cur_pos = (cur_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK;
      lookahead_size--;

      if (pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8])
      {
        int n;
        d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
        d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
        if ((n = tdefl_flush_block(d, 0)) != 0)
          return (n < 0) ? MZ_FALSE : MZ_TRUE;
        total_lz_bytes = d->m_total_lz_bytes; pLZ_code_buf = d->m_pLZ_code_buf; pLZ_flags = d->m_pLZ_flags; num_flags_left = d->m_num_flags_left;
      }
    }
  }

  d->m_lookahead_pos = lookahead_pos; d->m_lookahead_size = lookahead_size; d->m_dict_size = dict_size;
  d->m_total_lz_bytes = total_lz_bytes; d->m_pLZ_code_buf = pLZ_code_buf; d->m_pLZ_flags = pLZ_flags; d->m_num_flags_left = num_flags_left;
  return MZ_TRUE;
}
#endif // MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN

static MZ_FORCEINLINE void tdefl_record_literal(tdefl_compressor *d, mz_uint8 lit)
{
  d->m_total_lz_bytes++;
  *d->m_pLZ_code_buf++ = lit;
  *d->m_pLZ_flags = (mz_uint8)(*d->m_pLZ_flags >> 1); if (--d->m_num_flags_left == 0) { d->m_num_flags_left = 8; d->m_pLZ_flags = d->m_pLZ_code_buf++; }
  d->m_huff_count[0][lit]++;
}

static MZ_FORCEINLINE void tdefl_record_match(tdefl_compressor *d, mz_uint match_len, mz_uint match_dist)
{
  mz_uint32 s0, s1;

  MZ_ASSERT((match_len >= TDEFL_MIN_MATCH_LEN) && (match_dist >= 1) && (match_dist <= TDEFL_LZ_DICT_SIZE));

  d->m_total_lz_bytes += match_len;

  d->m_pLZ_code_buf[0] = (mz_uint8)(match_len - TDEFL_MIN_MATCH_LEN);

  match_dist -= 1;
  d->m_pLZ_code_buf[1] = (mz_uint8)(match_dist & 0xFF);
  d->m_pLZ_code_buf[2] = (mz_uint8)(match_dist >> 8); d->m_pLZ_code_buf += 3;

  *d->m_pLZ_flags = (mz_uint8)((*d->m_pLZ_flags >> 1) | 0x80); if (--d->m_num_flags_left == 0) { d->m_num_flags_left = 8; d->m_pLZ_flags = d->m_pLZ_code_buf++; }

  s0 = s_tdefl_small_dist_sym[match_dist & 511]; s1 = s_tdefl_large_dist_sym[(match_dist >> 8) & 127];
  d->m_huff_count[1][(match_dist < 512) ? s0 : s1]++;

  if (match_len >= TDEFL_MIN_MATCH_LEN) d->m_huff_count[0][s_tdefl_len_sym[match_len - TDEFL_MIN_MATCH_LEN]]++;
}

static mz_bool tdefl_compress_normal(tdefl_compressor *d)
{
  const mz_uint8 *pSrc = d->m_pSrc; size_t src_buf_left = d->m_src_buf_left;
  tdefl_flush flush = d->m_flush;

  while ((src_buf_left) || ((flush) && (d->m_lookahead_size)))
  {
    mz_uint len_to_move, cur_match_dist, cur_match_len, cur_pos;
    // Update dictionary and hash chains. Keeps the lookahead size equal to TDEFL_MAX_MATCH_LEN.
    if ((d->m_lookahead_size + d->m_dict_size) >= (TDEFL_MIN_MATCH_LEN - 1))
    {
      mz_uint dst_pos = (d->m_lookahead_pos + d->m_lookahead_size) & TDEFL_LZ_DICT_SIZE_MASK, ins_pos = d->m_lookahead_pos + d->m_lookahead_size - 2;
      mz_uint hash = (d->m_dict[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] << TDEFL_LZ_HASH_SHIFT) ^ d->m_dict[(ins_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK];
      mz_uint num_bytes_to_process = (mz_uint)MZ_MIN(src_buf_left, TDEFL_MAX_MATCH_LEN - d->m_lookahead_size);
      const mz_uint8 *pSrc_end = pSrc + num_bytes_to_process;
      src_buf_left -= num_bytes_to_process;
      d->m_lookahead_size += num_bytes_to_process;
      while (pSrc != pSrc_end)
      {
        mz_uint8 c = *pSrc++; d->m_dict[dst_pos] = c; if (dst_pos < (TDEFL_MAX_MATCH_LEN - 1)) d->m_dict[TDEFL_LZ_DICT_SIZE + dst_pos] = c;
        hash = ((hash << TDEFL_LZ_HASH_SHIFT) ^ c) & (TDEFL_LZ_HASH_SIZE - 1);
        d->m_next[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash]; d->m_hash[hash] = (mz_uint16)(ins_pos);
        dst_pos = (dst_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK; ins_pos++;
      }
    }
    else
    {
      while ((src_buf_left) && (d->m_lookahead_size < TDEFL_MAX_MATCH_LEN))
      {
        mz_uint8 c = *pSrc++;
        mz_uint dst_pos = (d->m_lookahead_pos + d->m_lookahead_size) & TDEFL_LZ_DICT_SIZE_MASK;
        src_buf_left--;
        d->m_dict[dst_pos] = c;
        if (dst_pos < (TDEFL_MAX_MATCH_LEN - 1))
          d->m_dict[TDEFL_LZ_DICT_SIZE + dst_pos] = c;
        if ((++d->m_lookahead_size + d->m_dict_size) >= TDEFL_MIN_MATCH_LEN)
        {
          mz_uint ins_pos = d->m_lookahead_pos + (d->m_lookahead_size - 1) - 2;
          mz_uint hash = ((d->m_dict[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] << (TDEFL_LZ_HASH_SHIFT * 2)) ^ (d->m_dict[(ins_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK] << TDEFL_LZ_HASH_SHIFT) ^ c) & (TDEFL_LZ_HASH_SIZE - 1);
          d->m_next[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash]; d->m_hash[hash] = (mz_uint16)(ins_pos);
        }
      }
    }
    d->m_dict_size = MZ_MIN(TDEFL_LZ_DICT_SIZE - d->m_lookahead_size, d->m_dict_size);
    if ((!flush) && (d->m_lookahead_size < TDEFL_MAX_MATCH_LEN))
      break;

    // Simple lazy/greedy parsing state machine.
    len_to_move = 1; cur_match_dist = 0; cur_match_len = d->m_saved_match_len ? d->m_saved_match_len : (TDEFL_MIN_MATCH_LEN - 1); cur_pos = d->m_lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK;
    if (d->m_flags & (TDEFL_RLE_MATCHES | TDEFL_FORCE_ALL_RAW_BLOCKS))
    {
      if ((d->m_dict_size) && (!(d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS)))
      {
        mz_uint8 c = d->m_dict[(cur_pos - 1) & TDEFL_LZ_DICT_SIZE_MASK];
        cur_match_len = 0; while (cur_match_len < d->m_lookahead_size) { if (d->m_dict[cur_pos + cur_match_len] != c) break; cur_match_len++; }
        if (cur_match_len < TDEFL_MIN_MATCH_LEN) cur_match_len = 0; else cur_match_dist = 1;
      }
    }
    else
    {
      tdefl_find_match(d, d->m_lookahead_pos, d->m_dict_size, d->m_lookahead_size, &cur_match_dist, &cur_match_len);
    }
    if (((cur_match_len == TDEFL_MIN_MATCH_LEN) && (cur_match_dist >= 8U*1024U)) || (cur_pos == cur_match_dist) || ((d->m_flags & TDEFL_FILTER_MATCHES) && (cur_match_len <= 5)))
    {
      cur_match_dist = cur_match_len = 0;
    }
    if (d->m_saved_match_len)
    {
      if (cur_match_len > d->m_saved_match_len)
      {
        tdefl_record_literal(d, (mz_uint8)d->m_saved_lit);
        if (cur_match_len >= 128)
        {
          tdefl_record_match(d, cur_match_len, cur_match_dist);
          d->m_saved_match_len = 0; len_to_move = cur_match_len;
        }
        else
        {
          d->m_saved_lit = d->m_dict[cur_pos]; d->m_saved_match_dist = cur_match_dist; d->m_saved_match_len = cur_match_len;
        }
      }
      else
      {
        tdefl_record_match(d, d->m_saved_match_len, d->m_saved_match_dist);
        len_to_move = d->m_saved_match_len - 1; d->m_saved_match_len = 0;
      }
    }
    else if (!cur_match_dist)
      tdefl_record_literal(d, d->m_dict[MZ_MIN(cur_pos, sizeof(d->m_dict) - 1)]);
    else if ((d->m_greedy_parsing) || (d->m_flags & TDEFL_RLE_MATCHES) || (cur_match_len >= 128))
    {
      tdefl_record_match(d, cur_match_len, cur_match_dist);
      len_to_move = cur_match_len;
    }
    else
    {
      d->m_saved_lit = d->m_dict[MZ_MIN(cur_pos, sizeof(d->m_dict) - 1)]; d->m_saved_match_dist = cur_match_dist; d->m_saved_match_len = cur_match_len;
    }
    // Move the lookahead forward by len_to_move bytes.
    d->m_lookahead_pos += len_to_move;
    MZ_ASSERT(d->m_lookahead_size >= len_to_move);
    d->m_lookahead_size -= len_to_move;
    d->m_dict_size = MZ_MIN(d->m_dict_size + len_to_move, TDEFL_LZ_DICT_SIZE);
    // Check if it's time to flush the current LZ codes to the internal output buffer.
    if ( (d->m_pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8]) ||
         ( (d->m_total_lz_bytes > 31*1024) && (((((mz_uint)(d->m_pLZ_code_buf - d->m_lz_code_buf) * 115) >> 7) >= d->m_total_lz_bytes) || (d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS))) )
    {
      int n;
      d->m_pSrc = pSrc; d->m_src_buf_left = src_buf_left;
      if ((n = tdefl_flush_block(d, 0)) != 0)
        return (n < 0) ? MZ_FALSE : MZ_TRUE;
    }
  }

  d->m_pSrc = pSrc; d->m_src_buf_left = src_buf_left;
  return MZ_TRUE;
}

static tdefl_status tdefl_flush_output_buffer(tdefl_compressor *d)
{
  if (d->m_pIn_buf_size)
  {
    *d->m_pIn_buf_size = d->m_pSrc - (const mz_uint8 *)d->m_pIn_buf;
  }

  if (d->m_pOut_buf_size)
  {
    size_t n = MZ_MIN(*d->m_pOut_buf_size - d->m_out_buf_ofs, d->m_output_flush_remaining);
    memcpy((mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs, d->m_output_buf + d->m_output_flush_ofs, n);
    d->m_output_flush_ofs += (mz_uint)n;
    d->m_output_flush_remaining -= (mz_uint)n;
    d->m_out_buf_ofs += n;

    *d->m_pOut_buf_size = d->m_out_buf_ofs;
  }

  return (d->m_finished && !d->m_output_flush_remaining) ? TDEFL_STATUS_DONE : TDEFL_STATUS_OKAY;
}

tdefl_status tdefl_compress(tdefl_compressor *d, const void *pIn_buf, size_t *pIn_buf_size, void *pOut_buf, size_t *pOut_buf_size, tdefl_flush flush)
{
  if (!d)
  {
    if (pIn_buf_size) *pIn_buf_size = 0;
    if (pOut_buf_size) *pOut_buf_size = 0;
    return TDEFL_STATUS_BAD_PARAM;
  }

  d->m_pIn_buf = pIn_buf; d->m_pIn_buf_size = pIn_buf_size;
  d->m_pOut_buf = pOut_buf; d->m_pOut_buf_size = pOut_buf_size;
  d->m_pSrc = (const mz_uint8 *)(pIn_buf); d->m_src_buf_left = pIn_buf_size ? *pIn_buf_size : 0;
  d->m_out_buf_ofs = 0;
  d->m_flush = flush;

  if ( ((d->m_pPut_buf_func != NULL) == ((pOut_buf != NULL) || (pOut_buf_size != NULL))) || (d->m_prev_return_status != TDEFL_STATUS_OKAY) ||
        (d->m_wants_to_finish && (flush != TDEFL_FINISH)) || (pIn_buf_size && *pIn_buf_size && !pIn_buf) || (pOut_buf_size && *pOut_buf_size && !pOut_buf) )
  {
    if (pIn_buf_size) *pIn_buf_size = 0;
    if (pOut_buf_size) *pOut_buf_size = 0;
    return (d->m_prev_return_status = TDEFL_STATUS_BAD_PARAM);
  }
  d->m_wants_to_finish |= (flush == TDEFL_FINISH);

  if ((d->m_output_flush_remaining) || (d->m_finished))
    return (d->m_prev_return_status = tdefl_flush_output_buffer(d));

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
  if (((d->m_flags & TDEFL_MAX_PROBES_MASK) == 1) &&
      ((d->m_flags & TDEFL_GREEDY_PARSING_FLAG) != 0) &&
      ((d->m_flags & (TDEFL_FILTER_MATCHES | TDEFL_FORCE_ALL_RAW_BLOCKS | TDEFL_RLE_MATCHES)) == 0))
  {
    if (!tdefl_compress_fast(d))
      return d->m_prev_return_status;
  }
  else
#endif // #if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
  {
    if (!tdefl_compress_normal(d))
      return d->m_prev_return_status;
  }

  if ((d->m_flags & (TDEFL_WRITE_ZLIB_HEADER | TDEFL_COMPUTE_ADLER32)) && (pIn_buf))
    d->m_adler32 = (mz_uint32)mz_adler32(d->m_adler32, (const mz_uint8 *)pIn_buf, d->m_pSrc - (const mz_uint8 *)pIn_buf);

  if ((flush) && (!d->m_lookahead_size) && (!d->m_src_buf_left) && (!d->m_output_flush_remaining))
  {
    if (tdefl_flush_block(d, flush) < 0)
      return d->m_prev_return_status;
    d->m_finished = (flush == TDEFL_FINISH);
    if (flush == TDEFL_FULL_FLUSH) { MZ_CLEAR_OBJ(d->m_hash); MZ_CLEAR_OBJ(d->m_next); d->m_dict_size = 0; }
  }

  return (d->m_prev_return_status = tdefl_flush_output_buffer(d));
}

tdefl_status tdefl_compress_buffer(tdefl_compressor *d, const void *pIn_buf, size_t in_buf_size, tdefl_flush flush)
{
  MZ_ASSERT(d->m_pPut_buf_func); return tdefl_compress(d, pIn_buf, &in_buf_size, NULL, NULL, flush);
}

tdefl_status tdefl_init(tdefl_compressor *d, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  d->m_pPut_buf_func = pPut_buf_func; d->m_pPut_buf_user = pPut_buf_user;
  d->m_flags = (mz_uint)(flags); d->m_max_probes[0] = 1 + ((flags & 0xFFF) + 2) / 3; d->m_greedy_parsing = (flags & TDEFL_GREEDY_PARSING_FLAG) != 0;
  d->m_max_probes[1] = 1 + (((flags & 0xFFF) >> 2) + 2) / 3;
  if (!(flags & TDEFL_NONDETERMINISTIC_PARSING_FLAG)) MZ_CLEAR_OBJ(d->m_hash);
  d->m_lookahead_pos = d->m_lookahead_size = d->m_dict_size = d->m_total_lz_bytes = d->m_lz_code_buf_dict_pos = d->m_bits_in = 0;
  d->m_output_flush_ofs = d->m_output_flush_remaining = d->m_finished = d->m_block_index = d->m_bit_buffer = d->m_wants_to_finish = 0;
  d->m_pLZ_code_buf = d->m_lz_code_buf + 1; d->m_pLZ_flags = d->m_lz_code_buf; d->m_num_flags_left = 8;
  d->m_pOutput_buf = d->m_output_buf; d->m_pOutput_buf_end = d->m_output_buf; d->m_prev_return_status = TDEFL_STATUS_OKAY;
  d->m_saved_match_dist = d->m_saved_match_len = d->m_saved_lit = 0; d->m_adler32 = 1;
  d->m_pIn_buf = NULL; d->m_pOut_buf = NULL;
  d->m_pIn_buf_size = NULL; d->m_pOut_buf_size = NULL;
  d->m_flush = TDEFL_NO_FLUSH; d->m_pSrc = NULL; d->m_src_buf_left = 0; d->m_out_buf_ofs = 0;
  memset(&d->m_huff_count[0][0], 0, sizeof(d->m_huff_count[0][0]) * TDEFL_MAX_HUFF_SYMBOLS_0);
  memset(&d->m_huff_count[1][0], 0, sizeof(d->m_huff_count[1][0]) * TDEFL_MAX_HUFF_SYMBOLS_1);
  return TDEFL_STATUS_OKAY;
}

tdefl_status tdefl_get_prev_return_status(tdefl_compressor *d)
{
  return d->m_prev_return_status;
}

mz_uint32 tdefl_get_adler32(tdefl_compressor *d)
{
  return d->m_adler32;
}

mz_bool tdefl_compress_mem_to_output(const void *pBuf, size_t buf_len, tdefl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags)
{
  tdefl_compressor *pComp; mz_bool succeeded; if (((buf_len) && (!pBuf)) || (!pPut_buf_func)) return MZ_FALSE;
  pComp = (tdefl_compressor*)MZ_MALLOC(sizeof(tdefl_compressor)); if (!pComp) return MZ_FALSE;
  succeeded = (tdefl_init(pComp, pPut_buf_func, pPut_buf_user, flags) == TDEFL_STATUS_OKAY);
  succeeded = succeeded && (tdefl_compress_buffer(pComp, pBuf, buf_len, TDEFL_FINISH) == TDEFL_STATUS_DONE);
  MZ_FREE(pComp); return succeeded;
}

typedef struct
{
  size_t m_size, m_capacity;
  mz_uint8 *m_pBuf;
  mz_bool m_expandable;
} tdefl_output_buffer;

static mz_bool tdefl_output_buffer_putter(const void *pBuf, int len, void *pUser)
{
  tdefl_output_buffer *p = (tdefl_output_buffer *)pUser;
  size_t new_size = p->m_size + len;
  if (new_size > p->m_capacity)
  {
    size_t new_capacity = p->m_capacity; mz_uint8 *pNew_buf; if (!p->m_expandable) return MZ_FALSE;
    do { new_capacity = MZ_MAX(128U, new_capacity << 1U); } while (new_size > new_capacity);
    pNew_buf = (mz_uint8*)MZ_REALLOC(p->m_pBuf, new_capacity); if (!pNew_buf) return MZ_FALSE;
    p->m_pBuf = pNew_buf; p->m_capacity = new_capacity;
  }
  memcpy((mz_uint8*)p->m_pBuf + p->m_size, pBuf, len); p->m_size = new_size;
  return MZ_TRUE;
}

void *tdefl_compress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags)
{
  tdefl_output_buffer out_buf; MZ_CLEAR_OBJ(out_buf);
  if (!pOut_len) return MZ_FALSE; else *pOut_len = 0;
  out_buf.m_expandable = MZ_TRUE;
  if (!tdefl_compress_mem_to_output(pSrc_buf, src_buf_len, tdefl_output_buffer_putter, &out_buf, flags)) return NULL;
  *pOut_len = out_buf.m_size; return out_buf.m_pBuf;
}

size_t tdefl_compress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags)
{
  tdefl_output_buffer out_buf; MZ_CLEAR_OBJ(out_buf);
  if (!pOut_buf) return 0;
  out_buf.m_pBuf = (mz_uint8*)pOut_buf; out_buf.m_capacity = out_buf_len;
  if (!tdefl_compress_mem_to_output(pSrc_buf, src_buf_len, tdefl_output_buffer_putter, &out_buf, flags)) return 0;
  return out_buf.m_size;
}

#ifndef MINIZ_NO_ZLIB_APIS
static const mz_uint s_tdefl_num_probes[11] = { 0, 1, 6, 32,  16, 32, 128, 256,  512, 768, 1500 };

// level may actually range from [0,10] (10 is a "hidden" max level, where we want a bit more compression and it's fine if throughput to fall off a cliff on some files).
mz_uint tdefl_create_comp_flags_from_zip_params(int level, int window_bits, int strategy)
{
  mz_uint comp_flags = s_tdefl_num_probes[(level >= 0) ? MZ_MIN(10, level) : MZ_DEFAULT_LEVEL] | ((level <= 3) ? TDEFL_GREEDY_PARSING_FLAG : 0);
  if (window_bits > 0) comp_flags |= TDEFL_WRITE_ZLIB_HEADER;

  if (!level) comp_flags |= TDEFL_FORCE_ALL_RAW_BLOCKS;
  else if (strategy == MZ_FILTERED) comp_flags |= TDEFL_FILTER_MATCHES;
  else if (strategy == MZ_HUFFMAN_ONLY) comp_flags &= ~TDEFL_MAX_PROBES_MASK;
  else if (strategy == MZ_FIXED) comp_flags |= TDEFL_FORCE_ALL_STATIC_BLOCKS;
  else if (strategy == MZ_RLE) comp_flags |= TDEFL_RLE_MATCHES;

  return comp_flags;
}
#endif //MINIZ_NO_ZLIB_APIS

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4204) // nonstandard extension used : non-constant aggregate initializer (also supported by GNU C and C99, so no big deal)
#endif

// Simple PNG writer function by Alex Evans, 2011. Released into the public domain: https://gist.github.com/908299, more context at
// http://altdevblogaday.org/2011/04/06/a-smaller-jpg-encoder/.
// This is actually a modification of Alex's original code so PNG files generated by this function pass pngcheck.
void *tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h, int num_chans, size_t *pLen_out, mz_uint level, mz_bool flip)
{
  // Using a local copy of this array here in case MINIZ_NO_ZLIB_APIS was defined.
  static const mz_uint s_tdefl_png_num_probes[11] = { 0, 1, 6, 32,  16, 32, 128, 256,  512, 768, 1500 };
  tdefl_compressor *pComp = (tdefl_compressor *)MZ_MALLOC(sizeof(tdefl_compressor)); tdefl_output_buffer out_buf; int i, bpl = w * num_chans, y, z; mz_uint32 c; *pLen_out = 0;
  if (!pComp) return NULL;
  MZ_CLEAR_OBJ(out_buf); out_buf.m_expandable = MZ_TRUE; out_buf.m_capacity = 57+MZ_MAX(64, (1+bpl)*h); if (NULL == (out_buf.m_pBuf = (mz_uint8*)MZ_MALLOC(out_buf.m_capacity))) { MZ_FREE(pComp); return NULL; }
  // write dummy header
  for (z = 41; z; --z) tdefl_output_buffer_putter(&z, 1, &out_buf);
  // compress image data
  tdefl_init(pComp, tdefl_output_buffer_putter, &out_buf, s_tdefl_png_num_probes[MZ_MIN(10, level)] | TDEFL_WRITE_ZLIB_HEADER);
  for (y = 0; y < h; ++y) { tdefl_compress_buffer(pComp, &z, 1, TDEFL_NO_FLUSH); tdefl_compress_buffer(pComp, (mz_uint8*)pImage + (flip ? (h - 1 - y) : y) * bpl, bpl, TDEFL_NO_FLUSH); }
  if (tdefl_compress_buffer(pComp, NULL, 0, TDEFL_FINISH) != TDEFL_STATUS_DONE) { MZ_FREE(pComp); MZ_FREE(out_buf.m_pBuf); return NULL; }
  // write real header
  *pLen_out = out_buf.m_size-41;
  {
    static const mz_uint8 chans[] = {0x00, 0x00, 0x04, 0x02, 0x06};
    mz_uint8 pnghdr[41]={0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
      0,0,(mz_uint8)(w>>8),(mz_uint8)w,0,0,(mz_uint8)(h>>8),(mz_uint8)h,8,chans[num_chans],0,0,0,0,0,0,0,
      (mz_uint8)(*pLen_out>>24),(mz_uint8)(*pLen_out>>16),(mz_uint8)(*pLen_out>>8),(mz_uint8)*pLen_out,0x49,0x44,0x41,0x54};
    c=(mz_uint32)mz_crc32(MZ_CRC32_INIT,pnghdr+12,17); for (i=0; i<4; ++i, c<<=8) ((mz_uint8*)(pnghdr+29))[i]=(mz_uint8)(c>>24);
    memcpy(out_buf.m_pBuf, pnghdr, 41);
  }
  // write footer (IDAT CRC-32, followed by IEND chunk)
  if (!tdefl_output_buffer_putter("\0\0\0\0\0\0\0\0\x49\x45\x4e\x44\xae\x42\x60\x82", 16, &out_buf)) { *pLen_out = 0; MZ_FREE(pComp); MZ_FREE(out_buf.m_pBuf); return NULL; }
  c = (mz_uint32)mz_crc32(MZ_CRC32_INIT,out_buf.m_pBuf+41-4, *pLen_out+4); for (i=0; i<4; ++i, c<<=8) (out_buf.m_pBuf+out_buf.m_size-16)[i] = (mz_uint8)(c >> 24);
  // compute final size of file, grab compressed data buffer and return
  *pLen_out += 57; MZ_FREE(pComp); return out_buf.m_pBuf;
}
void *tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h, int num_chans, size_t *pLen_out)
{
  // Level 6 corresponds to TDEFL_DEFAULT_MAX_PROBES or MZ_DEFAULT_LEVEL (but we can't depend on MZ_DEFAULT_LEVEL being available in case the zlib API's where #defined out)
  return tdefl_write_image_to_png_file_in_memory_ex(pImage, w, h, num_chans, pLen_out, 6, MZ_FALSE);
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif

// ------------------- .ZIP archive reading

#ifndef MINIZ_NO_ARCHIVE_APIS

#ifdef MINIZ_NO_STDIO
  #define MZ_FILE void *
#else
  #include <stdio.h>
  #include <sys/stat.h>

  #if defined(_MSC_VER) || defined(__MINGW64__)
    static FILE *mz_fopen(const char *pFilename, const char *pMode)
    {
      FILE* pFile = NULL;
      fopen_s(&pFile, pFilename, pMode);
      return pFile;
    }
    static FILE *mz_freopen(const char *pPath, const char *pMode, FILE *pStream)
    {
      FILE* pFile = NULL;
      if (freopen_s(&pFile, pPath, pMode, pStream))
        return NULL;
      return pFile;
    }
    #ifndef MINIZ_NO_TIME
      #include <sys/utime.h>
    #endif
    #define MZ_FILE FILE
    #define MZ_FOPEN mz_fopen
    #define MZ_FCLOSE fclose
    #define MZ_FREAD fread
    #define MZ_FWRITE fwrite
    #define MZ_FTELL64 _ftelli64
    #define MZ_FSEEK64 _fseeki64
    #define MZ_FILE_STAT_STRUCT _stat
    #define MZ_FILE_STAT _stat
    #define MZ_FFLUSH fflush
    #define MZ_FREOPEN mz_freopen
    #define MZ_DELETE_FILE remove
  #elif defined(__MINGW32__)
    #ifndef MINIZ_NO_TIME
      #include <sys/utime.h>
    #endif
    #define MZ_FILE FILE
    #define MZ_FOPEN(f, m) fopen(f, m)
    #define MZ_FCLOSE fclose
    #define MZ_FREAD fread
    #define MZ_FWRITE fwrite
    #define MZ_FTELL64 ftello64
    #define MZ_FSEEK64 fseeko64
    #define MZ_FILE_STAT_STRUCT _stat
    #define MZ_FILE_STAT _stat
    #define MZ_FFLUSH fflush
    #define MZ_FREOPEN(f, m, s) freopen(f, m, s)
    #define MZ_DELETE_FILE remove
  #elif defined(__TINYC__)
    #ifndef MINIZ_NO_TIME
      #include <sys/utime.h>
    #endif
    #define MZ_FILE FILE
    #define MZ_FOPEN(f, m) fopen(f, m)
    #define MZ_FCLOSE fclose
    #define MZ_FREAD fread
    #define MZ_FWRITE fwrite
    #define MZ_FTELL64 ftell
    #define MZ_FSEEK64 fseek
    #define MZ_FILE_STAT_STRUCT stat
    #define MZ_FILE_STAT stat
    #define MZ_FFLUSH fflush
    #define MZ_FREOPEN(f, m, s) freopen(f, m, s)
    #define MZ_DELETE_FILE remove
  #elif defined(__GNUC__) && _LARGEFILE64_SOURCE
    #ifndef MINIZ_NO_TIME
      #include <utime.h>
    #endif
    #define MZ_FILE FILE
    #define MZ_FOPEN(f, m) fopen64(f, m)
    #define MZ_FCLOSE fclose
    #define MZ_FREAD fread
    #define MZ_FWRITE fwrite
    #define MZ_FTELL64 ftello64
    #define MZ_FSEEK64 fseeko64
    #define MZ_FILE_STAT_STRUCT stat64
    #define MZ_FILE_STAT stat64
    #define MZ_FFLUSH fflush
    #define MZ_FREOPEN(p, m, s) freopen64(p, m, s)
    #define MZ_DELETE_FILE remove
  #else
    #ifndef MINIZ_NO_TIME
      #include <utime.h>
    #endif
    #define MZ_FILE FILE
    #define MZ_FOPEN(f, m) fopen(f, m)
    #define MZ_FCLOSE fclose
    #define MZ_FREAD fread
    #define MZ_FWRITE fwrite
    #define MZ_FTELL64 ftello
    #define MZ_FSEEK64 fseeko
    #define MZ_FILE_STAT_STRUCT stat
    #define MZ_FILE_STAT stat
    #define MZ_FFLUSH fflush
    #define MZ_FREOPEN(f, m, s) freopen(f, m, s)
    #define MZ_DELETE_FILE remove
  #endif // #ifdef _MSC_VER
#endif // #ifdef MINIZ_NO_STDIO

#define MZ_TOLOWER(c) ((((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c))

// Various ZIP archive enums. To completely avoid cross platform compiler alignment and platform endian issues, miniz.c doesn't use structs for any of this stuff.
enum
{
  // ZIP archive identifiers and record sizes
  MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG = 0x06054b50, MZ_ZIP_CENTRAL_DIR_HEADER_SIG = 0x02014b50, MZ_ZIP_LOCAL_DIR_HEADER_SIG = 0x04034b50,
  MZ_ZIP_LOCAL_DIR_HEADER_SIZE = 30, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE = 46, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE = 22,
  // Central directory header record offsets
  MZ_ZIP_CDH_SIG_OFS = 0, MZ_ZIP_CDH_VERSION_MADE_BY_OFS = 4, MZ_ZIP_CDH_VERSION_NEEDED_OFS = 6, MZ_ZIP_CDH_BIT_FLAG_OFS = 8,
  MZ_ZIP_CDH_METHOD_OFS = 10, MZ_ZIP_CDH_FILE_TIME_OFS = 12, MZ_ZIP_CDH_FILE_DATE_OFS = 14, MZ_ZIP_CDH_CRC32_OFS = 16,
  MZ_ZIP_CDH_COMPRESSED_SIZE_OFS = 20, MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS = 24, MZ_ZIP_CDH_FILENAME_LEN_OFS = 28, MZ_ZIP_CDH_EXTRA_LEN_OFS = 30,
  MZ_ZIP_CDH_COMMENT_LEN_OFS = 32, MZ_ZIP_CDH_DISK_START_OFS = 34, MZ_ZIP_CDH_INTERNAL_ATTR_OFS = 36, MZ_ZIP_CDH_EXTERNAL_ATTR_OFS = 38, MZ_ZIP_CDH_LOCAL_HEADER_OFS = 42,
  // Local directory header offsets
  MZ_ZIP_LDH_SIG_OFS = 0, MZ_ZIP_LDH_VERSION_NEEDED_OFS = 4, MZ_ZIP_LDH_BIT_FLAG_OFS = 6, MZ_ZIP_LDH_METHOD_OFS = 8, MZ_ZIP_LDH_FILE_TIME_OFS = 10,
  MZ_ZIP_LDH_FILE_DATE_OFS = 12, MZ_ZIP_LDH_CRC32_OFS = 14, MZ_ZIP_LDH_COMPRESSED_SIZE_OFS = 18, MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS = 22,
  MZ_ZIP_LDH_FILENAME_LEN_OFS = 26, MZ_ZIP_LDH_EXTRA_LEN_OFS = 28,
  // End of central directory offsets
  MZ_ZIP_ECDH_SIG_OFS = 0, MZ_ZIP_ECDH_NUM_THIS_DISK_OFS = 4, MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS = 6, MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS = 8,
  MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS = 10, MZ_ZIP_ECDH_CDIR_SIZE_OFS = 12, MZ_ZIP_ECDH_CDIR_OFS_OFS = 16, MZ_ZIP_ECDH_COMMENT_SIZE_OFS = 20,
};

typedef struct
{
  void *m_p;
  size_t m_size, m_capacity;
  mz_uint m_element_size;
} mz_zip_array;

struct mz_zip_internal_state_tag
{
  mz_zip_array m_central_dir;
  mz_zip_array m_central_dir_offsets;
  mz_zip_array m_sorted_central_dir_offsets;
  MZ_FILE *m_pFile;
  void *m_pMem;
  size_t m_mem_size;
  size_t m_mem_capacity;
};

#define MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(array_ptr, element_size) (array_ptr)->m_element_size = element_size
#define MZ_ZIP_ARRAY_ELEMENT(array_ptr, element_type, index) ((element_type *)((array_ptr)->m_p))[index]

static MZ_FORCEINLINE void mz_zip_array_clear(mz_zip_archive *pZip, mz_zip_array *pArray)
{
  pZip->m_pFree(pZip->m_pAlloc_opaque, pArray->m_p);
  memset(pArray, 0, sizeof(mz_zip_array));
}

static mz_bool mz_zip_array_ensure_capacity(mz_zip_archive *pZip, mz_zip_array *pArray, size_t min_new_capacity, mz_uint growing)
{
  void *pNew_p; size_t new_capacity = min_new_capacity; MZ_ASSERT(pArray->m_element_size); if (pArray->m_capacity >= min_new_capacity) return MZ_TRUE;
  if (growing) { new_capacity = MZ_MAX(1, pArray->m_capacity); while (new_capacity < min_new_capacity) new_capacity *= 2; }
  if (NULL == (pNew_p = pZip->m_pRealloc(pZip->m_pAlloc_opaque, pArray->m_p, pArray->m_element_size, new_capacity))) return MZ_FALSE;
  pArray->m_p = pNew_p; pArray->m_capacity = new_capacity;
  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_array_reserve(mz_zip_archive *pZip, mz_zip_array *pArray, size_t new_capacity, mz_uint growing)
{
  if (new_capacity > pArray->m_capacity) { if (!mz_zip_array_ensure_capacity(pZip, pArray, new_capacity, growing)) return MZ_FALSE; }
  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_array_resize(mz_zip_archive *pZip, mz_zip_array *pArray, size_t new_size, mz_uint growing)
{
  if (new_size > pArray->m_capacity) { if (!mz_zip_array_ensure_capacity(pZip, pArray, new_size, growing)) return MZ_FALSE; }
  pArray->m_size = new_size;
  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_array_ensure_room(mz_zip_archive *pZip, mz_zip_array *pArray, size_t n)
{
  return mz_zip_array_reserve(pZip, pArray, pArray->m_size + n, MZ_TRUE);
}

static MZ_FORCEINLINE mz_bool mz_zip_array_push_back(mz_zip_archive *pZip, mz_zip_array *pArray, const void *pElements, size_t n)
{
  size_t orig_size = pArray->m_size; if (!mz_zip_array_resize(pZip, pArray, orig_size + n, MZ_TRUE)) return MZ_FALSE;
  memcpy((mz_uint8*)pArray->m_p + orig_size * pArray->m_element_size, pElements, n * pArray->m_element_size);
  return MZ_TRUE;
}

#ifndef MINIZ_NO_TIME
static time_t mz_zip_dos_to_time_t(int dos_time, int dos_date)
{
  struct tm tm;
  memset(&tm, 0, sizeof(tm)); tm.tm_isdst = -1;
  tm.tm_year = ((dos_date >> 9) & 127) + 1980 - 1900; tm.tm_mon = ((dos_date >> 5) & 15) - 1; tm.tm_mday = dos_date & 31;
  tm.tm_hour = (dos_time >> 11) & 31; tm.tm_min = (dos_time >> 5) & 63; tm.tm_sec = (dos_time << 1) & 62;
  return mktime(&tm);
}

static void mz_zip_time_to_dos_time(time_t time, mz_uint16 *pDOS_time, mz_uint16 *pDOS_date)
{
#ifdef _MSC_VER
  struct tm tm_struct;
  struct tm *tm = &tm_struct;
  errno_t err = localtime_s(tm, &time);
  if (err)
  {
    *pDOS_date = 0; *pDOS_time = 0;
    return;
  }
#else
  struct tm *tm = localtime(&time);
#endif
  *pDOS_time = (mz_uint16)(((tm->tm_hour) << 11) + ((tm->tm_min) << 5) + ((tm->tm_sec) >> 1));
  *pDOS_date = (mz_uint16)(((tm->tm_year + 1900 - 1980) << 9) + ((tm->tm_mon + 1) << 5) + tm->tm_mday);
}
#endif

#ifndef MINIZ_NO_STDIO
static mz_bool mz_zip_get_file_modified_time(const char *pFilename, mz_uint16 *pDOS_time, mz_uint16 *pDOS_date)
{
#ifdef MINIZ_NO_TIME
  (void)pFilename; *pDOS_date = *pDOS_time = 0;
#else
  struct MZ_FILE_STAT_STRUCT file_stat;
  // On Linux with x86 glibc, this call will fail on large files (>= 0x80000000 bytes) unless you compiled with _LARGEFILE64_SOURCE. Argh.
  if (MZ_FILE_STAT(pFilename, &file_stat) != 0)
    return MZ_FALSE;
  mz_zip_time_to_dos_time(file_stat.st_mtime, pDOS_time, pDOS_date);
#endif // #ifdef MINIZ_NO_TIME
  return MZ_TRUE;
}

#ifndef MINIZ_NO_TIME
static mz_bool mz_zip_set_file_times(const char *pFilename, time_t access_time, time_t modified_time)
{
  struct utimbuf t; t.actime = access_time; t.modtime = modified_time;
  return !utime(pFilename, &t);
}
#endif // #ifndef MINIZ_NO_TIME
#endif // #ifndef MINIZ_NO_STDIO

static mz_bool mz_zip_reader_init_internal(mz_zip_archive *pZip, mz_uint32 flags)
{
  (void)flags;
  if ((!pZip) || (pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_INVALID))
    return MZ_FALSE;

  if (!pZip->m_pAlloc) pZip->m_pAlloc = def_alloc_func;
  if (!pZip->m_pFree) pZip->m_pFree = def_free_func;
  if (!pZip->m_pRealloc) pZip->m_pRealloc = def_realloc_func;

  pZip->m_zip_mode = MZ_ZIP_MODE_READING;
  pZip->m_archive_size = 0;
  pZip->m_central_directory_file_ofs = 0;
  pZip->m_total_files = 0;

  if (NULL == (pZip->m_pState = (mz_zip_internal_state *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(mz_zip_internal_state))))
    return MZ_FALSE;
  memset(pZip->m_pState, 0, sizeof(mz_zip_internal_state));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir, sizeof(mz_uint8));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir_offsets, sizeof(mz_uint32));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_sorted_central_dir_offsets, sizeof(mz_uint32));
  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_reader_filename_less(const mz_zip_array *pCentral_dir_array, const mz_zip_array *pCentral_dir_offsets, mz_uint l_index, mz_uint r_index)
{
  const mz_uint8 *pL = &MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, mz_uint8, MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, mz_uint32, l_index)), *pE;
  const mz_uint8 *pR = &MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, mz_uint8, MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, mz_uint32, r_index));
  mz_uint l_len = MZ_READ_LE16(pL + MZ_ZIP_CDH_FILENAME_LEN_OFS), r_len = MZ_READ_LE16(pR + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  mz_uint8 l = 0, r = 0;
  pL += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE; pR += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pE = pL + MZ_MIN(l_len, r_len);
  while (pL < pE)
  {
    if ((l = MZ_TOLOWER(*pL)) != (r = MZ_TOLOWER(*pR)))
      break;
    pL++; pR++;
  }
  return (pL == pE) ? (l_len < r_len) : (l < r);
}

#define MZ_SWAP_UINT32(a, b) do { mz_uint32 t = a; a = b; b = t; } MZ_MACRO_END

// Heap sort of lowercased filenames, used to help accelerate plain central directory searches by mz_zip_reader_locate_file(). (Could also use qsort(), but it could allocate memory.)
static void mz_zip_reader_sort_central_dir_offsets_by_filename(mz_zip_archive *pZip)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  const mz_zip_array *pCentral_dir_offsets = &pState->m_central_dir_offsets;
  const mz_zip_array *pCentral_dir = &pState->m_central_dir;
  mz_uint32 *pIndices = &MZ_ZIP_ARRAY_ELEMENT(&pState->m_sorted_central_dir_offsets, mz_uint32, 0);
  const int size = pZip->m_total_files;
  int start = (size - 2) >> 1, end;
  while (start >= 0)
  {
    int child, root = start;
    for ( ; ; )
    {
      if ((child = (root << 1) + 1) >= size)
        break;
      child += (((child + 1) < size) && (mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[child], pIndices[child + 1])));
      if (!mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[root], pIndices[child]))
        break;
      MZ_SWAP_UINT32(pIndices[root], pIndices[child]); root = child;
    }
    start--;
  }

  end = size - 1;
  while (end > 0)
  {
    int child, root = 0;
    MZ_SWAP_UINT32(pIndices[end], pIndices[0]);
    for ( ; ; )
    {
      if ((child = (root << 1) + 1) >= end)
        break;
      child += (((child + 1) < end) && mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[child], pIndices[child + 1]));
      if (!mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets, pIndices[root], pIndices[child]))
        break;
      MZ_SWAP_UINT32(pIndices[root], pIndices[child]); root = child;
    }
    end--;
  }
}

static mz_bool mz_zip_reader_read_central_dir(mz_zip_archive *pZip, mz_uint32 flags)
{
  mz_uint cdir_size, num_this_disk, cdir_disk_index;
  mz_uint64 cdir_ofs;
  mz_int64 cur_file_ofs;
  const mz_uint8 *p;
  mz_uint32 buf_u32[4096 / sizeof(mz_uint32)]; mz_uint8 *pBuf = (mz_uint8 *)buf_u32;
  mz_bool sort_central_dir = ((flags & MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY) == 0);
  // Basic sanity checks - reject files which are too small, and check the first 4 bytes of the file to make sure a local header is there.
  if (pZip->m_archive_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
    return MZ_FALSE;
  // Find the end of central directory record by scanning the file from the end towards the beginning.
  cur_file_ofs = MZ_MAX((mz_int64)pZip->m_archive_size - (mz_int64)sizeof(buf_u32), 0);
  for ( ; ; )
  {
    int i, n = (int)MZ_MIN(sizeof(buf_u32), pZip->m_archive_size - cur_file_ofs);
    if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, n) != (mz_uint)n)
      return MZ_FALSE;
    for (i = n - 4; i >= 0; --i)
      if (MZ_READ_LE32(pBuf + i) == MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG)
        break;
    if (i >= 0)
    {
      cur_file_ofs += i;
      break;
    }
    if ((!cur_file_ofs) || ((pZip->m_archive_size - cur_file_ofs) >= (0xFFFF + MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)))
      return MZ_FALSE;
    cur_file_ofs = MZ_MAX(cur_file_ofs - (sizeof(buf_u32) - 3), 0);
  }
  // Read and verify the end of central directory record.
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE) != MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
    return MZ_FALSE;
  if ((MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_SIG_OFS) != MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG) ||
      ((pZip->m_total_files = MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS)) != MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS)))
    return MZ_FALSE;

  num_this_disk = MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_NUM_THIS_DISK_OFS);
  cdir_disk_index = MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS);
  if (((num_this_disk | cdir_disk_index) != 0) && ((num_this_disk != 1) || (cdir_disk_index != 1)))
    return MZ_FALSE;

  if ((cdir_size = MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_CDIR_SIZE_OFS)) < pZip->m_total_files * MZ_ZIP_CENTRAL_DIR_HEADER_SIZE)
    return MZ_FALSE;

  cdir_ofs = MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_CDIR_OFS_OFS);
  if ((cdir_ofs + (mz_uint64)cdir_size) > pZip->m_archive_size)
    return MZ_FALSE;

  pZip->m_central_directory_file_ofs = cdir_ofs;

  if (pZip->m_total_files)
  {
     mz_uint i, n;

    // Read the entire central directory into a heap block, and allocate another heap block to hold the unsorted central dir file record offsets, and another to hold the sorted indices.
    if ((!mz_zip_array_resize(pZip, &pZip->m_pState->m_central_dir, cdir_size, MZ_FALSE)) ||
        (!mz_zip_array_resize(pZip, &pZip->m_pState->m_central_dir_offsets, pZip->m_total_files, MZ_FALSE)))
      return MZ_FALSE;

    if (sort_central_dir)
    {
      if (!mz_zip_array_resize(pZip, &pZip->m_pState->m_sorted_central_dir_offsets, pZip->m_total_files, MZ_FALSE))
        return MZ_FALSE;
    }

    if (pZip->m_pRead(pZip->m_pIO_opaque, cdir_ofs, pZip->m_pState->m_central_dir.m_p, cdir_size) != cdir_size)
      return MZ_FALSE;

    // Now create an index into the central directory file records, do some basic sanity checking on each record, and check for zip64 entries (which are not yet supported).
    p = (const mz_uint8 *)pZip->m_pState->m_central_dir.m_p;
    for (n = cdir_size, i = 0; i < pZip->m_total_files; ++i)
    {
      mz_uint total_header_size, comp_size, decomp_size, disk_index;
      if ((n < MZ_ZIP_CENTRAL_DIR_HEADER_SIZE) || (MZ_READ_LE32(p) != MZ_ZIP_CENTRAL_DIR_HEADER_SIG))
        return MZ_FALSE;
      MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, mz_uint32, i) = (mz_uint32)(p - (const mz_uint8 *)pZip->m_pState->m_central_dir.m_p);
      if (sort_central_dir)
        MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_sorted_central_dir_offsets, mz_uint32, i) = i;
      comp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
      decomp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);
      if (((!MZ_READ_LE32(p + MZ_ZIP_CDH_METHOD_OFS)) && (decomp_size != comp_size)) || (decomp_size && !comp_size) || (decomp_size == 0xFFFFFFFF) || (comp_size == 0xFFFFFFFF))
        return MZ_FALSE;
      disk_index = MZ_READ_LE16(p + MZ_ZIP_CDH_DISK_START_OFS);
      if ((disk_index != num_this_disk) && (disk_index != 1))
        return MZ_FALSE;
      if (((mz_uint64)MZ_READ_LE32(p + MZ_ZIP_CDH_LOCAL_HEADER_OFS) + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + comp_size) > pZip->m_archive_size)
        return MZ_FALSE;
      if ((total_header_size = MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS) + MZ_READ_LE16(p + MZ_ZIP_CDH_EXTRA_LEN_OFS) + MZ_READ_LE16(p + MZ_ZIP_CDH_COMMENT_LEN_OFS)) > n)
        return MZ_FALSE;
      n -= total_header_size; p += total_header_size;
    }
  }

  if (sort_central_dir)
    mz_zip_reader_sort_central_dir_offsets_by_filename(pZip);

  return MZ_TRUE;
}

mz_bool mz_zip_reader_init(mz_zip_archive *pZip, mz_uint64 size, mz_uint32 flags)
{
  if ((!pZip) || (!pZip->m_pRead))
    return MZ_FALSE;
  if (!mz_zip_reader_init_internal(pZip, flags))
    return MZ_FALSE;
  pZip->m_archive_size = size;
  if (!mz_zip_reader_read_central_dir(pZip, flags))
  {
    mz_zip_reader_end(pZip);
    return MZ_FALSE;
  }
  return MZ_TRUE;
}

static size_t mz_zip_mem_read_func(void *pOpaque, mz_uint64 file_ofs, void *pBuf, size_t n)
{
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  size_t s = (file_ofs >= pZip->m_archive_size) ? 0 : (size_t)MZ_MIN(pZip->m_archive_size - file_ofs, n);
  memcpy(pBuf, (const mz_uint8 *)pZip->m_pState->m_pMem + file_ofs, s);
  return s;
}

mz_bool mz_zip_reader_init_mem(mz_zip_archive *pZip, const void *pMem, size_t size, mz_uint32 flags)
{
  if (!mz_zip_reader_init_internal(pZip, flags))
    return MZ_FALSE;
  pZip->m_archive_size = size;
  pZip->m_pRead = mz_zip_mem_read_func;
  pZip->m_pIO_opaque = pZip;
#ifdef __cplusplus
  pZip->m_pState->m_pMem = const_cast<void *>(pMem);
#else
  pZip->m_pState->m_pMem = (void *)pMem;
#endif
  pZip->m_pState->m_mem_size = size;
  if (!mz_zip_reader_read_central_dir(pZip, flags))
  {
    mz_zip_reader_end(pZip);
    return MZ_FALSE;
  }
  return MZ_TRUE;
}

#ifndef MINIZ_NO_STDIO
static size_t mz_zip_file_read_func(void *pOpaque, mz_uint64 file_ofs, void *pBuf, size_t n)
{
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  mz_int64 cur_ofs = MZ_FTELL64(pZip->m_pState->m_pFile);
  if (((mz_int64)file_ofs < 0) || (((cur_ofs != (mz_int64)file_ofs)) && (MZ_FSEEK64(pZip->m_pState->m_pFile, (mz_int64)file_ofs, SEEK_SET))))
    return 0;
  return MZ_FREAD(pBuf, 1, n, pZip->m_pState->m_pFile);
}

mz_bool mz_zip_reader_init_file(mz_zip_archive *pZip, const char *pFilename, mz_uint32 flags)
{
  mz_uint64 file_size;
  MZ_FILE *pFile = MZ_FOPEN(pFilename, "rb");
  if (!pFile)
    return MZ_FALSE;
  if (MZ_FSEEK64(pFile, 0, SEEK_END))
  {
    MZ_FCLOSE(pFile);
    return MZ_FALSE;
  }
  file_size = MZ_FTELL64(pFile);
  if (!mz_zip_reader_init_internal(pZip, flags))
  {
    MZ_FCLOSE(pFile);
    return MZ_FALSE;
  }
  pZip->m_pRead = mz_zip_file_read_func;
  pZip->m_pIO_opaque = pZip;
  pZip->m_pState->m_pFile = pFile;
  pZip->m_archive_size = file_size;
  if (!mz_zip_reader_read_central_dir(pZip, flags))
  {
    mz_zip_reader_end(pZip);
    return MZ_FALSE;
  }
  return MZ_TRUE;
}
#endif // #ifndef MINIZ_NO_STDIO

mz_uint mz_zip_reader_get_num_files(mz_zip_archive *pZip)
{
  return pZip ? pZip->m_total_files : 0;
}

static MZ_FORCEINLINE const mz_uint8 *mz_zip_reader_get_cdh(mz_zip_archive *pZip, mz_uint file_index)
{
  if ((!pZip) || (!pZip->m_pState) || (file_index >= pZip->m_total_files) || (pZip->m_zip_mode != MZ_ZIP_MODE_READING))
    return NULL;
  return &MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir, mz_uint8, MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, mz_uint32, file_index));
}

mz_bool mz_zip_reader_is_file_encrypted(mz_zip_archive *pZip, mz_uint file_index)
{
  mz_uint m_bit_flag;
  const mz_uint8 *p = mz_zip_reader_get_cdh(pZip, file_index);
  if (!p)
    return MZ_FALSE;
  m_bit_flag = MZ_READ_LE16(p + MZ_ZIP_CDH_BIT_FLAG_OFS);
  return (m_bit_flag & 1);
}

mz_bool mz_zip_reader_is_file_a_directory(mz_zip_archive *pZip, mz_uint file_index)
{
  mz_uint filename_len, external_attr;
  const mz_uint8 *p = mz_zip_reader_get_cdh(pZip, file_index);
  if (!p)
    return MZ_FALSE;

  // First see if the filename ends with a '/' character.
  filename_len = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  if (filename_len)
  {
    if (*(p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_len - 1) == '/')
      return MZ_TRUE;
  }

  // Bugfix: This code was also checking if the internal attribute was non-zero, which wasn't correct.
  // Most/all zip writers (hopefully) set DOS file/directory attributes in the low 16-bits, so check for the DOS directory flag and ignore the source OS ID in the created by field.
  // FIXME: Remove this check? Is it necessary - we already check the filename.
  external_attr = MZ_READ_LE32(p + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS);
  if ((external_attr & 0x10) != 0)
    return MZ_TRUE;

  return MZ_FALSE;
}

mz_bool mz_zip_reader_file_stat(mz_zip_archive *pZip, mz_uint file_index, mz_zip_archive_file_stat *pStat)
{
  mz_uint n;
  const mz_uint8 *p = mz_zip_reader_get_cdh(pZip, file_index);
  if ((!p) || (!pStat))
    return MZ_FALSE;

  // Unpack the central directory record.
  pStat->m_file_index = file_index;
  pStat->m_central_dir_ofs = MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, mz_uint32, file_index);
  pStat->m_version_made_by = MZ_READ_LE16(p + MZ_ZIP_CDH_VERSION_MADE_BY_OFS);
  pStat->m_version_needed = MZ_READ_LE16(p + MZ_ZIP_CDH_VERSION_NEEDED_OFS);
  pStat->m_bit_flag = MZ_READ_LE16(p + MZ_ZIP_CDH_BIT_FLAG_OFS);
  pStat->m_method = MZ_READ_LE16(p + MZ_ZIP_CDH_METHOD_OFS);
#ifndef MINIZ_NO_TIME
  pStat->m_time = mz_zip_dos_to_time_t(MZ_READ_LE16(p + MZ_ZIP_CDH_FILE_TIME_OFS), MZ_READ_LE16(p + MZ_ZIP_CDH_FILE_DATE_OFS));
#endif
  pStat->m_crc32 = MZ_READ_LE32(p + MZ_ZIP_CDH_CRC32_OFS);
  pStat->m_comp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
  pStat->m_uncomp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);
  pStat->m_internal_attr = MZ_READ_LE16(p + MZ_ZIP_CDH_INTERNAL_ATTR_OFS);
  pStat->m_external_attr = MZ_READ_LE32(p + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS);
  pStat->m_local_header_ofs = MZ_READ_LE32(p + MZ_ZIP_CDH_LOCAL_HEADER_OFS);

  // Copy as much of the filename and comment as possible.
  n = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS); n = MZ_MIN(n, MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE - 1);
  memcpy(pStat->m_filename, p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n); pStat->m_filename[n] = '\0';

  n = MZ_READ_LE16(p + MZ_ZIP_CDH_COMMENT_LEN_OFS); n = MZ_MIN(n, MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE - 1);
  pStat->m_comment_size = n;
  memcpy(pStat->m_comment, p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS) + MZ_READ_LE16(p + MZ_ZIP_CDH_EXTRA_LEN_OFS), n); pStat->m_comment[n] = '\0';

  return MZ_TRUE;
}

mz_uint mz_zip_reader_get_filename(mz_zip_archive *pZip, mz_uint file_index, char *pFilename, mz_uint filename_buf_size)
{
  mz_uint n;
  const mz_uint8 *p = mz_zip_reader_get_cdh(pZip, file_index);
  if (!p) { if (filename_buf_size) pFilename[0] = '\0'; return 0; }
  n = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  if (filename_buf_size)
  {
    n = MZ_MIN(n, filename_buf_size - 1);
    memcpy(pFilename, p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n);
    pFilename[n] = '\0';
  }
  return n + 1;
}

static MZ_FORCEINLINE mz_bool mz_zip_reader_string_equal(const char *pA, const char *pB, mz_uint len, mz_uint flags)
{
  mz_uint i;
  if (flags & MZ_ZIP_FLAG_CASE_SENSITIVE)
    return 0 == memcmp(pA, pB, len);
  for (i = 0; i < len; ++i)
    if (MZ_TOLOWER(pA[i]) != MZ_TOLOWER(pB[i]))
      return MZ_FALSE;
  return MZ_TRUE;
}

static MZ_FORCEINLINE int mz_zip_reader_filename_compare(const mz_zip_array *pCentral_dir_array, const mz_zip_array *pCentral_dir_offsets, mz_uint l_index, const char *pR, mz_uint r_len)
{
  const mz_uint8 *pL = &MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_array, mz_uint8, MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, mz_uint32, l_index)), *pE;
  mz_uint l_len = MZ_READ_LE16(pL + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  mz_uint8 l = 0, r = 0;
  pL += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pE = pL + MZ_MIN(l_len, r_len);
  while (pL < pE)
  {
    if ((l = MZ_TOLOWER(*pL)) != (r = MZ_TOLOWER(*pR)))
      break;
    pL++; pR++;
  }
  return (pL == pE) ? (int)(l_len - r_len) : (l - r);
}

static int mz_zip_reader_locate_file_binary_search(mz_zip_archive *pZip, const char *pFilename)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  const mz_zip_array *pCentral_dir_offsets = &pState->m_central_dir_offsets;
  const mz_zip_array *pCentral_dir = &pState->m_central_dir;
  mz_uint32 *pIndices = &MZ_ZIP_ARRAY_ELEMENT(&pState->m_sorted_central_dir_offsets, mz_uint32, 0);
  const int size = pZip->m_total_files;
  const mz_uint filename_len = (mz_uint)strlen(pFilename);
  int l = 0, h = size - 1;
  while (l <= h)
  {
    int m = (l + h) >> 1, file_index = pIndices[m], comp = mz_zip_reader_filename_compare(pCentral_dir, pCentral_dir_offsets, file_index, pFilename, filename_len);
    if (!comp)
      return file_index;
    else if (comp < 0)
      l = m + 1;
    else
      h = m - 1;
  }
  return -1;
}

int mz_zip_reader_locate_file(mz_zip_archive *pZip, const char *pName, const char *pComment, mz_uint flags)
{
  mz_uint file_index; size_t name_len, comment_len;
  if ((!pZip) || (!pZip->m_pState) || (!pName) || (pZip->m_zip_mode != MZ_ZIP_MODE_READING))
    return -1;
  if (((flags & (MZ_ZIP_FLAG_IGNORE_PATH | MZ_ZIP_FLAG_CASE_SENSITIVE)) == 0) && (!pComment) && (pZip->m_pState->m_sorted_central_dir_offsets.m_size))
    return mz_zip_reader_locate_file_binary_search(pZip, pName);
  name_len = strlen(pName); if (name_len > 0xFFFF) return -1;
  comment_len = pComment ? strlen(pComment) : 0; if (comment_len > 0xFFFF) return -1;
  for (file_index = 0; file_index < pZip->m_total_files; file_index++)
  {
    const mz_uint8 *pHeader = &MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir, mz_uint8, MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, mz_uint32, file_index));
    mz_uint filename_len = MZ_READ_LE16(pHeader + MZ_ZIP_CDH_FILENAME_LEN_OFS);
    const char *pFilename = (const char *)pHeader + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
    if (filename_len < name_len)
      continue;
    if (comment_len)
    {
      mz_uint file_extra_len = MZ_READ_LE16(pHeader + MZ_ZIP_CDH_EXTRA_LEN_OFS), file_comment_len = MZ_READ_LE16(pHeader + MZ_ZIP_CDH_COMMENT_LEN_OFS);
      const char *pFile_comment = pFilename + filename_len + file_extra_len;
      if ((file_comment_len != comment_len) || (!mz_zip_reader_string_equal(pComment, pFile_comment, file_comment_len, flags)))
        continue;
    }
    if ((flags & MZ_ZIP_FLAG_IGNORE_PATH) && (filename_len))
    {
      int ofs = filename_len - 1;
      do
      {
        if ((pFilename[ofs] == '/') || (pFilename[ofs] == '\\') || (pFilename[ofs] == ':'))
          break;
      } while (--ofs >= 0);
      ofs++;
      pFilename += ofs; filename_len -= ofs;
    }
    if ((filename_len == name_len) && (mz_zip_reader_string_equal(pName, pFilename, filename_len, flags)))
      return file_index;
  }
  return -1;
}

mz_bool mz_zip_reader_extract_to_mem_no_alloc(mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size, mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size)
{
  int status = TINFL_STATUS_DONE;
  mz_uint64 needed_size, cur_file_ofs, comp_remaining, out_buf_ofs = 0, read_buf_size, read_buf_ofs = 0, read_buf_avail;
  mz_zip_archive_file_stat file_stat;
  void *pRead_buf;
  mz_uint32 local_header_u32[(MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) / sizeof(mz_uint32)]; mz_uint8 *pLocal_header = (mz_uint8 *)local_header_u32;
  tinfl_decompressor inflator;

  if ((buf_size) && (!pBuf))
    return MZ_FALSE;

  if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return MZ_FALSE;

  // Empty file, or a directory (but not always a directory - I've seen odd zips with directories that have compressed data which inflates to 0 bytes)
  if (!file_stat.m_comp_size)
    return MZ_TRUE;

  // Entry is a subdirectory (I've seen old zips with dir entries which have compressed deflate data which inflates to 0 bytes, but these entries claim to uncompress to 512 bytes in the headers).
  // I'm torn how to handle this case - should it fail instead?
  if (mz_zip_reader_is_file_a_directory(pZip, file_index))
    return MZ_TRUE;

  // Encryption and patch files are not supported.
  if (file_stat.m_bit_flag & (1 | 32))
    return MZ_FALSE;

  // This function only supports stored and deflate.
  if ((!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) && (file_stat.m_method != 0) && (file_stat.m_method != MZ_DEFLATED))
    return MZ_FALSE;

  // Ensure supplied output buffer is large enough.
  needed_size = (flags & MZ_ZIP_FLAG_COMPRESSED_DATA) ? file_stat.m_comp_size : file_stat.m_uncomp_size;
  if (buf_size < needed_size)
    return MZ_FALSE;

  // Read and parse the local directory entry.
  cur_file_ofs = file_stat.m_local_header_ofs;
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return MZ_FALSE;
  if (MZ_READ_LE32(pLocal_header) != MZ_ZIP_LOCAL_DIR_HEADER_SIG)
    return MZ_FALSE;

  cur_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE + MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS) + MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS);
  if ((cur_file_ofs + file_stat.m_comp_size) > pZip->m_archive_size)
    return MZ_FALSE;

  if ((flags & MZ_ZIP_FLAG_COMPRESSED_DATA) || (!file_stat.m_method))
  {
    // The file is stored or the caller has requested the compressed data.
    if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, (size_t)needed_size) != needed_size)
      return MZ_FALSE;
    return ((flags & MZ_ZIP_FLAG_COMPRESSED_DATA) != 0) || (mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pBuf, (size_t)file_stat.m_uncomp_size) == file_stat.m_crc32);
  }

  // Decompress the file either directly from memory or from a file input buffer.
  tinfl_init(&inflator);

  if (pZip->m_pState->m_pMem)
  {
    // Read directly from the archive in memory.
    pRead_buf = (mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
    read_buf_size = read_buf_avail = file_stat.m_comp_size;
    comp_remaining = 0;
  }
  else if (pUser_read_buf)
  {
    // Use a user provided read buffer.
    if (!user_read_buf_size)
      return MZ_FALSE;
    pRead_buf = (mz_uint8 *)pUser_read_buf;
    read_buf_size = user_read_buf_size;
    read_buf_avail = 0;
    comp_remaining = file_stat.m_comp_size;
  }
  else
  {
    // Temporarily allocate a read buffer.
    read_buf_size = MZ_MIN(file_stat.m_comp_size, MZ_ZIP_MAX_IO_BUF_SIZE);
#ifdef _MSC_VER
    if (((0, sizeof(size_t) == sizeof(mz_uint32))) && (read_buf_size > 0x7FFFFFFF))
#else
    if (((sizeof(size_t) == sizeof(mz_uint32))) && (read_buf_size > 0x7FFFFFFF))
#endif
      return MZ_FALSE;
    if (NULL == (pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)read_buf_size)))
      return MZ_FALSE;
    read_buf_avail = 0;
    comp_remaining = file_stat.m_comp_size;
  }

  do
  {
    size_t in_buf_size, out_buf_size = (size_t)(file_stat.m_uncomp_size - out_buf_ofs);
    if ((!read_buf_avail) && (!pZip->m_pState->m_pMem))
    {
      read_buf_avail = MZ_MIN(read_buf_size, comp_remaining);
      if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
      {
        status = TINFL_STATUS_FAILED;
        break;
      }
      cur_file_ofs += read_buf_avail;
      comp_remaining -= read_buf_avail;
      read_buf_ofs = 0;
    }
    in_buf_size = (size_t)read_buf_avail;
    status = tinfl_decompress(&inflator, (mz_uint8 *)pRead_buf + read_buf_ofs, &in_buf_size, (mz_uint8 *)pBuf, (mz_uint8 *)pBuf + out_buf_ofs, &out_buf_size, TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF | (comp_remaining ? TINFL_FLAG_HAS_MORE_INPUT : 0));
    read_buf_avail -= in_buf_size;
    read_buf_ofs += in_buf_size;
    out_buf_ofs += out_buf_size;
  } while (status == TINFL_STATUS_NEEDS_MORE_INPUT);

  if (status == TINFL_STATUS_DONE)
  {
    // Make sure the entire file was decompressed, and check its CRC.
    if ((out_buf_ofs != file_stat.m_uncomp_size) || (mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pBuf, (size_t)file_stat.m_uncomp_size) != file_stat.m_crc32))
      status = TINFL_STATUS_FAILED;
  }

  if ((!pZip->m_pState->m_pMem) && (!pUser_read_buf))
    pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);

  return status == TINFL_STATUS_DONE;
}

mz_bool mz_zip_reader_extract_file_to_mem_no_alloc(mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size)
{
  int file_index = mz_zip_reader_locate_file(pZip, pFilename, NULL, flags);
  if (file_index < 0)
    return MZ_FALSE;
  return mz_zip_reader_extract_to_mem_no_alloc(pZip, file_index, pBuf, buf_size, flags, pUser_read_buf, user_read_buf_size);
}

mz_bool mz_zip_reader_extract_to_mem(mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size, mz_uint flags)
{
  return mz_zip_reader_extract_to_mem_no_alloc(pZip, file_index, pBuf, buf_size, flags, NULL, 0);
}

mz_bool mz_zip_reader_extract_file_to_mem(mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size, mz_uint flags)
{
  return mz_zip_reader_extract_file_to_mem_no_alloc(pZip, pFilename, pBuf, buf_size, flags, NULL, 0);
}

void *mz_zip_reader_extract_to_heap(mz_zip_archive *pZip, mz_uint file_index, size_t *pSize, mz_uint flags)
{
  mz_uint64 comp_size, uncomp_size, alloc_size;
  const mz_uint8 *p = mz_zip_reader_get_cdh(pZip, file_index);
  void *pBuf;

  if (pSize)
    *pSize = 0;
  if (!p)
    return NULL;

  comp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
  uncomp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);

  alloc_size = (flags & MZ_ZIP_FLAG_COMPRESSED_DATA) ? comp_size : uncomp_size;
#ifdef _MSC_VER
  if (((0, sizeof(size_t) == sizeof(mz_uint32))) && (alloc_size > 0x7FFFFFFF))
#else
  if (((sizeof(size_t) == sizeof(mz_uint32))) && (alloc_size > 0x7FFFFFFF))
#endif
    return NULL;
  if (NULL == (pBuf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)alloc_size)))
    return NULL;

  if (!mz_zip_reader_extract_to_mem(pZip, file_index, pBuf, (size_t)alloc_size, flags))
  {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
    return NULL;
  }

  if (pSize) *pSize = (size_t)alloc_size;
  return pBuf;
}

void *mz_zip_reader_extract_file_to_heap(mz_zip_archive *pZip, const char *pFilename, size_t *pSize, mz_uint flags)
{
  int file_index = mz_zip_reader_locate_file(pZip, pFilename, NULL, flags);
  if (file_index < 0)
  {
    if (pSize) *pSize = 0;
    return MZ_FALSE;
  }
  return mz_zip_reader_extract_to_heap(pZip, file_index, pSize, flags);
}

mz_bool mz_zip_reader_extract_to_callback(mz_zip_archive *pZip, mz_uint file_index, mz_file_write_func pCallback, void *pOpaque, mz_uint flags)
{
  int status = TINFL_STATUS_DONE; mz_uint file_crc32 = MZ_CRC32_INIT;
  mz_uint64 read_buf_size, read_buf_ofs = 0, read_buf_avail, comp_remaining, out_buf_ofs = 0, cur_file_ofs;
  mz_zip_archive_file_stat file_stat;
  void *pRead_buf = NULL; void *pWrite_buf = NULL;
  mz_uint32 local_header_u32[(MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) / sizeof(mz_uint32)]; mz_uint8 *pLocal_header = (mz_uint8 *)local_header_u32;

  if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return MZ_FALSE;

  // Empty file, or a directory (but not always a directory - I've seen odd zips with directories that have compressed data which inflates to 0 bytes)
  if (!file_stat.m_comp_size)
    return MZ_TRUE;

  // Entry is a subdirectory (I've seen old zips with dir entries which have compressed deflate data which inflates to 0 bytes, but these entries claim to uncompress to 512 bytes in the headers).
  // I'm torn how to handle this case - should it fail instead?
  if (mz_zip_reader_is_file_a_directory(pZip, file_index))
    return MZ_TRUE;

  // Encryption and patch files are not supported.
  if (file_stat.m_bit_flag & (1 | 32))
    return MZ_FALSE;

  // This function only supports stored and deflate.
  if ((!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) && (file_stat.m_method != 0) && (file_stat.m_method != MZ_DEFLATED))
    return MZ_FALSE;

  // Read and parse the local directory entry.
  cur_file_ofs = file_stat.m_local_header_ofs;
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return MZ_FALSE;
  if (MZ_READ_LE32(pLocal_header) != MZ_ZIP_LOCAL_DIR_HEADER_SIG)
    return MZ_FALSE;

  cur_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE + MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS) + MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS);
  if ((cur_file_ofs + file_stat.m_comp_size) > pZip->m_archive_size)
    return MZ_FALSE;

  // Decompress the file either directly from memory or from a file input buffer.
  if (pZip->m_pState->m_pMem)
  {
    pRead_buf = (mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
    read_buf_size = read_buf_avail = file_stat.m_comp_size;
    comp_remaining = 0;
  }
  else
  {
    read_buf_size = MZ_MIN(file_stat.m_comp_size, MZ_ZIP_MAX_IO_BUF_SIZE);
    if (NULL == (pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)read_buf_size)))
      return MZ_FALSE;
    read_buf_avail = 0;
    comp_remaining = file_stat.m_comp_size;
  }

  if ((flags & MZ_ZIP_FLAG_COMPRESSED_DATA) || (!file_stat.m_method))
  {
    // The file is stored or the caller has requested the compressed data.
    if (pZip->m_pState->m_pMem)
    {
#ifdef _MSC_VER
      if (((0, sizeof(size_t) == sizeof(mz_uint32))) && (file_stat.m_comp_size > 0xFFFFFFFF))
#else
      if (((sizeof(size_t) == sizeof(mz_uint32))) && (file_stat.m_comp_size > 0xFFFFFFFF))
#endif
        return MZ_FALSE;
      if (pCallback(pOpaque, out_buf_ofs, pRead_buf, (size_t)file_stat.m_comp_size) != file_stat.m_comp_size)
        status = TINFL_STATUS_FAILED;
      else if (!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA))
        file_crc32 = (mz_uint32)mz_crc32(file_crc32, (const mz_uint8 *)pRead_buf, (size_t)file_stat.m_comp_size);
      cur_file_ofs += file_stat.m_comp_size;
      out_buf_ofs += file_stat.m_comp_size;
      comp_remaining = 0;
    }
    else
    {
      while (comp_remaining)
      {
        read_buf_avail = MZ_MIN(read_buf_size, comp_remaining);
        if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
        {
          status = TINFL_STATUS_FAILED;
          break;
        }

        if (!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA))
          file_crc32 = (mz_uint32)mz_crc32(file_crc32, (const mz_uint8 *)pRead_buf, (size_t)read_buf_avail);

        if (pCallback(pOpaque, out_buf_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
        {
          status = TINFL_STATUS_FAILED;
          break;
        }
        cur_file_ofs += read_buf_avail;
        out_buf_ofs += read_buf_avail;
        comp_remaining -= read_buf_avail;
      }
    }
  }
  else
  {
    tinfl_decompressor inflator;
    tinfl_init(&inflator);

    if (NULL == (pWrite_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, TINFL_LZ_DICT_SIZE)))
      status = TINFL_STATUS_FAILED;
    else
    {
      do
      {
        mz_uint8 *pWrite_buf_cur = (mz_uint8 *)pWrite_buf + (out_buf_ofs & (TINFL_LZ_DICT_SIZE - 1));
        size_t in_buf_size, out_buf_size = TINFL_LZ_DICT_SIZE - (out_buf_ofs & (TINFL_LZ_DICT_SIZE - 1));
        if ((!read_buf_avail) && (!pZip->m_pState->m_pMem))
        {
          read_buf_avail = MZ_MIN(read_buf_size, comp_remaining);
          if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf, (size_t)read_buf_avail) != read_buf_avail)
          {
            status = TINFL_STATUS_FAILED;
            break;
          }
          cur_file_ofs += read_buf_avail;
          comp_remaining -= read_buf_avail;
          read_buf_ofs = 0;
        }

        in_buf_size = (size_t)read_buf_avail;
        status = tinfl_decompress(&inflator, (const mz_uint8 *)pRead_buf + read_buf_ofs, &in_buf_size, (mz_uint8 *)pWrite_buf, pWrite_buf_cur, &out_buf_size, comp_remaining ? TINFL_FLAG_HAS_MORE_INPUT : 0);
        read_buf_avail -= in_buf_size;
        read_buf_ofs += in_buf_size;

        if (out_buf_size)
        {
          if (pCallback(pOpaque, out_buf_ofs, pWrite_buf_cur, out_buf_size) != out_buf_size)
          {
            status = TINFL_STATUS_FAILED;
            break;
          }
          file_crc32 = (mz_uint32)mz_crc32(file_crc32, pWrite_buf_cur, out_buf_size);
          if ((out_buf_ofs += out_buf_size) > file_stat.m_uncomp_size)
          {
            status = TINFL_STATUS_FAILED;
            break;
          }
        }
      } while ((status == TINFL_STATUS_NEEDS_MORE_INPUT) || (status == TINFL_STATUS_HAS_MORE_OUTPUT));
    }
  }

  if ((status == TINFL_STATUS_DONE) && (!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)))
  {
    // Make sure the entire file was decompressed, and check its CRC.
    if ((out_buf_ofs != file_stat.m_uncomp_size) || (file_crc32 != file_stat.m_crc32))
      status = TINFL_STATUS_FAILED;
  }

  if (!pZip->m_pState->m_pMem)
    pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
  if (pWrite_buf)
    pZip->m_pFree(pZip->m_pAlloc_opaque, pWrite_buf);

  return status == TINFL_STATUS_DONE;
}

mz_bool mz_zip_reader_extract_file_to_callback(mz_zip_archive *pZip, const char *pFilename, mz_file_write_func pCallback, void *pOpaque, mz_uint flags)
{
  int file_index = mz_zip_reader_locate_file(pZip, pFilename, NULL, flags);
  if (file_index < 0)
    return MZ_FALSE;
  return mz_zip_reader_extract_to_callback(pZip, file_index, pCallback, pOpaque, flags);
}

#ifndef MINIZ_NO_STDIO
static size_t mz_zip_file_write_callback(void *pOpaque, mz_uint64 ofs, const void *pBuf, size_t n)
{
  (void)ofs; return MZ_FWRITE(pBuf, 1, n, (MZ_FILE*)pOpaque);
}

mz_bool mz_zip_reader_extract_to_file(mz_zip_archive *pZip, mz_uint file_index, const char *pDst_filename, mz_uint flags)
{
  mz_bool status;
  mz_zip_archive_file_stat file_stat;
  MZ_FILE *pFile;
  if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return MZ_FALSE;
  pFile = MZ_FOPEN(pDst_filename, "wb");
  if (!pFile)
    return MZ_FALSE;
  status = mz_zip_reader_extract_to_callback(pZip, file_index, mz_zip_file_write_callback, pFile, flags);
  if (MZ_FCLOSE(pFile) == EOF)
    return MZ_FALSE;
#ifndef MINIZ_NO_TIME
  if (status)
    mz_zip_set_file_times(pDst_filename, file_stat.m_time, file_stat.m_time);
#endif
  return status;
}
#endif // #ifndef MINIZ_NO_STDIO

mz_bool mz_zip_reader_end(mz_zip_archive *pZip)
{
  if ((!pZip) || (!pZip->m_pState) || (!pZip->m_pAlloc) || (!pZip->m_pFree) || (pZip->m_zip_mode != MZ_ZIP_MODE_READING))
    return MZ_FALSE;

  if (pZip->m_pState)
  {
    mz_zip_internal_state *pState = pZip->m_pState; pZip->m_pState = NULL;
    mz_zip_array_clear(pZip, &pState->m_central_dir);
    mz_zip_array_clear(pZip, &pState->m_central_dir_offsets);
    mz_zip_array_clear(pZip, &pState->m_sorted_central_dir_offsets);

#ifndef MINIZ_NO_STDIO
    if (pState->m_pFile)
    {
      MZ_FCLOSE(pState->m_pFile);
      pState->m_pFile = NULL;
    }
#endif // #ifndef MINIZ_NO_STDIO

    pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
  }
  pZip->m_zip_mode = MZ_ZIP_MODE_INVALID;

  return MZ_TRUE;
}

#ifndef MINIZ_NO_STDIO
mz_bool mz_zip_reader_extract_file_to_file(mz_zip_archive *pZip, const char *pArchive_filename, const char *pDst_filename, mz_uint flags)
{
  int file_index = mz_zip_reader_locate_file(pZip, pArchive_filename, NULL, flags);
  if (file_index < 0)
    return MZ_FALSE;
  return mz_zip_reader_extract_to_file(pZip, file_index, pDst_filename, flags);
}
#endif

// ------------------- .ZIP archive writing

#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS

static void mz_write_le16(mz_uint8 *p, mz_uint16 v) { p[0] = (mz_uint8)v; p[1] = (mz_uint8)(v >> 8); }
static void mz_write_le32(mz_uint8 *p, mz_uint32 v) { p[0] = (mz_uint8)v; p[1] = (mz_uint8)(v >> 8); p[2] = (mz_uint8)(v >> 16); p[3] = (mz_uint8)(v >> 24); }
#define MZ_WRITE_LE16(p, v) mz_write_le16((mz_uint8 *)(p), (mz_uint16)(v))
#define MZ_WRITE_LE32(p, v) mz_write_le32((mz_uint8 *)(p), (mz_uint32)(v))

mz_bool mz_zip_writer_init(mz_zip_archive *pZip, mz_uint64 existing_size)
{
  if ((!pZip) || (pZip->m_pState) || (!pZip->m_pWrite) || (pZip->m_zip_mode != MZ_ZIP_MODE_INVALID))
    return MZ_FALSE;

  if (pZip->m_file_offset_alignment)
  {
    // Ensure user specified file offset alignment is a power of 2.
    if (pZip->m_file_offset_alignment & (pZip->m_file_offset_alignment - 1))
      return MZ_FALSE;
  }

  if (!pZip->m_pAlloc) pZip->m_pAlloc = def_alloc_func;
  if (!pZip->m_pFree) pZip->m_pFree = def_free_func;
  if (!pZip->m_pRealloc) pZip->m_pRealloc = def_realloc_func;

  pZip->m_zip_mode = MZ_ZIP_MODE_WRITING;
  pZip->m_archive_size = existing_size;
  pZip->m_central_directory_file_ofs = 0;
  pZip->m_total_files = 0;

  if (NULL == (pZip->m_pState = (mz_zip_internal_state *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(mz_zip_internal_state))))
    return MZ_FALSE;
  memset(pZip->m_pState, 0, sizeof(mz_zip_internal_state));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir, sizeof(mz_uint8));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir_offsets, sizeof(mz_uint32));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_sorted_central_dir_offsets, sizeof(mz_uint32));
  return MZ_TRUE;
}

static size_t mz_zip_heap_write_func(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n)
{
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  mz_zip_internal_state *pState = pZip->m_pState;
  mz_uint64 new_size = MZ_MAX(file_ofs + n, pState->m_mem_size);
#ifdef _MSC_VER
  if ((!n) || ((0, sizeof(size_t) == sizeof(mz_uint32)) && (new_size > 0x7FFFFFFF)))
#else
  if ((!n) || ((sizeof(size_t) == sizeof(mz_uint32)) && (new_size > 0x7FFFFFFF)))
#endif
    return 0;
  if (new_size > pState->m_mem_capacity)
  {
    void *pNew_block;
    size_t new_capacity = MZ_MAX(64, pState->m_mem_capacity); while (new_capacity < new_size) new_capacity *= 2;
    if (NULL == (pNew_block = pZip->m_pRealloc(pZip->m_pAlloc_opaque, pState->m_pMem, 1, new_capacity)))
      return 0;
    pState->m_pMem = pNew_block; pState->m_mem_capacity = new_capacity;
  }
  memcpy((mz_uint8 *)pState->m_pMem + file_ofs, pBuf, n);
  pState->m_mem_size = (size_t)new_size;
  return n;
}

mz_bool mz_zip_writer_init_heap(mz_zip_archive *pZip, size_t size_to_reserve_at_beginning, size_t initial_allocation_size)
{
  pZip->m_pWrite = mz_zip_heap_write_func;
  pZip->m_pIO_opaque = pZip;
  if (!mz_zip_writer_init(pZip, size_to_reserve_at_beginning))
    return MZ_FALSE;
  if (0 != (initial_allocation_size = MZ_MAX(initial_allocation_size, size_to_reserve_at_beginning)))
  {
    if (NULL == (pZip->m_pState->m_pMem = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, initial_allocation_size)))
    {
      mz_zip_writer_end(pZip);
      return MZ_FALSE;
    }
    pZip->m_pState->m_mem_capacity = initial_allocation_size;
  }
  return MZ_TRUE;
}

#ifndef MINIZ_NO_STDIO
static size_t mz_zip_file_write_func(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n)
{
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  mz_int64 cur_ofs = MZ_FTELL64(pZip->m_pState->m_pFile);
  if (((mz_int64)file_ofs < 0) || (((cur_ofs != (mz_int64)file_ofs)) && (MZ_FSEEK64(pZip->m_pState->m_pFile, (mz_int64)file_ofs, SEEK_SET))))
    return 0;
  return MZ_FWRITE(pBuf, 1, n, pZip->m_pState->m_pFile);
}

mz_bool mz_zip_writer_init_file(mz_zip_archive *pZip, const char *pFilename, mz_uint64 size_to_reserve_at_beginning)
{
  MZ_FILE *pFile;
  pZip->m_pWrite = mz_zip_file_write_func;
  pZip->m_pIO_opaque = pZip;
  if (!mz_zip_writer_init(pZip, size_to_reserve_at_beginning))
    return MZ_FALSE;
  if (NULL == (pFile = MZ_FOPEN(pFilename, "wb")))
  {
    mz_zip_writer_end(pZip);
    return MZ_FALSE;
  }
  pZip->m_pState->m_pFile = pFile;
  if (size_to_reserve_at_beginning)
  {
    mz_uint64 cur_ofs = 0; char buf[4096]; MZ_CLEAR_OBJ(buf);
    do
    {
      size_t n = (size_t)MZ_MIN(sizeof(buf), size_to_reserve_at_beginning);
      if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_ofs, buf, n) != n)
      {
        mz_zip_writer_end(pZip);
        return MZ_FALSE;
      }
      cur_ofs += n; size_to_reserve_at_beginning -= n;
    } while (size_to_reserve_at_beginning);
  }
  return MZ_TRUE;
}
#endif // #ifndef MINIZ_NO_STDIO

mz_bool mz_zip_writer_init_from_reader(mz_zip_archive *pZip, const char *pFilename)
{
  mz_zip_internal_state *pState;
  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_READING))
    return MZ_FALSE;
  // No sense in trying to write to an archive that's already at the support max size
  if ((pZip->m_total_files == 0xFFFF) || ((pZip->m_archive_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + MZ_ZIP_LOCAL_DIR_HEADER_SIZE) > 0xFFFFFFFF))
    return MZ_FALSE;

  pState = pZip->m_pState;

  if (pState->m_pFile)
  {
#ifdef MINIZ_NO_STDIO
    pFilename; return MZ_FALSE;
#else
    // Archive is being read from stdio - try to reopen as writable.
    if (pZip->m_pIO_opaque != pZip)
      return MZ_FALSE;
    if (!pFilename)
      return MZ_FALSE;
    pZip->m_pWrite = mz_zip_file_write_func;
    if (NULL == (pState->m_pFile = MZ_FREOPEN(pFilename, "r+b", pState->m_pFile)))
    {
      // The mz_zip_archive is now in a bogus state because pState->m_pFile is NULL, so just close it.
      mz_zip_reader_end(pZip);
      return MZ_FALSE;
    }
#endif // #ifdef MINIZ_NO_STDIO
  }
  else if (pState->m_pMem)
  {
    // Archive lives in a memory block. Assume it's from the heap that we can resize using the realloc callback.
    if (pZip->m_pIO_opaque != pZip)
      return MZ_FALSE;
    pState->m_mem_capacity = pState->m_mem_size;
    pZip->m_pWrite = mz_zip_heap_write_func;
  }
  // Archive is being read via a user provided read function - make sure the user has specified a write function too.
  else if (!pZip->m_pWrite)
    return MZ_FALSE;

  // Start writing new files at the archive's current central directory location.
  pZip->m_archive_size = pZip->m_central_directory_file_ofs;
  pZip->m_zip_mode = MZ_ZIP_MODE_WRITING;
  pZip->m_central_directory_file_ofs = 0;

  return MZ_TRUE;
}

mz_bool mz_zip_writer_add_mem(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, mz_uint level_and_flags)
{
  return mz_zip_writer_add_mem_ex(pZip, pArchive_name, pBuf, buf_size, NULL, 0, level_and_flags, 0, 0);
}

typedef struct
{
  mz_zip_archive *m_pZip;
  mz_uint64 m_cur_archive_file_ofs;
  mz_uint64 m_comp_size;
} mz_zip_writer_add_state;

static mz_bool mz_zip_writer_add_put_buf_callback(const void* pBuf, int len, void *pUser)
{
  mz_zip_writer_add_state *pState = (mz_zip_writer_add_state *)pUser;
  if ((int)pState->m_pZip->m_pWrite(pState->m_pZip->m_pIO_opaque, pState->m_cur_archive_file_ofs, pBuf, len) != len)
    return MZ_FALSE;
  pState->m_cur_archive_file_ofs += len;
  pState->m_comp_size += len;
  return MZ_TRUE;
}

static mz_bool mz_zip_writer_create_local_dir_header(mz_zip_archive *pZip, mz_uint8 *pDst, mz_uint16 filename_size, mz_uint16 extra_size, mz_uint64 uncomp_size, mz_uint64 comp_size, mz_uint32 uncomp_crc32, mz_uint16 method, mz_uint16 bit_flags, mz_uint16 dos_time, mz_uint16 dos_date)
{
  (void)pZip;
  memset(pDst, 0, MZ_ZIP_LOCAL_DIR_HEADER_SIZE);
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_SIG_OFS, MZ_ZIP_LOCAL_DIR_HEADER_SIG);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_VERSION_NEEDED_OFS, method ? 20 : 0);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_BIT_FLAG_OFS, bit_flags);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_METHOD_OFS, method);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_FILE_TIME_OFS, dos_time);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_FILE_DATE_OFS, dos_date);
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_CRC32_OFS, uncomp_crc32);
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_COMPRESSED_SIZE_OFS, comp_size);
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS, uncomp_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_FILENAME_LEN_OFS, filename_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_EXTRA_LEN_OFS, extra_size);
  return MZ_TRUE;
}

static mz_bool mz_zip_writer_create_central_dir_header(mz_zip_archive *pZip, mz_uint8 *pDst, mz_uint16 filename_size, mz_uint16 extra_size, mz_uint16 comment_size, mz_uint64 uncomp_size, mz_uint64 comp_size, mz_uint32 uncomp_crc32, mz_uint16 method, mz_uint16 bit_flags, mz_uint16 dos_time, mz_uint16 dos_date, mz_uint64 local_header_ofs, mz_uint32 ext_attributes)
{
  (void)pZip;
  memset(pDst, 0, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_SIG_OFS, MZ_ZIP_CENTRAL_DIR_HEADER_SIG);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_VERSION_NEEDED_OFS, method ? 20 : 0);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_BIT_FLAG_OFS, bit_flags);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_METHOD_OFS, method);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_FILE_TIME_OFS, dos_time);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_FILE_DATE_OFS, dos_date);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_CRC32_OFS, uncomp_crc32);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS, comp_size);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS, uncomp_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_FILENAME_LEN_OFS, filename_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_EXTRA_LEN_OFS, extra_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_COMMENT_LEN_OFS, comment_size);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS, ext_attributes);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_LOCAL_HEADER_OFS, local_header_ofs);
  return MZ_TRUE;
}

static mz_bool mz_zip_writer_add_to_central_dir(mz_zip_archive *pZip, const char *pFilename, mz_uint16 filename_size, const void *pExtra, mz_uint16 extra_size, const void *pComment, mz_uint16 comment_size, mz_uint64 uncomp_size, mz_uint64 comp_size, mz_uint32 uncomp_crc32, mz_uint16 method, mz_uint16 bit_flags, mz_uint16 dos_time, mz_uint16 dos_date, mz_uint64 local_header_ofs, mz_uint32 ext_attributes)
{
  mz_zip_internal_state *pState = pZip->m_pState;
  mz_uint32 central_dir_ofs = (mz_uint32)pState->m_central_dir.m_size;
  size_t orig_central_dir_size = pState->m_central_dir.m_size;
  mz_uint8 central_dir_header[MZ_ZIP_CENTRAL_DIR_HEADER_SIZE];

  // No zip64 support yet
  if ((local_header_ofs > 0xFFFFFFFF) || (((mz_uint64)pState->m_central_dir.m_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_size + extra_size + comment_size) > 0xFFFFFFFF))
    return MZ_FALSE;

  if (!mz_zip_writer_create_central_dir_header(pZip, central_dir_header, filename_size, extra_size, comment_size, uncomp_size, comp_size, uncomp_crc32, method, bit_flags, dos_time, dos_date, local_header_ofs, ext_attributes))
    return MZ_FALSE;

  if ((!mz_zip_array_push_back(pZip, &pState->m_central_dir, central_dir_header, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE)) ||
      (!mz_zip_array_push_back(pZip, &pState->m_central_dir, pFilename, filename_size)) ||
      (!mz_zip_array_push_back(pZip, &pState->m_central_dir, pExtra, extra_size)) ||
      (!mz_zip_array_push_back(pZip, &pState->m_central_dir, pComment, comment_size)) ||
      (!mz_zip_array_push_back(pZip, &pState->m_central_dir_offsets, &central_dir_ofs, 1)))
  {
    // Try to push the central directory array back into its original state.
    mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size, MZ_FALSE);
    return MZ_FALSE;
  }

  return MZ_TRUE;
}

static mz_bool mz_zip_writer_validate_archive_name(const char *pArchive_name)
{
  // Basic ZIP archive filename validity checks: Valid filenames cannot start with a forward slash, cannot contain a drive letter, and cannot use DOS-style backward slashes.
  if (*pArchive_name == '/')
    return MZ_FALSE;
  while (*pArchive_name)
  {
    if ((*pArchive_name == '\\') || (*pArchive_name == ':'))
      return MZ_FALSE;
    pArchive_name++;
  }
  return MZ_TRUE;
}

static mz_uint mz_zip_writer_compute_padding_needed_for_file_alignment(mz_zip_archive *pZip)
{
  mz_uint32 n;
  if (!pZip->m_file_offset_alignment)
    return 0;
  n = (mz_uint32)(pZip->m_archive_size & (pZip->m_file_offset_alignment - 1));
  return (pZip->m_file_offset_alignment - n) & (pZip->m_file_offset_alignment - 1);
}

static mz_bool mz_zip_writer_write_zeros(mz_zip_archive *pZip, mz_uint64 cur_file_ofs, mz_uint32 n)
{
  char buf[4096];
  memset(buf, 0, MZ_MIN(sizeof(buf), n));
  while (n)
  {
    mz_uint32 s = MZ_MIN(sizeof(buf), n);
    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_file_ofs, buf, s) != s)
      return MZ_FALSE;
    cur_file_ofs += s; n -= s;
  }
  return MZ_TRUE;
}

mz_bool mz_zip_writer_add_mem_ex(mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32)
{
  mz_uint16 method = 0, dos_time = 0, dos_date = 0;
  mz_uint level, ext_attributes = 0, num_alignment_padding_bytes;
  mz_uint64 local_dir_header_ofs = pZip->m_archive_size, cur_archive_file_ofs = pZip->m_archive_size, comp_size = 0;
  size_t archive_name_size;
  mz_uint8 local_dir_header[MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
  tdefl_compressor *pComp = NULL;
  mz_bool store_data_uncompressed;
  mz_zip_internal_state *pState;

  if ((int)level_and_flags < 0)
    level_and_flags = MZ_DEFAULT_LEVEL;
  level = level_and_flags & 0xF;
  store_data_uncompressed = ((!level) || (level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA));

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) || ((buf_size) && (!pBuf)) || (!pArchive_name) || ((comment_size) && (!pComment)) || (pZip->m_total_files == 0xFFFF) || (level > MZ_UBER_COMPRESSION))
    return MZ_FALSE;

  pState = pZip->m_pState;

  if ((!(level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) && (uncomp_size))
    return MZ_FALSE;
  // No zip64 support yet
  if ((buf_size > 0xFFFFFFFF) || (uncomp_size > 0xFFFFFFFF))
    return MZ_FALSE;
  if (!mz_zip_writer_validate_archive_name(pArchive_name))
    return MZ_FALSE;

#ifndef MINIZ_NO_TIME
  {
    time_t cur_time; time(&cur_time);
    mz_zip_time_to_dos_time(cur_time, &dos_time, &dos_date);
  }
#endif // #ifndef MINIZ_NO_TIME

  archive_name_size = strlen(pArchive_name);
  if (archive_name_size > 0xFFFF)
    return MZ_FALSE;

  num_alignment_padding_bytes = mz_zip_writer_compute_padding_needed_for_file_alignment(pZip);

  // no zip64 support yet
  if ((pZip->m_total_files == 0xFFFF) || ((pZip->m_archive_size + num_alignment_padding_bytes + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + comment_size + archive_name_size) > 0xFFFFFFFF))
    return MZ_FALSE;

  if ((archive_name_size) && (pArchive_name[archive_name_size - 1] == '/'))
  {
    // Set DOS Subdirectory attribute bit.
    ext_attributes |= 0x10;
    // Subdirectories cannot contain data.
    if ((buf_size) || (uncomp_size))
      return MZ_FALSE;
  }

  // Try to do any allocations before writing to the archive, so if an allocation fails the file remains unmodified. (A good idea if we're doing an in-place modification.)
  if ((!mz_zip_array_ensure_room(pZip, &pState->m_central_dir, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size + comment_size)) || (!mz_zip_array_ensure_room(pZip, &pState->m_central_dir_offsets, 1)))
    return MZ_FALSE;

  if ((!store_data_uncompressed) && (buf_size))
  {
    if (NULL == (pComp = (tdefl_compressor *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(tdefl_compressor))))
      return MZ_FALSE;
  }

  if (!mz_zip_writer_write_zeros(pZip, cur_archive_file_ofs, num_alignment_padding_bytes + sizeof(local_dir_header)))
  {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
    return MZ_FALSE;
  }
  local_dir_header_ofs += num_alignment_padding_bytes;
  if (pZip->m_file_offset_alignment) { MZ_ASSERT((local_dir_header_ofs & (pZip->m_file_offset_alignment - 1)) == 0); }
  cur_archive_file_ofs += num_alignment_padding_bytes + sizeof(local_dir_header);

  MZ_CLEAR_OBJ(local_dir_header);
  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name, archive_name_size) != archive_name_size)
  {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
    return MZ_FALSE;
  }
  cur_archive_file_ofs += archive_name_size;

  if (!(level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA))
  {
    uncomp_crc32 = (mz_uint32)mz_crc32(MZ_CRC32_INIT, (const mz_uint8*)pBuf, buf_size);
    uncomp_size = buf_size;
    if (uncomp_size <= 3)
    {
      level = 0;
      store_data_uncompressed = MZ_TRUE;
    }
  }

  if (store_data_uncompressed)
  {
    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pBuf, buf_size) != buf_size)
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
      return MZ_FALSE;
    }

    cur_archive_file_ofs += buf_size;
    comp_size = buf_size;

    if (level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)
      method = MZ_DEFLATED;
  }
  else if (buf_size)
  {
    mz_zip_writer_add_state state;

    state.m_pZip = pZip;
    state.m_cur_archive_file_ofs = cur_archive_file_ofs;
    state.m_comp_size = 0;

    if ((tdefl_init(pComp, mz_zip_writer_add_put_buf_callback, &state, tdefl_create_comp_flags_from_zip_params(level, -15, MZ_DEFAULT_STRATEGY)) != TDEFL_STATUS_OKAY) ||
        (tdefl_compress_buffer(pComp, pBuf, buf_size, TDEFL_FINISH) != TDEFL_STATUS_DONE))
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
      return MZ_FALSE;
    }

    comp_size = state.m_comp_size;
    cur_archive_file_ofs = state.m_cur_archive_file_ofs;

    method = MZ_DEFLATED;
  }

  pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
  pComp = NULL;

  // no zip64 support yet
  if ((comp_size > 0xFFFFFFFF) || (cur_archive_file_ofs > 0xFFFFFFFF))
    return MZ_FALSE;

  if (!mz_zip_writer_create_local_dir_header(pZip, local_dir_header, (mz_uint16)archive_name_size, 0, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date))
    return MZ_FALSE;

  if (pZip->m_pWrite(pZip->m_pIO_opaque, local_dir_header_ofs, local_dir_header, sizeof(local_dir_header)) != sizeof(local_dir_header))
    return MZ_FALSE;

  if (!mz_zip_writer_add_to_central_dir(pZip, pArchive_name, (mz_uint16)archive_name_size, NULL, 0, pComment, comment_size, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date, local_dir_header_ofs, ext_attributes))
    return MZ_FALSE;

  pZip->m_total_files++;
  pZip->m_archive_size = cur_archive_file_ofs;

  return MZ_TRUE;
}

#ifndef MINIZ_NO_STDIO
mz_bool mz_zip_writer_add_file(mz_zip_archive *pZip, const char *pArchive_name, const char *pSrc_filename, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags)
{
  mz_uint uncomp_crc32 = MZ_CRC32_INIT, level, num_alignment_padding_bytes;
  mz_uint16 method = 0, dos_time = 0, dos_date = 0, ext_attributes = 0;
  mz_uint64 local_dir_header_ofs = pZip->m_archive_size, cur_archive_file_ofs = pZip->m_archive_size, uncomp_size = 0, comp_size = 0;
  size_t archive_name_size;
  mz_uint8 local_dir_header[MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
  MZ_FILE *pSrc_file = NULL;

  if ((int)level_and_flags < 0)
    level_and_flags = MZ_DEFAULT_LEVEL;
  level = level_and_flags & 0xF;

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) || (!pArchive_name) || ((comment_size) && (!pComment)) || (level > MZ_UBER_COMPRESSION))
    return MZ_FALSE;
  if (level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)
    return MZ_FALSE;
  if (!mz_zip_writer_validate_archive_name(pArchive_name))
    return MZ_FALSE;

  archive_name_size = strlen(pArchive_name);
  if (archive_name_size > 0xFFFF)
    return MZ_FALSE;

  num_alignment_padding_bytes = mz_zip_writer_compute_padding_needed_for_file_alignment(pZip);

  // no zip64 support yet
  if ((pZip->m_total_files == 0xFFFF) || ((pZip->m_archive_size + num_alignment_padding_bytes + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + comment_size + archive_name_size) > 0xFFFFFFFF))
    return MZ_FALSE;

  if (!mz_zip_get_file_modified_time(pSrc_filename, &dos_time, &dos_date))
    return MZ_FALSE;

  pSrc_file = MZ_FOPEN(pSrc_filename, "rb");
  if (!pSrc_file)
    return MZ_FALSE;
  MZ_FSEEK64(pSrc_file, 0, SEEK_END);
  uncomp_size = MZ_FTELL64(pSrc_file);
  MZ_FSEEK64(pSrc_file, 0, SEEK_SET);

  if (uncomp_size > 0xFFFFFFFF)
  {
    // No zip64 support yet
    MZ_FCLOSE(pSrc_file);
    return MZ_FALSE;
  }
  if (uncomp_size <= 3)
    level = 0;

  if (!mz_zip_writer_write_zeros(pZip, cur_archive_file_ofs, num_alignment_padding_bytes + sizeof(local_dir_header)))
  {
    MZ_FCLOSE(pSrc_file);
    return MZ_FALSE;
  }
  local_dir_header_ofs += num_alignment_padding_bytes;
  if (pZip->m_file_offset_alignment) { MZ_ASSERT((local_dir_header_ofs & (pZip->m_file_offset_alignment - 1)) == 0); }
  cur_archive_file_ofs += num_alignment_padding_bytes + sizeof(local_dir_header);

  MZ_CLEAR_OBJ(local_dir_header);
  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name, archive_name_size) != archive_name_size)
  {
    MZ_FCLOSE(pSrc_file);
    return MZ_FALSE;
  }
  cur_archive_file_ofs += archive_name_size;

  if (uncomp_size)
  {
    mz_uint64 uncomp_remaining = uncomp_size;
    void *pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, MZ_ZIP_MAX_IO_BUF_SIZE);
    if (!pRead_buf)
    {
      MZ_FCLOSE(pSrc_file);
      return MZ_FALSE;
    }

    if (!level)
    {
      while (uncomp_remaining)
      {
        mz_uint n = (mz_uint)MZ_MIN(MZ_ZIP_MAX_IO_BUF_SIZE, uncomp_remaining);
        if ((MZ_FREAD(pRead_buf, 1, n, pSrc_file) != n) || (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pRead_buf, n) != n))
        {
          pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
          MZ_FCLOSE(pSrc_file);
          return MZ_FALSE;
        }
        uncomp_crc32 = (mz_uint32)mz_crc32(uncomp_crc32, (const mz_uint8 *)pRead_buf, n);
        uncomp_remaining -= n;
        cur_archive_file_ofs += n;
      }
      comp_size = uncomp_size;
    }
    else
    {
      mz_bool result = MZ_FALSE;
      mz_zip_writer_add_state state;
      tdefl_compressor *pComp = (tdefl_compressor *)pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, sizeof(tdefl_compressor));
      if (!pComp)
      {
        pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
        MZ_FCLOSE(pSrc_file);
        return MZ_FALSE;
      }

      state.m_pZip = pZip;
      state.m_cur_archive_file_ofs = cur_archive_file_ofs;
      state.m_comp_size = 0;

      if (tdefl_init(pComp, mz_zip_writer_add_put_buf_callback, &state, tdefl_create_comp_flags_from_zip_params(level, -15, MZ_DEFAULT_STRATEGY)) != TDEFL_STATUS_OKAY)
      {
        pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
        pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
        MZ_FCLOSE(pSrc_file);
        return MZ_FALSE;
      }

      for ( ; ; )
      {
        size_t in_buf_size = (mz_uint32)MZ_MIN(uncomp_remaining, MZ_ZIP_MAX_IO_BUF_SIZE);
        tdefl_status status;

        if (MZ_FREAD(pRead_buf, 1, in_buf_size, pSrc_file) != in_buf_size)
          break;

        uncomp_crc32 = (mz_uint32)mz_crc32(uncomp_crc32, (const mz_uint8 *)pRead_buf, in_buf_size);
        uncomp_remaining -= in_buf_size;

        status = tdefl_compress_buffer(pComp, pRead_buf, in_buf_size, uncomp_remaining ? TDEFL_NO_FLUSH : TDEFL_FINISH);
        if (status == TDEFL_STATUS_DONE)
        {
          result = MZ_TRUE;
          break;
        }
        else if (status != TDEFL_STATUS_OKAY)
          break;
      }

      pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);

      if (!result)
      {
        pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
        MZ_FCLOSE(pSrc_file);
        return MZ_FALSE;
      }

      comp_size = state.m_comp_size;
      cur_archive_file_ofs = state.m_cur_archive_file_ofs;

      method = MZ_DEFLATED;
    }

    pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
  }

  MZ_FCLOSE(pSrc_file); pSrc_file = NULL;

  // no zip64 support yet
  if ((comp_size > 0xFFFFFFFF) || (cur_archive_file_ofs > 0xFFFFFFFF))
    return MZ_FALSE;

  if (!mz_zip_writer_create_local_dir_header(pZip, local_dir_header, (mz_uint16)archive_name_size, 0, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date))
    return MZ_FALSE;

  if (pZip->m_pWrite(pZip->m_pIO_opaque, local_dir_header_ofs, local_dir_header, sizeof(local_dir_header)) != sizeof(local_dir_header))
    return MZ_FALSE;

  if (!mz_zip_writer_add_to_central_dir(pZip, pArchive_name, (mz_uint16)archive_name_size, NULL, 0, pComment, comment_size, uncomp_size, comp_size, uncomp_crc32, method, 0, dos_time, dos_date, local_dir_header_ofs, ext_attributes))
    return MZ_FALSE;

  pZip->m_total_files++;
  pZip->m_archive_size = cur_archive_file_ofs;

  return MZ_TRUE;
}
#endif // #ifndef MINIZ_NO_STDIO

mz_bool mz_zip_writer_add_from_zip_reader(mz_zip_archive *pZip, mz_zip_archive *pSource_zip, mz_uint file_index)
{
  mz_uint n, bit_flags, num_alignment_padding_bytes;
  mz_uint64 comp_bytes_remaining, local_dir_header_ofs;
  mz_uint64 cur_src_file_ofs, cur_dst_file_ofs;
  mz_uint32 local_header_u32[(MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) / sizeof(mz_uint32)]; mz_uint8 *pLocal_header = (mz_uint8 *)local_header_u32;
  mz_uint8 central_header[MZ_ZIP_CENTRAL_DIR_HEADER_SIZE];
  size_t orig_central_dir_size;
  mz_zip_internal_state *pState;
  void *pBuf; const mz_uint8 *pSrc_central_header;

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING))
    return MZ_FALSE;
  if (NULL == (pSrc_central_header = mz_zip_reader_get_cdh(pSource_zip, file_index)))
    return MZ_FALSE;
  pState = pZip->m_pState;

  num_alignment_padding_bytes = mz_zip_writer_compute_padding_needed_for_file_alignment(pZip);

  // no zip64 support yet
  if ((pZip->m_total_files == 0xFFFF) || ((pZip->m_archive_size + num_alignment_padding_bytes + MZ_ZIP_LOCAL_DIR_HEADER_SIZE + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE) > 0xFFFFFFFF))
    return MZ_FALSE;

  cur_src_file_ofs = MZ_READ_LE32(pSrc_central_header + MZ_ZIP_CDH_LOCAL_HEADER_OFS);
  cur_dst_file_ofs = pZip->m_archive_size;

  if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return MZ_FALSE;
  if (MZ_READ_LE32(pLocal_header) != MZ_ZIP_LOCAL_DIR_HEADER_SIG)
    return MZ_FALSE;
  cur_src_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE;

  if (!mz_zip_writer_write_zeros(pZip, cur_dst_file_ofs, num_alignment_padding_bytes))
    return MZ_FALSE;
  cur_dst_file_ofs += num_alignment_padding_bytes;
  local_dir_header_ofs = cur_dst_file_ofs;
  if (pZip->m_file_offset_alignment) { MZ_ASSERT((local_dir_header_ofs & (pZip->m_file_offset_alignment - 1)) == 0); }

  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_dst_file_ofs, pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE) != MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return MZ_FALSE;
  cur_dst_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE;

  n = MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS) + MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS);
  comp_bytes_remaining = n + MZ_READ_LE32(pSrc_central_header + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);

  if (NULL == (pBuf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)MZ_MAX(sizeof(mz_uint32) * 4, MZ_MIN(MZ_ZIP_MAX_IO_BUF_SIZE, comp_bytes_remaining)))))
    return MZ_FALSE;

  while (comp_bytes_remaining)
  {
    n = (mz_uint)MZ_MIN(MZ_ZIP_MAX_IO_BUF_SIZE, comp_bytes_remaining);
    if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs, pBuf, n) != n)
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
      return MZ_FALSE;
    }
    cur_src_file_ofs += n;

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_dst_file_ofs, pBuf, n) != n)
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
      return MZ_FALSE;
    }
    cur_dst_file_ofs += n;

    comp_bytes_remaining -= n;
  }

  bit_flags = MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_BIT_FLAG_OFS);
  if (bit_flags & 8)
  {
    // Copy data descriptor
    if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs, pBuf, sizeof(mz_uint32) * 4) != sizeof(mz_uint32) * 4)
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
      return MZ_FALSE;
    }

    n = sizeof(mz_uint32) * ((MZ_READ_LE32(pBuf) == 0x08074b50) ? 4 : 3);
    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_dst_file_ofs, pBuf, n) != n)
    {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
      return MZ_FALSE;
    }

    cur_src_file_ofs += n;
    cur_dst_file_ofs += n;
  }
  pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);

  // no zip64 support yet
  if (cur_dst_file_ofs > 0xFFFFFFFF)
    return MZ_FALSE;

  orig_central_dir_size = pState->m_central_dir.m_size;

  memcpy(central_header, pSrc_central_header, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE);
  MZ_WRITE_LE32(central_header + MZ_ZIP_CDH_LOCAL_HEADER_OFS, local_dir_header_ofs);
  if (!mz_zip_array_push_back(pZip, &pState->m_central_dir, central_header, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE))
    return MZ_FALSE;

  n = MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_FILENAME_LEN_OFS) + MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_EXTRA_LEN_OFS) + MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_COMMENT_LEN_OFS);
  if (!mz_zip_array_push_back(pZip, &pState->m_central_dir, pSrc_central_header + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n))
  {
    mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size, MZ_FALSE);
    return MZ_FALSE;
  }

  if (pState->m_central_dir.m_size > 0xFFFFFFFF)
    return MZ_FALSE;
  n = (mz_uint32)orig_central_dir_size;
  if (!mz_zip_array_push_back(pZip, &pState->m_central_dir_offsets, &n, 1))
  {
    mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size, MZ_FALSE);
    return MZ_FALSE;
  }

  pZip->m_total_files++;
  pZip->m_archive_size = cur_dst_file_ofs;

  return MZ_TRUE;
}

mz_bool mz_zip_writer_finalize_archive(mz_zip_archive *pZip)
{
  mz_zip_internal_state *pState;
  mz_uint64 central_dir_ofs, central_dir_size;
  mz_uint8 hdr[MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE];

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING))
    return MZ_FALSE;

  pState = pZip->m_pState;

  // no zip64 support yet
  if ((pZip->m_total_files > 0xFFFF) || ((pZip->m_archive_size + pState->m_central_dir.m_size + MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE) > 0xFFFFFFFF))
    return MZ_FALSE;

  central_dir_ofs = 0;
  central_dir_size = 0;
  if (pZip->m_total_files)
  {
    // Write central directory
    central_dir_ofs = pZip->m_archive_size;
    central_dir_size = pState->m_central_dir.m_size;
    pZip->m_central_directory_file_ofs = central_dir_ofs;
    if (pZip->m_pWrite(pZip->m_pIO_opaque, central_dir_ofs, pState->m_central_dir.m_p, (size_t)central_dir_size) != central_dir_size)
      return MZ_FALSE;
    pZip->m_archive_size += central_dir_size;
  }

  // Write end of central directory record
  MZ_CLEAR_OBJ(hdr);
  MZ_WRITE_LE32(hdr + MZ_ZIP_ECDH_SIG_OFS, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG);
  MZ_WRITE_LE16(hdr + MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS, pZip->m_total_files);
  MZ_WRITE_LE16(hdr + MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS, pZip->m_total_files);
  MZ_WRITE_LE32(hdr + MZ_ZIP_ECDH_CDIR_SIZE_OFS, central_dir_size);
  MZ_WRITE_LE32(hdr + MZ_ZIP_ECDH_CDIR_OFS_OFS, central_dir_ofs);

  if (pZip->m_pWrite(pZip->m_pIO_opaque, pZip->m_archive_size, hdr, sizeof(hdr)) != sizeof(hdr))
    return MZ_FALSE;
#ifndef MINIZ_NO_STDIO
  if ((pState->m_pFile) && (MZ_FFLUSH(pState->m_pFile) == EOF))
    return MZ_FALSE;
#endif // #ifndef MINIZ_NO_STDIO

  pZip->m_archive_size += sizeof(hdr);

  pZip->m_zip_mode = MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED;
  return MZ_TRUE;
}

mz_bool mz_zip_writer_finalize_heap_archive(mz_zip_archive *pZip, void **pBuf, size_t *pSize)
{
  if ((!pZip) || (!pZip->m_pState) || (!pBuf) || (!pSize))
    return MZ_FALSE;
  if (pZip->m_pWrite != mz_zip_heap_write_func)
    return MZ_FALSE;
  if (!mz_zip_writer_finalize_archive(pZip))
    return MZ_FALSE;

  *pBuf = pZip->m_pState->m_pMem;
  *pSize = pZip->m_pState->m_mem_size;
  pZip->m_pState->m_pMem = NULL;
  pZip->m_pState->m_mem_size = pZip->m_pState->m_mem_capacity = 0;
  return MZ_TRUE;
}

mz_bool mz_zip_writer_end(mz_zip_archive *pZip)
{
  mz_zip_internal_state *pState;
  mz_bool status = MZ_TRUE;
  if ((!pZip) || (!pZip->m_pState) || (!pZip->m_pAlloc) || (!pZip->m_pFree) || ((pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) && (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED)))
    return MZ_FALSE;

  pState = pZip->m_pState;
  pZip->m_pState = NULL;
  mz_zip_array_clear(pZip, &pState->m_central_dir);
  mz_zip_array_clear(pZip, &pState->m_central_dir_offsets);
  mz_zip_array_clear(pZip, &pState->m_sorted_central_dir_offsets);

#ifndef MINIZ_NO_STDIO
  if (pState->m_pFile)
  {
    MZ_FCLOSE(pState->m_pFile);
    pState->m_pFile = NULL;
  }
#endif // #ifndef MINIZ_NO_STDIO

  if ((pZip->m_pWrite == mz_zip_heap_write_func) && (pState->m_pMem))
  {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pState->m_pMem);
    pState->m_pMem = NULL;
  }

  pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
  pZip->m_zip_mode = MZ_ZIP_MODE_INVALID;
  return status;
}

#ifndef MINIZ_NO_STDIO
mz_bool mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename, const char *pArchive_name, const void *pBuf, size_t buf_size, const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags)
{
  mz_bool status, created_new_archive = MZ_FALSE;
  mz_zip_archive zip_archive;
  struct MZ_FILE_STAT_STRUCT file_stat;
  MZ_CLEAR_OBJ(zip_archive);
  if ((int)level_and_flags < 0)
     level_and_flags = MZ_DEFAULT_LEVEL;
  if ((!pZip_filename) || (!pArchive_name) || ((buf_size) && (!pBuf)) || ((comment_size) && (!pComment)) || ((level_and_flags & 0xF) > MZ_UBER_COMPRESSION))
    return MZ_FALSE;
  if (!mz_zip_writer_validate_archive_name(pArchive_name))
    return MZ_FALSE;
  if (MZ_FILE_STAT(pZip_filename, &file_stat) != 0)
  {
    // Create a new archive.
    if (!mz_zip_writer_init_file(&zip_archive, pZip_filename, 0))
      return MZ_FALSE;
    created_new_archive = MZ_TRUE;
  }
  else
  {
    // Append to an existing archive.
    if (!mz_zip_reader_init_file(&zip_archive, pZip_filename, level_and_flags | MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY))
      return MZ_FALSE;
    if (!mz_zip_writer_init_from_reader(&zip_archive, pZip_filename))
    {
      mz_zip_reader_end(&zip_archive);
      return MZ_FALSE;
    }
  }
  status = mz_zip_writer_add_mem_ex(&zip_archive, pArchive_name, pBuf, buf_size, pComment, comment_size, level_and_flags, 0, 0);
  // Always finalize, even if adding failed for some reason, so we have a valid central directory. (This may not always succeed, but we can try.)
  if (!mz_zip_writer_finalize_archive(&zip_archive))
    status = MZ_FALSE;
  if (!mz_zip_writer_end(&zip_archive))
    status = MZ_FALSE;
  if ((!status) && (created_new_archive))
  {
    // It's a new archive and something went wrong, so just delete it.
    int ignoredStatus = MZ_DELETE_FILE(pZip_filename);
    (void)ignoredStatus;
  }
  return status;
}

void *mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const char *pArchive_name, size_t *pSize, mz_uint flags)
{
  int file_index;
  mz_zip_archive zip_archive;
  void *p = NULL;

  if (pSize)
    *pSize = 0;

  if ((!pZip_filename) || (!pArchive_name))
    return NULL;

  MZ_CLEAR_OBJ(zip_archive);
  if (!mz_zip_reader_init_file(&zip_archive, pZip_filename, flags | MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY))
    return NULL;

  if ((file_index = mz_zip_reader_locate_file(&zip_archive, pArchive_name, NULL, flags)) >= 0)
    p = mz_zip_reader_extract_to_heap(&zip_archive, file_index, pSize, flags);

  mz_zip_reader_end(&zip_archive);
  return p;
}

#endif // #ifndef MINIZ_NO_STDIO

#endif // #ifndef MINIZ_NO_ARCHIVE_WRITING_APIS

#endif // #ifndef MINIZ_NO_ARCHIVE_APIS

#ifdef __cplusplus
}
#endif

#endif // MINIZ_HEADER_FILE_ONLY

/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org/>
*/

// ---------------------------------
// back to program...

#include <map>
#include <set>
#include <vector>

std::map< std::string, std::string > archive;

std::map< std::string, pugi::xml_document > docs;
std::vector< std::string > ts;

std::set< std::string > ini;
size_t bytes = 0;

namespace options {
    const char *version = VERSION;
    const char *argv0 = "xlsx";
    const char *linefeed_replacement = "<br>";
}

std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

bool parse() {
    // load all xml docs in archive
    for( auto &file : archive ) {
        printf( "reading %s\n", file.first.c_str() );
        docs[ file.first ].load_string( file.second.c_str(), pugi::parse_default | pugi::parse_ws_pcdata );
    }
    // parse [//si/t/text()] and [//si/r/t/text(),//si/r/t/text(),[...]] from dictionary
    auto &shared = docs["xl/sharedStrings.xml"];
    for( auto &node : shared.select_nodes("//si") ) {
        static std::string str; str.clear(); // small memalloc optimization
        auto child = node.node().child("t");
        if ( child ) {
            str = child.child_value();
        } else {
            for( auto &child : node.node().children("r") ) {
                str += child.child("t").child_value();
            }
        }

        // @todo: at this point,
        // @todo: if str starts with space it is likely that @xml:space="preserve" attribute was found
        // @todo: in that case, remove trailing space?

        // replace all linefeeds
        str = replace_all(str, "\n", options::linefeed_replacement);
        // store
        ts.emplace_back( str );
    }
    // parse toc (workbook)
    for( auto &doc : docs["xl/workbook.xml"] ) {
        puts("parsing workbook...");
        auto select = doc.select_nodes("//sheet");

        // parse sheets
        int sheetno = 0;
        for (auto &it : select) {
            auto *sheetname = it.node().attribute("name").value();
            printf("%03d%% parsing sheet: %s\n", (int)((sheetno * 100.f) / select.size()), sheetname);

            // parse cells
            auto &sheet = docs["xl/worksheets/sheet" + std::to_string(++sheetno) + ".xml" ];
            auto select = sheet.select_nodes("//c");
            for (auto &it : select) {
                const char *r = it.node().attribute("r").value(); // row
                const char *s = it.node().attribute("s").value(); // style
                const char *t = it.node().attribute("t").value(); // type
                const char *f = it.node().child_value("f");       // formula
                const char *v = it.node().child_value("v");       // value
                if( r[0] && v[0] && (v[0] != '#' /*!= "#REF!"*/) ) {} else continue;

                static std::string str; str.clear(); // small memalloc optimization
                str = t[0] && t[0] == 's' ? ts[ atoi(v) ] : v;

                // debug
                // printf("r=%s, s=%s, t=%s, f=%s, v=%s, str=%s\n", r,s,t,f,v, str.c_str() );

                // determinate head value of column. look up to /*3 places*/ 1 place
                int lineno = 0;
                static std::string colname; colname.clear(); // small memalloc optimization
                for( lineno = 0; lineno < 1 /*3*/ && colname.empty(); ++lineno ) {
                    auto query = std::string() + "//c[@r=\"" + (std::string()+r[0]) + (std::string()+(char)(lineno+'1')) + "\" and @t=\"s\"]/v/text()";
                    auto select = sheet.select_node(query.c_str());
                    colname = select.node().value();
                }
                if( colname.empty() ) {
                    //printf("ignored value because of missing column name %s at %s\n", r, sheetname);
                    continue;
                }
                colname = ts[ atoi(colname.c_str()) ];

                // convert row H5 -> 1 (ID): ignore alpha (H), then subtract linenumber (heading offset) from value
                while( *r && (*r >= 'A' && *r <= 'Z')) r++;
                std::string id = std::to_string( *r ? atoi(r) - lineno : 0 );

                std::string rowname = "A" + std::to_string( *r ? 1 + atoi(r) - lineno : 1 );
                auto query = std::string() + "//c[@r=\"" + rowname + "\" and @t=\"s\"]/v/text()";
                auto select = sheet.select_node(query.c_str());
                rowname = select.node().value();
                if( !rowname.empty() ) {
                    rowname = ts[ atoi(rowname.c_str()) ];
                    //puts(rowname.c_str());
                }

                // insert for exporter
                // str = std::string("/") + sheetname + "/" + colname + "/" + id + "=" + str;
                str = colname + "." + rowname + "=" + str;
                bytes += str.size();
                ini.insert( str );
            }
        }
    }
    return true;
}

bool export_ini( const std::string &outfile ) {
    if( ini.empty() ) {
        return false;
    }
    FILE *fp;

    puts("exporting ini file...");
    fp = fopen( outfile.c_str(), "wb");
    if( !fp ) {
        return false;
    }
    fprintf(fp, "[%s]\n", options::argv0);
    for( auto &i : ini ) {
        fprintf(fp, "%s\n", i.c_str());
    }
    fclose(fp);

    printf("%d raw lines exported, %d KiB\n", (int)ini.size(), (int)(bytes / 1024) );
    return true;
}

bool export_cpp( const std::string &outfile ) {
    if( ini.empty() ) {
        return false;
    }
    FILE *fp;

    puts("exporting c++11 file...");
    fp = fopen( outfile.c_str(), "wb");
    if( !fp ) {
        return false;
    }
    fprintf(fp, "const char *lockit[] = {\n", options::argv0);
    for( auto &i : ini ) {
        fprintf(fp, "\tR\"(%s)\",\n", i.c_str());
    }
    fprintf(fp, "\t0\n};\n");
    fclose(fp);

    printf("%d raw lines exported, %d KiB\n", (int)ini.size(), (int)(bytes / 1024) );
    return true;
}

bool unpack( const char *file ) {
    // Try to open the archive.
    mz_zip_archive zip_archive = {0};

    if( mz_zip_reader_init_file( &zip_archive, file, 0 ) ) {
        for( unsigned int i = 0; i < mz_zip_reader_get_num_files(&zip_archive); i++ ) {
            mz_zip_archive_file_stat file_stat;

            if( !mz_zip_reader_file_stat( &zip_archive, i, &file_stat ) ) {
                mz_zip_reader_end( &zip_archive );
                return false; // "mz_zip_reader_file_stat() failed!"
            }

            std::string &data = archive[ file_stat.m_filename ];

            size_t uncomp_size = file_stat.m_uncomp_size;
            data.resize( (unsigned int)uncomp_size );
            mz_bool ok = mz_zip_reader_extract_file_to_mem(&zip_archive, file_stat.m_filename, &data[0], uncomp_size, 0);

            if( !ok ) {
                mz_zip_reader_end(&zip_archive);
                return false; // "mz_zip_reader_extract_file_to_heap() failed!"
            }
        }
    }

    mz_zip_reader_end(&zip_archive);
    return true;
}

bool process( const char *infile, const char *outfile ) {
    puts("unpacking...");
    if( unpack(infile) ) {
        printf("unpack ok. parsing %d files...\n", (int)archive.size());
        if( parse() ) {
            printf("parse ok. exporting %d symbols...\n", (int)ini.size());

            bool is_ini = !!strstr(outfile, ".ini");
            if( (is_ini ? export_ini : export_cpp)( outfile ) ) {
                return true;
            }
        }
    }
    return false;
}

#include <time.h>

int main(int argc, char **argv) {
    clock_t timer = clock();

    int rc = -1;
    if( argc == 3 ) rc = process( argv[1], argv[2] ) ? 0 : rc;
    else {
        printf("%s %s (public domain)\n", options::version, options::argv0);
        printf("Usage: %s infile.xlsx outfile.ini\n", options::argv0);
        printf("Usage: %s infile.xlsx outfile.cpp\n", options::argv0);
    }

    printf("%s %5.2fs.\n", rc ? "Error!" : "Ok,", (clock() - timer) / (double)CLOCKS_PER_SEC);
    return rc;
}
