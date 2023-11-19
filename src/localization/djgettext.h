/*!
\file    djgettext.h
\brief   Localization / languages: gettext() and pgettext() etc.
\author  David Joffe

Copyright (C) 1995-2023 David Joffe
*/
//#include "../config.h"

// dj2023-11 for now disabling LoadAllPOFiles as it's causing issues on older C++ on some platforms (and not really needed yet anyway)

/define djLOCALIZE_ON
#define djLOCALIZE_USE_OWN

        ////#define djLOCALIZE_USE_GETTEXT

#ifdef djLOCALIZE_ON

    #ifdef djLOCALIZE_USE_OWN

    #include <string>

    // TO KEEP IN MIND - we may want multiple sets of translations

    // Either have our own here, or use 3rd party e.g. gettext lib ...
    //const char* pgettext(const char* /*context*/, const char* msgid);
    //const char* npgettext(const char* /*context*/, const char* msgid_singular, const char* msgid_plural, unsigned long n);
    // The names of these are so they work xgettext etc. (see https://www.gnu.org/software/gettext/manual/html_node/xgettext-Invocation.html)
    extern std::string gettext(const std::string& msgID);
    extern std::string pgettext(const std::string& contextStr, const std::string& msgID);
    extern std::string select_locale(const std::string& lang);
extern std::string djGetText_GetLang();
    //extern std::string bindcurrentdomain(const std::string& lang);
    extern void loadPOFile(const std::string& filename, const std::string& lang);

    //extern void LoadAllPOFiles(const std::string& sPath);

    #else

    // Add the necessary include code for gettext headers
    #include <gettext.h>

    #endif//djLOCALIZE_USE_OWN

#else//#ifdef djLOCALIZE_ON
// If localization is not enabled we want simple stubs/wrappers to just return the passed-in strings e.g.:
// pgettext("mainmenu", "Exit"); => return "Exit"

    #include <string>
    std::string select_locale(const std::string& lang);
    //extern void LoadAllPOFiles(const std::string& sPath);
    extern void loadPOFile(const std::string& filename, const std::string& lang);

    // Define gettext stub
    const char* gettext(const char* msgid);
    // Define pgettext stub
    const char* pgettext(const char* /*context*/, const char* msgid);
    // Define ngettext stub
    const char* ngettext(const char* msgid_singular, const char* msgid_plural, unsigned long n);
    // Define npgettext stub
    const char* npgettext(const char* /*context*/, const char* msgid_singular, const char* msgid_plural, unsigned long n);

    // Common convention to use _ for gettext()
    //#define _(szStr) gettext(szStr)

#endif //djLOCALIZE_ON
