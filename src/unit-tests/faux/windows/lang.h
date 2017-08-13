#ifndef CS_unittests_faux_windows_lang_h_
#define CS_unittests_faux_windows_lang_h_

#define MAKELANGID(p, s)       ((((WORD  )(s)) << 10) | (WORD  )(p))
#define PRIMARYLANGID(lgid)    ((WORD  )(lgid) & 0x3ff)
#define SUBLANGID(lgid)        ((WORD  )(lgid) >> 10)

#define LANG_NEUTRAL                     0x00
#define LANG_INVARIANT                   0x7f

#define SUBLANG_NEUTRAL                             0x00    // language neutral
#define SUBLANG_DEFAULT                             0x01    // user default
#define SUBLANG_SYS_DEFAULT                         0x02    // system default
#define SUBLANG_CUSTOM_DEFAULT                      0x03    // default custom language/locale
#define SUBLANG_CUSTOM_UNSPECIFIED                  0x04    // custom language/locale
#define SUBLANG_UI_CUSTOM_DEFAULT                   0x05    // Default custom MUI language/locale

#endif
