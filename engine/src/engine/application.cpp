#include "engine/engine.hpp"
#include <windows.h>
#include <windowsx.h>
#include <hidsdi.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace Engine
{
	void ThrowIfFailed(const bool Expression)
	{
		if (Expression)
		{
			RaiseException(GetLastError(), 0, 0, nullptr);
		}
	}

	class CApplication: public IApplication
	{
	private:
		HWND WindowHandle;
	public:
		CApplication(HWND WindowHandle): WindowHandle(WindowHandle)
		{
		}
	public:
		void* GetHandle() override
		{
			return this->WindowHandle;
		}

		void Close() override
		{
			DestroyWindow(this->WindowHandle);
		}

		void SetTitle(const std::string& Title) override
		{
			SetWindowTextW(this->WindowHandle, Utf8ToUnicode(Title).c_str());
		}

		void SetPosition(int X, int Y, int Width, int Height) override
		{
			const DWORD ExStyle = GetWindowLongW(this->WindowHandle, GWL_EXSTYLE), Style = GetWindowLongW(this->WindowHandle, GWL_STYLE);
			RECT Rect = { X, Y, Rect.left + Width, Rect.top + Height };
			ThrowIfFailed(!AdjustWindowRectEx(&Rect, Style, false, ExStyle));
			SetWindowPos(this->WindowHandle, nullptr, Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
		}

		int GetX() override
		{
			return GetRect().left;
		}

		int GetY() override
		{
			return GetRect().top;
		}

		int GetWidth() override
		{
			return GetRect().right;
		}

		int GetHeight() override
		{
			return GetRect().bottom;
		}
	private:
		RECT GetRect()
		{
			RECT Rect = {};
			GetClientRect(this->WindowHandle, &Rect);
			MapWindowPoints(this->WindowHandle, GetParent(this->WindowHandle), reinterpret_cast<LPPOINT>(&Rect), 2);
			return { Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top };
		}
	};

	LRESULT WINAPI WindowRedirectProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto Object = reinterpret_cast<IMain*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
		const std::unique_ptr<Engine::IApplication> App = std::make_unique<CApplication>(hWnd);

		switch (message)
		{

		case WM_CREATE:
		{
			const RAWINPUTDEVICE RawInputDevice[] = {
				{ HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_MOUSE, 0, hWnd },
				{ HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_KEYBOARD, 0, hWnd },
			};
			RegisterRawInputDevices(RawInputDevice, ARRAYSIZE(RawInputDevice), sizeof(RawInputDevice[0]));

			Object->Create(App);

			return 0;
		}

		case WM_PAINT:
			Object->Update(App);
			return 0;

		case WM_MOVE:
			Object->Move(App, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_SIZE:
			Object->Resize(App, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_MOUSEMOVE:
			Object->CursorMove(App, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_INPUT:
		{
			RAWINPUT RawInput = {};
			UINT RawInputSize = sizeof(RawInput);

			GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &RawInput, &RawInputSize, sizeof(RAWINPUTHEADER));

			switch (RawInput.header.dwType)
			{
			case RIM_TYPEMOUSE:
				Object->MouseMove(App, RawInput.data.mouse.lLastX, RawInput.data.mouse.lLastY);
				break;
			case RIM_TYPEKEYBOARD:
				Object->Keyboard(App, RawInput.data.keyboard.VKey, !(RawInput.data.keyboard.Flags & RI_KEY_BREAK));
				break;
			}
			return 0;
		}

		case WM_DESTROY:
			Object->Destroy(App);
			PostQuitMessage(0);
			return 0;

		}

		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	LRESULT WINAPI WindowSetupProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_NCCREATE)
		{
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<LPCREATESTRUCTW>(lParam)->lpCreateParams));
			SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowRedirectProc));
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	LONG WINAPI ExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
	{
		const DWORD ErrorCode = ExceptionInfo->ExceptionRecord->ExceptionCode;

		constexpr DWORD ErrorStringSize = 0xff;
		WCHAR ErrorString[ErrorStringSize] = { L"Unknown error" };
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, ErrorCode, 0, ErrorString, ErrorStringSize, nullptr);

		FatalAppExitW(0, ErrorString);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	int Run(IMain& MainClass)
	{
		SetUnhandledExceptionFilter(ExceptionFilter);

		WNDCLASSEXW WindowClass = {
			.cbSize = sizeof(WindowClass),
			.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
			.lpfnWndProc = WindowSetupProc,
			.hInstance = GetModuleHandleW(nullptr),
			.hIcon = LoadIconW(nullptr, IDI_APPLICATION),
			.hCursor = LoadCursorW(nullptr, IDC_ARROW),
			.hbrBackground = GetStockBrush(NULL_BRUSH),
			.lpszClassName = L"WindowClass",
			.hIconSm = WindowClass.hIcon,
		};
		ThrowIfFailed(!RegisterClassExW(&WindowClass));

		HWND WindowHandle = CreateWindowExW(0, WindowClass.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			nullptr, nullptr, WindowClass.hInstance, &MainClass);
		ThrowIfFailed(!WindowHandle);
		ShowWindow(WindowHandle, SW_SHOW);
		UpdateWindow(WindowHandle);

		MSG Message = {};
		while (GetMessageW(&Message, nullptr, 0, 0) > 0)
		{
			TranslateMessage(&Message);
			DispatchMessageW(&Message);
		}

		DestroyWindow(WindowHandle);
		UnregisterClassW(WindowClass.lpszClassName, WindowClass.hInstance);

		return static_cast<int>(Message.wParam);
	}
}
