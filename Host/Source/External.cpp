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
 * Filename:    External.cpp
 * Author(s):   Dries Staelens
 * Copyright:   Copyright (c) 2009 Dries Staelens
 * Description: TODOTODOTODO
 */

#include "stdafx.h"
#include "External.h"
#include "PopupMenu.h"
#include "NotifyIcon.h"
#include "MainWnd.h"
#include "File.h"
#include "Directory.h"
#include "XMPP.h"
#include "UTF.h"
#include "shortcut.h"
#include "comctrlwrapper.h"
#include "module.h"
#include "Hash.h"
#include "Base64.h"

#include "pdnwnd.h"

External::External(CPdnWnd& Wnd) : 
	m_pModule(0), m_ComCtrl(Wnd), m_Wnd(Wnd)
{
	m_ComCtrl.DisableSelfDelete();
}
External::~External()
{
}

/* IExternal */
HRESULT External::Init(void* pModule)
{
	m_pModule = (PdnModule *) pModule;

	return S_OK;
}

STDMETHODIMP External::get_wnd(VARIANT* pDisp)
{
	pDisp->vt = VT_DISPATCH;
	return m_Wnd.QueryInterface(IID_IDispatch, (void**)&pDisp->pdispVal);
}
STDMETHODIMP External::get_mainWnd(VARIANT* pDisp)
{
	pDisp->vt = VT_DISPATCH;

	MainWnd *pMainWnd = m_pModule->GetMainWnd();
	pMainWnd->QueryInterface(IID_IDispatch, (void**)&pDisp->pdispVal);
	pMainWnd->Release();

	return S_OK;
}
STDMETHODIMP External::get_windows(VARIANT* pDisp)
{
	pDisp->vt = VT_DISPATCH;
	ScrRun::IDictionaryPtr pWindows = m_pModule->GetWindows();
	return pWindows->QueryInterface(IID_IDispatch, (void**)&pDisp->pdispVal);
}
STDMETHODIMP External::get_globals(VARIANT* pDisp)
{
	pDisp->vt = VT_DISPATCH;
	ScrRun::IDictionaryPtr pGlobals = m_pModule->GetGlobals();
	return pGlobals->QueryInterface(IID_IDispatch, (void**)&pDisp->pdispVal);
}
STDMETHODIMP External::get_ComCtrl(VARIANT* pDisp)
{
	pDisp->vt = VT_DISPATCH;
	return m_ComCtrl.QueryInterface(IID_IDispatch, (void**)&pDisp->pdispVal);
}
STDMETHODIMP External::get_XMPP(VARIANT *pDisp)
{
	pDisp->vt = VT_DISPATCH;
	pDisp->pdispVal = m_pModule->GetXMPP();
	return S_OK;
}
STDMETHODIMP External::get_SASL(VARIANT* pDisp)
{
	pDisp->vt = VT_DISPATCH;
	pDisp->pdispVal = m_pModule->GetSASL();
	return S_OK;
}
STDMETHODIMP External::get_HTTPEngine(VARIANT* pDisp)
{
	pDisp->vt = VT_DISPATCH;
	pDisp->pdispVal = m_pModule->GetHTTP();
	return S_OK;
}
STDMETHODIMP External::createWindow(BSTR name, BSTR file, VARIANT* params,
									BOOL popup, VARIANT* pDisp)
{
	RECT rc = { 300, 300, rc.left + 500, rc.top + 300 };
	CPdnWnd* newWindow = new CPdnWnd;
    newWindow->Create(rc, name, file, _variant_t(params), m_pModule, popup);

	newWindow->QueryInterface(IID_IDispatch, (void**)&pDisp->pdispVal);
	pDisp->vt = VT_DISPATCH;
	return S_OK;
}
STDMETHODIMP External::shellExec(BSTR verb, BSTR file, BSTR params,
	BSTR dir, DWORD nShowCmd)
{
	::ShellExecute(::GetDesktopWindow(), verb, file, params, dir, nShowCmd);
	return S_OK;
}
STDMETHODIMP External::get_cursorX(VARIANT* retVal)
{
	POINT pt;
	::GetCursorPos(&pt);
	retVal->intVal = pt.x;
	retVal->vt = VT_I4;
	return S_OK;
}
STDMETHODIMP External::get_cursorY(VARIANT* retVal)
{
	POINT pt;
	::GetCursorPos(&pt);
	retVal->intVal = pt.y;
	retVal->vt = VT_I4;
	return S_OK;
}
STDMETHODIMP External::get_notifyIcon(VARIANT* pDisp)
{
	MainWnd *pMain = m_pModule->GetMainWnd();
	pMain->GetNotifyIcon(pDisp);
	pMain->Release();

	return S_OK;
}
STDMETHODIMP External::get_newPopupMenu(VARIANT* pDisp)
{
	(new CPopupMenu)->QueryInterface(IID_IDispatch, (void**)&(pDisp->pdispVal));
	pDisp->vt = VT_DISPATCH;
	return S_OK;
}
STDMETHODIMP External::get_IPs(VARIANT* pStr)
{
	pStr->vt = VT_BSTR;
	pStr->bstrVal = SysAllocString(L"error");

	char strHostName[1024];
	if(gethostname(strHostName, 1024) == SOCKET_ERROR) return S_FALSE;
	hostent* pHost = gethostbyname(strHostName);

	TCHAR strAddr[20];
	_bstr_t retStr = "";
	for(int i = 0; pHost->h_addr_list[i] != 0; i++)
	{
		swprintf(strAddr, TEXT("%u.%u.%u.%u"),
			(UCHAR) pHost->h_addr_list[i][0],
			(UCHAR) pHost->h_addr_list[i][1],
			(UCHAR) pHost->h_addr_list[i][2],
			(UCHAR) pHost->h_addr_list[i][3]);
		retStr += strAddr;
		retStr += TEXT(";");
	}

	SysFreeString(pStr->bstrVal);
	pStr->bstrVal = retStr.Detach();

	return S_OK;
}
STDMETHODIMP External::get_CmdLine(VARIANT* pStr)
{
	pStr->vt = VT_BSTR;
	pStr->bstrVal = ::SysAllocString(GetCommandLineW());
	return S_OK;
}
STDMETHODIMP External::sleep(DWORD dwMilliseconds)
{
	::Sleep(dwMilliseconds);
	return S_OK;
}
STDMETHODIMP External::File(BSTR path, VARIANT *pDisp)
{
	pDisp->vt = VT_DISPATCH;
	(new CFile)->QueryInterface(IID_IDispatch, (LPVOID*) &pDisp->pdispVal);
    
	((IPdnFile*)pDisp->pdispVal)->Create(path, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_ALWAYS);
	
	return S_OK;
}
STDMETHODIMP External::FileExists(BSTR path, BOOL *bExists)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path, &FindFileData);

	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		*bExists = TRUE;
	}
	else
		*bExists = FALSE;
    return S_OK;
}
STDMETHODIMP External::get_Directory(VARIANT *pDisp)
{
	pDisp->vt = VT_DISPATCH;
	return (new CDirectory)->QueryInterface(
		IID_IDispatch, (LPVOID*) &pDisp->pdispVal);
}
STDMETHODIMP External::StringToSHA1(BSTR str, BSTR *strSHA1)
{
	unsigned char digest[20];
	
	Hash::SHA1((const unsigned char*) UTF::utf16to8(str).c_str(),
		UTF::utf16to8(str).length(), digest);

	wchar_t hexHash[20*2+1];
	hexHash[20*2] = 0;
	for(unsigned i = 0; i <20; i++)
	{
		hexHash[2*i] = (digest[i] >> 4) + 
			(((digest[i] >> 4) < 0xA) ? L'0' : (L'a' - 0xA));
		hexHash[2*i+1] = (digest[i] & 0x0F) + 
			(((digest[i] & 0x0F) < 0xA) ? L'0' : (L'a' - 0xA));
	}
	*strSHA1 = _bstr_t(hexHash).Detach();
	return S_OK;
}
STDMETHODIMP External::GetSpecialFolder(int nFolder, BSTR *Path)
{
	TCHAR szPath[MAX_PATH];

	if(SUCCEEDED(::SHGetFolderPath(NULL, nFolder, NULL,
		SHGFP_TYPE_CURRENT, szPath)))
	{
		*Path = ::SysAllocString(szPath);
	}
	else
	{
        *Path = ::SysAllocString(L"");
	}
	return S_OK;
}

STDMETHODIMP External::RegRead(BSTR hKey, BSTR key, BSTR value,
	VARIANT* vRetVal)
{
	HKEY RegKey, RootKey = StringToRegRootKey(hKey);
	ULONG uBytes = 0, dwType = 0;
	*vRetVal = _variant_t(0).Detach();
	LSTATUS err = ::RegOpenKeyEx(RootKey, key, 0, KEY_READ, &RegKey);
	if(err == ERROR_SUCCESS)
	{
		err = ::RegQueryValueEx(RegKey, value, NULL, &dwType, NULL, &uBytes);
		if(err == ERROR_SUCCESS)
		{
			BYTE* pData = new BYTE[uBytes];
			err = ::RegQueryValueEx(RegKey, value, NULL,
				&dwType, pData, &uBytes);

			if(err == ERROR_SUCCESS && dwType == REG_DWORD)
				*vRetVal = _variant_t(*(LPDWORD)pData).Detach();
			else if(err == ERROR_SUCCESS && dwType == REG_SZ)
				*vRetVal = _variant_t((LPTSTR)pData).Detach();

			delete[] pData;
		}
		::RegCloseKey(RegKey);
	}

	return HRESULT_FROM_WIN32(err);
}
STDMETHODIMP External::RegWriteString(BSTR hKey, BSTR key, BSTR value,
	BSTR data)
{
	HKEY RegKey, RootKey = StringToRegRootKey(hKey);
	LSTATUS err = ::RegCreateKeyEx(RootKey, key, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &RegKey, NULL);
	if(err == ERROR_SUCCESS)
	{
		err = ::RegSetValueEx(RegKey, value, NULL, REG_SZ, (LPBYTE) data,
			::SysStringByteLen(data) + 2);
		::RegCloseKey(RegKey);
	}
	return HRESULT_FROM_WIN32(err);
}
STDMETHODIMP External::RegWriteDWORD(BSTR hKey, BSTR key, BSTR value,
	DWORD data)
{
	HKEY RegKey, RootKey = StringToRegRootKey(hKey);
	LSTATUS err = ::RegCreateKeyEx(RootKey, key, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &RegKey, NULL);
	if(err == ERROR_SUCCESS)
	{
		err = ::RegSetValueEx(RegKey, value, NULL, REG_DWORD, (LPBYTE) &data,
			sizeof(DWORD));
		::RegCloseKey(RegKey);
	}
	return HRESULT_FROM_WIN32(err);
}
STDMETHODIMP External::RegDeleteKey(BSTR hKey, BSTR key)
{
	HKEY RootKey = StringToRegRootKey(hKey);
	return HRESULT_FROM_WIN32(::RegDeleteKey(RootKey, key));
}
STDMETHODIMP External::RegDeleteValue(BSTR hKey, BSTR key, BSTR value)
{
	HKEY RegKey, RootKey = StringToRegRootKey(hKey);
	LSTATUS err = ERROR_SUCCESS;
	err = ::RegOpenKeyEx(RootKey, key, 0, KEY_SET_VALUE, &RegKey);
	err = ::RegDeleteValue(RegKey, value);
	::RegCloseKey(RegKey);
	return err == ERROR_SUCCESS ? S_OK : E_FAIL;
}
STDMETHODIMP External::get_Shortcut(VARIANT *pDisp)
{
	pDisp->vt = VT_DISPATCH;
	return (new Shortcut)->QueryInterface(IID_IDispatch,
		(LPVOID*) &pDisp->pdispVal);
}

voidpf zlibCustomOpen(voidpf opaque, const char* filename, int mode)
{
	std::wstring modeString = L"rb";
	return (voidpf) _wfopen(
		UTF::utf8to16(filename).c_str(), modeString.c_str());
}
uLong zlibCustomRead(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	return fread(buf, 1, size, (FILE*) stream);
}
uLong zlibCustomWrite(voidpf opaque, voidpf stream, const void* buf, uLong size)
{
	return fwrite(buf, 1, size, (FILE*) stream);
}
long zlibCustomTell(voidpf opaque, voidpf stream)
{
	return ftell((FILE*) stream);
}
long zlibCustomSeek(voidpf opaque, voidpf stream, uLong offset, int origin)
{
	return fseek((FILE*) stream, offset, origin);
}
int zlibCustomClose(voidpf opaque, voidpf stream)
{
	return fclose((FILE*) stream);
}
int zlibCustomTestError(voidpf opaque, voidpf stream)
{
	return ferror((FILE*) stream);
}

STDMETHODIMP External::UnZip(BSTR path, BSTR targetDir, int *nSuccess)
{
	zlib_filefunc_def ff;
	ff.zopen_file = zlibCustomOpen;
	ff.zread_file = zlibCustomRead;
	ff.zwrite_file = zlibCustomWrite;
	ff.ztell_file = zlibCustomTell;
	ff.zseek_file = zlibCustomSeek;
	ff.zclose_file = zlibCustomClose;
	ff.zerror_file = zlibCustomTestError;

	*nSuccess = 0;
	unzFile pZipFile = unzOpen2(UTF::utf16to8(path).c_str(), &ff);

	if(!pZipFile)
	{
        *nSuccess = -1;
		return S_OK;
	}

	if(unzGoToFirstFile(pZipFile) != UNZ_OK)
	{
		*nSuccess = -1;
		unzClose(pZipFile);
		return S_OK;
	}

	do
	{
		if(unzOpenCurrentFile(pZipFile) != UNZ_OK)
		{
			*nSuccess = -1;
			break;
		}
		unz_file_info_s file_info;
		char file_name[MAX_PATH] = "";
		wchar_t file_path[MAX_PATH] = L"";

		unzGetCurrentFileInfo(pZipFile, &file_info,
			file_name, MAX_PATH, 0, 0, 0, 0);

		::StringCchCopy(file_path, MAX_PATH, targetDir);
		::PathAppend(file_path, UTF::utf8to16(file_name).c_str());

		IPdnFile *target_file;
		(new CFile)->QueryInterface(__uuidof(IPdnFile),
			(LPVOID*) &target_file);
		if(target_file->Create(_bstr_t(file_path),
			GENERIC_WRITE, FILE_SHARE_READ,	OPEN_ALWAYS) == S_OK)
		{
			int nRead = 0;
			target_file->Seek(file_info.uncompressed_size, 0, FILE_BEGIN);
			target_file->SetEOF();
			target_file->Seek(0, 0, FILE_BEGIN);
			do
			{	
				BYTE buf[0x1000];
				nRead =	unzReadCurrentFile(pZipFile, buf, sizeof(buf));
				target_file->Write(buf, nRead);
			}
			while(nRead > 0);
		}
		else
		{
			*nSuccess = -1;
		}
		target_file->Release();
		unzCloseCurrentFile(pZipFile);
	}
	while(unzGoToNextFile(pZipFile) == UNZ_OK);

	unzClose(pZipFile);
	return S_OK;
}
STDMETHODIMP External::Base64ToString(BSTR b64String, BSTR *UTF16String)
{
	std::vector<BYTE> decodedData = Base64::Decode(UTF::utf16to8(b64String));
	std::string decodedString(decodedData.begin(), decodedData.end());
	*UTF16String = ::SysAllocString(UTF::utf8to16(decodedString).c_str());

	return S_OK;
}
STDMETHODIMP External::StringToBase64(BSTR UTF16String, BSTR *b64String)
{
	std::string UTF8String = UTF::utf16to8(UTF16String);
	*b64String = ::SysAllocString(
		Base64::Encode(UTF8String.c_str(),
		UTF8String.length(), false).c_str());
	return S_OK;
}
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	static int nFullScreen = 0;
	RECT WindowRect;
	GetWindowRect(hwnd, &WindowRect);

	if(WindowRect.left == 0 && WindowRect.top == 0 &&
		WindowRect.right == GetSystemMetrics(SM_CXSCREEN) &&
		WindowRect.bottom == GetSystemMetrics(SM_CYSCREEN))
	{
		nFullScreen++;
		if(nFullScreen> 2)
			*((bool *)lParam) = true;
	}
	return true;
}
STDMETHODIMP External::Fullscreen(BOOL *bFullscreen)
{
	*bFullscreen = false;
	EnumWindows(EnumWindowsProc, (LPARAM) bFullscreen);
	return S_OK;
}
STDMETHODIMP External::PlaySound(BSTR soundFile)
{
	DWORD flags = SND_FILENAME | SND_ASYNC;
	::PlaySound(soundFile, NULL, flags);
	return S_OK;
}

typedef HRESULT (__stdcall *CREATEURIPROC)(LPCWSTR, DWORD, DWORD, IUri**); 

STDMETHODIMP External::InstallMSIESearchProvider(BSTR url, BSTR* serviceID)
{
	HRESULT hr = E_FAIL;
	HMODULE urlmon_dll = ::LoadLibrary(TEXT("urlmon.dll"));

	if(urlmon_dll != NULL)
	{
		CREATEURIPROC CreateUriProc = 
			(CREATEURIPROC) ::GetProcAddress(urlmon_dll, "CreateUri");

		if(CreateUriProc != NULL)
		{
			IID IID_IOpenServiceManager;
			hr = ::IIDFromString(L"{5664125f-4e10-4e90-98e4-e4513d955a14}",
				&IID_IOpenServiceManager);
			CLSID CLSID_OpenServiceManager;
			hr = ::CLSIDFromString(L"{098870b6-39ea-480b-b8b5-dd0167c4db59}",
				&CLSID_OpenServiceManager);

			IOpenServiceManager* osm;
			hr = ::CoCreateInstance(CLSID_OpenServiceManager, NULL,
				CLSCTX_INPROC_SERVER, IID_IOpenServiceManager, (void**) &osm);

			if(SUCCEEDED(hr))
			{
				IUri* uri;

				hr = CreateUriProc(url, 0, 0, &uri);
				if(SUCCEEDED(hr))
				{
					IOpenService* os;
					hr = osm->InstallService(url, &os);
					if(SUCCEEDED(hr))
					{
						hr = os->GetID(serviceID);
						os->Release();
					}
					uri->Release();
				}				
				osm->Release();
			}
		}
		::FreeLibrary(urlmon_dll);
	}

	return hr;

}

STDMETHODIMP External::SetDefaultMSIESearchProvider(BSTR serviceID)
{
	IID IID_IOpenServiceManager;
	::IIDFromString(L"{5664125f-4e10-4e90-98e4-e4513d955a14}",
		&IID_IOpenServiceManager);
	CLSID CLSID_OpenServiceManager;
	::CLSIDFromString(L"{098870b6-39ea-480b-b8b5-dd0167c4db59}",
		&CLSID_OpenServiceManager);
	
	IOpenServiceManager* osm;
	HRESULT hr = ::CoCreateInstance(CLSID_OpenServiceManager, NULL,
		CLSCTX_INPROC_SERVER, IID_IOpenServiceManager, (void**) &osm);

	if(SUCCEEDED(hr))
	{
		IOpenService* os;
		WCHAR szEscaped[INTERNET_MAX_URL_LENGTH] = L"";
		DWORD cchEscaped = ARRAYSIZE(szEscaped);
		hr = UrlEscape(serviceID, szEscaped, &cchEscaped, URL_ESCAPE_SEGMENT_ONLY);

		if(SUCCEEDED(hr))
		{
			hr = osm->GetServiceByID(szEscaped, &os);
			if(SUCCEEDED(hr))
			{
				hr = os->SetDefault(TRUE, NULL);
				os->Release();
			}
		}
		osm->Release();
	}

	return hr;
}
HRESULT External::IsProcessRunning(BSTR processName, BOOL* isRunning)
{
	*isRunning = FALSE;
	DWORD pidList[8192], pidListUsed;

    if(::EnumProcesses(pidList, sizeof(pidList), &pidListUsed))
	{
		for(DWORD i = 0; i < pidListUsed / sizeof(DWORD); i++)
		{
			if(pidList[i] != 0)
			{
				HANDLE processHandle = ::OpenProcess(PROCESS_VM_READ |
					PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pidList[i]);
				if(processHandle != NULL)
				{
					TCHAR moduleBaseName[MAX_PATH];
					DWORD moduleBaseNameLength = ::GetModuleBaseName(
						processHandle, NULL, moduleBaseName, MAX_PATH);
					if(moduleBaseNameLength != 0 &&
						wcscmp(moduleBaseName, processName) == 0)
					{
						*isRunning = TRUE;
						break;
					}
					::CloseHandle(processHandle);
				}
			}
		}
	}
	return S_OK;
}
HRESULT External::IsRectOnMonitor(DWORD top, DWORD right,
								  DWORD bottom, DWORD left,
								  BOOL* onMonitor)
{
	RECT rect = {left, top, right, bottom};
	HMONITOR monitor = ::MonitorFromRect(&rect, MONITOR_DEFAULTTONULL);
	*onMonitor = monitor == NULL ? FALSE : TRUE;
	return S_OK;
}
HKEY External::StringToRegRootKey(BSTR strHKey)
{
	HKEY RootKey;
	if(wcsstr(strHKey, L"HKEY_CLASSES_ROOT"))
		RootKey = HKEY_CLASSES_ROOT;
	else if(wcsstr(strHKey, L"HKEY_CURRENT_USER"))
		RootKey = HKEY_CURRENT_USER;
	else if(wcsstr(strHKey, L"HKEY_LOCAL_MACHINE"))
		RootKey = HKEY_LOCAL_MACHINE;
	else if(wcsstr(strHKey, L"HKEY_USERS"))
		RootKey = HKEY_USERS;
	else if(wcsstr(strHKey, L"HKEY_PERFORMANCE_DATA"))
		RootKey = HKEY_PERFORMANCE_DATA;
	else if(wcsstr(strHKey, L"HKEY_PERFORMANCE_TEXT"))
		RootKey = HKEY_PERFORMANCE_TEXT;
	else if(wcsstr(strHKey, L"HKEY_PERFORMANCE_NLSTEXT"))
		RootKey = HKEY_PERFORMANCE_NLSTEXT;
	else if(wcsstr(strHKey, L"HKEY_CURRENT_CONFIG"))
		RootKey = HKEY_CURRENT_CONFIG;
	else if(wcsstr(strHKey, L"HKEY_DYN_DATA"))
		RootKey = HKEY_DYN_DATA;
	else
		RootKey = NULL;
	return RootKey;
}