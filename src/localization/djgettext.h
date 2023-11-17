//#include "../config.h"

//#define djLOCALIZE_ON
//#define djLOCALIZE_USE_OWN
////#define djLOCALIZE_USE_GETTEXT

#ifdef djLOCALIZE_ON

    #ifdef djLOCALIZE_USE_OWN

    // Either have our own here, or use 3rd party e.g. gettext lib ...
    const char* gettext(const char* msgid);
    const char* pgettext(const char* /*context*/, const char* msgid);
    const char* pgettext(const char* /*context*/, const char* msgid);
    const char* npgettext(const char* /*context*/, const char* msgid_singular, const char* msgid_plural, unsigned long n);

    #else

    // Add the necessary include code for gettext headers
    #include <gettext.h>

    #endif//djLOCALIZE_USE_OWN

#else//#ifdef djLOCALIZE_ON
// If localization is not enabled we want simple stubs/wrappers to just return the passed-in strings e.g.:
// pgettext("mainmenu", "Exit"); => return "Exit"

//#include <string>

// Define gettext stub
inline const char* gettext(const char* msgid) {
    return msgid;
}

// Define pgettext stub
inline const char* pgettext(const char* /*context*/, const char* msgid) {
    return msgid;
}

// Define ngettext stub
inline const char* ngettext(const char* msgid_singular, const char* msgid_plural, unsigned long n) {
    return n == 1 ? msgid_singular : msgid_plural;
}

// Define npgettext stub
inline const char* npgettext(const char* /*context*/, const char* msgid_singular, const char* msgid_plural, unsigned long n) {
    return n == 1 ? msgid_singular : msgid_plural;
}

// Common convention to use _ for gettext()
//#define _(szStr) gettext(szStr)


#endif //djLOCALIZE_ON
