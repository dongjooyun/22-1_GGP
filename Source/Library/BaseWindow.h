#pragma once

#include "Common.h"

namespace library
{

	template <class DerivedType>
	class BaseWindow
	{
	public:
		static LRESULT WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

		BaseWindow();
		~BaseWindow();

		virtual HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) = 0;
		virtual PCWSTR GetWindowClassName() const = 0;
		virtual LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) = 0;

		HWND GetWindow() const;

	protected:
		HINSTANCE m_hInstance;
		HWND m_hWnd;
		LPCWSTR m_pszWindowName;

		HRESULT initialize(
			_In_ HINSTANCE hInstance,
			_In_ INT nCmdShow,
			_In_ PCWSTR pszWindowName,
			_In_ DWORD dwStyle,
			_In_opt_ INT x = CW_USEDEFAULT,
			_In_opt_ INT y = CW_USEDEFAULT,
			_In_opt_ INT nWidth = CW_USEDEFAULT,
			_In_opt_ INT nHeight = CW_USEDEFAULT,
			_In_opt_ HWND hWndParent = nullptr,
			_In_opt_ HMENU hMenu = nullptr
		);
	};

	template <class DerivedType>
	LRESULT BaseWindow<DerivedType>::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DerivedType* pThis = nullptr;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			pThis = reinterpret_cast<DerivedType*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
			pThis->m_hWnd = hWnd;
		}
		else
		{
			pThis = reinterpret_cast<DerivedType*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}

		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}