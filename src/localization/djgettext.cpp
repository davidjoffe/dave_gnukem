/*!
\file    djgettext.cpp
\brief   Localization / languages: gettext() and pgettext() etc.
\author  David Joffe

Copyright (C) 1995-2024 David Joffe
*/
/*--------------------------------------------------------------------------*/

#include "djgettext.h"

//#define djLOCALIZE_ON
//#define djLOCALIZE_USE_OWN


#ifdef djLOCALIZE_ON

#ifdef djLOCALIZE_USE_OWN

// Localization on, using our own
#include <string>
#include <unordered_map>
// LOADING:
#include <fstream>
#include <sstream>

// e.g. StringsDB["fr"]["menu/start"]["Start"].sMsgStr = "Démarrer";
// e.g. StringsDB["fr"]["menu/file"]["Open"].sMsgStr = "Open (file)";
// e.g. StringsDB["fr"]["menu/printer"]["Open"].sMsgStr = "Open (printer)";
//---------------------------------------------------------------------------
 struct tsMessage {
    //std::string sContext; // [[should we have context here? effectively an extra copy]]
    std::string sMsgID;  // Original message
    std::string sMsgStr; // Translated message
};

struct tsContext {
    std::unordered_map<std::string, tsMessage> messages; // Map: MsgID -> tsMessage e.g. "Open" -> "Ouvrir"
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//using ContextMap = std::unordered_map<std::string, tsMessage>;
//using LanguageMap = std::unordered_map<std::string, ContextMap>;
using ContextMap = std::unordered_map<std::string, tsContext>;//tsMessage>; Map e.g. "menu" -> tsContext submap
using LanguageMap = std::unordered_map<std::string, ContextMap>;//Map "fr" to contextmap for "fr"

// Our localization gettext() database of strings
LanguageMap StringsDB; // Map: Language -> tsContext
//std::unordered_map<std::string, ContextMap> StringsDB; // Map: Language -> tsContext

class djGetText
{
public:
    const std::string& GetLang() const { return m_sCurLang; }

    std::string m_sCurLang;
};
djGetText g_GetText;
//---------------------------------------------------------------------------

// Function to handle escaped characters in .po files
std::string processEscapedCharacters(const std::string& input) {
    std::string result;
    bool escapeNextChar = false;

    for (char ch : input) {
        if (escapeNextChar) {
            if (ch == 'n') {
                result += '\n'; // Handle newline escape
            } else {
                result += ch; // Handle other escapes (like quotes)
            }
            escapeNextChar = false;
        } else if (ch == '\\') {
            escapeNextChar = true;
        } else {
            result += ch;
        }
    }

    return result;
}

#include <iostream>

void loadPOFile(const std::string& filename, LanguageMap& StringsDB, const std::string& lang) {
    if (filename.empty())
        return;
    printf("Loading PO file %s lang %s\n", filename.c_str(), lang.c_str());
    std::ifstream file(filename);
    if (!file.is_open()) {
        //throw std::runtime_error("Unable to open file: " + filename);
        printf("Unable to open file: %s\n", filename.c_str());
        return;
    }

    std::string line;
    std::string currentContext;
    std::string currentID;
    std::string currentStr;
    bool inMsgId = false;
    bool inMsgStr = false;

    while (getline(file, line)) {
        if (line.substr(0, 7) == "msgctxt") {
            currentContext = line.substr(9, line.length() - 10); // Remove quotes and keyword
        } else if (line.substr(0, 5) == "msgid") {
            // Save previous message if exists
            if (!currentID.empty()) {
                std::cout << "LOAD:" << lang << " " << currentContext << " " << currentID << " " << currentStr << std::endl;
                StringsDB[lang][currentContext].messages[currentID] = {currentID, currentStr};
            }
            currentID = line.substr(7, line.length() - 8); // Remove quotes and keyword
            inMsgId = true;
            inMsgStr = false;
            currentStr.clear();
        } else if (line.substr(0, 6) == "msgstr") {
            currentStr = line.substr(8, line.length() - 9); // Remove quotes and keyword
            inMsgId = false;
            inMsgStr = true;
        } else if (line.empty()) {
            // Save previous message if exists
            if (!currentID.empty() && !currentStr.empty()) {
                std::cout << "LOAD:" << lang << " " << currentContext << " " << currentID << " " << currentStr << std::endl;
                StringsDB[lang][currentContext].messages[currentID] = {currentID, currentStr};
            }

            // Reset for next entry
            currentContext.clear();
            currentID.clear();
            inMsgId = inMsgStr = false;
        } else {

            // Continue previous msgid or msgstr
            if (inMsgId) {
                currentID += processEscapedCharacters(line.substr(1, line.length() - 2)); // Remove quotes and process escapes
            } else if (inMsgStr) {
                currentStr += processEscapedCharacters(line.substr(1, line.length() - 2)); // Remove quotes and process escapes
            }
        }
    }

    // Save last message if exists
    if (!currentID.empty()) {
        std::cout << "LOAD:" << lang << " " << currentContext << " " << currentID << " " << currentStr << std::endl;
        StringsDB[lang][currentContext].messages[currentID] = {currentID, currentStr};
    }

    file.close();
}
void loadPOFile(const std::string& filename, const std::string& lang)
{
    printf("Loading PO file %s lang %s\n", filename.c_str(), lang.c_str());
    loadPOFile(filename, StringsDB, lang);
}

// dj2023-11 for now disabling LoadAllPOFiles as it's causing issues on older C++ on some platforms (and not really needed yet anyway)
/*
#include <iostream>
#include <filesystem>
//#include <string>

namespace fs = std::__fs::filesystem;


void loadPOFilesFromDirectory(const fs::path& directory, LanguageMap& StringsDB) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Provided path is not a directory or does not exist: " << directory << std::endl;
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".po") {
            std::string langCode = entry.path().stem().string();
            try {
                loadPOFile(entry.path().string(), StringsDB, langCode);
                std::cout << "Loaded .po file for language: " << langCode << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error loading .po file: " << entry.path() << " - " << e.what() << std::endl;
            }
        }
    }
}
*/
//*/
/*
void LoadAllPOFiles(const std::string& sPath)
{
    //return;
///    LanguageMap StringsDB;
    printf("Loading all PO files from %s\n", sPath.c_str());
    loadPOFilesFromDirectory(sPath, StringsDB);
}*/


//std::string g_sCurLang = "";
/*std::string bindcurrentdomain(const std::string& lang)
{
    // Load the appropriate language file
    //loadPOFile("locale/" + lang + "/LC_MESSAGES/messages.po", StringsDB, lang);
    g_GetText.m_sCurLang = lang;
    return lang;
}*/
extern std::string select_locale(const std::string& lang)
{
    g_GetText.m_sCurLang = lang;
    return lang;
}

std::string gettext(const std::string& lang, const std::string& msgID) {
    auto langIt = StringsDB.find(lang);
    if (langIt != StringsDB.end()) {
        const tsContext& context = langIt->second[""];//empty context assumed
        auto msgIt = context.messages.find(msgID);
        if (msgIt != context.messages.end()) {
            return msgIt->second.sMsgStr;
        }
    }
    return msgID; // Fallback to original message if not found
}
std::string gettext(const std::string& msgID) {
    return gettext(g_GetText.GetLang(), msgID);
}

std::string pgettext(const std::string& lang, const std::string& contextStr, const std::string& msgID) {
    auto const langIt = StringsDB.find(lang);
    if (langIt != StringsDB.end()) {
        auto& contexts = langIt->second; // Map: 
        auto contextIt = contexts.find(contextStr);
        if (contextIt != contexts.end()) {
            auto& messagesmap = contextIt->second;
            auto msgIt = messagesmap.messages.find(msgID);
            if (msgIt != messagesmap.messages.end()) {
                return msgIt->second.sMsgStr;
            }
        }
    }
    return msgID; // Fallback to original message if not found
}
std::string pgettext(const std::string& contextStr, const std::string& msgID) {
    return pgettext(g_GetText.GetLang(), contextStr, msgID);
}

std::string djGetText_GetLang()
{
    return g_GetText.GetLang();
}
    
/*
// fixme this is a bit gross and dangerous returning const pointers to strings that could potentially change
// As long as we don't e.g. re-load or add strings in the middle of a game we should be ok
const char* pgettext(const char* context, const char* msgid)
{
    if (msgid==nullptr||*msgid=='\0') return "";
    return pgettext("fr", std::string(context), std::string(msgid)).c_str();
}
const char* gettext(const char* msgid)
{
    if (msgid==nullptr||*msgid=='\0') return "";
    return gettext("fr", std::string(msgid)).c_str();
}
*/

//std::string translated = gettext("fr", "Start");
//std::string contextTranslated = pgettext("fr", "menu", "Start");



#else//!djLOCALIZE_USE_OWN
// Localization on, using gettext
/*

#include <iostream>
#include <libintl.h>
#include <locale.h>
#include <string>

// Initialize gettext and load the appropriate language file
void InitializeGetText(const std::string& sLang) {
    // Set the locale to the desired language
    setlocale(LC_ALL, sLang.c_str());

    // Specify the directory containing the translation files
    // This is the path to where your .mo files are stored
    // E.g., /usr/share/locale for system-wide or a path relative to your application
    bindtextdomain("messages", "/path/to/your/locale/directory");

    // Specify the domain (usually the application's name)
    // This is the prefix of your .mo files
    textdomain("messages");
}

int main() {
    // Example language code
    std::string sLang = "fr"; // French

    // Initialize gettext with the desired language
    InitializeGetText(sLang);

    // Now you can use gettext or the shorthand _() macro to retrieve translated strings
    // For example, gettext("Hello World") will look for the French translation of "Hello World"
    std::cout << gettext("Hello World") << std::endl;

    return 0;
}
*/
#endif//djLOCALIZE_USE_OWN

#else//!djLOCALIZE_ON

// Localization off, just simple wrapper stubs to return the passed-in strings (todo - make these #define's for faster?)
// Simple stubs to start
std::string select_locale(const std::string& lang)
{
    return "";
}
/*void LoadAllPOFiles(const std::string& sPath)
{
}*/
void loadPOFile(const std::string& filename, const std::string& lang)
{
}


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

#endif//djLOCALIZE_ON

