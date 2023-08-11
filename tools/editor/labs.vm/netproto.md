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

[PhoneNumber]                  // struct: variables never share same field number (1!=2)
1: number = 0                  // .field 1, .id number, .value 0, .type Integer
2: phonetype = OTHER?          // .field 2, .id phonetype, .value OTHER, .type PhoneType, .optional

[Name]                         // union: some variables may share a very same field number (1==1)
1: name = ""                   // .field 1, .id name, .value "", .type String
1: nickname = ""               // .field 1, .id nickname, .value "", .type String

[Profile]                      // struct
1: name = Name...              // .field 1, .id name, .type Name, .repeat
2: icon = "guest.png"?         // .field 2, .id icon, .type String, .value = "guest.png", .optional

[Contact]                      // struct
1: person = Profile
2: card_id = 0 bits:15         // .default = 0, .bits = 15
3: email = ""?                 // email is an optional string located in the third message field that defaults to an empy string
4: phone = PhoneType...        // .repeat
5: calls = 0... bits:64 packed // .default = 0, .repeat, .bits = 64, .packed
10..max:                       // user-defined, reserved extensions

[ContactExt <- Contact]        // struct that extends from Contact
1: phone_ext_number = 0?

[AddressBook]                  // struct
1: contact = ContactExt...
```
