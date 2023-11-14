/*
settings.cpp

Copyright (C) 2001-2022 David Joffe

09/2001
*/

#include "settings.h"
#include "djfile.h"
#include "djstring.h"
#include <stdio.h>
#include "djtypes.h"

CSettings g_Settings;

CSettings::CSettings()
{
}

CSettings::~CSettings()
{
	DeleteAllSettings();
}

bool CSettings::Load(const char *szFilename)
{
	DeleteAllSettings();

	// dj2022-12 adding this check so we don't log spurious scary-sounding message on first time run of application as it's normal for the file not to be there yet
	if (!djFileExists(szFilename))
	{
		//printf("User settings file not found, using default settings\n");
		return false;
	}

	FILE *pIn = djFile::dj_fopen(szFilename, "r");
	if (pIn==NULL)
		return false;

	// fixme should actually phase out these kinds of static buffer loading entirely .. must do carefully tho in a way that we can handle arb 'fuzztest' style data etc. without overruns or crashes or game freezing on incredibly long lines or trying to alloc 4GB text lines .. so not entirely trivial ... create some helpers maybe 
	char buf[4096]={0};

	//" If the End-of-File is encountered and no characters have been read, the contents of str remain unchanged and a null pointer is returned."
	#define djREADLINE() buf[0]=0; if ((fgets(buf, sizeof(buf), pIn) == NULL) && ferror(pIn)) goto error; djStripCRLF(buf)

	while (!feof(pIn))
	{
		djREADLINE();
		// Skip empty lines (or possible read of last line if empty string)
		if (buf[0] == 0) continue;

		char *pEquals = strchr(buf, '=');
		if (pEquals)
		{
			*pEquals = 0;
			pEquals++;
			SetSetting(buf, pEquals);
		}
	}
	fclose(pIn);
	return true;
error:
	fclose(pIn);
	return false;
}

bool CSettings::Save(const char *szFilename)
{
	unsigned int i;
	FILE *pOutput = djFile::dj_fopen(szFilename, "w");
	if (pOutput==NULL)
		return false;
	for ( i=0; i<m_aSettings.size(); i++ )
	{
		fprintf(pOutput, "%s=%s\n", m_aSettings[i].szKey, m_aSettings[i].szValue);
	}
	fclose(pOutput);
	return true;
}

void CSettings::SetDefaultSetting(const char *szKey, const char *szValue)
{
	if (FindSetting(szKey)==NULL)
		SetSetting(szKey, szValue);
}

void CSettings::SetDefaultSettingInt(const char *szKey, int nValue)
{
	if (FindSetting(szKey)==NULL)
		SetSettingInt(szKey, nValue);
}

const char *CSettings::FindSetting(const char *szKey)
{
	unsigned int i;
	for ( i=0; i<m_aSettings.size(); i++ )
	{
		if (!strcmp(szKey, m_aSettings[i].szKey))
			return m_aSettings[i].szValue;
	}
	return NULL;
}

int CSettings::FindSettingInt(const char *szKey)
{
	const char *szValue = FindSetting(szKey);
	return (szValue!=NULL ? atoi(szValue) : 0);
}

void CSettings::DeleteSetting(const char *szKey)
{
	unsigned int i;
	for ( i=0; i<m_aSettings.size(); i++ )
	{
		if (!strcmp(szKey, m_aSettings[i].szKey))
		{
			djDEL(m_aSettings[i].szKey);
			djDEL(m_aSettings[i].szValue);
			m_aSettings.erase(m_aSettings.begin() + i);
			return;
		}
	}
}

void CSettings::SetSetting(const char *szKey, const char *szValue)
{
	// Check if setting key already exists, if so update
	unsigned int i;
	for ( i=0; i<m_aSettings.size(); i++ )
	{
		if (!strcmp(szKey, m_aSettings[i].szKey))
		{
			djDELV(m_aSettings[i].szValue);
			m_aSettings[i].szValue = new char[strlen(szValue)+1];
			strcpy(m_aSettings[i].szValue, szValue);
			return;
		}
	}
	// Doesn't exist yet, so add a value
	SSetting Setting;
	Setting.szKey   = new char[strlen(szKey)+1];
	Setting.szValue = new char[strlen(szValue)+1];
	strcpy(Setting.szKey, szKey);
	strcpy(Setting.szValue, szValue);
	m_aSettings.push_back(Setting);
}

void CSettings::SetSettingInt(const char *szKey, int nValue)
{
	// Hmm, we must be careful to make sure this is large enough for e.g. 64-bit or maybe even 128-bit integer values
	char szBuf[128]={0};
	snprintf(szBuf, sizeof(szBuf), "%d", nValue);
	SetSetting(szKey, szBuf);
}

void CSettings::DeleteAllSettings()
{
	unsigned int i;
	for ( i=0; i<m_aSettings.size(); i++ )
	{
		djDELV(m_aSettings[i].szKey);
		djDELV(m_aSettings[i].szValue);
	}
	m_aSettings.clear();
}

