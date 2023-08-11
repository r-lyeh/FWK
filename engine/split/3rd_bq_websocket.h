/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Samuli Raivio
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
----------------------------------------
*/

#ifndef BQ_WEBSOCKET_H_INCLUDED
#define BQ_WEBSOCKET_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4200) // warning C4200: nonstandard extension used: zero-sized array in struct/union
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bqws_socket bqws_socket;

typedef enum bqws_error {
    BQWS_OK = 0,

    // Unknown error from non-BQWS peer
    BQWS_ERR_UNKNOWN,

    // Rejected with `bqws_server_reject()`
    BQWS_ERR_SERVER_REJECT,

    // Data over limits of `bqws_limits`
    BQWS_ERR_LIMIT_MAX_MEMORY_USED,
    BQWS_ERR_LIMIT_MAX_RECV_MSG_SIZE,
    BQWS_ERR_LIMIT_MAX_HANDSHAKE_SIZE,
    BQWS_ERR_LIMIT_MAX_PARTIAL_MESSAGE_PARTS,

    // Peer didn't respond to handshake, PING, or CLOSE message in time
    BQWS_ERR_CONNECT_TIMEOUT,
    BQWS_ERR_PING_TIMEOUT,
    BQWS_ERR_CLOSE_TIMEOUT,

    // Allocator returned NULL
    BQWS_ERR_ALLOCATOR,

    // Protocol errors
    BQWS_ERR_BAD_CONTINUATION,
    BQWS_ERR_UNFINISHED_PARTIAL,
    BQWS_ERR_PARTIAL_CONTROL,
    BQWS_ERR_BAD_OPCODE,
    BQWS_ERR_RESERVED_BIT,
    BQWS_ERR_IO_WRITE,
    BQWS_ERR_IO_READ,
    BQWS_ERR_BAD_HANDSHAKE,
    BQWS_ERR_UNSUPPORTED_VERSION,
    BQWS_ERR_TOO_MANY_HEADERS,
    BQWS_ERR_TOO_MANY_PROTOCOLS,
    BQWS_ERR_HEADER_KEY_TOO_LONG,
    BQWS_ERR_HEADER_BAD_ACCEPT,
    BQWS_ERR_HEADER_PARSE,

} bqws_error;

typedef enum bqws_state {
    BQWS_STATE_INVALID,
    BQWS_STATE_CONNECTING,
    BQWS_STATE_OPEN,
    BQWS_STATE_CLOSING,
    BQWS_STATE_CLOSED,
} bqws_state;

typedef enum bqws_close_reason {
    BQWS_CLOSE_INVALID = 0,

    BQWS_CLOSE_NORMAL            = 1000,
    BQWS_CLOSE_GOING_AWAY        = 1001,
    BQWS_CLOSE_PROTOCOL_ERROR    = 1002,
    BQWS_CLOSE_UNSUPPORTED_TYPE  = 1003,
    BQWS_CLOSE_NO_REASON         = 1005,
    BQWS_CLOSE_ABNORMAL          = 1006,
    BQWS_CLOSE_BAD_DATA          = 1007,
    BQWS_CLOSE_GENERIC_ERROR     = 1008,
    BQWS_CLOSE_MESSAGE_TOO_BIG   = 1009,
    BQWS_CLOSE_EXTENSION_MISSING = 1010,
    BQWS_CLOSE_SERVER_ERROR      = 1011,
} bqws_close_reason;

typedef enum bqws_msg_type {

    BQWS_MSG_INVALID = 0,

    // Basic full text/binary messages
    BQWS_MSG_TEXT = 0x0001,
    BQWS_MSG_BINARY = 0x0002,

    // Reported only if `bqws_opts.recv_partial_messages` is `true`
    BQWS_MSG_PARTIAL_TEXT = 0x0011,
    BQWS_MSG_PARTIAL_BINARY = 0x0012,
    BQWS_MSG_FINAL_TEXT   = 0x0111,
    BQWS_MSG_FINAL_BINARY   = 0x0112,

    // Reported only if `bqws_opts.recv_control_messages` is `true`
    BQWS_MSG_CONTROL_CLOSE     = 0x1000,
    BQWS_MSG_CONTROL_PING      = 0x2000,
    BQWS_MSG_CONTROL_PONG      = 0x3000,

    // Masks for inspecting groups of types
    BQWS_MSG_TYPE_MASK    = 0x000f,
    BQWS_MSG_PARTIAL_BIT  = 0x0010,
    BQWS_MSG_FINAL_BIT    = 0x0100,
    BQWS_MSG_CONTROL_MASK = 0xf000,

} bqws_msg_type;

// Message buffers managed by bq_websocket.

typedef struct bqws_msg {

    // The socket that originally allocated this message
    bqws_socket *socket;

    // Type enum/bitmask
    bqws_msg_type type;

    // Size of the message in bytes, may be smaller than the
    // allocated buffer at `data`
    size_t size;

    // Size of `data` in bytes
    size_t capacity;

    char data[0];
} bqws_msg;

// Message header

// -- Allocaiton functions

typedef void *bqws_alloc_fn(void *user, size_t size);
typedef void *bqws_realloc_fn(void *user, void *ptr, size_t old_size, size_t new_size);
typedef void bqws_free_fn(void *user, void *ptr, size_t size);

// -- IO functions

// Called once before anything else from the updating thread.
typedef void bqws_io_init_fn(void *user, bqws_socket *ws);

// Send `size` bytes of `data` to its peer.
// Return the number of bytes actually sent or `SIZE_MAX` if an IO error occurred.
typedef size_t bqws_io_send_fn(void *user, bqws_socket *ws, const void *data, size_t size);

// Read up to `max_size` bytes to `data`. It's safe to read `min_bytes` with blocking IO.
// Return the number of bytes actually read or `SIZE_MAX` if an IO error occurred.
typedef size_t bqws_io_recv_fn(void *user, bqws_socket *ws, void *data, size_t max_size, size_t min_size);

// Notification that there is more data to send from the socket.
typedef void bqws_io_notify_fn(void *user, bqws_socket *ws);

// Flush all buffered data to the peer.
typedef bool bqws_io_flush_fn(void *user, bqws_socket *ws);

// Close and free all the resources used by the IO
typedef void bqws_io_close_fn(void *user, bqws_socket *ws);

// -- Miscellaneous callback functions

// Called when the socket receives a message. Return `true` to consume/filter the message
// preventing it from entering the `bqws_recv()` queue.
typedef bool bqws_message_fn(void *user, bqws_socket *ws, bqws_msg *msg);

// Send a message directly without IO, for example using native WebSockets on web.
// Called repeatedly with the same message until you return `true`.
typedef bool bqws_send_message_fn(void *user, bqws_socket *ws, bqws_msg *msg);

// Peek at all messages (including control messages).
typedef void bqws_peek_fn(void *user, bqws_socket *ws, bqws_msg *msg, bool received);

// Log state transitions, errors and optionally sent/received messages.
typedef void bqws_log_fn(void *user, bqws_socket *ws, const char *line);

// Called when the socket encounters an error.
typedef void bqws_error_fn(void *user, bqws_socket *ws, bqws_error error);

// Allocator callbacks with user context pointer
typedef struct bqws_allocator {
    void *user;
    bqws_alloc_fn *alloc_fn;
    bqws_realloc_fn *realloc_fn;
    bqws_free_fn *free_fn;
} bqws_allocator;

// IO callbacks with user context pointer,
// see prototypes above for description
typedef struct bqws_io {
    void *user;
    bqws_io_init_fn *init_fn;
    bqws_io_send_fn *send_fn;
    bqws_io_recv_fn *recv_fn;
    bqws_io_notify_fn *notify_fn;
    bqws_io_flush_fn *flush_fn;
    bqws_io_close_fn *close_fn;
} bqws_io;

typedef struct bqws_limits {

    // Maximum total memory used
    // default: 262144
    size_t max_memory_used;

    // Maximum received message length
    // default: 262144
    size_t max_recv_msg_size;

    // Maximum handshake length
    // default: 262144
    size_t max_handshake_size;

    // Maximum number of queued received messages
    // default: 1024
    size_t max_recv_queue_messages;

    // Maximum size of queued received messages in bytes
    // default: 262144
    size_t max_recv_queue_size;

    // Maximum number of parts in a chunked message
    // default: 16384
    size_t max_partial_message_parts;

} bqws_limits;

typedef struct bqws_opts {

    // Name for the socket for debugging
    const char *name;

    bqws_io io;
    bqws_allocator allocator;
    bqws_limits limits;

    // Message callback
    bqws_message_fn *message_fn;
    void *message_user;

    // Peek at all control/partial incoming messages even if
    // `recv_partial_messages` and `recv_control_messages are disabled.
    bqws_peek_fn *peek_fn;
    void *peek_user;

    // Verbose log of all events for this socket
    bqws_log_fn *log_fn;
    void *log_user;

    // Log also send/receive events
    bool log_send;
    bool log_recv;

    // Error callback
    bqws_error_fn *error_fn;
    void *error_user;

    // Send messages from this socket manually without IO
    bqws_send_message_fn *send_message_fn;
    void *send_message_user;

    // User data block, if `user_size > 0` but `user_data == NULL`
    // the data will be zero-initialized
    void *user_data;
    size_t user_size;

    // How long to wait (milliseconds) for the connecting to succeed before giving up.
    // Use SIZE_MAX to disable the timeout.
    // default: 10000
    size_t connect_timeout;

    // How often (milliseconds) to send PING messages if there is no traffic,
    // use SIZE_MAX to disable automatic PING
    // default: server: 20000, client: 10000
    size_t ping_interval;

    // How long to wait (milliseconds) for the close response before forcing the
    // state to be BQWS_STATE_CLOSED. Use SIZE_MAX to disable
    // the close timeout.
    // default: 5000
    size_t close_timeout;

    // How long to wait (milliseconds) for a ping response before forcing
    // the state to be BQWS_STATE_CLOSED. Use SIZE_MAX to disable.
    // the close timeout.
    // default: 4 * ping_interval
    size_t ping_response_timeout;

    // If set returns `BQWS_MSG_PARTIAL_*` messages from `bqws_recv()`
    bool recv_partial_messages;

    // If set returns `BQWS_MSG_CONTROL_*` messages from `bqws_recv()`
    bool recv_control_messages;

    // Mask messages sent by the server as well
    bool mask_server;

    // Don't mask client messages, violates the spec!
    bool unsafe_dont_mask_client;

    // Start the connection in BQWS_STATE_OPEN state
    bool skip_handshake;

} bqws_opts;

#define BQWS_MAX_HEADERS 64
#define BQWS_MAX_PROTOCOLS 64

// HTTP header key-value pair.
typedef struct bqws_header {
    const char *name;
    const char *value;
} bqws_header;

typedef struct bqws_client_opts {

    // Standard HTTP headers used by the handshake
    const char *path;
    const char *host;
    const char *origin;

    // WebSocket protocols to request
    const char *protocols[BQWS_MAX_PROTOCOLS];
    size_t num_protocols;

    // Extra HTTP headers
    bqws_header headers[BQWS_MAX_HEADERS];
    size_t num_headers;

    // Random key (optional)
    bool use_random_key;
    uint8_t random_key[16];

} bqws_client_opts;

// Call `bqws_server_accept()` or `bqws_server_reject()` here to handle the socket
typedef void bqws_verify_fn(void *user, bqws_socket *ws, const bqws_client_opts *opts);

typedef struct bqws_server_opts {

    // Automatically verify connections matching these client options.
    bqws_client_opts *verify_filter;

    // Verify callback, same as polling `bqws_server_get_client_options()`
    // and calling `bqws_server_accept()`
    bqws_verify_fn *verify_fn;
    void *verify_user;

} bqws_server_opts;

// [wss://][host.example.com][:12345][/directory]
//  scehme       host          port      path
typedef struct bqws_url {
    bool secure;
    uint16_t port;
    char scheme[16];
    char host[256];
    const char *path;
} bqws_url;

typedef struct bqws_io_stats {
    uint64_t total_messages;
    uint64_t total_bytes;

    size_t queued_messages;
    size_t queued_bytes;
} bqws_io_stats;

typedef struct bqws_stats {
    bqws_io_stats recv;
    bqws_io_stats send;
} bqws_stats;

// -- WebSocket management

// Create a new client/server socket. `opts`, `client_opts`, `server_opts` are all optional.
bqws_socket *bqws_new_client(const bqws_opts *opts, const bqws_client_opts *client_opts);
bqws_socket *bqws_new_server(const bqws_opts *opts, const bqws_server_opts *server_opts);

// Call at any point to destroy the socket and free all used resources.
void bqws_free_socket(bqws_socket *ws);

// Graceful shutdown: Prepare to close the socket by sending a close message.
// `bqws_close()` sends the close message as soon as possible while `bqws_queue_close()`
// sends all other queued messages first.
void bqws_close(bqws_socket *ws, bqws_close_reason reason, const void *data, size_t size);
void bqws_queue_close(bqws_socket *ws, bqws_close_reason reason, const void *data, size_t size);

// -- Server connect

// Accept or reject connections based on headers.
// Valid only until you call `bqws_server_connect()` or `bqws_free_socket()`!
bqws_client_opts *bqws_server_get_client_opts(bqws_socket *ws);
void bqws_server_accept(bqws_socket *ws, const char *protocol);
void bqws_server_reject(bqws_socket *ws);

// -- Query state

bqws_state bqws_get_state(const bqws_socket *ws);
bqws_error bqws_get_error(const bqws_socket *ws);
bool bqws_is_connecting(const bqws_socket *ws);
bool bqws_is_closed(const bqws_socket *ws);
size_t bqws_get_memory_used(const bqws_socket *ws);
bool bqws_is_server(const bqws_socket *ws);
void *bqws_user_data(const bqws_socket *ws);
size_t bqws_user_data_size(const bqws_socket *ws);
const char *bqws_get_name(const bqws_socket *ws);
bqws_stats bqws_get_stats(const bqws_socket *ws);
void *bqws_get_io_user(const bqws_socket *ws);
bool bqws_get_io_closed(const bqws_socket *ws);

// Get/update limits
bqws_limits bqws_get_limits(const bqws_socket *ws);
void bqws_set_limits(bqws_socket *ws, const bqws_limits *limits);

// Peer closing
bqws_close_reason bqws_get_peer_close_reason(const bqws_socket *ws);
bqws_error bqws_get_peer_error(const bqws_socket *ws);

// Get the chosen protocol, returns "" if none chosen but the connection is open
// Returns NULL if the connection is not established
const char *bqws_get_protocol(const bqws_socket *ws);

// -- Communication

// Receive a message, use `bqws_free_msg()` to free the returned pointer
bqws_msg *bqws_recv(bqws_socket *ws);
void bqws_free_msg(bqws_msg *msg);

// Single message
void bqws_send(bqws_socket *ws, bqws_msg_type type, const void *data, size_t size);
void bqws_send_binary(bqws_socket *ws, const void *data, size_t size);
void bqws_send_text(bqws_socket *ws, const char *str);
void bqws_send_text_len(bqws_socket *ws, const void *str, size_t len);

// Write to socket-provided memory
bqws_msg *bqws_allocate_msg(bqws_socket *ws, bqws_msg_type type, size_t size);
void bqws_send_msg(bqws_socket *ws, bqws_msg *msg);

// Streaming messages
void bqws_send_begin(bqws_socket *ws, bqws_msg_type type);
void bqws_send_append(bqws_socket *ws, const void *data, size_t size);
void bqws_send_append_str(bqws_socket *ws, const char *str);
void bqws_send_append_msg(bqws_socket *ws, bqws_msg *msg);
void bqws_send_finish(bqws_socket *ws);

// Send manual control messages
void bqws_send_ping(bqws_socket *ws, const void *data, size_t size);
void bqws_send_pong(bqws_socket *ws, const void *data, size_t size);

// -- Updating and IO

// Keep the socket alive, reads/writes buffered data and responds to pings/pongs
// Semantically equivalent to bqws_update_state() and bqws_update_io()
void bqws_update(bqws_socket *ws);

// Send/respond to PING/PONG, update close timeouts, etc...
void bqws_update_state(bqws_socket *ws);

// Call user-provided IO callbacks for reading/writing or both.
void bqws_update_io(bqws_socket *ws);
void bqws_update_io_read(bqws_socket *ws);
void bqws_update_io_write(bqws_socket *ws);

// Non-callback IO: Read data to send to the peer or write data received from the peer.
size_t bqws_read_from(bqws_socket *ws, const void *data, size_t size);
size_t bqws_write_to(bqws_socket *ws, void *data, size_t size);

// Direct control
void bqws_direct_push_msg(bqws_socket *ws, bqws_msg *msg);
void bqws_direct_set_override_state(bqws_socket *ws, bqws_state state);
void bqws_direct_fail(bqws_socket *ws, bqws_error err);

// -- Utility

// Parse `str` to `url` returning `true` on success.
// `url->path` still refers to `str` and is not a copy!
bool bqws_parse_url(bqws_url *url, const char *str);

// Enum -> string conversion
const char *bqws_error_str(bqws_error error);
const char *bqws_msg_type_str(bqws_msg_type type);
const char *bqws_state_str(bqws_state state);

void *bqws_allocator_alloc(const bqws_allocator *at, size_t size);
void *bqws_allocator_realloc(const bqws_allocator *at, void *ptr, size_t old_size, size_t new_size);
void bqws_allocator_free(const bqws_allocator *at, void *ptr, size_t size);

#ifdef __cplusplus
}
#endif

#ifdef _MSC_VER
    #pragma warning(push)
#endif

#endif // BQ_WEBSOCKET_H_INCLUDED

// -----------------------------------------------------------------------------

#ifndef BQ_WEBSOCKET_PLATFORM_H_INCLUDED
#define BQ_WEBSOCKET_PLATFORM_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BQWS_PT_MAX_ADDRESS_SIZE 16
#define BQWS_PT_MAX_ADDRESS_FORMAT_LENGTH 64

typedef struct bqws_pt_server bqws_pt_server;

typedef enum bqws_pt_error_type {
    BQWS_PT_ERRTYPE_NONE,

    // bqws_pt_error_code
    BQWS_PT_ERRTYPE_PT,

    // Windows Sockets error codes
    // https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
    BQWS_PT_ERRTYPE_WSA,

    // POSIX errno codes
    // https://www-numi.fnal.gov/offline_software/srt_public_context/WebDocs/Errors/unix_system_errors.html
    BQWS_PT_ERRTYPE_POSIX,

    // getaddrinfo() error codes
    // http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
    BQWS_PT_ERRTYPE_GETADDRINFO,

    // OpenSSL error codes
    BQWS_PT_ERRTYPE_OPENSSL,

} bqws_pt_error_type;

typedef enum bqws_pt_error_code {
    BQWS_PT_OK,
    BQWS_PT_ERR_NO_TLS,
    BQWS_PT_ERR_NO_SERVER_SUPPORT,
    BQWS_PT_ERR_OUT_OF_MEMORY,
    BQWS_PT_ERR_BAD_URL,
} bqws_pt_error_code;

typedef struct bqws_pt_error {
    const char *function;
    bqws_pt_error_type type;
    int64_t data;
} bqws_pt_error;

typedef enum bqws_pt_address_type {
    BQWS_PT_ADDRESS_UNKNOWN,
    BQWS_PT_ADDRESS_WEBSOCKET,
    BQWS_PT_ADDRESS_IPV4,
    BQWS_PT_ADDRESS_IPV6,
} bqws_pt_address_type;

typedef struct bqws_pt_address {
    bqws_pt_address_type type;
    uint16_t port;
    uint8_t address[BQWS_PT_MAX_ADDRESS_SIZE];
} bqws_pt_address;

typedef struct bqws_pt_init_opts {

    // CA certificate file location
    // For example: https://curl.haxx.se/docs/caextract.html
    const char *ca_filename;

} bqws_pt_init_opts;

typedef struct bqws_pt_connect_opts {

    // Disable host verification for TLS (secure) connections
    bool insecure_no_verify_host;

} bqws_pt_connect_opts;

typedef struct bqws_pt_listen_opts {

    // Use TLS for incoming connections
    bool secure;

    // TLS certificate, used only if `secure`
    const char *certificate_file; // Passed to `SSL_CTX_use_certificate_file()`
    const char *private_key_file; // Passed to `SSL_CTX_use_PrivateKey_file()`

    // Port to bind to
    // default: 80 if `!secure`, 443 if `secure`
    uint16_t port;

    // Number of connections to queue for `bqws_pt_accept()`
    // default: 128
    size_t backlog;

    // Attempt to share a port with other processes ie. `SO_REUSEPORT`
    bool reuse_port;

    // Allocator callbacks
    bqws_allocator allocator;

} bqws_pt_listen_opts;

// -- Global initialization

// Call these before/after any other functions
bool bqws_pt_init(const bqws_pt_init_opts *opts);
void bqws_pt_shutdown();

// Thread local error
void bqws_pt_clear_error();
bool bqws_pt_get_error(bqws_pt_error *err);

// -- Platform socket creation

// Client

bqws_socket *bqws_pt_connect(const char *url, const bqws_pt_connect_opts *pt_opts, const bqws_opts *opts, const bqws_client_opts *client_opts);
bqws_socket *bqws_pt_connect_url(const bqws_url *url, const bqws_pt_connect_opts *pt_opts, const bqws_opts *opts, const bqws_client_opts *client_opts);

// Server

bqws_pt_server *bqws_pt_listen(const bqws_pt_listen_opts *pt_opts);
void bqws_pt_free_server(bqws_pt_server *sv);

bqws_socket *bqws_pt_accept(bqws_pt_server *sv, const bqws_opts *opts, const bqws_server_opts *server_opts);

// Query

bqws_pt_address bqws_pt_get_address(const bqws_socket *ws);

// -- Utility

void bqws_pt_format_address(char *dst, size_t size, const bqws_pt_address *addr);

void bqws_pt_get_error_desc(char *dst, size_t size, const bqws_pt_error *err);

void bqws_pt_sleep_ms(uint32_t ms);

const char *bqws_pt_error_type_str(bqws_pt_error_type type);
const char *bqws_pt_error_code_str(bqws_pt_error_code err);

#ifdef __cplusplus
}
#endif

#endif // BQ_WEBSOCKET_PLATFORM_H_INCLUDED

// -----------------------------------------------------------------------------

#ifdef BQ_WEBSOCKET_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>

// -- Config

#if defined(_MSC_VER)
    #define bqws_forceinline __forceinline

    #if defined(_M_IX86) || defined(_M_X64)
        #include <intrin.h>
        #include <xmmintrin.h>
        #define BQWS_USE_SSE 1
    #endif

#elif defined(__GNUC__) || defined(__clang__)
    #define bqws_forceinline __attribute__((always_inline)) inline

    #if defined(__i386__) || defined(__x86_64__)
        #include <x86intrin.h>
        #include <xmmintrin.h>
        #define BQWS_USE_SSE 1
    #endif

#else
    #define bqws_forceinline
#endif

#ifndef bqws_assert
#include <assert.h>
#define bqws_assert(x) assert(x)
#endif

#ifndef bqws_malloc
#define bqws_malloc(size) malloc((size))
#endif

#ifndef bqws_realloc
#define bqws_realloc(ptr, old_size, new_size) realloc((ptr), (new_size))
#endif

#ifndef bqws_free
#define bqws_free(ptr, size) free((ptr))
#endif

// TODO: QueryPerformanceCounter() or clock_gettime() might be faster
typedef clock_t bqws_timestamp;

static bqws_timestamp bqws_get_timestamp()
{
    return clock();
}

static size_t bqws_timestamp_delta_to_ms(bqws_timestamp begin, bqws_timestamp end)
{
    return (size_t)((double)(end - begin) * 1000.0 / (double)CLOCKS_PER_SEC);
}

#ifndef BQWS_DEBUG
#if defined(NDEBUG)
    #define BQWS_DEBUG 0
#else
    #define BQWS_DEBUG 1
#endif
#endif

#ifndef BQWS_SINGLE_THREAD
#define BQWS_SINGLE_THREAD 0
#endif

#ifndef bqws_mutex

#if defined(_WIN32) && !BQWS_SINGLE_THREAD

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

typedef struct {
    CRITICAL_SECTION cs;
#if BQWS_DEBUG
    DWORD thread;
#endif
} bqws_mutex;

static void bqws_mutex_init(bqws_mutex *m)
{
    InitializeCriticalSection(&m->cs);
#if BQWS_DEBUG
    m->thread = 0;
#endif
}

static void bqws_mutex_free(bqws_mutex *m)
{
#if BQWS_DEBUG
    m->thread = 0;
#endif
    DeleteCriticalSection(&m->cs);
}

static void bqws_mutex_lock(bqws_mutex *m)
{
    EnterCriticalSection(&m->cs);
#if BQWS_DEBUG
    m->thread = GetCurrentThreadId();
#endif
}

static void bqws_mutex_unlock(bqws_mutex *m)
{
#if BQWS_DEBUG
    m->thread = 0;
#endif
    LeaveCriticalSection(&m->cs);
}

#if BQWS_DEBUG
    #define bqws_assert_locked(m) bqws_assert((m)->thread == GetCurrentThreadId());
#else
    #define bqws_assert_locked(m) (void)0
#endif

#elif (defined(__APPLE__) || defined(__linux__) || defined(__unix__)) && (!defined(__EMSCRIPTEN__) || defined(__EMSCRIPTEN_PTHREADS__)) && !BQWS_SINGLE_THREAD

#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
#if BQWS_DEBUG
    bool locked;
    pthread_t thread;
#endif
} bqws_mutex;

static void bqws_mutex_init(bqws_mutex *m)
{
    pthread_mutex_init(&m->mutex, NULL);
#if BQWS_DEBUG
    m->locked = false;
#endif
}

static void bqws_mutex_free(bqws_mutex *m)
{
#if BQWS_DEBUG
    m->locked = false;
#endif
    pthread_mutex_destroy(&m->mutex);
}

static void bqws_mutex_lock(bqws_mutex *m)
{
    pthread_mutex_lock(&m->mutex);
#if BQWS_DEBUG
    m->locked = true;
    m->thread = pthread_self();
#endif
}

static void bqws_mutex_unlock(bqws_mutex *m)
{
#if BQWS_DEBUG
    m->locked = false;
#endif
    pthread_mutex_unlock(&m->mutex);
}

#if BQWS_DEBUG
    #define bqws_assert_locked(m) ((m)->locked && (m)->thread == pthread_self())
#else
    #define bqws_assert_locked(m) (void)0
#endif

#else

typedef struct {
    bool is_locked;
} bqws_mutex;

static void bqws_mutex_init(bqws_mutex *m)
{
    m->is_locked = false;
}

static void bqws_mutex_free(bqws_mutex *m)
{
    bqws_assert(!m->is_locked);
}

static void bqws_mutex_lock(bqws_mutex *m)
{
    bqws_assert(!m->is_locked);
    m->is_locked = true;
}
static void bqws_mutex_unlock(bqws_mutex *m)
{
    bqws_assert(m->is_locked);
    m->is_locked = false;
}

#define bqws_assert_locked(m) bqws_assert((m)->is_locked)

#endif

#else // not defined bqws_mutex

#ifndef bqws_assert_locked
#define bqws_assert_locked(m) (void)0
#endif

#endif // not defined bqws_mutex

// -- Magic constants

#define BQWS_DELETED_MAGIC   0xbdbdbdbd
#define BQWS_SOCKET_MAGIC    0x7773636b
#define BQWS_MSG_MAGIC       0x776d7367
#define BQWS_FILTER_MAGIC    0x77666c74

// -- Types

// Message implementation struct, message data is always allocated
// to follow the struct in memory.

typedef struct bqws_msg_imp bqws_msg_imp;
struct bqws_msg_imp {
    uint32_t magic; // = BQWS_MSG_MAGIC

    // Socket that is responsible of freeing this message
    // or NULL if it's owned by the user.
    bqws_socket *owner;

    // Allocator used to allocate this message
    bqws_allocator allocator;

    // Linked list in `bqws_msg_queue`
    bqws_msg_imp *prev;

    bqws_msg msg;
};

#define msg_imp(msg) (bqws_msg_imp*)((char*)msg - offsetof(bqws_msg_imp, msg))
#define msg_alloc_size(msg) (sizeof(bqws_msg_imp) + (msg)->capacity)

typedef struct {
    bqws_mutex mutex;
    bqws_msg_imp *first, *last;

    size_t num_messages;
    size_t byte_size;

    uint64_t total_messages;
    uint64_t total_size;
} bqws_msg_queue;

typedef struct {
    bqws_msg_imp *msg;
    size_t offset;
    size_t header_offset;
    size_t header_size;
    bool finished;
    bool masked;
    uint32_t mask_key;
    bqws_msg_type partial_type;
} bqws_msg_buffer;

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
    size_t write_offset;
    size_t read_offset;
} bqws_handshake_buffer;

typedef struct {
    uint32_t magic;

    const char *path;
    const char *host;
    const char *origin;

    const char *protocols[BQWS_MAX_PROTOCOLS];
    size_t num_protocols;

    bqws_verify_fn *verify_fn;
    void *verify_user;

    size_t text_size;
    char text_data[];
} bqws_verify_filter;

// Random entropy source
typedef struct {
    void (*function_pointer)(bqws_socket *ws, const bqws_client_opts *opts);
    void *stack_pointer;
    void *heap_pointer;
    clock_t clock;
    time_t time;
    uint32_t mask_key;
} bqws_random_entropy;

typedef struct {
    uint8_t code_be[2];
    uint8_t magic[4];
    uint8_t error_be[4];
} bqws_err_close_data;

// Main socket/context type, passed everywhere as the first argument.

struct bqws_socket {

    // -- Constant data

    uint32_t magic; // = BQWS_SOCKET_MAGIC
    char *name; // Name high up for debugging
    bool is_server;

    // Copied from `opts`
    bqws_allocator allocator;
    bqws_io user_io;
    bqws_limits limits;
    bool recv_partial_messages;
    bool recv_control_messages;
    bool mask_server;
    bool unsafe_dont_mask_client;
    bqws_verify_fn *verify_fn;
    void *verify_user;
    bqws_message_fn *message_fn;
    void *message_user;
    bqws_peek_fn *peek_fn;
    void *peek_user;
    bqws_log_fn *log_fn;
    void *log_user;
    bool log_send;
    bool log_recv;
    bqws_error_fn *error_fn;
    void *error_user;
    bqws_send_message_fn *send_message_fn;
    void *send_message_user;
    size_t user_size;
    size_t ping_interval;
    size_t connect_timeout;
    size_t close_timeout;
    size_t ping_response_timeout;

    // -- Internally synchronized

    // Current error state, set to the first error that occurs
    // Error writes are protected by `err_mutex` checking `err` can
    // be done without a mutex to check for errors from the same thread.
    bqws_mutex err_mutex;
    bqws_error err;

    // Message queues
    bqws_msg_queue recv_partial_queue;
    bqws_msg_queue recv_queue;
    bqws_msg_queue send_queue;

    // -- State of the socket, errors
    struct {
        bqws_mutex mutex;

        // Connection state
        bqws_state state;
        bqws_state override_state;

        // Pre-allocated error close message storage
        void *pointer_align;
        char error_msg_data[sizeof(bqws_msg_imp) + sizeof(bqws_err_close_data)];
        bqws_close_reason peer_reason;
        bqws_error peer_err;
        bool stop_write;
        bool stop_read;
        bool close_sent;
        bool close_received;
        bool io_started;
        bool io_closed;

        char *chosen_protocol;

        bqws_timestamp start_closing_ts;

        // Priority messages
        bqws_msg_imp *close_to_send;
        bqws_msg_imp *pong_to_send;

    } state;

    // -- Allocation
    struct {
        bqws_mutex mutex;

        // TODO: Make this atomic?
        // Total memory allocated through `allocator` at the moment
        size_t memory_used;
    } alloc;

    // -- IO
    struct {
        bqws_mutex mutex;

        bqws_timestamp start_connect_ts;
        bqws_timestamp last_write_ts;
        bqws_timestamp last_read_ts;
        bqws_timestamp last_ping_ts;
        size_t recv_partial_size;

        // Handshake
        bqws_handshake_buffer handshake;
        bqws_handshake_buffer handshake_overflow;
        bqws_client_opts *opts_from_client;
        char client_key_base64[32];
        bool client_handshake_done;
        bool client_has_protocol;

        // Masking random state
        uint64_t mask_random_state;
        uint64_t mask_random_stream;

        // Write/read buffers `recv_header` is also used to buffer
        // multiple small messages
        char recv_header[512];
        bqws_msg_buffer recv_buf;
        char send_header[16];
        bqws_msg_buffer send_buf;
    } io;

    // -- API
    struct {
        bqws_mutex mutex;

        bqws_msg_imp *next_partial_to_send;
        bqws_msg_type send_partial_type;
    } partial;

    // User data follows in memory
    char user_data[];
};

// -- Utility

// Mark the socket as failed with an error. Only updates the
// error flag if it's not set.

static void null_free(void *user, void *ptr, size_t size) { }

static void ws_log(bqws_socket *ws, const char *str)
{
    if (ws->log_fn) ws->log_fn(ws->log_user, ws, str);
}

static void ws_log2(bqws_socket *ws, const char *a, const char *b)
{
    if (!ws->log_fn) return;

    char line[256];
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    bqws_assert(len_a + len_b < sizeof(line));

    char *ptr = line;
    memcpy(ptr, a, len_a); ptr += len_a;
    memcpy(ptr, b, len_b); ptr += len_b;
    *ptr = '\0';

    ws->log_fn(ws->log_user, ws, line);
}

static void ws_close(bqws_socket *ws)
{
    bqws_assert_locked(&ws->state.mutex);

    if (ws->state.state != BQWS_STATE_CLOSED) {
        ws_log(ws, "State: CLOSED");

        if (ws->user_io.close_fn && !ws->state.io_closed) {
            ws->user_io.close_fn(ws->user_io.user, ws);
        }
        ws->state.io_closed = true;

        ws->state.state = BQWS_STATE_CLOSED;
        ws->state.stop_read = true;
        ws->state.stop_write = true;
    }
}

static void ws_fail(bqws_socket *ws, bqws_error err)
{
    bool should_report = false;

    bqws_mutex_lock(&ws->state.mutex);

    bqws_assert(err != BQWS_OK);

    bqws_mutex_lock(&ws->err_mutex);
    if (!ws->err) {
        should_report = true;

        // vvv Breakpoint here to stop on first error
        ws->err = err;

        bqws_mutex_unlock(&ws->err_mutex);

        ws_log2(ws, "Fail: ", bqws_error_str(err));

        // Try to send an error close message
        if (ws->state.state == BQWS_STATE_OPEN && !ws->state.close_to_send) {
            bqws_msg_imp *close_msg = (bqws_msg_imp*)ws->state.error_msg_data;
            close_msg->magic = BQWS_MSG_MAGIC;
            close_msg->allocator.free_fn = &null_free;
            close_msg->owner = ws;
            close_msg->prev = NULL;
            close_msg->msg.socket = ws;
            close_msg->msg.capacity = sizeof(bqws_err_close_data);
            close_msg->msg.size = sizeof(bqws_err_close_data);
            close_msg->msg.type = BQWS_MSG_CONTROL_CLOSE;

            bqws_close_reason reason;
            switch (err) {
            case BQWS_ERR_LIMIT_MAX_RECV_MSG_SIZE:
                reason = BQWS_CLOSE_MESSAGE_TOO_BIG;
                break;

            case BQWS_ERR_BAD_CONTINUATION:
            case BQWS_ERR_UNFINISHED_PARTIAL:
            case BQWS_ERR_PARTIAL_CONTROL:
            case BQWS_ERR_BAD_OPCODE:
            case BQWS_ERR_RESERVED_BIT:
                reason = BQWS_CLOSE_PROTOCOL_ERROR;
                break;

            default:
                reason = BQWS_CLOSE_SERVER_ERROR;
                break;
            }

            bqws_err_close_data *data = (bqws_err_close_data*)close_msg->msg.data;
            data->code_be[0] = (uint8_t)(reason >> 8);
            data->code_be[1] = (uint8_t)(reason >> 0);
            memcpy(data->magic, "BQWS", 4);
            data->error_be[0] = (uint8_t)(err >> 24);
            data->error_be[1] = (uint8_t)(err >> 16);
            data->error_be[2] = (uint8_t)(err >> 8);
            data->error_be[3] = (uint8_t)(err >> 0);

            ws->state.close_to_send = close_msg;
            ws->state.state = BQWS_STATE_CLOSING;
            ws->state.start_closing_ts = bqws_get_timestamp();

        } else if (ws->state.state == BQWS_STATE_CONNECTING) {

            // If there's an error during connection close
            // the connection immediately
            ws_close(ws);

        }

    } else {
        bqws_mutex_unlock(&ws->err_mutex);
    }

    // IO errors should close their respective channels
    if (err == BQWS_ERR_IO_READ) ws->state.stop_read = true;
    if (err == BQWS_ERR_IO_WRITE) ws->state.stop_write = true;

    bqws_mutex_unlock(&ws->state.mutex);

    if (ws->error_fn && should_report) {
        ws->error_fn(ws->error_user, ws, err);
    }
}

static void bqws_sha1(uint8_t digest[20], const void *data, size_t size);

// Callback writer

typedef struct {
    char *ptr, *end;
} bqws_mem_stream;

static size_t mem_stream_send(void *user, bqws_socket *ws, const void *data, size_t size)
{
    // Copy as many bytes as fit in the stream
    bqws_mem_stream *s = (bqws_mem_stream*)user;
    size_t left = s->end - s->ptr;
    size_t to_copy = size;
    if (to_copy > left) to_copy = left;
    memcpy(s->ptr, data, to_copy);
    s->ptr += to_copy;
    return to_copy;
}

static size_t mem_stream_recv(void *user, bqws_socket *ws, void *data, size_t max_size, size_t min_size)
{
    // Copy as many bytes as fit in the stream
    bqws_mem_stream *s = (bqws_mem_stream*)user;
    size_t left = s->end - s->ptr;
    size_t to_copy = max_size;
    if (to_copy > left) to_copy = left;
    memcpy(data, s->ptr, to_copy);
    s->ptr += to_copy;
    return to_copy;
}

// -- Allocation

// Direct allocator functions. Prefer using `ws_alloc()` if there is an `bqws_socket`
// avaialable (which there should almost always be). These functions just call the
// user callbacks or defaults passing in the user pointer.

void *bqws_allocator_alloc(const bqws_allocator *at, size_t size)
{
    if (at->alloc_fn) {
        // User defined alloc directly
        return at->alloc_fn(at->user, size);
    } else if (at->realloc_fn) {
        // Realloc with zero initial size
        return at->realloc_fn(at->user, NULL, 0, size);
    } else {
        // Default: malloc()
        return bqws_malloc(size);
    }
}

void *bqws_allocator_realloc(const bqws_allocator *at, void *ptr, size_t old_size, size_t new_size)
{
    if (old_size == 0) {
        // Realloc with `old_size==0` is equivalent to malloc
        return bqws_allocator_alloc(at, new_size);
    } else if (new_size == 0) {
        // Realloc with `new_size==0` is equivalent to free
        bqws_allocator_free(at, ptr, old_size);
        return NULL;
    }

    if (at->realloc_fn) {
        // User defined realloc directly
        return at->realloc_fn(at->user, ptr, old_size, new_size);
    } else if (at->alloc_fn) {
        // No realloc, but alloc is defined. Allocate and copy the data
        // if it succeeded and free the old pointer (if free is defined)
        void *new_ptr = at->alloc_fn(at->user, new_size);
        if (!new_ptr) return NULL;
        memcpy(new_ptr, ptr, old_size);
        if (at->free_fn) {
            at->free_fn(at->user, ptr, old_size);
        }
        return new_ptr;
    } else {
        // Default: realloc()
        return bqws_realloc(ptr, old_size, new_size);
    }
}

void bqws_allocator_free(const bqws_allocator *at, void *ptr, size_t size)
{
    if (size == 0) return;
    bqws_assert(ptr != NULL);

    if (at->free_fn) {
        // Use defined free directly
        at->free_fn(at->user, ptr, size);
    } else if (at->realloc_fn) {
        // Use realloc with zero new size
        at->realloc_fn(at->user, ptr, size, 0);
    } else {
        bqws_assert(at->alloc_fn == NULL);

        // Default: free(), only if there is no user defined allocator
        bqws_free(ptr, size);
    }
}

// WebSocket allocation functions. These keep track of total used memory and
// update the error flag.

static bool ws_add_memory_used(bqws_socket *ws, size_t size)
{
    // TODO: Atomics
    bqws_mutex_lock(&ws->alloc.mutex);

    bool ok = (size <= ws->limits.max_memory_used - ws->alloc.memory_used);
    if (ok) {
        ws->alloc.memory_used += size;
    } else {
        ws_fail(ws, BQWS_ERR_LIMIT_MAX_MEMORY_USED);
    }

    bqws_mutex_unlock(&ws->alloc.mutex);
    return ok;
}

static void ws_remove_memory_used(bqws_socket *ws, size_t size)
{
    if (size == 0) return;

    // TODO: Atomics
    bqws_mutex_lock(&ws->alloc.mutex);

    bqws_assert(ws->alloc.memory_used >= size);
    ws->alloc.memory_used -= size;

    bqws_mutex_unlock(&ws->alloc.mutex);
}

static void *ws_alloc(bqws_socket *ws, size_t size)
{
    if (!ws_add_memory_used(ws, size)) return NULL;

    void *ptr = bqws_allocator_alloc(&ws->allocator, size);
    if (!ptr) ws_fail(ws, BQWS_ERR_ALLOCATOR);

    return ptr;
}

static void *ws_realloc(bqws_socket *ws, void *ptr, size_t old_size, size_t new_size)
{
    if (!ws_add_memory_used(ws, new_size)) return NULL;
    ws_remove_memory_used(ws, old_size);

    void *new_ptr = bqws_allocator_realloc(&ws->allocator, ptr, old_size, new_size);
    if (!new_ptr) ws_fail(ws, BQWS_ERR_ALLOCATOR);

    return new_ptr;
}

static void ws_free(bqws_socket *ws, void *ptr, size_t size)
{
    ws_remove_memory_used(ws, size);
    bqws_allocator_free(&ws->allocator, ptr, size);
}

static char *ws_copy_str(bqws_socket *ws, const char *str)
{
    size_t len = strlen(str) + 1;
    char *dst = (char*)ws_alloc(ws, len);
    if (!dst) return NULL;
    memcpy(dst, str, len);
    return dst;
}

static void ws_free_str(bqws_socket *ws, char *ptr)
{
    if (!ptr) return;
    ws_free(ws, ptr, strlen(ptr) + 1);
}

// Message allocation

static bqws_msg_imp *msg_alloc(bqws_socket *ws, bqws_msg_type type, size_t size)
{
    size_t capacity = size;

    // Space for NULL-terminator
    if (type & BQWS_MSG_TEXT) capacity += 1;

    size_t alloc_size = sizeof(bqws_msg_imp) + capacity;
    bqws_msg_imp *msg = (bqws_msg_imp*)ws_alloc(ws, alloc_size);
    if (!msg) return NULL;

    msg->magic = BQWS_MSG_MAGIC;
    msg->owner = ws;
    msg->allocator = ws->allocator;
    msg->prev = NULL;
    msg->msg.socket = ws;
    msg->msg.type = type;
    msg->msg.size = size;
    msg->msg.capacity = capacity;

    if (type & BQWS_MSG_TEXT) {
        msg->msg.data[size] = '\0';
    }

    return msg;
}

static void msg_release_ownership(bqws_socket *ws, bqws_msg_imp *msg)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(msg && msg->magic == BQWS_MSG_MAGIC);
    bqws_assert(msg->owner == ws);

    ws_remove_memory_used(ws, msg_alloc_size(&msg->msg));

    msg->owner = NULL;
}

static bool msg_acquire_ownership(bqws_socket *ws, bqws_msg_imp *msg)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(msg && msg->magic == BQWS_MSG_MAGIC);
    bqws_assert(msg->owner == NULL);

    if (!ws_add_memory_used(ws, msg_alloc_size(&msg->msg))) {
        // We still own the message so need to delete it
        bqws_free_msg(&msg->msg);
        return false;
    }
    msg->owner = ws;

    return true;
}

static void msg_free_owned(bqws_socket *ws, bqws_msg_imp *msg)
{
    if (!msg) return;
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(msg->magic == BQWS_MSG_MAGIC);
    bqws_assert(msg->owner == ws);

    msg->magic = BQWS_DELETED_MAGIC;
    msg->owner = NULL;

    size_t size = msg_alloc_size(&msg->msg);

    // no-mutex(state): We are only referring to the address of `error_msg_data`
    if ((char*)msg != ws->state.error_msg_data) {
        ws_remove_memory_used(ws, size);

        bqws_allocator at = msg->allocator;
        bqws_allocator_free(&at, msg, size);
    }
}

static void msg_enqueue(bqws_msg_queue *mq, bqws_msg_imp *msg)
{
    bqws_mutex_lock(&mq->mutex);

    // Adjust the last message to point to `msg` and replace
    // it as the last in the queue
    bqws_assert(msg && msg->magic == BQWS_MSG_MAGIC && msg->prev == NULL);
    if (mq->last) {
        bqws_assert(mq->first);
        bqws_assert(mq->last->magic == BQWS_MSG_MAGIC && mq->last->prev == NULL);
        mq->last->prev = msg;
    } else {
        bqws_assert(!mq->first);
        mq->first = msg;
    }
    mq->last = msg;

    mq->byte_size += msg->msg.size;
    mq->num_messages++;

    mq->total_size += msg->msg.size;
    mq->total_messages++;

    bqws_mutex_unlock(&mq->mutex);
}

static bqws_msg_imp *msg_dequeue(bqws_msg_queue *mq)
{
    bqws_mutex_lock(&mq->mutex);

    bqws_msg_imp *msg = mq->first;

    if (msg) {
        bqws_assert(mq->last);
        bqws_assert(msg->magic == BQWS_MSG_MAGIC);

        bqws_msg_imp *prev = msg->prev;
        msg->prev = NULL;
        mq->first = prev;
        if (prev) {
            bqws_assert(prev->magic == BQWS_MSG_MAGIC);
        } else {
            bqws_assert(mq->last == msg);
            mq->last = NULL;
        }

        bqws_assert(mq->byte_size >= msg->msg.size);
        bqws_assert(mq->num_messages > 0);
        mq->byte_size -= msg->msg.size;
        mq->num_messages--;

    } else {
        bqws_assert(!mq->last);
    }

    bqws_mutex_unlock(&mq->mutex);

    return msg;
}

static void msg_init_queue(bqws_socket *ws, bqws_msg_queue *mq)
{
    bqws_mutex_init(&mq->mutex);
}

static void msg_free_queue(bqws_socket *ws, bqws_msg_queue *mq)
{
    bqws_msg_imp *imp;
    while ((imp = msg_dequeue(mq)) != 0) {
        msg_free_owned(ws, imp);
    }

    bqws_mutex_free(&mq->mutex);
}

static void msg_queue_add_to_total(bqws_msg_queue *mq, size_t size)
{
    bqws_mutex_lock(&mq->mutex);
    mq->total_messages++;
    mq->total_size += size;
    bqws_mutex_unlock(&mq->mutex);
}

static void msg_queue_get_stats(bqws_msg_queue *mq, bqws_io_stats *stats)
{
    bqws_mutex_lock(&mq->mutex);

    stats->total_bytes = mq->total_size;
    stats->total_messages = mq->total_messages;

    stats->queued_bytes = mq->byte_size;
    stats->queued_messages = mq->num_messages;

    bqws_mutex_unlock(&mq->mutex);
}

// Masking

static uint32_t mask_make_key(bqws_socket *ws)
{
    bqws_assert_locked(&ws->io.mutex);
    // PCG Random step
    const uint64_t c = UINT64_C(6364136223846793005);
    uint64_t s = ws->io.mask_random_state * c + ws->io.mask_random_stream;
    uint32_t xs = (uint32_t)(((s >> 18u) ^ s) >> 27u), r = s >> 59u;
    ws->io.mask_random_state = s;
    uint32_t rng = (xs >> r) | (xs << (((uint32_t)-(int32_t)r) & 31));
    return rng ^ (uint32_t)bqws_get_timestamp();
}

static void mask_apply(void *data, size_t size, uint32_t mask)
{
    size_t left = size;

    // Process SIMD width at a time
    char *data_simd = (char*)data;
    #if defined(BQWS_USE_SSE)
    {
        __m128i sse_mask = _mm_set1_epi32(mask);
        while (left >= 16) {
            __m128i w = _mm_loadu_si128((__m128i*)data_simd);
            w = _mm_xor_si128(w, sse_mask);
            _mm_storeu_si128((__m128i*)data_simd, w);
            data_simd += 16;
            left -= 16;
        }
    }
    #endif

    // Process word at a time
    uint32_t *dst32 = (uint32_t*)data_simd;
    while (left >= 4) {
        *dst32++ ^= mask;
        left -= 4;
    }

    // Mask rest
    if (left > 0) {
        bqws_assert(left < 4);
        uint8_t mask_bytes[4];
        memcpy(mask_bytes, &mask, 4);
        uint8_t *dst8 = (uint8_t*)dst32;
        uint8_t *src = mask_bytes;
        while (left > 0) {
            *dst8++ ^= *src++;
            left--;
        }
    }
}

// -- Handshake

static bqws_forceinline bool str_nonempty(const char *s)
{
    return s && *s;
}

static void hs_push_size(bqws_socket *ws, const char *data, size_t size)
{
    if (ws->err) return;

    bqws_assert_locked(&ws->io.mutex);

    if (size > ws->io.handshake.capacity - ws->io.handshake.size) {
        // Grow the buffer geometrically up to `max_handshake_size`
        size_t new_cap = ws->io.handshake.capacity * 2;
        if (new_cap == 0) new_cap = 512;
        if (new_cap > ws->limits.max_handshake_size) new_cap = ws->limits.max_handshake_size;
        if (new_cap == ws->io.handshake.capacity) {
            ws_fail(ws, BQWS_ERR_LIMIT_MAX_HANDSHAKE_SIZE);
            return;
        }

        char *new_data = (char*)ws_realloc(ws, ws->io.handshake.data, ws->io.handshake.capacity, new_cap);
        if (!new_data) return;
        ws->io.handshake.data = new_data;
        ws->io.handshake.capacity = new_cap;
    }

    memcpy(ws->io.handshake.data + ws->io.handshake.size, data, size);
    ws->io.handshake.size += size;
}

static void hs_push(bqws_socket *ws, const char *a)
{
    hs_push_size(ws, a, strlen(a));
}

static void hs_push2(bqws_socket *ws, const char *a, const char *b)
{
    hs_push(ws, a); hs_push(ws, b);
}

static void hs_push3(bqws_socket *ws, const char *a, const char *b, const char *c)
{
    hs_push(ws, a); hs_push(ws, b); hs_push(ws, c);
}

static const char *base64_tab = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static bool hs_to_base64(void *buf, size_t buf_size, const void *data, size_t size)
{
    bqws_assert(size == 0 || data);
    const uint8_t *b = (const uint8_t*)data;

    char *dst = (char*)buf, *end = dst + buf_size;
    ptrdiff_t left = (ptrdiff_t)size;
    while (left > 0) {
        if (end - dst < 5) return false;

        uint32_t a = (uint32_t)b[0] << 16u
            | (left >= 2 ? (uint32_t)b[1] : 0u) << 8u
            | (left >= 3 ? (uint32_t)b[2] : 0u);
        dst[0] = base64_tab[a >> 18];
        dst[1] = base64_tab[(a >> 12) & 0x3f];
        dst[2] = left >= 2 ? base64_tab[(a >> 6) & 0x3f] : '=';
        dst[3] = left >= 3 ? base64_tab[a & 0x3f] : '=';

        dst += 4;
        b += 3;
        left -= 3;
    }

    *dst = '\0';

    return true;
}

static const char *key_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static void hs_solve_challenge(char dst[32], const char *key_base64)
{
    char challenge[128];
    size_t base64_len = strlen(key_base64);
    size_t guid_len = strlen(key_guid);
    size_t challenge_len = base64_len + guid_len;
    bqws_assert(challenge_len <= sizeof(challenge));
    memcpy(challenge, key_base64, base64_len);
    memcpy(challenge + base64_len, key_guid, guid_len);

    uint8_t digest[20];
    bqws_sha1(digest, challenge, challenge_len);

    bool ret = hs_to_base64(dst, 32, digest, sizeof(digest));
    bqws_assert(ret == true); // 32 bytes should always be enough
}

static void hs_client_handshake(bqws_socket *ws, const bqws_client_opts *opts)
{
    bqws_assert_locked(&ws->io.mutex);

    bqws_assert(!ws->is_server);

    const char *path = str_nonempty(opts->path) ? opts->path : "/";
    hs_push3(ws, "GET ", path, " HTTP/1.1\r\n");

    // Static headers
    hs_push(ws,
        "Connection: Upgrade\r\n"
        "Upgrade: websocket\r\n"
    );

    // User headers
    if (str_nonempty(opts->host)) hs_push3(ws, "Host: ", opts->host, "\r\n");
    if (str_nonempty(opts->origin)) hs_push3(ws, "Origin: ", opts->origin, "\r\n");
    if (opts->num_protocols > 0) {
        hs_push(ws, "Sec-WebSocket-Protocol: ");
        for (size_t i = 0; i < opts->num_protocols; i++) {
            hs_push2(ws, i > 0 ? ", " : "", opts->protocols[i]);
        }
        hs_push(ws, "\r\n");
    }

    // Version (fixed currently, TODO multi-version support)
    hs_push(ws, "Sec-WebSocket-Version: 13\r\n");

    // Random key
    bqws_random_entropy entropy;
    entropy.clock = clock();
    entropy.time = time(NULL);
    entropy.function_pointer = &hs_client_handshake;
    entropy.stack_pointer = &entropy;
    entropy.heap_pointer = ws;
    entropy.mask_key = mask_make_key(ws);

    uint8_t digest[20];
    bqws_sha1(digest, &entropy, sizeof(entropy));
    const uint8_t *key = digest;

    if (opts->use_random_key) {
        key = (const uint8_t*)opts->random_key;
    }

    // We need to retain the key until we have parsed the server handshake
    bool ret = hs_to_base64(ws->io.client_key_base64, sizeof(ws->io.client_key_base64), key, 16);
    bqws_assert(ret == true); // 32 bytes should always be enough
    hs_push3(ws, "Sec-WebSocket-Key: ", ws->io.client_key_base64, "\r\n");

    // Final CRLF
    hs_push(ws, "\r\n");
}


static void hs_server_handshake(bqws_socket *ws)
{
    bqws_assert_locked(&ws->io.mutex);

    bqws_assert(ws->is_server);
    bqws_assert(ws->io.opts_from_client);

    // Fixed header
    hs_push(ws,
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
    );

    // Protocol
    bqws_mutex_lock(&ws->state.mutex);
    const char *protocol = ws->state.chosen_protocol;
    bqws_mutex_unlock(&ws->state.mutex);

    bqws_assert(protocol);
    if (*protocol && ws->io.client_has_protocol) {
        hs_push3(ws, "Sec-WebSocket-Protocol: ", protocol, "\r\n");
    }

    // SHA-1 challenge
    char accept[32];
    hs_solve_challenge(accept, ws->io.client_key_base64);
    hs_push3(ws, "Sec-WebSocket-Accept: ", accept, "\r\n");

    // Final CRLF
    hs_push(ws, "\r\n");

    // Free the handshake state
    ws_free(ws, ws->io.opts_from_client, sizeof(bqws_client_opts));
    ws->io.opts_from_client = NULL;
}

// -- Handshake parsing

static bool hs_parse_literal(bqws_socket *ws, size_t *pos, const char *str)
{
    bqws_assert_locked(&ws->io.mutex);

    size_t len = strlen(str);
    if (ws->io.handshake.size - *pos < len) return false;
    const char *ref = ws->io.handshake.data + *pos;
    if (memcmp(ref, str, len) != 0) return false;
    *pos += len;
    return true;
}

static char *hs_parse_token(bqws_socket *ws, size_t *pos, char end)
{
    bqws_assert_locked(&ws->io.mutex);

    size_t begin = *pos, p = begin;
    while (p != ws->io.handshake.size) {
        char c = ws->io.handshake.data[p];
        if (c == end) {
            ws->io.handshake.data[p] = '\0';
            *pos = p + 1;
            return ws->io.handshake.data + begin;
        }
        if (c == '\r' || c == '\n') return NULL;
        p++;
    }
    return NULL;
}

static void hs_skip_space(bqws_socket *ws, size_t *pos)
{
    bqws_assert_locked(&ws->io.mutex);

    while (*pos < ws->io.handshake.size) {
        char c = ws->io.handshake.data[*pos];
        if (c != ' ' && c != '\t') break;
        ++*pos;
    }
}

// Case-insensitive (ASCII) string compare
static bool streq_ic(const char *sa, const char *sb)
{
    for (;;) {
        char a = *sa++, b = *sb++;
        if ((unsigned)(unsigned char)a < 0x80u) a = (char)tolower(a);
        if ((unsigned)(unsigned char)b < 0x80u) b = (char)tolower(b);
        if (a != b) return false;
        if (a == 0) return true;
    }
}

static bool hs_parse_client_handshake(bqws_socket *ws)
{
    bqws_assert_locked(&ws->io.mutex);

    bqws_assert(ws->is_server);
    bqws_assert(!ws->io.opts_from_client);

    size_t pos = 0;

    bqws_client_opts *opts = (bqws_client_opts*)ws_alloc(ws, sizeof(bqws_client_opts));
    if (!opts) return false;
    memset(opts, 0, sizeof(bqws_client_opts));
    ws->io.opts_from_client = opts;

    // GET /path HTTP/1.1
    if (!hs_parse_literal(ws, &pos, "GET")) return false;
    hs_skip_space(ws, &pos);
    opts->path = hs_parse_token(ws, &pos, ' ');
    if (!opts->path) return false;
    hs_skip_space(ws, &pos);
    if (!hs_parse_literal(ws, &pos, "HTTP/1.1\r\n")) return false;

    // Headers
    while (!hs_parse_literal(ws, &pos, "\r\n")) {
        if (opts->num_headers >= BQWS_MAX_HEADERS) {
            ws_fail(ws, BQWS_ERR_TOO_MANY_HEADERS);
            return false;
        }

        bqws_header *header = &opts->headers[opts->num_headers];
        header->name = hs_parse_token(ws, &pos, ':');
        hs_skip_space(ws, &pos);

        size_t value_pos = pos;
        header->value = hs_parse_token(ws, &pos, '\r');
        if (!header->name || !header->value) return false;
        if (!hs_parse_literal(ws, &pos, "\n")) return false;

        if (streq_ic(header->name, "Host")) {
            opts->host = header->value;
            opts->num_headers++;
        } else if (streq_ic(header->name, "Origin")) {
            opts->origin = header->value;
            opts->num_headers++;
        } else if (streq_ic(header->name, "Sec-Websocket-Protocol")) {
            size_t cur_pos = pos;
            ws->io.client_has_protocol = true;

            // Parse protocols
            pos = value_pos;
            while (pos < cur_pos) {
                // Either token ',' or final token that is zero-terminated
                // already since it's the last thing in `header->value`.
                char *protocol = hs_parse_token(ws, &pos, ',');
                hs_skip_space(ws, &pos);
                if (!protocol) {
                    protocol = ws->io.handshake.data + pos;
                    pos = cur_pos;
                }

                if (opts->num_protocols >= BQWS_MAX_PROTOCOLS) {
                    ws_fail(ws, BQWS_ERR_TOO_MANY_PROTOCOLS);
                    return false;
                }
                opts->protocols[opts->num_protocols++] = protocol;
            }

            pos = cur_pos;
        } else if (streq_ic(header->name, "Sec-Websocket-Key")) {
            size_t len = strlen(header->value) + 1;
            if (len > sizeof(ws->io.client_key_base64)) {
                ws_fail(ws, BQWS_ERR_HEADER_KEY_TOO_LONG);
                return false;
            }
            memcpy(ws->io.client_key_base64, header->value, len);
        } else if (streq_ic(header->name, "Sec-Websocket-Version")) {
            // TODO: Version negotiatoin
            if (strcmp(header->value, "13") != 0) {
                ws_fail(ws, BQWS_ERR_UNSUPPORTED_VERSION);
                return false;
            }
        } else {
            opts->num_headers++;
        }
    }

    // Store the end of the parsed header in case we read past the
    // header in the beginning.
    ws->io.handshake.read_offset = pos;

    if (!opts->host) opts->host = "";
    if (!opts->origin) opts->origin = "";

    return true;
}

static bool hs_parse_server_handshake(bqws_socket *ws)
{
    bqws_assert_locked(&ws->io.mutex);

    bqws_assert(!ws->is_server);

    size_t pos = 0;

    // HTTP/1.1 101 Switching Protocols
    if (!hs_parse_literal(ws, &pos, "HTTP/1.1 101")) return false;
    hs_parse_token(ws, &pos, '\r'); // Skip description
    if (!hs_parse_literal(ws, &pos, "\n")) return false;

    // Headers
    while (!hs_parse_literal(ws, &pos, "\r\n")) {
        // TODO: Keep headers?

        bqws_header header;
        header.name = hs_parse_token(ws, &pos, ':');
        hs_skip_space(ws, &pos);

        header.value = hs_parse_token(ws, &pos, '\r');
        if (!header.name || !header.value) return false;
        if (!hs_parse_literal(ws, &pos, "\n")) return false;

        if (streq_ic(header.name, "Sec-Websocket-Accept")) {

            // Check the SHA of the challenge
            char reference[32];
            hs_solve_challenge(reference, ws->io.client_key_base64);
            if (strcmp(header.value, reference) != 0) {
                ws_fail(ws, BQWS_ERR_HEADER_BAD_ACCEPT);
                return false;
            }

        } else if (streq_ic(header.name, "Sec-Websocket-Protocol")) {
            // Protocol that the server chose

            // Keep the first one if there's duplicates
            bqws_mutex_lock(&ws->state.mutex);
            if (!ws->state.chosen_protocol) {
                char *copy = ws_copy_str(ws, header.value);

                if (!ws->state.chosen_protocol) {
                    ws->state.chosen_protocol = copy;
                } else {
                    ws_free_str(ws, copy);
                }
            }
            bqws_mutex_unlock(&ws->state.mutex);
        }
    }

    // Store the end of the parsed header in case we read past the
    // header in the beginning.
    ws->io.handshake.read_offset = pos;

    // If the server didn't choose any protocol set it as ""
    bqws_mutex_lock(&ws->state.mutex);
    if (!ws->state.chosen_protocol) {
        char *copy = ws_copy_str(ws, "");

        if (!ws->state.chosen_protocol) {
            ws->state.chosen_protocol = copy;
        } else {
            ws_free_str(ws, copy);
        }
    }
    bqws_mutex_unlock(&ws->state.mutex);

    return true;
}

static void hs_finish_handshake(bqws_socket *ws)
{
    bqws_assert_locked(&ws->io.mutex);

    if (ws->err) return;

    ws_log(ws, "State: OPEN");

    bqws_mutex_lock(&ws->state.mutex);
    ws->state.state = BQWS_STATE_OPEN;
    bqws_mutex_unlock(&ws->state.mutex);

    // Free the handshake buffer
    ws_free(ws, ws->io.handshake.data, ws->io.handshake.capacity);
    ws->io.handshake.data = NULL;
    ws->io.handshake.size = 0;
    ws->io.handshake.capacity = 0;

    // Notify IO that the connection is open
    if (ws->user_io.notify_fn) {
        ws->user_io.notify_fn(ws->user_io.user, ws);
    }
}

static void hs_store_handshake_overflow(bqws_socket *ws)
{
    bqws_assert_locked(&ws->io.mutex);

    size_t offset = ws->io.handshake.read_offset;
    size_t left = ws->io.handshake.size - offset;
    if (left == 0) return;

    ws->io.handshake_overflow.data = (char*)ws_alloc(ws, left);
    if (!ws->io.handshake_overflow.data) return;
    memcpy(ws->io.handshake_overflow.data, ws->io.handshake.data + offset, left);
    ws->io.handshake_overflow.capacity = left;
    ws->io.handshake_overflow.size = left;
}

// Control messages

static void ws_enqueue_send(bqws_socket *ws, bqws_msg_imp *msg)
{
    msg_enqueue(&ws->send_queue, msg);

    if (ws->user_io.notify_fn) {
        ws->user_io.notify_fn(ws->user_io.user, ws);
    }
}

static void ws_enqueue_recv(bqws_socket *ws, bqws_msg_imp *msg)
{
    // If the user callback returns true the message won't be
    // enqueued to the receive queue.
    if (ws->message_fn) {
        msg_release_ownership(ws, msg);
        if (ws->message_fn(ws->message_user, ws, &msg->msg)) {
            // Message was consumed and won't be processed so add
            // it to the total count
            msg_queue_add_to_total(&ws->recv_queue, msg->msg.size);
        }
        if (!msg_acquire_ownership(ws, msg)) return;
    }

    msg_enqueue(&ws->recv_queue, msg);
}

static void ws_handle_control(bqws_socket *ws, bqws_msg_imp *msg)
{
    bqws_msg_type type = msg->msg.type;
    bqws_msg_imp *msg_to_enqueue = msg;

    if (type == BQWS_MSG_CONTROL_CLOSE) {

        bqws_mutex_lock(&ws->state.mutex);

        // Set peer close reason from the message
        if (msg->msg.size >= 2) {
            ws->state.peer_reason = (bqws_close_reason)(
                ((uint32_t)(uint8_t)msg->msg.data[0] << 8) |
                ((uint32_t)(uint8_t)msg->msg.data[1] << 0) );
        } else {
            ws->state.peer_reason = BQWS_CLOSE_NO_REASON;
        }

        // Set unknown error if the connection was closed with an error
        if (ws->state.peer_reason != BQWS_CLOSE_NORMAL && ws->state.peer_reason != BQWS_CLOSE_GOING_AWAY) {
            ws->state.peer_err = BQWS_ERR_UNKNOWN;
        }

        // Potentially patch bqws-specific info
        if (msg->msg.size == sizeof(bqws_err_close_data)) {
            bqws_err_close_data *data = (bqws_err_close_data*)msg->msg.data;
            if (!memcmp(data->magic, "BQWS", 4)) {
                ws->state.peer_err = (bqws_error)(
                    ((uint32_t)(uint8_t)data->error_be[0] << 24) |
                    ((uint32_t)(uint8_t)data->error_be[1] << 16) |
                    ((uint32_t)(uint8_t)data->error_be[2] <<  8) |
                    ((uint32_t)(uint8_t)data->error_be[3] <<  0) );
            }
        }

        // Echo the close message back
        if (!ws->state.close_to_send) {
            ws->state.close_to_send = msg;

            // Don't free the message as it will be re-sent
            msg = NULL;
        }

        // Peer has closed connection so we go directly to CLOSED
        if (ws->state.state == BQWS_STATE_OPEN) {
            ws_log(ws, "State: CLOSING (received Close from peer)");
            ws->state.start_closing_ts = bqws_get_timestamp();
            ws->state.state = BQWS_STATE_CLOSING;
        }

        ws->state.stop_read = true;
        ws->state.close_received = true;
        if (ws->state.close_sent) {
            ws_close(ws);
        }

        bqws_mutex_unlock(&ws->state.mutex);

    } else if (type == BQWS_MSG_CONTROL_PING) {
        if (ws->recv_control_messages) {
            // We want to re-use the PING message to send it back
            // so we need to copy it for receiving
            bqws_msg_imp *copy = msg_alloc(ws, type, msg->msg.size);
            if (!copy) return;
            memcpy(copy->msg.data, msg->msg.data, msg->msg.size);
            msg_to_enqueue = copy;
        }

        // Turn the PING message into a PONG
        msg->msg.type = BQWS_MSG_CONTROL_PONG;

        bqws_mutex_lock(&ws->state.mutex);

        // Only retain the latest PONG to send back
        if (ws->state.pong_to_send) {
            msg_free_owned(ws, ws->state.pong_to_send);
        }
        ws->state.pong_to_send = msg;

        bqws_mutex_unlock(&ws->state.mutex);

        // Don't free the message as it will be re-sent
        msg = NULL;

    } else if (type == BQWS_MSG_CONTROL_PONG) {
        // PONG messages don't require any kind of handling
    } else {
        bqws_assert(0 && "Unexpected control message");
    }

    // Receive control messages
    if (ws->recv_control_messages) {
        ws_enqueue_recv(ws, msg_to_enqueue);
    } else if (msg) {
        msg_free_owned(ws, msg);
    }
}

// Input / output

// Read data into a buffer, returns amount of bytes used read.
// Returns 0 and sets `ws->err` if parsing fails.

static size_t ws_recv_from_handshake_overflow(void *user, bqws_socket *ws, void *data, size_t max_size, size_t min_size)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert_locked(&ws->io.mutex);

    size_t offset = ws->io.handshake_overflow.read_offset;
    size_t left = ws->io.handshake_overflow.size - offset;
    size_t to_copy = max_size;
    if (to_copy > left) to_copy = left;
    memcpy(data, ws->io.handshake_overflow.data + offset, to_copy);
    ws->io.handshake_overflow.read_offset += to_copy;
    return to_copy;
}

static bool ws_read_handshake(bqws_socket *ws, bqws_io_recv_fn recv_fn, void *user)
{
    bqws_assert_locked(&ws->io.mutex);

    for (;;) {
        if (ws->io.handshake.size == ws->io.handshake.capacity) {
            // Grow the buffer geometrically up to `max_handshake_size`
            size_t new_cap = ws->io.handshake.capacity * 2;
            if (new_cap == 0) new_cap = 512;
            if (new_cap > ws->limits.max_handshake_size) new_cap = ws->limits.max_handshake_size;
            if (new_cap == ws->io.handshake.capacity) {
                ws_fail(ws, BQWS_ERR_LIMIT_MAX_HANDSHAKE_SIZE);
                return false;
            }

            char *data = (char*)ws_realloc(ws, ws->io.handshake.data, ws->io.handshake.capacity, new_cap);
            if (!data) return false;
            ws->io.handshake.data = data;
            ws->io.handshake.capacity = new_cap;
        }

        // TODO: min_size can be up to 4 depending on the suffix of the buffer

        // Read some data
        size_t to_read = ws->io.handshake.capacity - ws->io.handshake.size;
        size_t num_read = recv_fn(user, ws, ws->io.handshake.data + ws->io.handshake.size, to_read, 1);
        if (num_read == 0) return false;
        if (num_read == SIZE_MAX) {
            ws_fail(ws, BQWS_ERR_IO_READ);
            return false;
        }
        bqws_assert(num_read <= to_read);
        ws->io.handshake.size += num_read;

        // Scan for \r\n\r\n
        ptrdiff_t begin = (ptrdiff_t)ws->io.handshake.size - num_read - 4;
        if (begin < 0) begin = 0;
        char *ptr = ws->io.handshake.data + begin;
        char *end = ws->io.handshake.data + ws->io.handshake.size;
        while ((ptr = (char*)memchr(ptr, '\r', end - ptr)) != NULL) {
            if (end - ptr >= 4 && !memcmp(ptr, "\r\n\r\n", 4)) {
                return true;
            } else {
                ptr++;
            }
        }

        if (num_read != to_read) break;
    }

    return false;
}

static bool ws_read_data(bqws_socket *ws, bqws_io_recv_fn recv_fn, void *user)
{
    bqws_assert_locked(&ws->io.mutex);
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    bqws_msg_buffer *buf = &ws->io.recv_buf;

    bqws_state state;

    bqws_mutex_lock(&ws->state.mutex);
    if (ws->state.stop_read) {
        bqws_mutex_unlock(&ws->state.mutex);
        return false;
    }
    state = ws->state.state;
    bqws_mutex_unlock(&ws->state.mutex);

    if (state == BQWS_STATE_CONNECTING) {

        if (ws->is_server) {
            // Server: read the client handshake first, after it's done wait for
            // `ws_write_data()` to set `ws->state == BQWS_STATE_OPEN`
            if (!ws->io.client_handshake_done) {
                // Read the client handshake
                if (ws_read_handshake(ws, recv_fn, user)) {
                    if (!hs_parse_client_handshake(ws)) {
                        ws_fail(ws, BQWS_ERR_HEADER_PARSE);
                        return false;
                    }
                    ws->io.client_handshake_done = true;

                    // Re-use the handshake buffer for the response, but copy
                    // remaining data to be read later
                    hs_store_handshake_overflow(ws);
                    ws->io.handshake.size = 0;

                    // Notify IO that there is a handshake to send
                    if (ws->user_io.notify_fn) {
                        ws->user_io.notify_fn(ws->user_io.user, ws);
                    }
                }
            }

            // Wait that the response is sent
            return false;
        } else {
            // Client: Send the request first before trying to read the response
            if (!ws->io.client_handshake_done) return false;
            if (!ws_read_handshake(ws, recv_fn, user)) return false;

            if (!hs_parse_server_handshake(ws)) {
                ws_fail(ws, BQWS_ERR_HEADER_PARSE);
                return false;
            }

            // Store remaining data before deleting the handshake
            hs_store_handshake_overflow(ws);

            // Client handshake is done!
            hs_finish_handshake(ws);
        }
    }

    // If there's still data in the handshake buffer empty it before
    // reading any new data
    if (ws->io.handshake_overflow.data && recv_fn != &ws_recv_from_handshake_overflow) {

        // Read from the handshake until we reach the end
        while (!ws->err && ws->io.handshake_overflow.read_offset < ws->io.handshake_overflow.size) {
            if (!ws_read_data(ws, &ws_recv_from_handshake_overflow, NULL)) {
                return false;
            }
        }

        if (ws->err) return false;

        // Free the handshake
        ws_free(ws, ws->io.handshake_overflow.data, ws->io.handshake_overflow.capacity);
        ws->io.handshake_overflow.data = NULL;
        ws->io.handshake_overflow.size = 0;
        ws->io.handshake_overflow.capacity = 0;

        // Continue with reading from the actual data source
    }

    // Header has not been parsed yet
    if (!buf->msg) {

        // Check if we can fit a new message to the receive queue
        if (ws->recv_queue.num_messages >= ws->limits.max_recv_queue_messages
            || ws->recv_queue.byte_size >= ws->limits.max_recv_queue_size) {
            return false;
        }

        // We need to read at least two bytes to determine
        // the header size
        if (buf->header_size == 0) {
            if (buf->header_offset < 2) {
                size_t to_read = sizeof(ws->io.recv_header) - buf->header_offset;
                size_t min_read = 2 - buf->header_offset;
                size_t num_read = recv_fn(user, ws, ws->io.recv_header + buf->header_offset, to_read, min_read);
                if (num_read == 0) return false;
                if (num_read == SIZE_MAX) {
                    ws_fail(ws, BQWS_ERR_IO_READ);
                    return false;
                }
                bqws_assert(num_read <= to_read);
                buf->header_offset += num_read;

                if (ws->ping_interval != SIZE_MAX) {
                    ws->io.last_read_ts = bqws_get_timestamp();
                }
            }

            if (buf->header_offset < 2) return false;

            uint8_t mask_len = ws->io.recv_header[1];
            uint32_t len = mask_len & 0x7f;

            // Minimum header size
            size_t header_size = 2;

            // MASK bit set: contains 32-bit mask field
            if (mask_len & 0x80) header_size += 4;

            // 16/64-bit message length
            if (len == 126) header_size += 2;
            else if (len == 127) header_size += 8;

            buf->header_size = header_size;
            bqws_assert(buf->header_size <= sizeof(ws->io.recv_header));
        }

        // Read more header data if we need it
        if (buf->header_offset < buf->header_size) {
            size_t to_read = sizeof(ws->io.recv_header) - buf->header_offset;
            size_t min_read = buf->header_size - buf->header_offset;
            size_t num_read = recv_fn(user, ws, ws->io.recv_header + buf->header_offset, to_read, min_read);
            if (num_read == 0) return false;
            if (num_read == SIZE_MAX) {
                ws_fail(ws, BQWS_ERR_IO_READ);
                return false;
            }
            bqws_assert(num_read <= to_read);
            buf->header_offset += num_read;

            if (ws->ping_interval != SIZE_MAX) {
                ws->io.last_read_ts = bqws_get_timestamp();
            }

            return false;
        }

        if (buf->header_offset < buf->header_size) return false;

        // Parse the header and allocate the message
        const uint8_t *h = (const uint8_t*)ws->io.recv_header;

        // Static header bits
        bool fin = (h[0] & 0x80) != 0;
        if (h[0] & 0x70) {
            // Reserved bits RSV1-3
            ws_fail(ws, BQWS_ERR_RESERVED_BIT);
            return false;
        }
        uint32_t opcode = (uint32_t)(h[0] & 0x0f);
        uint32_t mask = (uint32_t)(h[1] & 0x80) != 0;
        uint64_t payload_length = (uint64_t)(h[1] & 0x7f);
        h += 2;

        // Extended length: Read 2 or 8 bytes of big
        // endian payload length.
        size_t payload_ext = 0;
        if (payload_length == 126) {
            payload_ext = 2;
            payload_length = 0;
        } else if (payload_length == 127) {
            payload_ext = 8;
            payload_length = 0;
        }
        for (size_t i = 0; i < payload_ext; i++) {
            size_t shift = (payload_ext - i - 1) * 8;
            payload_length |= (uint64_t)h[i] << shift;
        }
        h += payload_ext;

        // Check the payload length and cast to `size_t`
        if (payload_length > (uint64_t)ws->limits.max_recv_msg_size) {
            ws_fail(ws, BQWS_ERR_LIMIT_MAX_RECV_MSG_SIZE);
            return false;
        }
        size_t msg_size = (size_t)payload_length;

        // Masking key
        buf->masked = mask;
        if (mask) {
            memcpy(&buf->mask_key, h, 4);
            h += 4;
        }

        bqws_assert((size_t)((const char*)h - ws->io.recv_header) == buf->header_size);

        bqws_msg_type type = BQWS_MSG_INVALID;

        // Resolve the type of the message
        if (opcode == 0x0) {
            // Continuation frame

            if (buf->partial_type == BQWS_MSG_INVALID) {
                // Continuation frame without a prior partial frame
                ws_fail(ws, BQWS_ERR_BAD_CONTINUATION);
                return false;
            }

            type = (bqws_msg_type)(buf->partial_type | BQWS_MSG_PARTIAL_BIT);
            if (fin) {
                type = (bqws_msg_type)(type | BQWS_MSG_FINAL_BIT);
                buf->partial_type = BQWS_MSG_INVALID;
            }

        } else if (opcode == 0x1 || opcode == 0x2) {
            // Text or Binary
            type = opcode == 0x1 ? BQWS_MSG_TEXT : BQWS_MSG_BINARY;
            if (!fin) {
                if (buf->partial_type != BQWS_MSG_INVALID) {
                    // New partial message even though one is already
                    // being sent
                    ws_fail(ws, BQWS_ERR_UNFINISHED_PARTIAL);
                    return false;
                }

                buf->partial_type = type;
                type = (bqws_msg_type)(type | BQWS_MSG_PARTIAL_BIT);
            }
        } else if (opcode >= 0x8 && opcode <= 0xa) {
            // Control frames
            if      (opcode == 0x8) type = BQWS_MSG_CONTROL_CLOSE;
            else if (opcode == 0x9) type = BQWS_MSG_CONTROL_PING;
            else if (opcode == 0xa) type = BQWS_MSG_CONTROL_PONG;

            if (!fin) {
                // Control frames may not be fragmented
                ws_fail(ws, BQWS_ERR_PARTIAL_CONTROL);
                return false;
            }
        } else {
            // Unsupported opcode
            ws_fail(ws, BQWS_ERR_BAD_OPCODE);
            return false;
        }
        bqws_assert(type != BQWS_MSG_INVALID);

        // All good, allocate the message
        bqws_msg_imp *imp = msg_alloc(ws, type, msg_size);
        if (!imp) return false;

        buf->msg = imp;
        buf->offset = 0;

        // Copy rest of the header bytes to the message
        size_t offset = buf->header_size;
        size_t left = buf->header_offset - offset;
        if (left > 0) {
            size_t to_copy = left;
            if (to_copy > imp->msg.size) to_copy = imp->msg.size;
            memcpy(imp->msg.data, ws->io.recv_header + offset, to_copy);
            buf->offset += to_copy;
            offset += to_copy;
            left -= to_copy;
        }

        // If there's still some data shift it as the next header
        if (left > 0) {
            memmove(ws->io.recv_header, ws->io.recv_header + offset, left);
        }
        buf->header_offset = left;
    }

    bqws_msg_imp *msg = buf->msg;

    // Read message data if the message is not empty
    bqws_assert(buf->offset <= msg->msg.size);
    if (msg->msg.size > 0 && buf->offset < msg->msg.size) {

        size_t to_read = msg->msg.size - buf->offset;
        size_t num_read = recv_fn(user, ws, msg->msg.data + buf->offset, to_read, to_read);
        if (num_read == 0) return false;
        if (num_read == SIZE_MAX) {
            ws_fail(ws, BQWS_ERR_IO_READ);
            return false;
        }
        bqws_assert(num_read <= to_read);

        if (ws->ping_interval != SIZE_MAX) {
            ws->io.last_read_ts = bqws_get_timestamp();
        }

        buf->offset += num_read;
        if (num_read < to_read) return false;
    }

    if (buf->masked) {
        mask_apply(msg->msg.data, msg->msg.size, buf->mask_key);
    }

    bqws_assert(buf->offset == msg->msg.size);

    // Peek at all incoming messages before processing
    if (ws->peek_fn) {
        ws->peek_fn(ws->peek_user, ws, &msg->msg, true);
    }

    // If we copied the last bytes of the message we can push it
    // to the queue and clear the buffer.
    bqws_msg_type type = msg->msg.type;

    if (ws->log_recv) {
        ws_log2(ws, "Received: ", bqws_msg_type_str(buf->msg->msg.type));
    }

    if ((type & BQWS_MSG_PARTIAL_BIT) != 0 && !ws->recv_partial_messages) {

        // Only allow partial messages that combine up to the maximum message size
        bqws_assert(msg->msg.size <= ws->limits.max_recv_msg_size);
        if (ws->io.recv_partial_size >= ws->limits.max_recv_msg_size - msg->msg.size) {
            ws_fail(ws, BQWS_ERR_LIMIT_MAX_RECV_MSG_SIZE);
            return false;
        }

        ws->io.recv_partial_size += msg->msg.size;

        // If we dont expose partial messages collect them to `recv_partial_queue`.
        if (type & BQWS_MSG_FINAL_BIT) {
            // If this is the final message concatenate all the partial messages
            // in the queue and enqueue the final one>

            bqws_msg_type base_type = (bqws_msg_type)(msg->msg.type & BQWS_MSG_TYPE_MASK);
            bqws_msg_imp *combined = msg_alloc(ws, base_type, ws->io.recv_partial_size);
            if (!combined) return false;

            size_t offset = 0;

            // `recv_queue` with this message as the last part.
            bqws_msg_imp *part;
            while ((part = msg_dequeue(&ws->recv_partial_queue)) != NULL) {
                bqws_assert(part->magic == BQWS_MSG_MAGIC);
                bqws_assert((part->msg.type & BQWS_MSG_TYPE_MASK) == base_type);

                memcpy(combined->msg.data + offset, part->msg.data, part->msg.size);
                offset += part->msg.size;

                // Delete the part
                msg_free_owned(ws, part);
            }

            // Final part
            memcpy(combined->msg.data + offset, msg->msg.data, msg->msg.size);
            offset += msg->msg.size;
            msg_free_owned(ws, msg);

            bqws_assert(offset == combined->msg.size);

            ws_enqueue_recv(ws, combined);

            // Clear the partial total size
            ws->io.recv_partial_size = 0;
        } else {

            if (ws->recv_partial_queue.num_messages >= ws->limits.max_partial_message_parts) {
                ws_fail(ws, BQWS_ERR_LIMIT_MAX_PARTIAL_MESSAGE_PARTS);
                return false;
            }

            msg_enqueue(&ws->recv_partial_queue, msg);
        }

    } else {
        if (type & BQWS_MSG_CONTROL_MASK) {
            // Control message, handle it. `ws_handle_control()` enqueues the
            // message to `recv_queue` internally if required.
            ws_handle_control(ws, msg);
        } else {
            // Non-partial data message
            ws_enqueue_recv(ws, msg);
        }
    }
    buf->offset = 0;
    buf->header_size = 0;
    buf->msg = NULL;

    return true;
}

static bool ws_write_handshake(bqws_socket *ws, bqws_io_send_fn *send_fn, void *user)
{
    bqws_assert_locked(&ws->io.mutex);

    size_t to_send = ws->io.handshake.size - ws->io.handshake.write_offset;
    size_t sent = send_fn(user, ws, ws->io.handshake.data + ws->io.handshake.write_offset, to_send);
    if (sent == SIZE_MAX) {
        ws_fail(ws, BQWS_ERR_IO_WRITE);
        return false;
    }

    bqws_assert(sent <= to_send);
    ws->io.handshake.write_offset += sent;
    return sent == to_send;
}

static bool ws_write_data(bqws_socket *ws, bqws_io_send_fn *send_fn, void *user)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert_locked(&ws->io.mutex);

    bqws_msg_buffer *buf = &ws->io.send_buf;

    bqws_state state;
    char *protocol;

    bqws_mutex_lock(&ws->state.mutex);
    if (ws->state.stop_write) {
        bqws_mutex_unlock(&ws->state.mutex);
        return false;
    }
    state = ws->state.state;
    protocol = ws->state.chosen_protocol;
    bqws_mutex_unlock(&ws->state.mutex);

    if (state == BQWS_STATE_CONNECTING) {

        if (ws->is_server) {
            // Server: read the client handshake first
            if (!ws->io.client_handshake_done) return false;

            // Wait for the user to accept/reject the connection
            if (!protocol) return false;

            // Write the server handshake on demand
            if (ws->io.handshake.size == 0) {
                hs_server_handshake(ws);
                if (ws->err) return false;
            }

            // Write the server handshake
            if (!ws_write_handshake(ws, send_fn, user)) return false;

            // Server handshake is done!
            hs_finish_handshake(ws);

        } else {
            // Client: Send the request and always wait for response
            if (!ws->io.client_handshake_done) {
                if (!ws_write_handshake(ws, send_fn, user)) return false;

                // Re-use the handshake buffer for the response,
                ws->io.handshake.size = 0;
                ws->io.client_handshake_done = true;
            }

            return false;
        }
    }

    if (!buf->msg) {
        // No message: Send high priority messages first.

        bqws_mutex_lock(&ws->state.mutex);

        if (ws->state.close_to_send && !ws->state.close_sent) {
            // First priority: Send close message
            buf->msg = ws->state.close_to_send;
            ws->state.close_to_send = NULL;
            bqws_assert(buf->msg->msg.type == BQWS_MSG_CONTROL_CLOSE);
        } else if (ws->state.state != BQWS_STATE_OPEN) {
            // Stop sending anything if the state is not open
        } else if (ws->state.pong_to_send) {
            // Try to respond to PING messages fast
            buf->msg = ws->state.pong_to_send;
            ws->state.pong_to_send = NULL;
            bqws_assert(buf->msg->msg.type == BQWS_MSG_CONTROL_PONG);
        } else {
            // Send user message if there is one
            buf->msg = msg_dequeue(&ws->send_queue);
        }

        bqws_mutex_unlock(&ws->state.mutex);

        // Did not find any message
        if (!buf->msg) return false;

        bqws_assert(buf->msg && buf->msg->magic == BQWS_MSG_MAGIC);

    }
    bqws_msg_imp *msg = buf->msg;
    bqws_assert(msg && msg->magic == BQWS_MSG_MAGIC);

    // Re-assign the public socket to be this one for the callback
    msg->msg.socket = ws;

    // Peek at all outgoing messages before processing
    if (ws->peek_fn) {
        ws->peek_fn(ws->peek_user, ws, &msg->msg, false);
    }

    if (ws->send_message_fn) {
        msg_release_ownership(ws, msg);
        if (ws->send_message_fn(ws->send_message_user, ws, &msg->msg)) {
            if (ws->log_send) {
                ws_log2(ws, "Direct send: ", bqws_msg_type_str(msg->msg.type));
            }
            buf->msg = NULL;
            return true;
        } else {
            msg_acquire_ownership(ws, msg);
            return false;
        }
    }

    if (ws->ping_interval != SIZE_MAX) {
        ws->io.last_write_ts = bqws_get_timestamp();
    }

    if (buf->header_size == 0) {
        bqws_msg_type type = msg->msg.type;
        bool mask = ws->is_server ? ws->mask_server : !ws->unsafe_dont_mask_client;
        bool fin = true;
        uint32_t opcode = ~0u;

        if (type & BQWS_MSG_TYPE_MASK) {
            bqws_msg_type base_type = (bqws_msg_type)(type & BQWS_MSG_TYPE_MASK);
            opcode = base_type == BQWS_MSG_TEXT ? 0x1 : 0x2;

            if (type & BQWS_MSG_PARTIAL_BIT) {
                if (buf->partial_type != BQWS_MSG_INVALID) {
                    // Partial continuation
                    bqws_assert(buf->partial_type == base_type);
                    opcode = 0x0;
                }

                if (type & BQWS_MSG_FINAL_BIT) {
                    // This can be either the end of a partial message
                    // or just a single-part partial message.
                    buf->partial_type = BQWS_MSG_INVALID;
                } else {
                    // Partial begin or continuation
                    buf->partial_type = base_type;
                    fin = false;
                }
            }

        } else if (type & BQWS_MSG_CONTROL_MASK) {
            // Control message
            if (type == BQWS_MSG_CONTROL_CLOSE)     opcode = 0x8;
            else if (type == BQWS_MSG_CONTROL_PING) opcode = 0x9;
            else if (type == BQWS_MSG_CONTROL_PONG) opcode = 0xa;
        } else {
            bqws_assert(0 && "Trying to send non-data non-control message");
        }

        bqws_assert(opcode != ~0u);

        // Use the smallest payload length representation
        size_t payload_ext = 0;
        size_t payload_len = msg->msg.size;
        if (payload_len > 65535u) {
            payload_len = 127;
            payload_ext = 8;
        } else if (payload_len > 125) {
            payload_len = 126;
            payload_ext = 2;
        }

        uint8_t *h = (uint8_t*)ws->io.send_header;
        // Static header bits
        h[0] = (fin ? 0x80 : 0x0) | (uint8_t)opcode;
        h[1] = (mask ? 0x80 : 0x0) | (uint8_t)payload_len;
        h += 2;

        // Extended length: Read 2 or 8 bytes of big
        // endian payload length.
        for (size_t i = 0; i < payload_ext; i++) {
            size_t shift = (payload_ext - i - 1) * 8;
            h[i] = (uint8_t)((uint64_t)msg->msg.size >> shift);
        }
        h += payload_ext;

        // Masking key
        buf->masked = mask;
        if (mask) {
            uint32_t mask_key = mask_make_key(ws);
            buf->mask_key = mask_key;
            memcpy(h, &buf->mask_key, 4);
            h += 4;

            // Apply the mask
            mask_apply(msg->msg.data, msg->msg.size, mask_key);
        }

        buf->header_size = (char*)h - ws->io.send_header;
        bqws_assert(buf->header_size <= sizeof(ws->io.send_header));
    }

    // Send the header
    if (buf->header_offset < buf->header_size) {
        size_t to_send = buf->header_size - buf->header_offset;
        size_t sent = send_fn(user, ws, ws->io.send_header + buf->header_offset, to_send);
        if (sent == SIZE_MAX) {
            ws_fail(ws, BQWS_ERR_IO_WRITE);
            return false;
        }
        bqws_assert(sent <= to_send);
        buf->header_offset += sent;
        if (sent < to_send) return false;
    }

    // Send the message
    {
        size_t to_send = msg->msg.size - buf->offset;
        size_t sent = send_fn(user, ws, msg->msg.data + buf->offset, to_send);
        if (sent == SIZE_MAX) {
            ws_fail(ws, BQWS_ERR_IO_WRITE);
            return false;
        }
        bqws_assert(sent <= to_send);
        buf->offset += sent;
        if (sent < to_send) return false;
    }

    if (ws->log_send) {
        ws_log2(ws, "Sent: ", bqws_msg_type_str(buf->msg->msg.type));
    }

    // Mark close as been sent
    if (msg->msg.type == BQWS_MSG_CONTROL_CLOSE) {
        bqws_mutex_lock(&ws->state.mutex);
        if (ws->state.state == BQWS_STATE_OPEN) {
            ws_log(ws, "State: CLOSING (queued user close)");
            ws->state.state = BQWS_STATE_CLOSING;
            ws->state.start_closing_ts = bqws_get_timestamp();
        }
        ws->state.close_sent = true;
        if (ws->state.close_received) {
            ws_close(ws);
        }
        bqws_mutex_unlock(&ws->state.mutex);
    }

    // Delete the message
    msg_free_owned(ws, msg);

    // Sent everything, clear status
    buf->offset = 0;
    buf->header_offset = 0;
    buf->header_size = 0;
    buf->msg = NULL;
    return true;
}

// WebSocket initialization

static char *verify_filter_str(bqws_verify_filter *f, size_t *offset, const char *str)
{
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char *dst = f->text_data + *offset;
    memcpy(dst, str, len);
    *offset += len;
    return dst;
}

static void bqws_internal_filter_verify(void *user, bqws_socket *ws, const bqws_client_opts *opts)
{
    bqws_verify_filter *f = (bqws_verify_filter*)user;
    bool ok = true;

    // Check common headers
    ok = ok && (!f->path || !strcmp(f->path, opts->path));
    ok = ok && (!f->host || streq_ic(f->host, opts->host));
    ok = ok && (!f->origin || streq_ic(f->origin, opts->origin));

    const char *protocol = NULL;
    if (f->num_protocols > 0) {
        // If the filter has protocols try to find one
        // O(n^2) but bounded by BQWS_MAX_PROTOCOLS
        for (size_t ci = 0; ci < opts->num_protocols && !protocol; ci++) {
            for (size_t fi = 0; fi < f->num_protocols; fi++) {
                if (!strcmp(f->protocols[fi], opts->protocols[ci])) {
                    protocol = f->protocols[fi];
                    break;
                }
            }
        }
        ok = ok && protocol != NULL;
    } else {
        // If not don't use any protocol name
        protocol = "";
    }

    if (ok) {
        bqws_assert(protocol != NULL);
        if (f->verify_fn) {
            f->verify_fn(f->verify_user, ws, opts);
        } else {
            bqws_server_accept(ws, protocol);
        }
    } else {
        bqws_server_reject(ws);
    }
}

static void ws_expand_default_limits(bqws_limits *limits)
{
#define WS_DEFAULT(p_name, p_value) if (!limits->p_name) limits->p_name = p_value

    WS_DEFAULT(max_memory_used, 262144);
    WS_DEFAULT(max_recv_msg_size, 262144);
    WS_DEFAULT(max_handshake_size, 262144);
    WS_DEFAULT(max_recv_queue_messages, 1024);
    WS_DEFAULT(max_recv_queue_size, 262144);
    WS_DEFAULT(max_partial_message_parts, 16384);

#undef WS_DEFAULT
}

static bqws_socket *ws_new_socket(const bqws_opts *opts, bool is_server)
{
    bqws_opts null_opts;
    if (!opts) {
        memset(&null_opts, 0, sizeof(null_opts));
        opts = &null_opts;
    }

    bqws_socket *ws = (bqws_socket*)bqws_allocator_alloc(&opts->allocator, sizeof(bqws_socket) + opts->user_size);
    if (!ws) return NULL;

    memset(ws, 0, sizeof(bqws_socket));
    ws->magic = BQWS_SOCKET_MAGIC;
    ws->is_server = is_server;
    ws->allocator = opts->allocator;
    ws->user_io = opts->io;
    ws->limits = opts->limits;
    ws->recv_partial_messages = opts->recv_partial_messages;
    ws->recv_control_messages = opts->recv_control_messages;
    ws->mask_server = opts->mask_server;
    ws->message_fn = opts->message_fn;
    ws->message_user = opts->message_user;
    ws->peek_fn = opts->peek_fn;
    ws->peek_user = opts->peek_user;
    ws->log_fn = opts->log_fn;
    ws->log_user = opts->log_user;
    ws->log_send = opts->log_send;
    ws->log_recv = opts->log_recv;
    ws->error_fn = opts->error_fn;
    ws->error_user = opts->error_user;
    ws->send_message_fn = opts->send_message_fn;
    ws->send_message_user = opts->send_message_user;
    ws->user_size = opts->user_size;

    ws_expand_default_limits(&ws->limits);

    bqws_mutex_init(&ws->err_mutex);
    bqws_mutex_init(&ws->state.mutex);
    bqws_mutex_init(&ws->io.mutex);
    bqws_mutex_init(&ws->alloc.mutex);
    bqws_mutex_init(&ws->partial.mutex);

    msg_init_queue(ws, &ws->recv_queue);
    msg_init_queue(ws, &ws->recv_partial_queue);
    msg_init_queue(ws, &ws->send_queue);

    if (opts->ping_interval) {
        ws->ping_interval = opts->ping_interval;
    } else {
        ws->ping_interval = is_server ? 20000 : 10000;
    }

    ws->connect_timeout = opts->connect_timeout ? opts->connect_timeout : 10000;
    ws->close_timeout = opts->close_timeout ? opts->close_timeout : 5000;
    ws->ping_response_timeout = opts->ping_response_timeout ? opts->ping_response_timeout : 4 * ws->ping_interval;

    bqws_assert(ws->ping_interval > 0);
    if (ws->ping_interval != SIZE_MAX || ws->connect_timeout != SIZE_MAX) {
        bqws_timestamp ts = bqws_get_timestamp();
        ws->io.start_connect_ts = ts;
        ws->io.last_write_ts = ts;
        ws->io.last_read_ts = ts;
        ws->io.last_ping_ts = ts;
    }

    // Copy or zero-init user data
    if (opts->user_size > 0) {
        if (opts->user_data) {
            memcpy(ws->user_data, opts->user_data, opts->user_size);
        } else {
            memset(ws->user_data, 0, opts->user_size);
        }
    }

    if (opts->name) ws->name = ws_copy_str(ws, opts->name);

    if (opts->skip_handshake) {
        ws_log(ws, "State: OPEN (skip handhake)");
        ws->state.state = BQWS_STATE_OPEN;
    } else {
        ws_log(ws, "State: CONNECTING");
        ws->state.state = BQWS_STATE_CONNECTING;
    }

    ws->io.mask_random_state = (uint32_t)(uintptr_t)ws ^ (uint32_t)time(NULL);
    ws->io.mask_random_stream = (uint32_t)bqws_get_timestamp() | 1u;

    if (ws->err) {
        bqws_free_socket(ws);
        return NULL;
    }

    return ws;
}

// -- API

bqws_socket *bqws_new_client(const bqws_opts *opts, const bqws_client_opts *client_opts)
{
    bqws_socket *ws = ws_new_socket(opts, false);
    if (!ws) return NULL;

    // Setup client handshake immediately if the socket is not open already
    if (ws->state.state == BQWS_STATE_CONNECTING) {
        bqws_client_opts null_opts;
        if (!client_opts) {
            memset(&null_opts, 0, sizeof(null_opts));
            client_opts = &null_opts;
        }

        bqws_mutex_lock(&ws->io.mutex);
        hs_client_handshake(ws, client_opts);
        bqws_mutex_unlock(&ws->io.mutex);

        // Notify IO that there's a client handshake to send
        if (ws->user_io.notify_fn) {
            ws->user_io.notify_fn(ws->user_io.user, ws);
        }
    }

    return ws;
}

bqws_socket *bqws_new_server(const bqws_opts *opts, const bqws_server_opts *server_opts)
{
    bqws_socket *ws = ws_new_socket(opts, true);
    if (!ws) return NULL;

    {
        bqws_server_opts null_opts;
        if (!server_opts) {
            memset(&null_opts, 0, sizeof(null_opts));
            server_opts = &null_opts;
        }

        ws->verify_fn = server_opts->verify_fn;
        ws->verify_user = server_opts->verify_user;

        // Setup automatic verify filter if needed
        if (server_opts->verify_filter) {
            bqws_client_opts *filter = server_opts->verify_filter;
            size_t text_size = 0;

            text_size += filter->path ? strlen(filter->path) + 1 : 0;
            text_size += filter->host ? strlen(filter->host) + 1 : 0;
            text_size += filter->origin ? strlen(filter->origin) + 1 : 0;
            for (size_t i = 0; i < filter->num_protocols; i++) {
                bqws_assert(filter->protocols[i] && *filter->protocols[i]);
                text_size += strlen(filter->protocols[i]) + 1;
            }

            bqws_verify_filter *copy = (bqws_verify_filter*)ws_alloc(ws, sizeof(bqws_verify_filter) + text_size);
            if (!copy) {
                bqws_free_socket(ws);
                return NULL;
            }

            memset(copy, 0, sizeof(bqws_verify_filter));
            copy->magic = BQWS_FILTER_MAGIC;
            copy->text_size = text_size;
            size_t offset = 0;

            copy->path = verify_filter_str(copy, &offset, filter->path);
            copy->host = verify_filter_str(copy, &offset, filter->host);
            copy->origin = verify_filter_str(copy, &offset, filter->origin);
            copy->num_protocols = filter->num_protocols;
            for (size_t i = 0; i < filter->num_protocols; i++) {
                copy->protocols[i] = verify_filter_str(copy, &offset, filter->protocols[i]);
            }

            bqws_assert(offset == text_size);

            copy->verify_fn = ws->verify_fn;
            copy->verify_user = ws->verify_user;
            ws->verify_fn = &bqws_internal_filter_verify;
            ws->verify_user = copy;
        }
    }

    return ws;
}

void bqws_close(bqws_socket *ws, bqws_close_reason reason, const void *data, size_t size)
{
    if (ws->err) return;

    bqws_mutex_lock(&ws->state.mutex);

    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(size == 0 || data);
    if (ws->state.close_to_send || ws->state.state >= BQWS_STATE_CLOSING) {
        bqws_mutex_unlock(&ws->state.mutex);
        return;
    }

    bqws_msg_imp *imp = msg_alloc(ws, BQWS_MSG_CONTROL_CLOSE, size + 2);
    if (imp) {
        imp->msg.data[0] = (uint8_t)(reason >> 8);
        imp->msg.data[1] = (uint8_t)(reason & 0xff);
        memcpy(imp->msg.data + 2, data, size);

        ws->state.close_to_send = imp;
        ws->state.start_closing_ts = bqws_get_timestamp();
        ws->state.state = BQWS_STATE_CLOSING;

        ws_log(ws, "State: CLOSING (user close)");
    }

    bqws_mutex_unlock(&ws->state.mutex);
}

void bqws_queue_close(bqws_socket *ws, bqws_close_reason reason, const void *data, size_t size)
{
    if (ws->err) return;

    bqws_mutex_lock(&ws->state.mutex);

    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(size == 0 || data);
    if (ws->state.close_to_send || ws->state.state >= BQWS_STATE_CLOSING) {
        bqws_mutex_unlock(&ws->state.mutex);
        return;
    }

    bqws_msg_imp *imp = msg_alloc(ws, BQWS_MSG_CONTROL_CLOSE, size + 2);
    if (imp) {
        imp->msg.data[0] = (uint8_t)(reason >> 8);
        imp->msg.data[1] = (uint8_t)(reason & 0xff);
        memcpy(imp->msg.data + 2, data, size);
        ws_enqueue_send(ws, imp);
    }

    bqws_mutex_unlock(&ws->state.mutex);
}

void bqws_free_socket(bqws_socket *ws)
{
    if (!ws) return;
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    ws_log(ws, "Freed");

    if (ws->user_io.close_fn && !ws->state.io_closed) {
        ws->user_io.close_fn(ws->user_io.user, ws);
    }

    // Free everything, as the socket may have errored it can
    // be in almost any state

    // Pending messages
    msg_free_queue(ws, &ws->recv_queue);
    msg_free_queue(ws, &ws->recv_partial_queue);
    msg_free_queue(ws, &ws->send_queue);
    if (ws->state.pong_to_send) msg_free_owned(ws, ws->state.pong_to_send);
    if (ws->state.close_to_send) msg_free_owned(ws, ws->state.close_to_send);

    // Read/write buffers
    ws_free(ws, ws->io.handshake.data, ws->io.handshake.capacity);
    ws_free(ws, ws->io.handshake_overflow.data, ws->io.handshake_overflow.capacity);
    if (ws->io.recv_buf.msg) msg_free_owned(ws, ws->io.recv_buf.msg);
    if (ws->io.send_buf.msg) msg_free_owned(ws, ws->io.send_buf.msg);
    if (ws->partial.next_partial_to_send) msg_free_owned(ws, ws->partial.next_partial_to_send);

    // Misc buffers
    if (ws->io.opts_from_client) ws_free(ws, ws->io.opts_from_client, sizeof(bqws_client_opts));

    // String copies
    ws_free_str(ws, ws->state.chosen_protocol);
    ws_free_str(ws, ws->name);

    // Verify filter copy
    if (ws->verify_fn == &bqws_internal_filter_verify) {
        bqws_verify_filter *filter = (bqws_verify_filter*)ws->verify_user;
        bqws_assert(filter->magic == BQWS_FILTER_MAGIC);
        filter->magic = BQWS_DELETED_MAGIC;
        ws_free(ws, filter, sizeof(bqws_verify_filter) + filter->text_size);
    }

    // Mutexes
    bqws_mutex_free(&ws->err_mutex);
    bqws_mutex_free(&ws->state.mutex);
    bqws_mutex_free(&ws->io.mutex);
    bqws_mutex_free(&ws->alloc.mutex);
    bqws_mutex_free(&ws->partial.mutex);

    bqws_assert(ws->alloc.memory_used == 0);

    ws->magic = BQWS_DELETED_MAGIC;

    bqws_allocator at = ws->allocator;
    bqws_allocator_free(&at, ws, sizeof(bqws_socket) + ws->user_size);
}

bqws_client_opts *bqws_server_get_client_opts(bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(ws->is_server);
    // no-mutex(state): There's an inherent race condition with multiple accepts
    bqws_assert(ws->state.state == BQWS_STATE_CONNECTING);

    bqws_mutex_lock(&ws->io.mutex);
    bqws_client_opts *opts = ws->io.opts_from_client;
    bqws_mutex_unlock(&ws->io.mutex);

    return opts;
}

void bqws_server_accept(bqws_socket *ws, const char *protocol)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(ws->is_server);
    // no-mutex(state): There's an inherent race condition with multiple accepts
    bqws_assert(ws->state.state == BQWS_STATE_CONNECTING);
    if (ws->err) return;

    // Use emtpy string to differentiate from not set
    if (!protocol) protocol = "";

    bqws_mutex_lock(&ws->state.mutex);
    if (!ws->state.chosen_protocol) {
        ws->state.chosen_protocol = ws_copy_str(ws, protocol);
    }
    bqws_mutex_unlock(&ws->state.mutex);
}

void bqws_server_reject(bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(ws->is_server);

    ws_fail(ws, BQWS_ERR_SERVER_REJECT);
}

bqws_state bqws_get_state(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    // no-mutex(state): We can always underestimate the state
    bqws_state state = ws->state.state, override_state = ws->state.override_state;
    if (override_state > state) state = override_state;
    return state;
}

bqws_error bqws_get_error(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    return ws->err;
}

bool bqws_is_connecting(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    // no-mutex(state): We can always underestimate the state
    bqws_state state = ws->state.state, override_state = ws->state.override_state;
    if (override_state > state) state = override_state;
    return state == BQWS_STATE_CONNECTING;
}

bool bqws_is_closed(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    // no-mutex(state): We can always underestimate the state
    bqws_state state = ws->state.state, override_state = ws->state.override_state;
    if (override_state > state) state = override_state;
    return state == BQWS_STATE_CLOSED;
}

size_t bqws_get_memory_used(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    // no-mutex(alloc): This doesn't need to be accurate
    return ws->alloc.memory_used;
}

bool bqws_is_server(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    return ws->is_server;
}

void *bqws_user_data(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    return (void*)ws->user_data;
}

size_t bqws_user_data_size(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    return ws->user_size;
}

const char *bqws_get_name(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    return ws->name;
}

bqws_stats bqws_get_stats(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    bqws_stats stats;
    msg_queue_get_stats((bqws_msg_queue*)&ws->recv_queue, &stats.recv);
    msg_queue_get_stats((bqws_msg_queue*)&ws->send_queue, &stats.send);
    return stats;
}

void *bqws_get_io_user(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    return ws->user_io.user;
}

bool bqws_get_io_closed(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    // no-mutex(state): This can be inaccurate
    return ws->state.io_closed;
}

bqws_limits bqws_get_limits(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    return ws->limits;
}

void bqws_set_limits(bqws_socket *ws, const bqws_limits *limits)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(limits);

    bqws_limits copy = *limits;
    ws_expand_default_limits(&copy);
    ws->limits = copy;
}

bqws_close_reason bqws_get_peer_close_reason(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    bqws_mutex_lock((bqws_mutex*)&ws->state.mutex);
    bqws_close_reason reason = ws->state.peer_reason;
    bqws_mutex_unlock((bqws_mutex*)&ws->state.mutex);

    return reason;
}

bqws_error bqws_get_peer_error(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    bqws_mutex_lock((bqws_mutex*)&ws->state.mutex);
    bqws_error err = ws->state.peer_err;
    bqws_mutex_unlock((bqws_mutex*)&ws->state.mutex);

    return err;
}

const char *bqws_get_protocol(const bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    // TODO: Cache this pointer outside of the state mutex
    bqws_mutex_lock((bqws_mutex*)&ws->state.mutex);
    const char *protocol = ws->state.chosen_protocol;
    bqws_mutex_unlock((bqws_mutex*)&ws->state.mutex);

    return protocol;
}

bqws_msg *bqws_recv(bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    if (ws->err) return NULL;

    // Messages are re-combined in `recv_queue` if
    // `recv_partial_messages` is disabled.

    bqws_msg_imp *imp = msg_dequeue(&ws->recv_queue);
    if (!imp) return NULL;
    bqws_assert(imp->magic == BQWS_MSG_MAGIC);

    msg_release_ownership(ws, imp);
    return &imp->msg;
}

void bqws_free_msg(bqws_msg *msg)
{
    if (!msg) return;

    bqws_msg_imp *imp = msg_imp(msg);
    bqws_assert(imp->magic == BQWS_MSG_MAGIC);
    bqws_assert(imp->owner == NULL);

    imp->magic = BQWS_DELETED_MAGIC;

    bqws_allocator at = imp->allocator;
    bqws_allocator_free(&at, imp, msg_alloc_size(msg));
}

void bqws_send(bqws_socket *ws, bqws_msg_type type, const void *data, size_t size)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert((type & BQWS_MSG_PARTIAL_BIT) == 0);
    if (ws->err) return;
    bqws_assert(size == 0 || data);

    bqws_msg_imp *imp = msg_alloc(ws, type, size);
    if (!imp) return;

    memcpy(imp->msg.data, data, size);
    ws_enqueue_send(ws, imp);
}

void bqws_send_binary(bqws_socket *ws, const void *data, size_t size)
{
    bqws_send(ws, BQWS_MSG_BINARY, data, size);
}

void bqws_send_text(bqws_socket *ws, const char *str)
{
    bqws_assert(str);
    bqws_send(ws, BQWS_MSG_TEXT, str, strlen(str));
}

void bqws_send_text_len(bqws_socket *ws, const void *str, size_t len)
{
    bqws_send(ws, BQWS_MSG_TEXT, str, len);
}

bqws_msg *bqws_allocate_msg(bqws_socket *ws, bqws_msg_type type, size_t size)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    if (ws->err) return NULL;

    bqws_msg_imp *imp = msg_alloc(ws, type, size);
    if (!imp) return NULL;

    msg_release_ownership(ws, imp);
    return &imp->msg;
}

void bqws_send_msg(bqws_socket *ws, bqws_msg *msg)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(msg && (msg->type == BQWS_MSG_TEXT || msg->type == BQWS_MSG_BINARY));
    bqws_assert(msg->size <= msg->capacity);

    bqws_msg_imp *imp = msg_imp(msg);
    bqws_assert(imp->magic == BQWS_MSG_MAGIC);

    if (ws->err) return;

    if (!msg_acquire_ownership(ws, imp)) return;

    ws_enqueue_send(ws, imp);
}

void bqws_send_begin(bqws_socket *ws, bqws_msg_type type)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(type == BQWS_MSG_TEXT || type == BQWS_MSG_BINARY);
    if (ws->err) return;

    bqws_mutex_lock(&ws->partial.mutex);

    bqws_assert(ws->partial.send_partial_type == BQWS_MSG_INVALID);
    bqws_assert(ws->partial.next_partial_to_send == NULL);

    ws->partial.send_partial_type = type;

    bqws_mutex_unlock(&ws->partial.mutex);
}

void bqws_send_append(bqws_socket *ws, const void *data, size_t size)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    if (ws->err) return;

    bqws_mutex_lock(&ws->partial.mutex);

    bqws_assert(ws->partial.send_partial_type != BQWS_MSG_INVALID);

    if (ws->partial.next_partial_to_send) {
        bqws_assert(ws->partial.next_partial_to_send->magic == BQWS_MSG_MAGIC);
        ws_enqueue_send(ws, ws->partial.next_partial_to_send);
    }

    bqws_msg_type partial_type = (bqws_msg_type)(ws->partial.send_partial_type | BQWS_MSG_PARTIAL_BIT);
    bqws_msg_imp *imp = msg_alloc(ws, partial_type, size);
    if (imp) {
        memcpy(imp->msg.data, data, size);
        ws->partial.next_partial_to_send = imp;
    }

    bqws_mutex_unlock(&ws->partial.mutex);
}

void bqws_send_append_str(bqws_socket *ws, const char *str)
{
    bqws_send_append(ws, str, strlen(str));
}

void bqws_send_append_msg(bqws_socket *ws, bqws_msg *msg)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(msg->type == BQWS_MSG_TEXT || msg->type == BQWS_MSG_BINARY);
    if (ws->err) return;

    bqws_mutex_lock(&ws->partial.mutex);

    bqws_assert(ws->partial.send_partial_type != BQWS_MSG_INVALID);
    bqws_assert((ws->partial.send_partial_type & BQWS_MSG_TYPE_MASK) == msg->type);

    if (ws->partial.next_partial_to_send) {
        bqws_assert(ws->partial.next_partial_to_send->magic == BQWS_MSG_MAGIC);
        ws_enqueue_send(ws, ws->partial.next_partial_to_send);
    }

    bqws_msg_imp *imp = msg_imp(msg);
    if (!msg_acquire_ownership(ws, imp)) return;

    msg->type = (bqws_msg_type)(ws->partial.send_partial_type | BQWS_MSG_PARTIAL_BIT);
    ws->partial.next_partial_to_send = imp;

    bqws_mutex_unlock(&ws->partial.mutex);
}

void bqws_send_finish(bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    if (ws->err) return;

    bqws_mutex_lock(&ws->partial.mutex);

    bqws_assert(ws->partial.send_partial_type != BQWS_MSG_INVALID);

    if (ws->partial.next_partial_to_send) {
        bqws_assert(ws->partial.next_partial_to_send->magic == BQWS_MSG_MAGIC);
        bqws_msg_type type = ws->partial.next_partial_to_send->msg.type;
        ws->partial.next_partial_to_send->msg.type = (bqws_msg_type)(type | BQWS_MSG_FINAL_BIT);
        ws_enqueue_send(ws, ws->partial.next_partial_to_send);
        ws->partial.next_partial_to_send = NULL;
    }

    ws->partial.send_partial_type = BQWS_MSG_INVALID;

    bqws_mutex_unlock(&ws->partial.mutex);
}

void bqws_send_ping(bqws_socket *ws, const void *data, size_t size)
{
    bqws_send(ws, BQWS_MSG_CONTROL_PING, data, size);
}

void bqws_send_pong(bqws_socket *ws, const void *data, size_t size)
{
    bqws_send(ws, BQWS_MSG_CONTROL_PONG, data, size);
}

void bqws_update(bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    bqws_update_state(ws);
    bqws_update_io(ws);
}

void bqws_update_state(bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    bqws_mutex_lock(&ws->state.mutex);
    bqws_state state = ws->state.state;
    char *protocol = ws->state.chosen_protocol;
    bqws_timestamp start_closing_ts = ws->state.start_closing_ts;
    bqws_mutex_unlock(&ws->state.mutex);

    bqws_mutex_lock(&ws->io.mutex);

    if (state == BQWS_STATE_CONNECTING) {

        // If we're connecting but haven't set a protocol and the user
        // has provided a verify function or filter run it here.
        if (ws->io.client_handshake_done && !protocol && ws->verify_fn) {
            bqws_assert(ws->is_server);
            bqws_assert(ws->io.opts_from_client);
            ws->verify_fn(ws->verify_user, ws, ws->io.opts_from_client);
        }

        // Connect timeout
        if (ws->connect_timeout != SIZE_MAX && state == BQWS_STATE_CONNECTING) {
            bqws_timestamp time = bqws_get_timestamp();
            size_t delta = bqws_timestamp_delta_to_ms(ws->io.start_connect_ts, time);
            if (delta > ws->connect_timeout) {
                ws_fail(ws, BQWS_ERR_CONNECT_TIMEOUT);

                bqws_mutex_lock(&ws->state.mutex);
                ws_close(ws);
                bqws_mutex_unlock(&ws->state.mutex);
            }
        }

    } else if (state == BQWS_STATE_OPEN) {

        // Automatic PING send
        if (ws->ping_interval != SIZE_MAX) {
            bqws_timestamp time = bqws_get_timestamp();
            size_t delta_read = bqws_timestamp_delta_to_ms(ws->io.last_read_ts, time);
            size_t delta_ping = bqws_timestamp_delta_to_ms(ws->io.last_ping_ts, time);
            size_t delta_write = bqws_timestamp_delta_to_ms(ws->io.last_write_ts, time);

            size_t delta = delta_read >= delta_write ? delta_read : delta_write;
            size_t delta_from_ping = delta <= delta_ping ? delta : delta_ping;

            if (delta_from_ping > ws->ping_interval) {
                ws->io.last_ping_ts = time;
                // Maybe send PONG only?
                bqws_send_ping(ws, NULL, 0);
            }

            if (ws->ping_response_timeout != SIZE_MAX) {
                if (delta >= ws->ping_response_timeout) {
                    ws_fail(ws, BQWS_ERR_PING_TIMEOUT);

                    bqws_mutex_lock(&ws->state.mutex);
                    ws_close(ws);
                    bqws_mutex_unlock(&ws->state.mutex);
                }
            }
        }

    } else if (state == BQWS_STATE_CLOSING) {

        // Close timeout
        if (ws->close_timeout != SIZE_MAX) {
            bqws_timestamp time = bqws_get_timestamp();
            size_t delta = bqws_timestamp_delta_to_ms(start_closing_ts, time);
            if (delta > ws->close_timeout) {
                ws_fail(ws, BQWS_ERR_CLOSE_TIMEOUT);

                bqws_mutex_lock(&ws->state.mutex);
                ws_close(ws);
                bqws_mutex_unlock(&ws->state.mutex);
            }
        }

    }

    bqws_mutex_unlock(&ws->io.mutex);
}

void bqws_update_io(bqws_socket *ws)
{
    bqws_update_io_write(ws);
    bqws_update_io_read(ws);
}

void bqws_update_io_read(bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    bool do_read = true;

    bqws_mutex_lock(&ws->io.mutex);

    // If read and write are stopped close the IO
    bqws_mutex_lock(&ws->state.mutex);
    if (!ws->state.io_started) {
        if (ws->user_io.init_fn) {
            ws->user_io.init_fn(ws->user_io.user, ws);
        }
        ws->state.io_started = true;
    }
    if (ws->state.stop_read && ws->state.stop_write) {
        if (ws->user_io.close_fn && !ws->state.io_closed) {
            ws->user_io.close_fn(ws->user_io.user, ws);
        }
        ws->state.io_closed = true;
    }
    do_read = !ws->state.stop_read;
    bqws_mutex_unlock(&ws->state.mutex);

    // TODO: Throttle reads
    if (do_read) {
        if (ws->user_io.recv_fn) {
            while (ws_read_data(ws, ws->user_io.recv_fn, ws->user_io.user)) {
                // Keep reading as long as there is space
            }
        }
    }

    bqws_mutex_unlock(&ws->io.mutex);
}

void bqws_update_io_write(bqws_socket *ws)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    bool do_write = true;

    bqws_mutex_lock(&ws->io.mutex);

    // If read and write are stopped close the IO
    bqws_mutex_lock(&ws->state.mutex);
    if (!ws->state.io_started) {
        if (ws->user_io.init_fn) {
            ws->user_io.init_fn(ws->user_io.user, ws);
        }
        ws->state.io_started = true;
    }
    if (ws->state.stop_read && ws->state.stop_write) {
        if (ws->user_io.close_fn && !ws->state.io_closed) {
            ws->user_io.close_fn(ws->user_io.user, ws);
        }
        ws->state.io_closed = true;
    }
    do_write = !ws->state.stop_write;
    bqws_mutex_unlock(&ws->state.mutex);

    if (do_write) {
        if (ws->user_io.send_fn) {
            while (ws_write_data(ws, ws->user_io.send_fn, ws->user_io.user)) {
                // Keep writing as long as there is space
            }
        }

        // Re-check if we should stop write if the socket got closed
        bqws_mutex_lock(&ws->state.mutex);
        do_write = !ws->state.stop_write;
        bqws_mutex_unlock(&ws->state.mutex);

        if (ws->user_io.flush_fn && do_write) {
            if (!ws->user_io.flush_fn(ws->user_io.user, ws)) {
                ws_fail(ws, BQWS_ERR_IO_WRITE);
            }
        }
    }

    bqws_mutex_unlock(&ws->io.mutex);
}

size_t bqws_read_from(bqws_socket *ws, const void *data, size_t size)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(!ws->user_io.recv_fn);

    bqws_mutex_lock(&ws->io.mutex);

    bqws_mem_stream s;
    s.ptr = (char*)data;
    s.end = s.ptr + size;

    while (ws_read_data(ws, &mem_stream_recv, &s)) {
        // Keep reading as long as there is space
    }

    bqws_mutex_unlock(&ws->io.mutex);

    return s.ptr - (char*)data;
}

size_t bqws_write_to(bqws_socket *ws, void *data, size_t size)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(!ws->user_io.send_fn);

    bqws_mutex_lock(&ws->io.mutex);

    bqws_mem_stream s;
    s.ptr = (char*)data;
    s.end = s.ptr + size;

    while (ws_write_data(ws, &mem_stream_send, &s)) {
        // Keep writing as long as there is space
    }

    if (ws->user_io.flush_fn) {
        if (!ws->user_io.flush_fn(ws->user_io.user, ws)) {
            ws_fail(ws, BQWS_ERR_IO_WRITE);
        }
    }

    bqws_mutex_unlock(&ws->io.mutex);

    return s.ptr - (char*)data;
}

void bqws_direct_push_msg(bqws_socket *ws, bqws_msg *msg)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);
    bqws_assert(msg && msg->size <= msg->capacity);

    bqws_msg_imp *imp = msg_imp(msg);
    bqws_assert(imp->magic == BQWS_MSG_MAGIC);

    if (ws->err) return;

    if (!msg_acquire_ownership(ws, imp)) return;

    if (ws->log_recv) {
        ws_log2(ws, "Direct recv: ", bqws_msg_type_str(msg->type));
    }

    ws_enqueue_recv(ws, imp);
}

void bqws_direct_set_override_state(bqws_socket *ws, bqws_state state)
{
    bqws_assert(ws && ws->magic == BQWS_SOCKET_MAGIC);

    ws_log2(ws, "Override state: ", bqws_state_str(state));

    bqws_mutex_lock(&ws->state.mutex);
    ws->state.override_state = state;
    bqws_mutex_unlock(&ws->state.mutex);
}

void bqws_direct_fail(bqws_socket *ws, bqws_error err)
{
    ws_fail(ws, err);
}

bool bqws_parse_url(bqws_url *url, const char *str)
{
    // Format [wss://][host.example.com][:1234][/path]

    const char *scheme = str;
    const char *scheme_end = strstr(scheme, "://");
    const char *host = scheme_end ? scheme_end + 3 : scheme;
    const char *port_start = host;
    if (*host == '[') {
        // Skip IPv6 address
        port_start = strstr(host, "]");
        if (!port_start) return false;
    }
    const char *port = strstr(port_start, ":");
    const char *path = strstr(port_start, "/");
    if (!path) path = port_start + strlen(port_start);
    if (port && port > path) port = NULL;
    const char *host_end = port ? port : path;

    size_t scheme_len = scheme_end ? scheme_end - scheme : 0;
    size_t host_len = host_end - host;
    if (scheme_len >= sizeof(url->scheme)) return false;
    if (host_len >= sizeof(url->host)) return false;

    bool secure = scheme_len == 3 && !memcmp(scheme, "wss", 3);

    int port_num;
    if (port) {
        char *port_end;
        port_num = (int)strtol(port + 1, &port_end, 10);
        if (port_end != path) return false;
        if (port_num < 0 || port_num > UINT16_MAX) return false;
        port_num = (uint16_t)port_num;
    } else {
        port_num = secure ? 443 : 80;
    }

    // vv No fails below, no writes above ^^

    url->port = (uint16_t)port_num;

    memcpy(url->scheme, scheme, scheme_len);
    url->scheme[scheme_len] = '\0';

    memcpy(url->host, host, host_len);
    url->host[host_len] = '\0';

    url->path = *path ? path : "/";

    url->secure = secure;

    return true;
}

const char *bqws_error_str(bqws_error error)
{
    switch (error) {
    case BQWS_OK: return "OK";
    case BQWS_ERR_UNKNOWN: return "UNKNOWN";
    case BQWS_ERR_SERVER_REJECT: return "SERVER_REJECT";
    case BQWS_ERR_LIMIT_MAX_MEMORY_USED: return "LIMIT_MAX_MEMORY_USED";
    case BQWS_ERR_LIMIT_MAX_RECV_MSG_SIZE: return "LIMIT_MAX_RECV_MSG_SIZE";
    case BQWS_ERR_LIMIT_MAX_HANDSHAKE_SIZE: return "LIMIT_MAX_HANDSHAKE_SIZE";
    case BQWS_ERR_LIMIT_MAX_PARTIAL_MESSAGE_PARTS: return "LIMIT_MAX_PARTIAL_MESSAGE_PARTS";
    case BQWS_ERR_CONNECT_TIMEOUT: return "BQWS_ERR_CONNECT_TIMEOUT";
    case BQWS_ERR_PING_TIMEOUT: return "BQWS_ERR_PING_TIMEOUT";
    case BQWS_ERR_CLOSE_TIMEOUT: return "BQWS_ERR_CLOSE_TIMEOUT";
    case BQWS_ERR_ALLOCATOR: return "ALLOCATOR";
    case BQWS_ERR_BAD_CONTINUATION: return "BAD_CONTINUATION";
    case BQWS_ERR_UNFINISHED_PARTIAL: return "UNFINISHED_PARTIAL";
    case BQWS_ERR_PARTIAL_CONTROL: return "PARTIAL_CONTROL";
    case BQWS_ERR_BAD_OPCODE: return "BAD_OPCODE";
    case BQWS_ERR_RESERVED_BIT: return "RESERVED_BIT";
    case BQWS_ERR_IO_WRITE: return "IO_WRITE";
    case BQWS_ERR_IO_READ: return "IO_READ";
    case BQWS_ERR_BAD_HANDSHAKE: return "BAD_HANDSHAKE";
    case BQWS_ERR_UNSUPPORTED_VERSION: return "UNSUPPORTED_VERSION";
    case BQWS_ERR_TOO_MANY_HEADERS: return "TOO_MANY_HEADERS";
    case BQWS_ERR_TOO_MANY_PROTOCOLS: return "TOO_MANY_PROTOCOLS";
    case BQWS_ERR_HEADER_KEY_TOO_LONG: return "HEADER_KEY_TOO_LONG";
    case BQWS_ERR_HEADER_BAD_ACCEPT: return "HEADER_BAD_ACCEPT";
    case BQWS_ERR_HEADER_PARSE: return "HEADER_PARSE";
    default: return "(unknown)";
    }
}

const char *bqws_msg_type_str(bqws_msg_type type)
{
    switch (type) {
    case BQWS_MSG_TEXT: return "TEXT";
    case BQWS_MSG_BINARY: return "BINARY";
    case BQWS_MSG_PARTIAL_TEXT: return "PARTIAL_TEXT";
    case BQWS_MSG_PARTIAL_BINARY: return "PARTIAL_BINARY";
    case BQWS_MSG_FINAL_TEXT: return "FINAL_TEXT";
    case BQWS_MSG_FINAL_BINARY: return "FINAL_BINARY";
    case BQWS_MSG_CONTROL_CLOSE: return "CONTROL_CLOSE";
    case BQWS_MSG_CONTROL_PING: return "CONTROL_PING";
    case BQWS_MSG_CONTROL_PONG: return "CONTROL_PONG";
    default: return "(unknown)";
    }
}

const char *bqws_state_str(bqws_state state)
{
    switch (state) {
    case BQWS_STATE_INVALID: return "INVALID";
    case BQWS_STATE_CONNECTING: return "CONNECTING";
    case BQWS_STATE_OPEN: return "OPEN";
    case BQWS_STATE_CLOSING: return "CLOSING";
    case BQWS_STATE_CLOSED: return "CLOSED";
    default: return "(unknown)";
    }
}

// TODO: Add a define for this

/* ================ sha1.c ================ */
/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain
Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

/* Hash a single 512-bit block. This is the core of the algorithm. */

static void SHA1Transform(uint32_t state[5], const void *buffer)
{
    uint32_t a, b, c, d, e;
    typedef union {
        unsigned char c[64];
        uint32_t l[16];
    } CHAR64LONG16;
    CHAR64LONG16 block_buf, *block = &block_buf;
    memcpy(block, buffer, 64);
    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

/* SHA1Init - Initialize new context */

static void SHA1Init(SHA1_CTX* context)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

/* Run your data through this. */

static void SHA1Update(SHA1_CTX* context, const void* data, uint32_t len)
{
    uint32_t i, j;
    const char *bytes = (const char *)data;

    j = context->count[0];
    if ((context->count[0] += len << 3) < j)
    context->count[1]++;
    context->count[1] += (len>>29);
    j = (j >> 3) & 63;
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64-j));
        SHA1Transform(context->state, context->buffer);
        for ( ; i + 63 < len; i += 64) {
            SHA1Transform(context->state, &bytes[i]);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&context->buffer[j], &bytes[i], len - i);
}


/* Add padding and return the message digest. */

static void SHA1Final(unsigned char digest[20], SHA1_CTX* context)
{
    unsigned i;
    unsigned char finalcount[8];
    unsigned char c;

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
         >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }
    c = 0200;
    SHA1Update(context, &c, 1);
    while ((context->count[0] & 504) != 448) {
    c = 0000;
        SHA1Update(context, &c, 1);
    }
    SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
    for (i = 0; i < 20; i++) {
        digest[i] = (unsigned char)
         ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }
}
/* ================ end of sha1.c ================ */

static void bqws_sha1(uint8_t digest[20], const void *data, size_t size)
{
    SHA1_CTX ctx;
    SHA1Init(&ctx);
    SHA1Update(&ctx, data, (uint32_t)size);
    SHA1Final((unsigned char*)digest, &ctx);
}

#endif // BQ_WEBSOCKET_IMPLEMENTATION

//------------------------------------------------------------------------------

#ifdef BQ_PLATFORM_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// -- Generic

#ifndef BQWS_PT_USE_OPENSSL
#define BQWS_PT_USE_OPENSSL 0
#endif

#if defined(_WIN32) && !defined(__MINGW32__) //<@r-lyeh
__declspec(thread) static bqws_pt_error t_err;
#else
static __thread bqws_pt_error t_err;
#endif

#define BQWS_PT_DELETED_MAGIC  0xbdbdbdbd
#define BQWS_PT_IO_MAGIC       0x77737074
#define BQWS_PT_EM_MAGIC       0x7773656d
#define BQWS_PT_SERVER_MAGIC   0x77737376

#ifndef bqws_assert
#include <assert.h>
#define bqws_assert(x) assert(x)
#endif

static void pt_fail_pt(const char *func, bqws_pt_error_code code)
{
    t_err.function = func;
    t_err.type = BQWS_PT_ERRTYPE_PT;
    t_err.data = code;
}

#if defined(__EMSCRIPTEN__)

#include <emscripten.h>

#if defined(__EMSCRIPTEN_PTHREADS__)
    #include <pthread.h>
    typedef pthread_mutex_t pt_em_mutex;
    #define pt_em_mutex_init(m) pthread_mutex_init((m), NULL)
    #define pt_em_mutex_free(m) pthread_mutex_destroy((m))
    #define pt_em_mutex_lock(m) pthread_mutex_lock((m))
    #define pt_em_mutex_unlock(m) pthread_mutex_unlock((m))
#else
    typedef int pt_em_mutex;
    #define pt_em_mutex_init(m) (void)0
    #define pt_em_mutex_free(m) (void)0
    #define pt_em_mutex_lock(m) (void)0
    #define pt_em_mutex_unlock(m) (void)0
#endif

typedef struct pt_em_partial {
    struct pt_em_partial *next;
    size_t size;
    char data[];
} pt_em_partial;

typedef struct {
    uint32_t magic;
    int handle;
    pt_em_partial *partial_first;
    pt_em_partial *partial_last;
    size_t partial_size;

    pt_em_mutex ws_mutex;
    bqws_socket *ws;

    // Packed zero separated url + protocols
    size_t num_protocols;
    size_t connect_data_size;
    char *connect_data;

    bqws_allocator allocator;

} pt_em_socket;

static void pt_sleep_ms(uint32_t ms)
{
    emscripten_sleep(ms);
}

static void pt_em_free(pt_em_socket *em)
{
    bqws_assert(em->magic == BQWS_PT_EM_MAGIC);
    em->magic = BQWS_PT_DELETED_MAGIC;

    bqws_allocator allocator = em->allocator;
    bqws_allocator_free(&allocator, em, sizeof(pt_em_socket));
}

static bool pt_em_try_lock(pt_em_socket *em)
{
    pt_em_mutex_lock(&em->ws_mutex);
    if (!em->ws) {
        // TODO: Free the em context here
        pt_em_mutex_unlock(&em->ws_mutex);
        pt_em_free(em);
        return false;
    }
    return true;
}

static void pt_em_unlock(pt_em_socket *em)
{
    pt_em_mutex_unlock(&em->ws_mutex);
}

EMSCRIPTEN_KEEPALIVE void *pt_em_msg_alloc(pt_em_socket *em, size_t size, int type)
{
    if (!pt_em_try_lock(em)) return em; /* HACK: return the handle on close! */
    bqws_msg *msg = bqws_allocate_msg(em->ws, (bqws_msg_type)type, size);
    pt_em_unlock(em);
    if (!msg) return NULL;
    return msg->data;
}

EMSCRIPTEN_KEEPALIVE int pt_em_msg_recv(pt_em_socket *em, void *ptr)
{
    if (!pt_em_try_lock(em)) return 1;
    bqws_msg *msg = (bqws_msg*)((char*)ptr - offsetof(bqws_msg, data));
    bqws_direct_push_msg(em->ws, msg);
    pt_em_unlock(em);
    return 0;
}

EMSCRIPTEN_KEEPALIVE int pt_em_on_open(pt_em_socket *em)
{
    if (!pt_em_try_lock(em)) return 1;
    bqws_direct_set_override_state(em->ws, BQWS_STATE_OPEN);
    pt_em_unlock(em);
    return 0;
}

EMSCRIPTEN_KEEPALIVE int pt_em_on_close(pt_em_socket *em)
{
    if (!pt_em_try_lock(em)) return 1;
    bqws_direct_set_override_state(em->ws, BQWS_STATE_CLOSED);
    em->handle = -1;
    pt_em_unlock(em);
    return 0;
}

EMSCRIPTEN_KEEPALIVE int pt_em_is_closed(pt_em_socket *em)
{
    return em->ws == NULL;
}

EM_JS(int, pt_em_connect_websocket, (pt_em_socket *em, const char *url, const char **protocols, size_t num_protocols), {
    var webSocket = typeof WebSocket !== "undefined" ? WebSocket : require("ws");

    var url_str = UTF8ToString(url);
    var protocols_str = [];
    for (var i = 0; i < num_protocols; i++) {
        var protocol = HEAPU32[(protocols >> 2) + i];
        protocols_str.push(UTF8ToString(protocol));
    }
    var ws = new webSocket(url_str, protocols_str);

    ws.binaryType = "arraybuffer";

    if (Module.g_bqws_pt_sockets === undefined) {
        Module.g_bqws_pt_sockets = {
            sockets: [null],
            em_sockets: [null],
            free_list: [],
        };
    }

    var handle = null;
    if (Module.g_bqws_pt_sockets.free_list.length > 0) {
        handle = Module.g_bqws_pt_sockets.free_list.pop();
        Module.g_bqws_pt_sockets.sockets[handle] = ws;
        Module.g_bqws_pt_sockets.em_sockets[handle] = em;
    } else {
        handle = Module.g_bqws_pt_sockets.sockets.length;
        Module.g_bqws_pt_sockets.sockets.push(ws);
        Module.g_bqws_pt_sockets.em_sockets.push(em);
    }

    var interval = setInterval(function() {
        if (_pt_em_is_closed(em)) {
            ws.close();
        }
    }, 1000);

    ws.onopen = function(e) {
        if (Module.g_bqws_pt_sockets.sockets[handle] !== ws) return;

        if (_pt_em_on_open(em)) {
            ws.close();
            Module.g_bqws_pt_sockets.sockets[handle] = null;
            Module.g_bqws_pt_sockets.em_sockets[handle] = null;
            Module.g_bqws_pt_sockets.free_list.push(handle);
        }
    };
    ws.onclose = function(e) {
        if (interval !== null) {
            clearInterval(interval);
            interval = null;
        }
        if (Module.g_bqws_pt_sockets.sockets[handle] !== ws) return;

        _pt_em_on_close(em);
        Module.g_bqws_pt_sockets.sockets[handle] = null;
        Module.g_bqws_pt_sockets.em_sockets[handle] = null;
        Module.g_bqws_pt_sockets.free_list.push(handle);
    };
    ws.onerror = function(e) {
        if (Module.g_bqws_pt_sockets.sockets[handle] !== ws) return;

        _pt_em_on_close(em);
        ws.close();
        Module.g_bqws_pt_sockets.sockets[handle] = null;
        Module.g_bqws_pt_sockets.em_sockets[handle] = null;
        Module.g_bqws_pt_sockets.free_list.push(handle);
    };

    ws.onmessage = function(e) {
        if (Module.g_bqws_pt_sockets.sockets[handle] !== ws) return;

        var deleted = 0;
        if (typeof e.data === "string") {
            var size = lengthBytesUTF8(e.data);
            var ptr = _pt_em_msg_alloc(em, size, 1);
            if (ptr == em) {
                // HACK: pt_em_msg_alloc() returns `em` if deleted
                deleted = 1;
            } else if (ptr != 0) {
                stringToUTF8(e.data, ptr, size + 1);
                deleted = _pt_em_msg_recv(em, ptr);
            }
        } else {
            var size = e.data.byteLength;
            var ptr = _pt_em_msg_alloc(em, size, 2);
            if (ptr == em) {
                // HACK: pt_em_msg_alloc() returns `em` if deleted
                deleted = 1;
            } else if (ptr != 0) {
                HEAPU8.set(new Uint8Array(e.data), ptr);
                deleted = _pt_em_msg_recv(em, ptr);
            }
        }

        if (deleted != 0) {
            ws.close();
            Module.g_bqws_pt_sockets.sockets[handle] = null;
            Module.g_bqws_pt_sockets.em_sockets[handle] = null;
            Module.g_bqws_pt_sockets.free_list.push(handle);
        }
    };

    return handle;
});

EM_JS(int, pt_em_websocket_send_binary, (int handle, pt_em_socket *em, const char *data, size_t size), {
    if (!Module.g_bqws_pt_sockets || em !== Module.g_bqws_pt_sockets.em_sockets[handle]) {
        console.error("WebSocket '0x" + em.toString(16) + "' not found in thread: Make sure to call bqws_update() only from a single thread per socket in WASM!");
        return 0;
    }
    var ws = Module.g_bqws_pt_sockets.sockets[handle];
    if (ws.readyState == 0) {
        return 0;
    } else if (ws.readyState != 1) {
        return 1;
    }

    #if defined(__EMSCRIPTEN_PTHREADS__)
        ws.send(new Uint8Array(HEAPU8.subarray(data, data + size)));
    #else
        ws.send(HEAPU8.subarray(data, data + size));
    #endif
    return 1;
});

EM_JS(int, pt_em_websocket_send_text, (int handle, pt_em_socket *em, const char *data, size_t size), {
    if (!Module.g_bqws_pt_sockets || em !== Module.g_bqws_pt_sockets.em_sockets[handle]) {
        console.error("WebSocket '0x" + em.toString(16) + "' not found in thread: Make sure to call bqws_update() only from a single thread per socket in WASM!");
        return 0;
    }
    var ws = Module.g_bqws_pt_sockets.sockets[handle];
    if (ws.readyState == 0) {
        return 0;
    } else if (ws.readyState != 1) {
        return 1;
    }

    var str = UTF8ToString(data, size);
    ws.send(str);
    return 1;
});

EM_JS(void, pt_em_websocket_close, (int handle, pt_em_socket *em, int code), {
    if (!Module.g_bqws_pt_sockets || em !== Module.g_bqws_pt_sockets.em_sockets[handle]) {
        console.error("WebSocket '0x" + em.toString(16) + "' not found in thread: Make sure to call bqws_update() only from a single thread per socket in WASM!");
        return 0;
    }
    var ws = Module.g_bqws_pt_sockets.sockets[handle];
    if (ws.readyState >= 2) {
        return 0;
    }

    ws.close(code);
    return 1;
});

EM_JS(int, pt_em_try_free_websocket, (int handle, pt_em_socket *em), {
    if (!Module.g_bqws_pt_sockets || em !== Module.g_bqws_pt_sockets.em_sockets[handle]) { return 0; }
    var ws = Module.g_bqws_pt_sockets.sockets[handle];
    if (ws.readyState < 2) ws.close();

    Module.g_bqws_pt_sockets.sockets[handle] = null;
    Module.g_bqws_pt_sockets.em_sockets[handle] = null;
    Module.g_bqws_pt_sockets.free_list.push(handle);
    return 1;
});

static bool pt_send_message(void *user, bqws_socket *ws, bqws_msg *msg)
{
    pt_em_socket *em = (pt_em_socket*)user;
    void *partial_buf = NULL;

    bqws_msg_type type = msg->type;
    size_t size = msg->size;
    void *data = msg->data;

    if (type & BQWS_MSG_PARTIAL_BIT) {

        pt_em_partial *part = (pt_em_partial*)bqws_allocator_alloc(&em->allocator, sizeof(pt_em_partial) + size);
        part->next = NULL;
        part->size = size;
        memcpy(part->data, data, size);
        em->partial_size += size;

        if (em->partial_last) {
            em->partial_last->next = part;
            em->partial_last = part;
        } else {
            em->partial_first = part;
            em->partial_last = part;
        }

        if (type & BQWS_MSG_FINAL_BIT) {
            char *ptr = (char*)bqws_allocator_alloc(&em->allocator, em->partial_size);

            partial_buf = ptr;
            data = ptr;
            size = em->partial_size;
            type = (bqws_msg_type)(type & BQWS_MSG_TYPE_MASK);

            pt_em_partial *next = em->partial_first;
            while (next) {
                pt_em_partial *part = next;
                next = part->next;

                memcpy(ptr, part->data, part->size);
                ptr += part->size;

                bqws_allocator_free(&em->allocator, part, sizeof(pt_em_partial) + part->size);
            }

        } else {
            bqws_free_msg(msg);
            return true;
        }
    }

    bool ret = true;

    if (type == BQWS_MSG_BINARY) {
        ret = (bool)pt_em_websocket_send_binary(em->handle, em, (const char *)data, size);
    } else if (type == BQWS_MSG_TEXT) {
        ret = (bool)pt_em_websocket_send_text(em->handle, em, (const char *)data, size);
    } else if (type == BQWS_MSG_CONTROL_CLOSE) {
        unsigned code = 1000;
        if (msg->size >= 2) {
            code = (unsigned)(uint8_t)msg->data[0] << 8 | (unsigned)(uint8_t)msg->data[1];
        }
        pt_em_websocket_close(em->handle, em, (int)code);
        ret = true;
    } else {
        // Don't send control messages
    }

    if (partial_buf) {
        bqws_allocator_free(&em->allocator, partial_buf, size);
        if (ret) {
            em->partial_first = NULL;
            em->partial_last = NULL;
            em->partial_size = 0;
        }
    }

    if (ret) {
        bqws_free_msg(msg);
    }

    return ret;
}

static bool pt_init(const bqws_pt_init_opts *opts)
{
    return true;
}

static void pt_shutdown()
{
}

static size_t pt_io_send(void *user, bqws_socket *ws, const void *data, size_t size)
{
    bqws_assert(0 && "Should never get here");
    return SIZE_MAX;
}

static void pt_io_init(void *user, bqws_socket *ws)
{
    pt_em_socket *em = (pt_em_socket*)user;

    if (!pt_em_try_lock(em)) return;

    const char *protocols[BQWS_MAX_PROTOCOLS];
    const char *url_str = em->connect_data;

    const char *ptr = url_str;
    for (size_t i = 0; i < em->num_protocols; i++) {
        ptr += strlen(ptr);
        protocols[i] = ptr;
    }

    int handle = pt_em_connect_websocket(em, url_str, protocols, em->num_protocols);
    em->handle = handle;

    bqws_allocator_free(&em->allocator, em->connect_data, em->connect_data_size);
    em->connect_data = NULL;

    pt_em_unlock(em);
}

static void pt_io_close(void *user, bqws_socket *ws)
{
    pt_em_socket *em = (pt_em_socket*)user;

    pt_em_mutex_lock(&em->ws_mutex);

    pt_em_partial *next = em->partial_first;
    while (next) {
        pt_em_partial *part = next;
        next = part->next;
        bqws_allocator_free(&em->allocator, part, sizeof(pt_em_partial) + part->size);
    }

    if (em->connect_data) {
        bqws_allocator_free(&em->allocator, em->connect_data, em->connect_data_size);
    }

    bool do_free = false;
    if (em->handle >= 0) {
        if (pt_em_try_free_websocket(em->handle, em)) {
            do_free = true;
        }
    } else {
        do_free = true;
    }

    em->ws = NULL;
    pt_em_mutex_unlock(&em->ws_mutex);

    if (do_free) {
        pt_em_free(em);
    }
}

static bqws_socket *pt_connect(const bqws_url *url, const bqws_pt_connect_opts *pt_opts, const bqws_opts *opts, const bqws_client_opts *client_opts)
{
    char url_str[2048];
    int len = snprintf(url_str, sizeof(url_str), "%s://%s:%d%s", url->scheme, url->host, url->port, url->path);
    if (len >= sizeof(url_str)) return NULL;

    bqws_opts opt;
    if (opts) {
        opt = *opts;
    } else {
        memset(&opt, 0, sizeof(opt));
    }

    bqws_client_opts copt;
    if (client_opts) {
        copt = *client_opts;
    } else {
        memset(&copt, 0, sizeof(copt));
    }

    opt.ping_interval = SIZE_MAX;
    opt.ping_response_timeout = SIZE_MAX;
    opt.close_timeout = SIZE_MAX;

    pt_em_socket *em = (pt_em_socket*)bqws_allocator_alloc(&opts->allocator, sizeof(pt_em_socket));
    memset(em, 0, sizeof(pt_em_socket));
    em->magic = BQWS_PT_EM_MAGIC;
    em->allocator = opts->allocator;

    opt.send_message_fn = &pt_send_message;
    opt.send_message_user = em;
    opt.io.user = em;
    opt.io.init_fn = &pt_io_init;
    opt.io.send_fn = &pt_io_send;
    opt.io.close_fn = &pt_io_close;
    opt.skip_handshake = true;

    bqws_socket *ws = bqws_new_client(&opt, &copt);
    if (!ws) {
        bqws_allocator_free(&opts->allocator, em, sizeof(pt_em_socket));
        return NULL;
    }

    bqws_direct_set_override_state(ws, BQWS_STATE_CONNECTING);

    // Retain connect data and connect at the first update
    size_t url_size = strlen(url_str) + 1;
    size_t protocol_size[BQWS_MAX_PROTOCOLS];
    size_t connect_data_size = url_size + 1;
    for (size_t i = 0; i < copt.num_protocols; i++) {
        protocol_size[i] = strlen(copt.protocols[i]) + 1;
        connect_data_size += protocol_size[i];
    }
    em->connect_data = (char*)bqws_allocator_alloc(&em->allocator, connect_data_size);
    em->num_protocols = copt.num_protocols;
    {
        char *ptr = em->connect_data;
        memcpy(ptr, url_str, url_size);
        ptr += url_size;
        for (size_t i = 0; i < copt.num_protocols; i++) {
            memcpy(ptr, copt.protocols[i], protocol_size[i]);
            ptr += protocol_size[i];
        }
    }

    pt_em_mutex_init(&em->ws_mutex);
    em->ws = ws;
    em->handle = -1;

    return ws;
}

static bqws_pt_server *pt_listen(const bqws_pt_listen_opts *opts)
{
    pt_fail_pt("pt_listen()", BQWS_PT_ERR_NO_SERVER_SUPPORT);
    return NULL;
}

static bqws_socket *pt_accept(bqws_pt_server *sv, const bqws_opts *opts, const bqws_server_opts *server_opts)
{
    return NULL;
}

static void pt_free_server(bqws_pt_server *sv)
{
}

static bqws_pt_address pt_get_address(const bqws_socket *ws)
{
    pt_em_socket *em = (pt_em_socket*)bqws_get_io_user(ws);
    bqws_assert(em && em->magic == BQWS_PT_EM_MAGIC);
    bqws_pt_address addr = { BQWS_PT_ADDRESS_WEBSOCKET };
    memcpy(addr.address, &em->handle, sizeof(int));
    return addr;
}


#elif (defined(_WIN32) || defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)))

// -- Shared for Windows and POSIX

static const uint8_t ipv4_mapped_ipv6_prefix[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
};

static void addr_parse_ipv4(bqws_pt_address *dst, const void *addr, uint16_t port_native)
{
    dst->port = port_native;
    dst->type = BQWS_PT_ADDRESS_IPV4;
    memcpy(dst->address, addr, 4);
}

static void addr_parse_ipv6(bqws_pt_address *dst, const void *addr, uint16_t port_native)
{
    dst->port = port_native;
    if (!memcmp(addr, ipv4_mapped_ipv6_prefix, sizeof(ipv4_mapped_ipv6_prefix))) {
        dst->type = BQWS_PT_ADDRESS_IPV4;
        memcpy(dst->address, (const char*)addr + 12, 4);
    } else {
        dst->type = BQWS_PT_ADDRESS_IPV6;
        memcpy(dst->address, addr, 16);
    }
}

#if defined(_WIN32)
// -- OS: Windows

#include <WinSock2.h>
#include <WS2tcpip.h>

#define _WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma comment(lib, "ws2_32") //< @r-lyeh removed .lib (tcc support)

typedef SOCKET os_socket;
#define OS_BAD_SOCKET INVALID_SOCKET

static void pt_fail_wsa(const char *func)
{
    t_err.function = func;
    t_err.type = BQWS_PT_ERRTYPE_WSA;
    t_err.data = (uint32_t)WSAGetLastError();
}

static void pt_sleep_ms(uint32_t ms)
{
    Sleep((DWORD)ms);
}

static bool os_init(const bqws_pt_init_opts *opts)
{
    WSADATA data;

    int res = WSAStartup(MAKEWORD(2,2), &data);
    if (res != 0) { pt_fail_wsa("WSAStartup()"); return false; }

    return true;
}

static void os_shutdown()
{
    WSACleanup();
}

static bool os_imp_config_data_socket(os_socket s)
{
    int res;

    // Set the socket to be non-blocking
    u_long nb_flag = 1;
    res = ioctlsocket(s, FIONBIO, &nb_flag);
    if (res != 0) { pt_fail_wsa("ioctlsocket(FIONBIO)"); return false; }

    // Disable Nagle's algorithm to make writes immediate
    BOOL nd_flag = 1;
    res = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char *)&nd_flag, sizeof(nd_flag));
    if (res != 0) { pt_fail_wsa("setsockopt(TCP_NODELAY)"); return false; }

    return true;
}

static os_socket os_imp_try_connect(ADDRINFOW *info, int family, ADDRINFOW **used)
{
    for (; info; info = info->ai_next) {
        if (info->ai_family != family) continue;

        SOCKET s = socket(family, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) { pt_fail_wsa("socket()"); return s; }
        int res = connect(s, info->ai_addr, (int)info->ai_addrlen);
        if (res == 0) {
            *used = info;
            return s;
        }
        pt_fail_wsa("connect()");
        closesocket(s);
    }

    return INVALID_SOCKET;
}

static void os_imp_parse_address(bqws_pt_address *dst, struct sockaddr *addr)
{
    if (addr->sa_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in*)addr;
        addr_parse_ipv4(dst, &sa->sin_addr, ntohs(sa->sin_port));
    } else if (addr->sa_family == AF_INET6) {
        struct sockaddr_in6 *sa = (struct sockaddr_in6*)addr;
        addr_parse_ipv6(dst, &sa->sin6_addr, ntohs(sa->sin6_port));
    }
}

static os_socket os_socket_connect(const bqws_url *url, bqws_pt_address *addr)
{
    wchar_t whost[256];
    char service[32];
    wchar_t wservice[32];

    snprintf(service, sizeof(service), "%u", url->port);
    int res = MultiByteToWideChar(CP_UTF8, 0, service, -1, wservice, sizeof(wservice) / sizeof(*wservice));
    if (res == 0) return OS_BAD_SOCKET;

    res = MultiByteToWideChar(CP_UTF8, 0, url->host, -1, whost, sizeof(whost) / sizeof(*whost));
    if (res == 0) return OS_BAD_SOCKET;

    ADDRINFOW hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    ADDRINFOW *info;
    res = GetAddrInfoW(whost, wservice, &hints, &info);
    if (res != 0) {
        pt_fail_wsa("GetAddrInfoW()");
        return INVALID_SOCKET;
    }

    ADDRINFOW *used_info = NULL;
    SOCKET s = os_imp_try_connect(info, AF_INET6, &used_info);
    if (s == INVALID_SOCKET) {
        s = os_imp_try_connect(info, AF_INET, &used_info);
    }

    if (s != INVALID_SOCKET) {
        os_imp_parse_address(addr, used_info->ai_addr);
    }

    FreeAddrInfoW(info);

    if (!os_imp_config_data_socket(s)) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    return s;
}

static os_socket os_socket_listen(const bqws_pt_listen_opts *pt_opts)
{
    os_socket s = OS_BAD_SOCKET;
    int res;

    do {
        s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) { pt_fail_wsa("socket()"); break; }

        // Make sure the socket supports both IPv4 and IPv6
        DWORD ipv6_flag = 0;
        res = setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&ipv6_flag, sizeof(ipv6_flag));
        if (res != 0) { pt_fail_wsa("setsockopt(IPPROTO_IPV6)"); break; }

        // Set the socket to be non-blocking
        u_long nb_flag = 1;
        res = ioctlsocket(s, FIONBIO, &nb_flag);
        if (res != 0) { pt_fail_wsa("ioctlsocket(FIONBIO)"); break; }

        struct sockaddr_in6 addr = { 0 };
        addr.sin6_family = AF_INET6;
        addr.sin6_addr = in6addr_any;
        addr.sin6_port = htons(pt_opts->port);

        res = bind(s, (struct sockaddr*)&addr, sizeof(addr));
        if (res != 0) { pt_fail_wsa("bind()"); break; }

        res = listen(s, (int)pt_opts->backlog);
        if (res != 0) { pt_fail_wsa("listen()"); break; }

        return s;

    } while (false);

    if (s != INVALID_SOCKET) closesocket(s);
    return INVALID_SOCKET;
}

static os_socket os_socket_accept(os_socket listen_s, bqws_pt_address *addr)
{
    struct sockaddr_in6 addr_in;
    int addr_len = sizeof(addr_in);
    SOCKET s = accept(listen_s, (struct sockaddr*)&addr_in, &addr_len);
    if (s == INVALID_SOCKET) return INVALID_SOCKET;

    os_imp_parse_address(addr, (struct sockaddr*)&addr_in);

    if (!os_imp_config_data_socket(s)) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    return s;
}

static size_t os_socket_recv(os_socket s, void *data, size_t size)
{
    if (size > INT_MAX) size = INT_MAX;

    int res = recv(s, (char*)data, (int)size, 0);
    if (res < 0) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) return 0;
        t_err.function = "recv()";
        t_err.type = BQWS_PT_ERRTYPE_WSA;
        t_err.data = err;
        return SIZE_MAX;
    }
    return (size_t)res;
}

static size_t os_socket_send(os_socket s, const void *data, size_t size)
{
    if (size > INT_MAX) size = INT_MAX;

    int res = send(s, (const char*)data, (int)size, 0);
    if (res < 0) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) return 0;
        t_err.function = "send()";
        t_err.type = BQWS_PT_ERRTYPE_WSA;
        t_err.data = err;
        return SIZE_MAX;
    }

    return (size_t)res;
}

static void os_socket_close(os_socket s)
{
    shutdown(s, SD_BOTH);
    closesocket(s);
}

#else

#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>

// TODO: Guard this with macros?
#if 1
    #include <netdb.h>
    #define BQWS_HAS_GAI_STRERROR
#endif

typedef int os_socket;
#define OS_BAD_SOCKET -1

static void pt_fail_posix(const char *func)
{
    t_err.function = func;
    t_err.type = BQWS_PT_ERRTYPE_POSIX;
    t_err.data = errno;
}

static void pt_sleep_ms(uint32_t ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    while (nanosleep(&ts, &ts)) { }
}

static bool os_init(const bqws_pt_init_opts *opts)
{
    return true;
}

static void os_shutdown()
{
}

static bool os_imp_config_data_socket(os_socket s)
{
    int res;

    // Set the socket to be non-blocking
    int nb_flag = 1;
    res = ioctl(s, FIONBIO, &nb_flag);
    if (res != 0) { pt_fail_posix("ioctl(FIONBIO)"); return false; }

    // Disable Nagle's algorithm to make writes immediate
    int nd_flag = 1;
    res = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &nd_flag, sizeof(nd_flag));
    if (res != 0) { pt_fail_posix("setsockopt(TCP_NODELAY)"); return false; }

    return true;
}

static os_socket os_imp_try_connect(struct addrinfo *info, int family, struct addrinfo **used)
{
    for (; info; info = info->ai_next) {
        if (info->ai_family != family) continue;

        int s = socket(family, SOCK_STREAM, IPPROTO_TCP);
        if (s == -1) { pt_fail_posix("socket()"); return s; }
        int res = connect(s, info->ai_addr, (int)info->ai_addrlen);
        if (res == 0) {
            *used = info;
            return s;
        }
        pt_fail_posix("connect()");
        close(s);
    }

    return -1;
}

static void os_imp_parse_address(bqws_pt_address *dst, struct sockaddr *addr)
{
    if (addr->sa_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in*)addr;
        addr_parse_ipv4(dst, &sa->sin_addr, ntohs(sa->sin_port));
    } else if (addr->sa_family == AF_INET6) {
        struct sockaddr_in6 *sa = (struct sockaddr_in6*)addr;
        addr_parse_ipv6(dst, &sa->sin6_addr, ntohs(sa->sin6_port));
    }
}

static os_socket os_socket_connect(const bqws_url *url, bqws_pt_address *addr)
{
    char service[64];
    snprintf(service, sizeof(service), "%d", (int)url->port);

    struct addrinfo hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *info;
    int res = getaddrinfo(url->host, service, &hints, &info);
    if (res != 0) {
        t_err.function = "getaddrinfo()";
        t_err.type = BQWS_PT_ERRTYPE_GETADDRINFO;
        t_err.data = res;
        return -1;
    }

    struct addrinfo *used_info = NULL;
    int s = os_imp_try_connect(info, AF_INET6, &used_info);
    if (s == -1) {
        s = os_imp_try_connect(info, AF_INET, &used_info);
    }

    if (s != -1) {
        os_imp_parse_address(addr, used_info->ai_addr);
    }

    freeaddrinfo(info);

    if (!os_imp_config_data_socket(s)) {
        close(s);
        return -1;
    }

    return s;
}

static os_socket os_socket_listen(const bqws_pt_listen_opts *pt_opts)
{
    os_socket s = -1;
    int res;

    do {
        s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (s == -1) { pt_fail_posix("socket()"); break; }

        // Make sure the socket supports both IPv4 and IPv6
        int ipv6_flag = 0;
        res = setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_flag, sizeof(ipv6_flag));
        if (res != 0) { pt_fail_posix("setsockopt(IPPROTO_IPV6)"); break; }

        if (pt_opts->reuse_port) {
            int reuse_flag = 1;
            setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &reuse_flag, sizeof(reuse_flag));
        }

        // Set the socket to be non-blocking
        int nb_flag = 1;
        res = ioctl(s, FIONBIO, &nb_flag);
        if (res != 0) { pt_fail_posix("ioctl(FIONBIO)"); break; }

        struct sockaddr_in6 addr = { 0 };
        addr.sin6_family = AF_INET6;
        addr.sin6_addr = in6addr_any;
        addr.sin6_port = htons(pt_opts->port);

        res = bind(s, (struct sockaddr*)&addr, sizeof(addr));
        if (res != 0) { pt_fail_posix("bind()"); break; }

        res = listen(s, (int)pt_opts->backlog);
        if (res != 0) { pt_fail_posix("listen()"); break; }

        return s;

    } while (false);

    if (s != -1) close(s);
    return -1;
}

static os_socket os_socket_accept(os_socket listen_s, bqws_pt_address *addr)
{
    struct sockaddr_in6 addr_in;
    socklen_t addr_len = sizeof(addr_in);
    int s = accept(listen_s, (struct sockaddr*)&addr_in, &addr_len);
    if (s == -1) return -1;

    os_imp_parse_address(addr, (struct sockaddr*)&addr_in);

    if (!os_imp_config_data_socket(s)) {
        close(s);
        return -1;
    }

    return s;
}

static size_t os_socket_recv(os_socket s, void *data, size_t size)
{
    int res = read(s, data, size);
    if (res < 0) {
        int err = errno;
        if (err == EAGAIN || err == EWOULDBLOCK) return 0;
        t_err.function = "read()";
        t_err.type = BQWS_PT_ERRTYPE_POSIX;
        t_err.data = err;
        return SIZE_MAX;
    }
    return (size_t)res;
}

static size_t os_socket_send(os_socket s, const void *data, size_t size)
{
    int res = write(s, data, size);
    if (res < 0) {
        int err = errno;
        if (err == EAGAIN || err == EWOULDBLOCK) return 0;
        t_err.function = "write()";
        t_err.type = BQWS_PT_ERRTYPE_POSIX;
        t_err.data = err;
        return SIZE_MAX;
    }
    return (size_t)res;
}

static void os_socket_close(os_socket s)
{
    shutdown(s, SHUT_RDWR);
    close(s);
}

#endif

// -- TLS

#if BQWS_PT_USE_OPENSSL

#include <openssl/ssl.h>
#include <openssl/err.h>
#define BQWS_PT_HAS_OPENSSL

typedef struct {
    bool connected;
    SSL *ssl;
} pt_tls;

typedef struct {
    SSL_CTX *ctx;
} pt_tls_server;

typedef struct {
    SSL_CTX *client_ctx;
} pt_tls_global;

static pt_tls_global g_tls;

static void pt_fail_ssl(const char *func)
{
    t_err.function = func;
    t_err.type = BQWS_PT_ERRTYPE_OPENSSL;
    t_err.data = ERR_get_error();
}

static bool tls_init(const bqws_pt_init_opts *opts)
{
    int res;

    SSL_library_init();

    g_tls.client_ctx = SSL_CTX_new(SSLv23_client_method());
    if (!g_tls.client_ctx) { pt_fail_ssl("SSL_CTX_new()"); return false; }

    if (opts->ca_filename) {
        res = SSL_CTX_load_verify_locations(g_tls.client_ctx, opts->ca_filename, NULL);
        if (!res) { pt_fail_ssl("SSL_CTX_load_verify_locations()"); return false; }
    }

    long flags = SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(g_tls.client_ctx, flags);

    long mode = SSL_MODE_ENABLE_PARTIAL_WRITE;
    SSL_CTX_set_mode(g_tls.client_ctx, mode);

    return true;
}

static void tls_shutdown()
{
    SSL_CTX_free(g_tls.client_ctx);
}

static bool tls_init_client(pt_tls *tls, os_socket s, const bqws_pt_connect_opts *pt_opts, const bqws_opts *opts, const bqws_client_opts *client_opts)
{
    tls->ssl = SSL_new(g_tls.client_ctx);
    if (!tls->ssl) return false;

    BIO *bio = BIO_new_socket((int)s, 0);
    if (!bio) return false;

    // SSL_free() will free the BIO internally
    SSL_set_bio(tls->ssl, bio, bio);

    if (!pt_opts->insecure_no_verify_host) {
        const char *host = client_opts->host;
        if (!host || !*host) return false;

        X509_VERIFY_PARAM *param = SSL_get0_param(tls->ssl);
        X509_VERIFY_PARAM_set_hostflags(param, /* X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS */ 0x4);
        X509_VERIFY_PARAM_set1_host(param, host, 0);

        SSL_set_verify(tls->ssl, SSL_VERIFY_PEER, NULL);
    }
    return true;
}

static bool tls_init_server(pt_tls_server *tls, const bqws_pt_listen_opts *pt_opts)
{
    tls->ctx = SSL_CTX_new(SSLv23_server_method());
    if (!tls->ctx) { pt_fail_ssl("SSL_CTX_new()"); return false; }

    int res;

    if (pt_opts->certificate_file) {
        res = SSL_CTX_use_certificate_file(tls->ctx, pt_opts->certificate_file, SSL_FILETYPE_PEM);
        if (!res) { pt_fail_ssl("SSL_CTX_use_certificate_file()"); return false; }
    }

    if (pt_opts->private_key_file) {
        res = SSL_CTX_use_PrivateKey_file(tls->ctx, pt_opts->private_key_file, SSL_FILETYPE_PEM);
        if (!res) { pt_fail_ssl("SSL_CTX_use_PrivateKey_file()"); return false; }
    }

    long flags = SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(tls->ctx, flags);

    long mode = SSL_MODE_ENABLE_PARTIAL_WRITE;
    SSL_CTX_set_mode(tls->ctx, mode);

    return true;
}

static void tls_free_server(pt_tls_server *tls)
{
    if (tls->ctx) {
        SSL_CTX_free(tls->ctx);
    }
}

static bool tls_init_accept(pt_tls *tls, pt_tls_server *tls_server, os_socket s)
{
    tls->ssl = SSL_new(tls_server->ctx);
    if (!tls->ssl) return false;

    BIO *bio = BIO_new_socket((int)s, 0);
    if (!bio) return false;

    // SSL_free() will free the BIO internally
    SSL_set_bio(tls->ssl, bio, bio);

    return true;
}

static void tls_free(pt_tls *tls)
{
    if (tls->ssl) SSL_free(tls->ssl);
}

static bool tls_imp_connect(pt_tls *tls)
{
    int res = SSL_connect(tls->ssl);
    if (res <= 0) {
        int err = SSL_get_error(tls->ssl, res);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            // Did not fail, just did not connect yet
            return true;
        } else {
            pt_fail_ssl("SSL_connect()");
            return false;
        }
    }
    tls->connected = true;
    return true;
}

static size_t tls_send(pt_tls *tls, const void *data, size_t size)
{
    if (!tls->connected) {
        if (!tls_imp_connect(tls)) return SIZE_MAX;
        if (!tls->connected) return 0;
    }

    if (size > INT_MAX) size = INT_MAX;
    int res = SSL_write(tls->ssl, data, (int)size);
    if (res <= 0) {
        int err = SSL_get_error(tls->ssl, res);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            return 0;
        } else {
            pt_fail_ssl("SSL_write()");
            return SIZE_MAX;
        }
    }
    return (size_t)res;
}

static size_t tls_recv(pt_tls *tls, void *data, size_t size)
{
    if (!tls->connected) {
        if (!tls_imp_connect(tls)) return SIZE_MAX;
        if (!tls->connected) return 0;
    }

    if (size > INT_MAX) size = INT_MAX;
    int res = SSL_read(tls->ssl, data, (int)size);
    if (res <= 0) {
        int err = SSL_get_error(tls->ssl, res);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            return 0;
        } else {
            pt_fail_ssl("SSL_read()");
            return SIZE_MAX;
        }
    }
    return (size_t)res;
}

#else

typedef struct {
    int unused;
} pt_tls;

typedef struct {
    int unused;
} pt_tls_server;

static bool tls_init(const bqws_pt_init_opts *opts)
{
    return true;
}

static void tls_shutdown()
{
}

static bool tls_init_client(pt_tls *tls, os_socket s, const bqws_pt_connect_opts *pt_opts, const bqws_opts *opts, const bqws_client_opts *client_opts)
{
    pt_fail_pt("tls_init_client()", BQWS_PT_ERR_NO_TLS);
    return false;
}

static bool tls_init_server(pt_tls_server *tls, const bqws_pt_listen_opts *pt_opts)
{
    pt_fail_pt("tls_init_client()", BQWS_PT_ERR_NO_TLS);
    return false;
}

static void tls_free_server(pt_tls_server *tls)
{
}

static bool tls_init_accept(pt_tls *tls, pt_tls_server *tls_server, os_socket s)
{
    bqws_assert(0 && "Should never get here");
    return false;
}

static void tls_free(pt_tls *tls)
{
}

static size_t tls_send(pt_tls *tls, const void *data, size_t size)
{
    bqws_assert(0 && "Should never get here");
    return SIZE_MAX;
}

static size_t tls_recv(pt_tls *tls, void *data, size_t size)
{
    bqws_assert(0 && "Should never get here");
    return SIZE_MAX;
}

#endif

#if defined(__APPLE__)

// -- CF socket implementation

#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFStream.h>
#include <CFNetwork/CFNetwork.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

typedef struct {
    bool enabled;
    bool has_address;
    bool set_nonblocking;
    CFWriteStreamRef write;
    CFReadStreamRef read;
} pt_cf;

static void cf_free(pt_cf *cf)
{
    if (cf->read) CFRelease(cf->read);
    if (cf->write) CFRelease(cf->write);
}

static size_t cf_send(pt_cf *cf, const void *data, size_t size)
{
    if (size == 0) return 0;

    switch (CFWriteStreamGetStatus(cf->write)) {
        case kCFStreamStatusOpening: return 0;
        case kCFStreamStatusError: case kCFStreamStatusClosed: return SIZE_MAX;
        default: if (!CFWriteStreamCanAcceptBytes(cf->write)) return 0;
    }

    if (!cf->set_nonblocking) {
        cf->set_nonblocking = true;
        CFDataRef socket_data = (CFDataRef)CFWriteStreamCopyProperty(cf->write, kCFStreamPropertySocketNativeHandle);
        if (socket_data) {
            CFSocketNativeHandle s = -1;
            CFDataGetBytes(socket_data, CFRangeMake(0, sizeof(CFSocketNativeHandle)), (UInt8*)&s);
            if (s >= 0) {
                int nd_flag = 1;
                setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &nd_flag, sizeof(nd_flag));
            }
            CFRelease(socket_data);
        }
    }

    CFIndex res = CFWriteStreamWrite(cf->write, (const UInt8*)data, size);
    if (res < 0) return SIZE_MAX;
    return (size_t)res;
}

static size_t cf_recv(pt_cf *cf, void *data, size_t max_size)
{
    if (max_size == 0) return 0;

    switch (CFReadStreamGetStatus(cf->read)) {
        case kCFStreamStatusOpening: return 0;
        case kCFStreamStatusError: case kCFStreamStatusClosed: return SIZE_MAX;
        default: if (!CFReadStreamHasBytesAvailable(cf->read)) return 0;
    }

    CFIndex res = CFReadStreamRead(cf->read, (UInt8*)data, (CFIndex)max_size);
    if (res < 0) return SIZE_MAX;
    return (size_t)res;
}

static bool cf_connect(const bqws_url *url, pt_cf *cf)
{
    CFAllocatorRef ator = kCFAllocatorDefault;

    do {
        memset(cf, 0, sizeof(pt_cf));

        CFStringRef host_ref = CFStringCreateWithCString(ator, url->host, kCFStringEncodingUTF8);
        CFStreamCreatePairWithSocketToHost(ator, host_ref, url->port, &cf->read, &cf->write);
        CFRelease(host_ref);

        if (!cf->read || !cf->write) {
            pt_fail_pt("CFStreamCreatePairWithSocketToHost()", BQWS_PT_ERR_OUT_OF_MEMORY);
            break;
        }

        if (url->secure) {
            CFStringRef keys[] = { kCFStreamPropertySocketSecurityLevel };
            CFStringRef values[] = { kCFStreamSocketSecurityLevelTLSv1 };
            CFDictionaryRef dict = CFDictionaryCreate(ator, (const void**)keys, (const void**)values, 1,
                &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

            CFWriteStreamSetProperty(cf->write, kCFStreamPropertySSLSettings, dict);
            CFReadStreamSetProperty(cf->read, kCFStreamPropertySSLSettings, dict);

            CFRelease(dict);
        }

        CFWriteStreamOpen(cf->write);
        CFReadStreamOpen(cf->read);

        cf->enabled = true;
        return true;
    } while (false);

    if (cf) cf_free(cf);
    return false;
}

static void cf_get_address(pt_cf *cf, bqws_pt_address *address)
{
    if (!cf->has_address) {

        CFDataRef socket_data = (CFDataRef)CFWriteStreamCopyProperty(cf->write, kCFStreamPropertySocketNativeHandle);
        if (socket_data) {
            CFSocketNativeHandle s = -1;
            CFDataGetBytes(socket_data, CFRangeMake(0, sizeof(CFSocketNativeHandle)), (UInt8*)&s);
            if (s >= 0) {
                struct sockaddr_in6 addr;
                socklen_t addr_len = sizeof(addr);
                if (getsockname(s, (struct sockaddr*)&addr, &addr_len) == 0
                    && addr_len >= sizeof(struct sockaddr_in)) {
                    os_imp_parse_address(address, (struct sockaddr*)&addr);
                }
            }
            CFRelease(socket_data);
        }

        cf->has_address = true;
    }
}

#define cf_enabled(cf) ((cf)->enabled)

#else

typedef struct {
    int unused;
} pt_cf;

static void cf_free(pt_cf *cf) { }
static size_t cf_send(pt_cf *cf, const void *data, size_t size) { return SIZE_MAX; }
static size_t cf_recv(pt_cf *cf, void *data, size_t max_size) { return SIZE_MAX; }
static bool cf_connect(const bqws_url *url, pt_cf *cf) { return false; }
static void cf_get_address(pt_cf *cf, bqws_pt_address *address) { }

#define cf_enabled(cf) (false)

#endif

// -- POSIX socket implementation

typedef struct {
    uint32_t magic;

    os_socket s;
    size_t send_size;
    char send_buf[512];

    bool secure;
    pt_tls tls;

    pt_cf cf;

    bqws_pt_address address;
    bqws_allocator allocator;
} pt_io;

struct bqws_pt_server {
    uint32_t magic;

    os_socket s;
    bool secure;
    pt_tls_server tls;

    bqws_allocator allocator;
};

static size_t io_imp_send(pt_io *io, const void *data, size_t size)
{
    if (size == 0) return 0;

    if (cf_enabled(&io->cf)) {
        return cf_send(&io->cf, data, size);
    } else if (io->secure) {
        return tls_send(&io->tls, data, size);
    } else {
        return os_socket_send(io->s, data, size);
    }
}

static bool io_flush_imp(pt_io *io)
{
    size_t size = io->send_size;
    size_t sent = io_imp_send(io, io->send_buf, size);
    if (sent == 0) return true;
    if (sent == SIZE_MAX) return false;

    size_t left = size - sent;
    io->send_size = left;
    if (left > 0) {
        memmove(io->send_buf, io->send_buf + sent, left);
    }
    return true;
}

static size_t io_push_imp(pt_io *io, const char *ptr, const char *end)
{
    size_t size = end - ptr;
    size_t offset = io->send_size;
    size_t to_copy = sizeof(io->send_buf) - offset;
    if (to_copy > size) to_copy = size;
    memcpy(io->send_buf + offset, ptr, to_copy);
    io->send_size += to_copy;
    return to_copy;
}

static void io_free(pt_io *io)
{
    if (cf_enabled(&io->cf)) cf_free(&io->cf);
    if (io->secure) tls_free(&io->tls);
    if (io->s != OS_BAD_SOCKET) os_socket_close(io->s);
    io->magic = BQWS_PT_DELETED_MAGIC;

    bqws_allocator allocator = io->allocator;
    bqws_allocator_free(&allocator, io, sizeof(pt_io));
}

static size_t pt_io_send(void *user, bqws_socket *ws, const void *data, size_t size)
{
    if (size == 0) return 0;

    pt_io *io = (pt_io*)user;

    const char *begin = (const char*)data, *end = begin + size;
    const char *ptr = begin;

    // TODO: Try 2*sizeof(io->send_buf) - io->send_size
    if (size <= sizeof(io->send_buf)) {
        ptr += io_push_imp(io, ptr, end);
        if (ptr != end) {
            if (!io_flush_imp(io)) return SIZE_MAX;
            ptr += io_push_imp(io, ptr, end);
        }
    } else {
        if (io->send_size > 0) {
            ptr += io_push_imp(io, ptr, end);
            if (!io_flush_imp(io)) return SIZE_MAX;
        }

        size_t sent = io_imp_send(io, ptr, end - ptr);
        if (sent == SIZE_MAX) return SIZE_MAX;
        ptr += sent;
    }

    return ptr - begin;
}

static size_t pt_io_recv(void *user, bqws_socket *ws, void *data, size_t max_size, size_t min_size)
{
    if (max_size == 0) return 0;

    pt_io *io = (pt_io*)user;
    if (cf_enabled(&io->cf)) {
        return cf_recv(&io->cf, data, max_size);
    } else if (io->secure) {
        return tls_recv(&io->tls, data, max_size);
    } else {
        return os_socket_recv(io->s, data, max_size);
    }
}

static bool pt_io_flush(void *user, bqws_socket *ws)
{
    pt_io *io = (pt_io*)user;
    return io_flush_imp(io);
}

static void pt_io_close(void *user, bqws_socket *ws)
{
    pt_io *io = (pt_io*)user;
    io_free(io);
}

static bool pt_init(const bqws_pt_init_opts *opts)
{
    if (!os_init(opts)) return false;
    if (!tls_init(opts)) {
        os_shutdown();
        return false;
    }

    return true;
}

static void pt_shutdown()
{
    tls_shutdown();
    os_shutdown();
}

static bqws_socket *pt_connect(const bqws_url *url, const bqws_pt_connect_opts *pt_opts, const bqws_opts *opts, const bqws_client_opts *client_opts)
{
    pt_io *io = NULL;

    do {
        io = (pt_io*)bqws_allocator_alloc(&opts->allocator, sizeof(pt_io));
        if (!io) break;

        memset(io, 0, sizeof(pt_io));
        io->allocator = opts->allocator;
        io->s = OS_BAD_SOCKET;
    io->magic = BQWS_PT_IO_MAGIC;

        if (!cf_connect(url, &io->cf)) {
            bqws_pt_address addr = { BQWS_PT_ADDRESS_UNKNOWN };
            io->s = os_socket_connect(url, &addr);
            if (io->s == OS_BAD_SOCKET) break;

            io->address = addr;

            if (url->secure) {
                io->secure = true;
                if (!tls_init_client(&io->tls, io->s, pt_opts, opts, client_opts)) break;
            }
        }

        bqws_opts opt;
        if (opts) {
            opt = *opts;
        } else {
            memset(&opt, 0, sizeof(opt));
        }

        opt.io.user = io;
        opt.io.send_fn = &pt_io_send;
        opt.io.recv_fn = &pt_io_recv;
        opt.io.flush_fn = &pt_io_flush;
        opt.io.close_fn = &pt_io_close;

        bqws_socket *ws = bqws_new_client(&opt, client_opts);
        if (!ws) break;

        return ws;

    } while (false);

    if (io) io_free(io);
    return NULL;
}

static bqws_pt_server *pt_listen(const bqws_pt_listen_opts *pt_opts)
{
    bqws_pt_server *sv = (bqws_pt_server*)bqws_allocator_alloc(&pt_opts->allocator, sizeof(bqws_pt_server));
    if (!sv) { pt_fail_pt("pt_listen()", BQWS_PT_ERR_OUT_OF_MEMORY); return NULL; }
    memset(sv, 0, sizeof(bqws_pt_server));
    sv->magic = BQWS_PT_SERVER_MAGIC;
    sv->allocator = pt_opts->allocator;

    if (pt_opts->secure) {
        sv->secure = true;
        if (!tls_init_server(&sv->tls, pt_opts)) {
            bqws_allocator_free(&pt_opts->allocator, sv, sizeof(bqws_pt_server));
            return NULL;
        }
    }

    sv->s = os_socket_listen(pt_opts);
    if (sv->s == OS_BAD_SOCKET) {
        bqws_allocator_free(&pt_opts->allocator, sv, sizeof(bqws_pt_server));
        return NULL;
    }

    return sv;
}

static bqws_socket *pt_accept(bqws_pt_server *sv, const bqws_opts *opts, const bqws_server_opts *server_opts)
{
    bqws_assert(sv && sv->magic == BQWS_PT_SERVER_MAGIC);

    bqws_pt_address addr = { BQWS_PT_ADDRESS_UNKNOWN };
    os_socket s = os_socket_accept(sv->s, &addr);
    if (s == OS_BAD_SOCKET) return NULL;

    pt_io *io = NULL;

    do {
        io = (pt_io*)bqws_allocator_alloc(&opts->allocator, sizeof(pt_io));
        if (!io) break;

        memset(io, 0, sizeof(pt_io));
        io->magic = BQWS_PT_IO_MAGIC;
        io->s = s;
        io->allocator = opts->allocator;

        io->address = addr;
        s = OS_BAD_SOCKET;

        if (sv->secure) {
            io->secure = true;
            if (!tls_init_accept(&io->tls, &sv->tls, s)) break;
        }

        bqws_opts opt;
        if (opts) {
            opt = *opts;
        } else {
            memset(&opt, 0, sizeof(opt));
        }

        opt.io.user = io;
        opt.io.send_fn = &pt_io_send;
        opt.io.recv_fn = &pt_io_recv;
        opt.io.flush_fn = &pt_io_flush;
        opt.io.close_fn = &pt_io_close;

        bqws_socket *ws = bqws_new_server(&opt, server_opts);
        if (!ws) break;

        return ws;

    } while (false);

    if (io) io_free(io);
    os_socket_close(s);
    return NULL;
}

static void pt_free_server(bqws_pt_server *sv)
{
    bqws_assert(sv && sv->magic == BQWS_PT_SERVER_MAGIC);

    if (sv->secure) {
        tls_free_server(&sv->tls);
    }
    os_socket_close(sv->s);
    sv->magic = BQWS_PT_DELETED_MAGIC;

    bqws_allocator allocator = sv->allocator;
    bqws_allocator_free(&allocator, sv, sizeof(bqws_pt_server));
}

static bqws_pt_address pt_get_address(const bqws_socket *ws)
{
    pt_io *io = (pt_io*)bqws_get_io_user(ws);
    bqws_assert(io && io->magic == BQWS_PT_IO_MAGIC);
        if (cf_enabled(&io->cf)) cf_get_address(&io->cf, &io->address);

    return io->address;
}

#else
    #error "Unsupported platform"
#endif

// -- API

bool bqws_pt_init(const bqws_pt_init_opts *opts)
{
    bqws_pt_init_opts opt;

    if (opts) {
        opt = *opts;
    } else {
        memset(&opt, 0, sizeof(opt));
    }

    return pt_init(&opt);
}

void bqws_pt_shutdown()
{
    pt_shutdown();
}

void bqws_pt_clear_error()
{
    t_err.function = NULL;
    t_err.type = BQWS_PT_ERRTYPE_NONE;
    t_err.data = 0;
}

bool bqws_pt_get_error(bqws_pt_error *err)
{
    if (t_err.type == BQWS_PT_ERRTYPE_NONE) return false;
    if (err) *err = t_err;
    return true;
}

bqws_socket *bqws_pt_connect(const char *url, const bqws_pt_connect_opts *pt_opts, const bqws_opts *opts, const bqws_client_opts *client_opts)
{
    bqws_pt_clear_error();

    bqws_url parsed_url;
    if (!bqws_parse_url(&parsed_url, url)) {
        pt_fail_pt("bqws_parse_url()", BQWS_PT_ERR_BAD_URL);
        return NULL;
    }
    return bqws_pt_connect_url(&parsed_url, pt_opts, opts, client_opts);
}

bqws_socket *bqws_pt_connect_url(const bqws_url *url, const bqws_pt_connect_opts *pt_opts, const bqws_opts *opts, const bqws_client_opts *client_opts)
{
    bqws_pt_clear_error();

    bqws_pt_connect_opts popt;
    if (pt_opts) {
        popt = *pt_opts;
    } else {
        memset(&popt, 0, sizeof(popt));
    }

    bqws_opts opt;
    if (opts) {
        opt = *opts;
    } else {
        memset(&opt, 0, sizeof(opt));
    }

    bqws_client_opts copt;
    if (client_opts) {
        copt = *client_opts;
    } else {
        memset(&copt, 0, sizeof(copt));
    }

    if (!copt.host) copt.host = url->host;
    if (!copt.path) copt.path = url->path;

    return pt_connect(url, &popt, &opt, &copt);
}

bqws_pt_server *bqws_pt_listen(const bqws_pt_listen_opts *pt_opts)
{
    bqws_pt_clear_error();

    bqws_pt_listen_opts opts;
    if (pt_opts) {
        opts = *pt_opts;
    } else {
        memset(&opts, 0, sizeof(opts));
    }

    if (!opts.port) {
        opts.port = opts.secure ? 443 : 80;
    }
    if (!opts.backlog) {
        opts.backlog = 128;
    } else if (opts.backlog > INT32_MAX) {
        opts.backlog = INT32_MAX;
    }

    return pt_listen(&opts);
}

void bqws_pt_free_server(bqws_pt_server *sv)
{
    if (!sv) return;
    pt_free_server(sv);
}

bqws_socket *bqws_pt_accept(bqws_pt_server *sv, const bqws_opts *opts, const bqws_server_opts *server_opts)
{
    bqws_pt_clear_error();

    bqws_opts opt;
    if (opts) {
        opt = *opts;
    } else {
        memset(&opt, 0, sizeof(opt));
    }

    return pt_accept(sv, &opt, server_opts);
}

bqws_pt_address bqws_pt_get_address(const bqws_socket *ws)
{
    bqws_assert(ws);
    if (bqws_get_io_closed(ws)) {
        bqws_pt_address null_addr = { BQWS_PT_ADDRESS_UNKNOWN };
        return null_addr;
    }
    return pt_get_address(ws);
}

void bqws_pt_format_address(char *dst, size_t size, const bqws_pt_address *addr)
{
    if (size == 0) return;

    switch (addr->type) {

    case BQWS_PT_ADDRESS_UNKNOWN:
        snprintf(dst, size, "(unknown)");
        break;

    case BQWS_PT_ADDRESS_WEBSOCKET:
        snprintf(dst, size, "websocket[%d]", *(int*)addr->address);
        break;

    case BQWS_PT_ADDRESS_IPV4:
        snprintf(dst, size, "%u.%u.%u.%u:%u",
            (unsigned)addr->address[0], (unsigned)addr->address[1],
            (unsigned)addr->address[2], (unsigned)addr->address[3],
            (unsigned)addr->port);
        break;

    case BQWS_PT_ADDRESS_IPV6:
        {
            const uint8_t *a = addr->address;

            // Find the leftmost longest run of zeros that's longer than one
            size_t longest_begin = SIZE_MAX;
            size_t longest_zeros = 1;
            {
                size_t zeros = 0;
                size_t zero_begin = 0;
                for (size_t i = 0; i < 16; i += 2) {
                    if (a[i] == 0 && a[i + 1] == 0) {
                        if (zeros == 0) {
                            zero_begin = i;
                        }
                        zeros++;
                        if (zeros > longest_zeros) {
                            longest_begin = zero_begin;
                            longest_zeros = zeros;
                        }
                    } else {
                        zeros = 0;
                    }
                }
            }

            bool need_colon = false;
            char *ptr = dst, *end = dst + size;
            ptr += snprintf(ptr, end - ptr, "[");
            for (size_t i = 0; i < 16; i += 2) {
                if (i == longest_begin) {
                    ptr += snprintf(ptr, end - ptr, "::");
                    need_colon = false;
                    i += (longest_zeros - 1) * 2;
                    continue;
                }

                unsigned v = (unsigned)a[i] << 8 | (unsigned)a[i + 1];
                ptr += snprintf(ptr, end - ptr, need_colon ? ":%x" : "%x", v);

                need_colon = true;
            }
            ptr += snprintf(ptr, end - ptr, "]:%u", (unsigned)addr->port);
        }
        break;

    default:
        snprintf(dst, size, "(bad type)");
        break;
    }
}

void bqws_pt_get_error_desc(char *dst, size_t size, const bqws_pt_error *err)
{
    if (size == 0) return;

    *dst = '\0';

    switch (err->type) {

    case BQWS_PT_ERRTYPE_NONE:
        // Nop, empty description
        break;

    case BQWS_PT_ERRTYPE_PT:
        {
            const char *str = bqws_pt_error_code_str((bqws_pt_error_code)err->data);
            size_t len = strlen(str);
            if (len > size) len = size;
            memcpy(dst, str, len);
            dst[len] = '\0';
        }
        break;

    case BQWS_PT_ERRTYPE_WSA:
        #if defined(_WIN32)
        {
            wchar_t *buf;
            FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, (DWORD)err->data,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPWSTR)&buf, 0, NULL);

            int int_size = size < INT_MAX ? (int)size : INT_MAX;
            int res = WideCharToMultiByte(CP_UTF8, 0, buf, -1, dst, int_size, NULL, NULL);
            if (res == 0) {
                *dst = '\0';
            } else if (res >= int_size) {
                dst[int_size] = '\0';
            }
        }
        #endif
        break;

    case BQWS_PT_ERRTYPE_POSIX:
        {
        #if defined(_WIN32)
            strerror_s(dst, size, (int)err->data);
        #else
            const char *ptr = (const char*)(uintptr_t)strerror_r((int)err->data, dst, size);
            if (dst[0] == '\0' && ptr != dst) {
                const char *err_str = strerror((int)err->data);
                size_t len = strlen(err_str);
                if (len >= size - 1) len = size - 1;
                memcpy(dst, err_str, len);
                dst[len] = '\0';
            }
        #endif
        }
        break;

    case BQWS_PT_ERRTYPE_GETADDRINFO:
        #if defined(BQWS_HAS_GAI_STRERROR)
        {
            const char *str = gai_strerror((int)err->data);
            size_t len = strlen(str);
            if (len > size) len = size;
            memcpy(dst, str, len);
            dst[len] = '\0';
        }
        #endif
        break;

    case BQWS_PT_ERRTYPE_OPENSSL:
        #if defined(BQWS_PT_HAS_OPENSSL) && !defined(__EMSCRIPTEN__)
            ERR_error_string_n((unsigned long)err->data, dst, size);
        #endif
        break;

    }
}

void bqws_pt_sleep_ms(uint32_t ms)
{
    pt_sleep_ms(ms);
}

const char *bqws_pt_error_type_str(bqws_pt_error_type type)
{
    switch (type) {
    case BQWS_PT_ERRTYPE_NONE: return "NONE";
    case BQWS_PT_ERRTYPE_PT: return "PT";
    case BQWS_PT_ERRTYPE_WSA: return "WSA";
    case BQWS_PT_ERRTYPE_POSIX: return "POSIX";
    case BQWS_PT_ERRTYPE_GETADDRINFO: return "GETADDRINFO";
    case BQWS_PT_ERRTYPE_OPENSSL: return "OPENSSL";
    default: return "(unknown)";
    }
}

const char *bqws_pt_error_code_str(bqws_pt_error_code err)
{
    switch (err) {
    case BQWS_PT_OK: return "OK";
    case BQWS_PT_ERR_NO_TLS: return "NO_TLS: bq_websocket_platform.c was built without TLS support";
    case BQWS_PT_ERR_NO_SERVER_SUPPORT: return "NO_SERVER_SUPPORT: The platform doesn't support server sockets";
    case BQWS_PT_ERR_OUT_OF_MEMORY: return "OUT_OF_MEMORY: Failed to allocate memory";
    case BQWS_PT_ERR_BAD_URL: return "BAD_URL: Could not parse URL";
    default: return "(unknown)";
    }
}

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Samuli Raivio
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
----------------------------------------
*/

#endif // BQ_PLATFORM_IMPLEMENTATION
