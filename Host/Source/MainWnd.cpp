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
 * Filename:    MainWnd.cpp
 * Author(s):   Dries Staelens
 * Copyright:   Copyright (c) 2009 Dries Staelens
 * Description: TODOTODOTODO
 */
#include "stdafx.h"
#include "NotifyIcon.h"
#include "Module.h"
#include "MainWnd.h"

MainWnd::MainWnd() : CPdnWnd(), m_pNotIc(NULL)
{
	m_WindowClass.lpszClassName = GetMainWindowClassname();
}
MainWnd::~MainWnd()
{
	if (m_pNotIc) {
		m_pNotIc->Release();
		m_pNotIc = NULL;
	}
}

void MainWnd::GetNotifyIcon(VARIANT* pDisp)
{
	if(m_pNotIc)
	{
		m_pNotIc->QueryInterface(IID_IDispatch, (void **)&pDisp->pdispVal);
		pDisp->vt = VT_DISPATCH;
	}
	else
	{
		pDisp->pdispVal = NULL;
		pDisp->vt = VT_DISPATCH;
	}
}

LRESULT MainWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_TaskbarRestartMessage = RegisterWindowMessage(TEXT("TaskbarCreated"));

	m_pNotIc = new CNotifyIcon;
	m_pNotIc->AddRef();
	m_pNotIc->init(m_hWnd, WM_NOTIFYICON);

	return CPdnWnd::OnCreate(uMsg, wParam, lParam);
}
LRESULT MainWnd::OnNotifyIcon(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(lParam == WM_MOUSEMOVE || !lParam)
		return 1;

	VARIANT* v = new _variant_t((long) lParam);
	FireEvent(m_pNotIc->getHandler(), v, 1);
	delete v;
	return false;
}
LRESULT MainWnd::OnCopyData(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	COPYDATASTRUCT* pCDS = (COPYDATASTRUCT*) lParam;
	if(pCDS->dwData == COPYDATA_CMDLINE)
	{
		_variant_t v((OLECHAR *) pCDS->lpData);

		if(m_sCmdLineHandler.length())
			FireEvent(m_sCmdLineHandler, &v, 1);

		return 1;
	}
	return 0;
}
LRESULT MainWnd::OnTaskbarRestart(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_pNotIc->show();
	return 0;
}

LRESULT MainWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CPdnWnd::OnClose(uMsg, wParam, lParam);
}
LRESULT MainWnd::OnFinalMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PostQuitMessage(0);
	return CPdnWnd::OnFinalMessage(hWnd, uMsg, wParam, lParam);
}
STDMETHODIMP MainWnd::desktopAlert(BSTR from, BSTR message, DWORD timeOut)
{
	m_pNotIc->AddRef();
	m_pNotIc->alert(from, message, timeOut);
	m_pNotIc->Release();
	return S_OK;
}
STDMETHODIMP MainWnd::close()
{
	VARIANT* pvElements;
	ScrRun::IDictionaryPtr pWindows = m_Module->GetWindows();
	_variant_t vWndItems = pWindows->Items();

	::SafeArrayLock(vWndItems.parray);
	::SafeArrayAccessData(vWndItems.parray, (void**) &pvElements);
	for(int i = vWndItems.parray->rgsabound->cElements - 1; i >= 0; i--)
	{
		((CPdnWnd*)pvElements[i].pdispVal)->CPdnWnd::close();
	}
	::SafeArrayUnlock(vWndItems.parray);
	::SafeArrayDestroy(vWndItems.parray);

	return S_OK;
}
HWND MainWnd::GetMainWindow()
{
	return ::FindWindow(GetMainWindowClassname(), NULL);
}
LPCWSTR MainWnd::GetMainWindowClassname()
{
	static WCHAR moduleName[MAX_PATH], classname[MAX_PATH + 20];
	::GetModuleFileName(NULL, moduleName, MAX_PATH);
	::PathCanonicalize(classname, moduleName);
	::StringCchCat(classname, MAX_PATH + 20, L" Main Window Class");
	return classname;
}