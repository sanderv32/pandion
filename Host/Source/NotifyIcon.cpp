/**
 * This file is part of Pandion.
 *
 * Pandion is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pandion is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pandion.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Filename:    NotifyIcon.cpp
 * Author(s):   Dries Staelens
 * Copyright:   Copyright (c) 2009 Dries Staelens
 * Description: TODOTODOTODO
 */
#include "stdafx.h"
#include "NotifyIcon.h"

#define SHELLDLL_WIN2K_OR_LATER		0x00050000
#define SHELLDLL_WINXP_OR_LATER		0x00060000
#define SHELLDLL_WIN7_OR_LATER		0x00060001

#ifndef NIF_REALTIME 
#define NIF_REALTIME				0x00000040
#endif

#ifndef NIF_SHOWTIP
#define NIF_SHOWTIP					0x00000080
#endif

#ifndef NIIF_RESPECT_QUIET_TIME
#define NIIF_RESPECT_QUIET_TIME		0x00000080
#endif

// {23758EB3-3116-4203-8796-20334285F415}
static const GUID pandionGUID = 
{ 0x23758eb3, 0x3116, 0x4203, { 0x87, 0x96, 0x20, 0x33, 0x42, 0x85, 0xf4, 0x15 } };


CNotifyIcon::CNotifyIcon() :
	m_text(L""), m_handler(L""), m_hIcon(0),
	m_info(L""), m_infoTitle(L""), m_timeOut(5),
	m_shellDllVersion(0),
	m_shellDllBuild(0)
{
	HMODULE hShellDll = ::LoadLibrary(L"shell32.dll");
	if (hShellDll != NULL) {
		DLLGETVERSIONPROC DllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hShellDll, "DllGetVersion");

		if (DllGetVersion != NULL) {
			DLLVERSIONINFO dvi;
			::ZeroMemory(&dvi, sizeof(DLLVERSIONINFO));
			dvi.cbSize = sizeof(DLLVERSIONINFO);

			if (SUCCEEDED(DllGetVersion(&dvi))) {
				m_shellDllVersion = MAKELONG(dvi.dwMinorVersion, dvi.dwMajorVersion);
				m_shellDllBuild = dvi.dwBuildNumber;
			}
		}

		::FreeLibrary(hShellDll);
	}
}

CNotifyIcon::~CNotifyIcon()
{
	if(m_hIcon != 0)
	{
		::DestroyIcon(m_hIcon);
	}
}

STDMETHODIMP CNotifyIcon::init(HWND hWnd, UINT CBMsg)
{
	m_hWnd  = hWnd;
	m_CBMsg = CBMsg;
	return S_OK;
}

STDMETHODIMP CNotifyIcon::show()
{
	HRESULT result;

	if ((result = shellNotify(NIM_ADD)) == S_OK) {
		m_created = TRUE;
	}

	return result;
}

STDMETHODIMP CNotifyIcon::update()
{
	return shellNotify(NIM_MODIFY);
}
STDMETHODIMP CNotifyIcon::remove()
{
	return shellNotify(NIM_DELETE);
}

STDMETHODIMP CNotifyIcon::alert(BSTR From, BSTR Message, DWORD timeOut)
{
	m_info = Message;
	m_infoTitle = From;
	m_timeOut = timeOut;

	if (!m_created) {
		show();
	} else {
		update();
	}

	m_info = L"";
	m_infoTitle = L"";
	m_timeOut = 0;

	return S_OK;
}

STDMETHODIMP CNotifyIcon::shellNotify(DWORD dwMessage)
{
	if(!m_hWnd || (m_text.length() == 0 && m_hIcon == 0))
		return S_FALSE;

	NOTIFYICONDATA NotifyIconData;
	ZeroMemory(&NotifyIconData, sizeof(NOTIFYICONDATA));

	NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	NotifyIconData.hWnd = m_hWnd;

	if (m_CBMsg) {
		NotifyIconData.uFlags |= NIF_MESSAGE;
		NotifyIconData.uCallbackMessage = m_CBMsg;
	}

	if (m_info.length() && m_infoTitle.length() && m_timeOut) {
		NotifyIconData.uTimeout = m_timeOut*1000;
		NotifyIconData.uFlags |= NIF_INFO;
		NotifyIconData.dwInfoFlags |= (m_hIcon != 0 ? NIIF_USER : NIIF_INFO);

		if (m_shellDllVersion >= SHELLDLL_WINXP_OR_LATER) {
			NotifyIconData.dwInfoFlags |= NIIF_NOSOUND;

			if (m_shellDllBuild >= 6) {
				NotifyIconData.uFlags |= NIF_REALTIME;
			}

			NotifyIconData.guidItem = pandionGUID;
		}

		if (m_shellDllVersion >= SHELLDLL_WIN7_OR_LATER) {
			NotifyIconData.dwInfoFlags |= NIIF_RESPECT_QUIET_TIME;
		}

		::StringCchCopy(NotifyIconData.szInfoTitle, 64, m_infoTitle.c_str());
		::StringCchCopy(NotifyIconData.szInfo, 200, m_info.c_str());
	}

	if(m_hIcon != 0)
	{
        NotifyIconData.hIcon = m_hIcon;
		NotifyIconData.uFlags |= NIF_ICON;
	}

	if(m_text.length())
	{
		::StringCchCopy(NotifyIconData.szTip, 64, m_text.c_str());
		NotifyIconData.uFlags |= NIF_TIP;
	}

	if (::Shell_NotifyIcon(dwMessage, &NotifyIconData) == TRUE) {
		return S_OK;
	}

	return E_FAIL;
}
STDMETHODIMP CNotifyIcon::setText(BSTR text)
{
	m_text = text;
	return S_OK;
}
STDMETHODIMP CNotifyIcon::setIcon(BSTR strIcon, int nIndex)
{
	if(m_hIcon != 0)
	{
		::DestroyIcon(m_hIcon);
	}
	m_hIcon = ::ExtractIcon(GetModuleHandle(NULL), strIcon, nIndex);
	return S_OK;
}
STDMETHODIMP CNotifyIcon::setHandler(BSTR handler)
{
	m_handler = handler;
	return S_OK;
}
BSTR CNotifyIcon::getHandler()
{
	return m_handler;
}