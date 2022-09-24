#pragma once

#include "types.hpp"
#include <wrl/client.h>
#include <d3d11.h>
#include <d3dcompiler.h>

namespace Engine
{
	std::wstring Utf8ToUnicode(const std::string& String);
	std::string UnicodeToUtf8(const std::wstring& String);

	double GetTime();
	int GetSceenWidth();
	int GetSceenHeight();

	std::string GetBasePath();
	std::string ReadFile(const std::string& FileName);

	class IApplication: public IBaseInterface
	{
	public:
		virtual void* GetHandle() = 0;
		virtual void Close() = 0;
		virtual void SetTitle(const std::string& Title) = 0;
		virtual void SetPosition(int X, int Y, int Width, int Height) = 0;
		virtual int GetX() = 0;
		virtual int GetY() = 0;
		virtual int GetWidth() = 0;
		virtual int GetHeight() = 0;
	};

	class IMain: public IBaseInterface
	{
	public:
		virtual void Create(const std::unique_ptr<IApplication>& App) {}
		virtual void Destroy(const std::unique_ptr<IApplication>& App) {}
		virtual void Update(const std::unique_ptr<IApplication>& App) {}

		virtual void Move(const std::unique_ptr<IApplication>& App, int X, int Y) {}
		virtual void Resize(const std::unique_ptr<IApplication>& App, int Width, int Height) {}

		virtual void CursorMove(const std::unique_ptr<IApplication>& App, int X, int Y) {}
		virtual void MouseMove(const std::unique_ptr<IApplication>& App, int DeltaX, int DeltaY) {}
		virtual void Keyboard(const std::unique_ptr<IApplication>& App, unsigned char KeyCode, bool Action) {}
	};

	int Run(IMain& MainClass);

	template<class T>
	int Run()
	{
		T MainClass;
		return Run(MainClass);
	}
}
