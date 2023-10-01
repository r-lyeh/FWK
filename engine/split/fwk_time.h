// -----------------------------------------------------------------------------
// time framework utils
// - rlyeh, public domain.

API uint64_t    date();        // YYYYMMDDhhmmss
API uint64_t    date_epoch();  // linux epoch
API char*       date_string(); // "YYYY-MM-DD hh:mm:ss"
API double      time_hh();
API double      time_mm();
API double      time_ss();
API uint64_t    time_ms();
API uint64_t    time_us();
API uint64_t    time_ns();
API void        sleep_ss(double ss);
API void        sleep_ms(double ms);
API void        sleep_us(double us);
API void        sleep_ns(double us);

API unsigned    timer(unsigned ms, unsigned (*callback)(unsigned ms, void *arg), void *arg);
API void        timer_destroy(unsigned timer_handle);

// time sortable unique identifier (similar to ksuid/tuid; others: sno/xid/cuid/ulid)
// - rlyeh, public domain.
//
// also similar to a mongo object id, 12 bytes as follows:
// - 4-byte timestamp (ss). epoch: Tuesday, 12 September 2023 6:06:56
// - 2-byte (machine, hash or app id)
// - 2-byte (thread-id)
// - 4-byte (rand counter, that gets increased at every id creation)

typedef vec3i guid;

API guid        guid_create();

/*
AUTORUN {
    guid g1 = guid_create();
    guid g2 = guid_create();
    print3i(g1);
    hexdump(&g1, sizeof(g1));
    print3i(g2);
    hexdump(&g2, sizeof(g2));
}
*/
