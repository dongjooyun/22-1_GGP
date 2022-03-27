#pragma once

#include "Common.h"
#include "Window/BaseWindow.h"

namespace libraby
{
	class MainWindow
	{
	public:
		MainWindow();
		~MainWindow();
		HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) override;
		PCWSTR GetWindowClassName() const override;
		LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) override;
	};
}
