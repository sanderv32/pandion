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

CNotifyIcon::CNotifyIcon() :
	m_text(L""), m_handler(L""), m_hIcon(0), m_info(L""),m_infoTitle(L""),m_timeOut(5)
{
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
	memset(&NotifyIconData, 0, sizeof(NOTIFYICONDATA));
	NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	NotifyIconData.hWnd = m_hWnd;
	NotifyIconData.uID = 0;

	if (m_CBMsg) {
		NotifyIconData.uFlags |= NIF_MESSAGE;
		NotifyIconData.uCallbackMessage = m_CBMsg;
	}

	if (m_info.length() && m_infoTitle.length() && m_timeOut) {
		NotifyIconData.uTimeout = m_timeOut*1000;
		NotifyIconData.uFlags |= NIF_INFO;
		NotifyIconData.dwInfoFlags = (m_hIcon != 0 ? NIIF_USER : NIIF_INFO);

		::StringCchCopy(NotifyIconData.szInfoTitle, 64, m_infoTitle.c_str());
		::StringCchCopy(NotifyIconData.szInfo, 256, m_info.c_str());
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