
#include "djgettext.h"

#ifdef djLOCALIZE_ON
#ifdef djLOCALIZE_USE_OWN

// Simple stubs to start

// Define gettext stub
const char* gettext(const char* msgid) {
    return msgid;
}

// Define pgettext stub
const char* pgettext(const char* /*context*/, const char* msgid) {
    return msgid;
}

// Define ngettext stub
const char* ngettext(const char* msgid_singular, const char* msgid_plural, unsigned long n) {
    return n == 1 ? msgid_singular : msgid_plural;
}

// Define npgettext stub
const char* npgettext(const char* /*context*/, const char* msgid_singular, const char* msgid_plural, unsigned long n) {
    return n == 1 ? msgid_singular : msgid_plural;
}

#endif
#endif
