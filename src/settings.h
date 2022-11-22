/*!
\file    settings.h
\brief   Game configuration file
\author  David Joffe

Copyright (C) 2001-2018 David Joffe

09/2001
*/
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <vector>
#include <string.h>
#include <stdlib.h>
using namespace std;

//! Configuration class
class CSettings
{
public:
	//! Constructor
	CSettings();
	//! Destructor
	~CSettings();
	
	//! Load settings from file
	bool Load(const char *szFilename);
	//! Save settings to file
	bool Save(const char *szFilename);
	
	//! Return string value associated with given key
	const char *FindSetting(const char *szKey);
	
	//! Set the value of the given key
	void SetSetting(const char *szKey, const char *szValue);
	//! Set the "default" setting. The given value will only be associated with this key
	//! if this key does not already exist in the settings database.
	void SetDefaultSetting(const char *szKey, const char *szValue);
	
	//! Delete a specific setting given its key
	void DeleteSetting(const char *szKey);
	//! Delete all settings
	void DeleteAllSettings();
	
	//! Return integer value associated with given key
	int FindSettingInt(const char *szKey);
	//! Set default setting, but with integer value
	void SetDefaultSettingInt(const char *szKey, int nValue);
	//! Set the integer value of the given key
	void SetSettingInt(const char *szKey, int nValue);
	
protected:
	struct SSetting
	{
		// Setting "key"
		char *szKey = nullptr;
		// Value associated with key
		char *szValue = nullptr;
	};
	vector<SSetting> m_aSettings;
};

//! Global game settings
extern CSettings g_Settings;

#define CONFIG_FILE "davegnukem.cfg"

#endif
