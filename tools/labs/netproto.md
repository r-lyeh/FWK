// minimalist network proto/scheme that resembles a .ini file.
// - rlyeh, public domain

## features

```json
types:
    floating
    integer
    string

signed:
    yes/no

sizes:
    1,8,12,15,16,24,32,64,...

containers:
    lists
    maps (can be simulated by using two contiguous lists of keys and values)

blocks:
    enum
    union (can be extended)
    struct (can be extended)

options:
    zigzag, decay (double->half), packed, default,

messaging:
    numbered fields
    optional
    required
    repeated
    extensions
```

## binary format

```ini
[cellsize in bytes:8]
[num key-values:cellsize]
    [key id:cellsize][value id:cellsize]
    [...]
[num types:cellsize]
    [type class:cellsize][num items:cellsize][item1][item2][...]
    [...] // would be nice to put strings at the very end, because of their length-variable nature
```

## text format

```ini
// namespace
[[Demo]]

// free-standing options
package = "tutorial"
version = 123
pi = 3.14159

[PhoneType]                    // enumeration: all members are uppercase + no need for explicit values.
HOME
WORK
OTHER = HOME
MOBILE

[PhoneNumber]                  // struct: variables never share same address
number = 0
phonetype? = OTHER             // optional

[Name]                         // union: some variables may share a very same field number (name&nickname)
name = ""
|nickname = ""

[Profile]                      // struct
name... = "Guest"              // repeat
icon? = "guest.png"            // optional

[Contact]                      // struct
person = Profile
card_id:15 = 0              // bits = 15
email?= ""                  // optional
phone... = PhoneType        // .repeat
calls:64p... = 0            // .repeat, .bits = 64, .packed

[ContactExt <- Contact]        // struct that extends from Contact
phone_ext_number? = 0

[AddressBook]                  // struct
contact... = ContactExt
```
-