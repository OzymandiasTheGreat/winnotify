#define UNICODE
#define _UNICODE
#define _WIN32_IE _WIN32_IE_IE60
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#define _WIN32_WINDOWS _WIN32_WINNT_VISTA
#define WINVER _WIN32_WINNT_VISTA
#define NTDDI_VERSION NTDDI_VISTA
#define ISOLAATION_AWARE_ENABLED 1


#include <Python.h>
#include <Windows.h>
#include <Shellapi.h>
#include <CommCtrl.h>
#include <strsafe.h>
#include <math.h>


typedef HRESULT (*LIWSDP)(HINSTANCE, PCWSTR, int, int, HICON*);
LIWSDP LIWSD;
#define LoadIconWithScaleDown LIWSD


const LPCWSTR WCLASS = L"NotifyHiddenWClass";
const int ICONID = 13;
const u_int WM_CALLBACK = WM_APP + 666;
const char PNGHEAD[8] = {137, 80, 78, 71, 13, 10, 26, 10};
const char ICOHEAD[4] = {0, 0, 1, 0};
double SCALE;
HINSTANCE INSTANCE = NULL;
BOOL RUNNING = FALSE;
HWND WINDOW = NULL;
HANDLE MAINLOOP = NULL;
PyObject * NOTIFYCALLBACK = NULL;
NOTIFYICONDATA NID;
BOOL VISIBLE = FALSE;


// Utility functions
wchar_t *Py2UTF16(const char *string) {

	wchar_t *wstring;
	int char_length;

	char_length = MultiByteToWideChar(CP_UTF8, 0, string, -1, NULL, 0);
	wstring = (wchar_t *)malloc(sizeof(wchar_t) * char_length);
	MultiByteToWideChar(CP_UTF8, 0, string, -1, wstring, char_length);
	return wstring;
}

BOOL StringCopy(TCHAR *dest, size_t cdest, TCHAR *src) {

	if (SUCCEEDED(StringCchCopyNEx(dest, cdest, src, cdest - 1, NULL, NULL, STRSAFE_IGNORE_NULLS))) {
		return TRUE;
	} else {
		return FALSE;
	}
}

int TraySize(void) {

	return lround(16.0 * SCALE);
}

int BalloonSize(void) {

	return lround(48.0 * SCALE);
}


// Window thread
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	PyGILState_STATE gil_state;

	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
	} else if (msg == WM_CALLBACK) {
		switch (LOWORD(lParam)) {
			case NIN_BALLOONSHOW:
				VISIBLE = TRUE;
				break;
			case NIN_BALLOONUSERCLICK:
				gil_state = PyGILState_Ensure();
				PyObject_CallObject(NOTIFYCALLBACK, NULL);
				PyGILState_Release(gil_state);
				break;
			case NIN_BALLOONHIDE:
			case NIN_BALLOONTIMEOUT:
				VISIBLE = FALSE;
				break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL InitMessageWindow(void) {

	WNDCLASSEX wcx;

	wcx.cbSize = sizeof(wcx);
	wcx.style = 0;
	wcx.lpfnWndProc = WindowProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = INSTANCE;
	wcx.hIcon = NULL;
	wcx.hCursor = NULL;
	wcx.hbrBackground = NULL;
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = WCLASS;
	wcx.hIconSm = NULL;

	RegisterClassEx(&wcx);
	WINDOW = CreateWindowEx(
		0, WCLASS, L"NotifyHiddenWindow", WS_OVERLAPPED | WS_SYSMENU,
		-1, -1, 1, 1, HWND_MESSAGE, NULL, INSTANCE, NULL);
	UpdateWindow(WINDOW);
	return (WINDOW != NULL);
}

DWORD WINAPI MainLoop(void* Parameter) {

	MSG msg;

	if (!InitMessageWindow()) {
		return 1;
	}

	while (RUNNING) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}
		Sleep(100);
	}
	if (DestroyWindow(WINDOW)) {
		WINDOW = NULL;
		if (!UnregisterClass(WCLASS, INSTANCE)) {
			return 3;
		}
	} else {
		return 2;
	}
	return 0;
}


// Main
BOOL InitTrayIcon(HICON icon) {

	u_int uflags = NIF_MESSAGE | NIF_STATE;
	BOOL icon_set, version_set;

	if (icon != NULL) {
		uflags |= NIF_ICON;
	}

	NID.cbSize = sizeof(NID);
	NID.hWnd = WINDOW;
	NID.uID = ICONID;
	NID.uFlags = uflags;
	NID.uCallbackMessage = WM_CALLBACK;
	NID.hIcon = icon;
	NID.dwState = NIS_HIDDEN;
	NID.dwStateMask = NIS_HIDDEN;
	icon_set = Shell_NotifyIcon(NIM_ADD, &NID);

	NID.uVersion = NOTIFYICON_VERSION_4;
	version_set = Shell_NotifyIcon(NIM_SETVERSION, &NID);

	return (icon_set && version_set);
}

BOOL Init(HICON icon) {

	DWORD exit_code;
	HDC screendc;

	INSTANCE = GetModuleHandle(NULL);
	screendc = GetDC(NULL);
	SCALE = GetDeviceCaps(screendc, LOGPIXELSX) / 96.0;
	ReleaseDC(NULL, screendc);
	RUNNING = TRUE;
	MAINLOOP = CreateThread(NULL, 0, &MainLoop, NULL, 0, NULL);
	if (MAINLOOP == NULL) {
		RUNNING = FALSE;
		return FALSE;
	}
	Sleep(10);
	if (GetExitCodeThread(MAINLOOP, &exit_code)) {
		if (exit_code != STILL_ACTIVE) {
			RUNNING = FALSE;
			return FALSE;
		}
	}
	if (!InitTrayIcon(icon)) {
		return FALSE;
	}
	return TRUE;
}

DWORD WINAPI TrayIconHider(void *Parameter) {

	u_int timeout = (UINT_PTR)Parameter;

	Sleep(timeout * 1000);
	NID.dwState = NIS_HIDDEN;
	NID.dwStateMask = NIS_HIDDEN;
	StringCopy(NID.szInfo, 256, NULL);
	StringCopy(NID.szInfoTitle, 64, NULL);
	NID.hBalloonIcon = NULL;
	return (DWORD)Shell_NotifyIcon(NIM_MODIFY, &NID);
}

BOOL Notify(
		wchar_t *body, wchar_t *title, HICON icon, BOOL realtime,
		u_int timeout, u_int flags) {

	u_int uflags = NIF_INFO | NIF_STATE;
	BOOL notified;
	HANDLE thread;

	if (realtime) {
		uflags |= NIF_REALTIME;
	}
	if (!(flags & NIIF_USER)) {
		icon = NULL;
	}
	NID.uFlags = uflags;
	NID.dwState = 0;
	StringCopy(NID.szInfo, 256, body);
	StringCopy(NID.szInfoTitle, 64, title);
	NID.hBalloonIcon = icon;
	NID.dwInfoFlags = flags;
	notified = Shell_NotifyIcon(NIM_MODIFY, &NID);

	thread = CreateThread(
		NULL, 0, &TrayIconHider, (void *)(UINT_PTR)timeout, 0, NULL);

	if (notified && (thread != NULL)) {
		return TRUE;
	} else {
		return FALSE;
	}
}


// Python functions
static PyObject* notify_InitFromIcon(PyObject *self, PyObject *args) {

	const char *icon_path;
	const wchar_t *wicon_path;
	HICON icon;
	BOOL ret;

	if (!PyArg_ParseTuple(args, "zO", &icon_path, &NOTIFYCALLBACK)) {
		return NULL;
	}

	wicon_path = Py2UTF16(icon_path);
	if (icon_path == NULL) {
		icon = NULL;
	} else {
		if (FAILED(LoadIconMetric(NULL, wicon_path, LIM_SMALL, &icon))) {
			PyErr_SetString(
				PyExc_FileNotFoundError, "Could not load or locate icon.");
			return NULL;
		}
	}

	ret = Init(icon);
	DestroyIcon(icon);
	switch (ret) {
		case TRUE:
			Py_RETURN_TRUE;
		case FALSE:
			Py_RETURN_FALSE;
	}
}

static PyObject* notify_InitFromResource(PyObject *self, PyObject *args) {

	const char *resource;
	u_int ordinal;
	const wchar_t *wresource;
	HMODULE lib;
	HICON icon;
	BOOL ret;

	if (!PyArg_ParseTuple(args, "(zI)O", &resource, &ordinal, &NOTIFYCALLBACK)) {
		return NULL;
	}

	wresource = Py2UTF16(resource);
	lib = LoadLibraryEx(wresource, NULL, LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE);
	if (lib == NULL) {
		PyErr_SetString(
			PyExc_FileNotFoundError, "Could not load or locate resource.");
		return NULL;
	}
	if (FAILED(LoadIconMetric(lib, MAKEINTRESOURCE(ordinal), LIM_SMALL, &icon))) {
		PyErr_SetString(
			PyExc_FileNotFoundError, "Could not load or locate icon.");
		return NULL;
	}
	ret = Init(icon);
	DestroyIcon(icon);
	FreeLibrary(lib);
	switch (ret) {
		case TRUE:
			Py_RETURN_TRUE;
		case FALSE:
			Py_RETURN_FALSE;
	}
}

static PyObject* notify_InitFromBuffer(PyObject *self, PyObject *args) {

	const char *buffer;
	int length;
	HICON icon;
	BOOL ret;

	if (!PyArg_ParseTuple(args, "y#O", &buffer, &length, &NOTIFYCALLBACK)) {
		return NULL;
	}

	if (!memcmp(buffer, PNGHEAD, 8)) {
		icon = CreateIconFromResourceEx(
			(BYTE *)buffer, length, TRUE, 0x00030000, TraySize(), TraySize(), 0);
	} else if (!memcmp(buffer, ICOHEAD, 4)) {
		int offset = LookupIconIdFromDirectoryEx(
			(BYTE *)buffer, TRUE, TraySize(), TraySize(), 0);
		icon = CreateIconFromResourceEx(
			(BYTE *)buffer + offset, length - offset, TRUE, 0x00030000,
			TraySize(), TraySize(), 0);
	} else {
		PyErr_SetString(PyExc_TypeError, "Unrecognized buffer type.");
		return NULL;
	}
	ret = Init(icon);
	DestroyIcon(icon);
	switch (ret) {
		case TRUE:
			Py_RETURN_TRUE;
		case FALSE:
			Py_RETURN_FALSE;
	}
}

static PyObject* notify_Uninit(PyObject *self, PyObject *args) {

	BOOL icon_unset, thread_unset;
	DWORD exit_code;

	icon_unset = Shell_NotifyIcon(NIM_DELETE, &NID);
	RUNNING = FALSE;
	Sleep(110);
	if (GetExitCodeThread(MAINLOOP, &exit_code)) {
		switch (exit_code) {
			case 1:
			case 2:
			case 3:
			case STILL_ACTIVE:
				thread_unset = FALSE;
				break;
			case 0:
				MAINLOOP = NULL;
				thread_unset = TRUE;
				break;
		}
	}
	switch (icon_unset && thread_unset) {
		case TRUE:
			Py_RETURN_TRUE;
		case FALSE:
			Py_RETURN_FALSE;
	}
}

static PyObject* notify_IsInitted(PyObject *self, PyObject *args) {

	switch (WINDOW != NULL && RUNNING == TRUE) {
		case TRUE:
			Py_RETURN_TRUE;
		case FALSE:
			Py_RETURN_FALSE;
	}
}

static PyObject* notify_NotifyFromIcon(PyObject *self, PyObject *args) {

	const char *body, *title, *icon_path;
	const wchar_t *wicon_path;
	int realtime;
	u_int timeout, flags;
	HICON icon;
	BOOL ret;

	if (!PyArg_ParseTuple(
			args, "zzziII", &body, &title, &icon_path, &realtime,
			&timeout, &flags)) {
		return NULL;
	}

	wicon_path = Py2UTF16(icon_path);
	if (icon_path == NULL) {
		icon = NULL;
	} else {
		if (FAILED(LoadIconWithScaleDown(
				NULL, wicon_path, BalloonSize(), BalloonSize(), &icon))) {
			PyErr_SetString(
				PyExc_FileNotFoundError, "Could not load or locate icon.");
			return NULL;
		}
	}
	ret = Notify(Py2UTF16(body), Py2UTF16(title), icon, realtime, timeout, flags);
	DestroyIcon(icon);
	switch (ret) {
		case TRUE:
			Py_RETURN_TRUE;
		case FALSE:
			Py_RETURN_FALSE;
	}
}

static PyObject* notify_NotifyFromResource(PyObject *self, PyObject *args) {

	const char *body, *title, *resource;
	u_int ordinal;
	const wchar_t *wresource;
	int realtime;
	u_int timeout, flags;
	HMODULE lib;
	HICON icon;
	BOOL ret;

	if (!PyArg_ParseTuple(
			args, "zz(zI)iII", &body, &title, &resource, &ordinal, &realtime,
			&timeout, &flags)) {
		return NULL;
	}

	wresource = Py2UTF16(resource);
	lib = LoadLibraryEx(wresource, NULL, LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE);
	if (lib == NULL) {
		PyErr_SetString(
			PyExc_FileNotFoundError, "Could not load or locate resource.");
		return NULL;
	}
	if (FAILED(LoadIconWithScaleDown(
			lib, MAKEINTRESOURCE(ordinal), BalloonSize(), BalloonSize(), &icon))) {
		PyErr_SetString(
			PyExc_FileNotFoundError, "Could not load or locate icon.");
		return NULL;
	}
	ret = Notify(Py2UTF16(body), Py2UTF16(title), icon, realtime, timeout, flags);
	DestroyIcon(icon);
	FreeLibrary(lib);
	switch (ret) {
		case TRUE:
			Py_RETURN_TRUE;
		case FALSE:
			Py_RETURN_FALSE;
	}
}

static PyObject* notify_NotifyFromBuffer(PyObject *self, PyObject *args) {

	const char *body, *title, *buffer;
	int length, realtime;
	u_int timeout, flags;
	HICON icon;
	BOOL ret;

	if (!PyArg_ParseTuple(
			args, "zzy#iII", &body, &title, &buffer, &length, &realtime,
			&timeout, &flags)) {
		return NULL;
	}

	if (!memcmp(buffer, PNGHEAD, 8)) {
		icon = CreateIconFromResourceEx(
			(BYTE *)buffer, length, TRUE, 0x00030000, BalloonSize(), BalloonSize(), 0);
	} else if (!memcmp(buffer, ICOHEAD, 4)) {
		int offset = LookupIconIdFromDirectoryEx(
			(BYTE *)buffer, TRUE, BalloonSize(), BalloonSize(), 0);
		icon = CreateIconFromResourceEx(
			(BYTE *)buffer + offset, length - offset, TRUE, 0x00030000,
			BalloonSize(), BalloonSize(), 0);
	} else {
		PyErr_SetString(PyExc_TypeError, "Unrecognized buffer type.");
		return NULL;
	}
	ret = Notify(Py2UTF16(body), Py2UTF16(title), icon, realtime, timeout, flags);
	DestroyIcon(icon);
	switch (ret) {
		case TRUE:
			Py_RETURN_TRUE;
		case FALSE:
			Py_RETURN_FALSE;
	}
}

static PyObject* notify_IsVisible(PyObject *self, PyObject *args) {

	switch (VISIBLE) {
		case TRUE:
			Py_RETURN_TRUE;
		case FALSE:
			Py_RETURN_FALSE;
	}
}


// Python Interface
static PyMethodDef NotifyMethods[] = {

	{"InitFromIcon", notify_InitFromIcon, METH_VARARGS, NULL},
	{"InitFromResource", notify_InitFromResource, METH_VARARGS, NULL},
	{"InitFromBuffer", notify_InitFromBuffer, METH_VARARGS, NULL},
	{"Uninit", notify_Uninit, METH_NOARGS, NULL},
	{"IsInitted", notify_IsInitted, METH_NOARGS, NULL},
	{"NotifyFromIcon", notify_NotifyFromIcon, METH_VARARGS, NULL},
	{"NotifyFromResource", notify_NotifyFromResource, METH_VARARGS, NULL},
	{"NotifyFromBuffer", notify_NotifyFromBuffer, METH_VARARGS, NULL},
	{"IsVisible", notify_IsVisible, METH_NOARGS, NULL},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef _notifymodule = {

	PyModuleDef_HEAD_INIT,
	"_notify",
	NULL,
	-1,
	NotifyMethods
};

PyMODINIT_FUNC PyInit__notify(void) {

	INITCOMMONCONTROLSEX ccx;
	HMODULE comctl;

	ccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ccx.dwICC = 0;
	InitCommonControlsEx(&ccx);
	comctl = GetModuleHandle(L"comctl32");
	LIWSD = (LIWSDP)GetProcAddress(comctl, "LoadIconWithScaleDown");

	return PyModule_Create(&_notifymodule);
}
