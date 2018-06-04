/*
settings.cpp

Copyright (C) 2001-2018 David Joffe

09/2001
*/

#include "settings.h"
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
	FILE *pInput = fopen(szFilename, "r");
	if (pInput==NULL)
		return false;
	char szBuf[1024]={0};

	fgets(szBuf, sizeof(szBuf), pInput);
	szBuf[strlen(szBuf)-1] = 0;
	while (!feof(pInput))
	{
		char *pEquals;
		pEquals = strchr(szBuf, '=');
		if (pEquals)
		{
			*pEquals = 0;
			pEquals++;
			SetSetting(szBuf, pEquals);
		}
		fgets(szBuf, sizeof(szBuf), pInput);
		szBuf[strlen(szBuf)-1] = 0;
	}
	fclose(pInput);
	return true;
}

bool CSettings::Save(const char *szFilename)
{
	unsigned int i;
	FILE *pOutput = fopen(szFilename, "w");
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
	char szBuf[64]={0};
	sprintf(szBuf, "%d", nValue);
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

