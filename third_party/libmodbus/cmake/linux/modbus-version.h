#ifndef MODBUS_VERSION_H
#define MODBUS_VERSION_H

#define LIBMODBUS_VERSION_MAJOR (3)
#define LIBMODBUS_VERSION_MINOR (1)
#define LIBMODBUS_VERSION_MICRO (10)
#define LIBMODBUS_VERSION        (3.1.10)
#define LIBMODBUS_VERSION_STRING "3.1.10"
#define LIBMODBUS_VERSION_HEX                                                                      \
    ((LIBMODBUS_VERSION_MAJOR << 16) | (LIBMODBUS_VERSION_MINOR << 8) |                            \
     (LIBMODBUS_VERSION_MICRO << 0))
#define LIBMODBUS_VERSION_CHECK(major, minor, micro)                                               \
    (LIBMODBUS_VERSION_MAJOR > (major) ||                                                          \
     (LIBMODBUS_VERSION_MAJOR == (major) && LIBMODBUS_VERSION_MINOR > (minor)) ||                  \
     (LIBMODBUS_VERSION_MAJOR == (major) && LIBMODBUS_VERSION_MINOR == (minor) &&                  \
      LIBMODBUS_VERSION_MICRO >= (micro)))

#endif
