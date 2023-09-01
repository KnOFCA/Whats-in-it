#if 1
#include <windows.h>
#include <d2d1.h>
#include <dwmapi.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "Dwmapi")


#include "BaseWindow.hpp"

template <class T>
void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

class MainWindow : public BaseWindow<MainWindow>
{
	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	//user define
	ID2D1SolidColorBrush* pBrush;
	D2D1_ELLIPSE            ellipse;
	//user define end

	void    CalculateLayout();
	HRESULT CreateGraphicsResources();
	void    DiscardGraphicsResources();
	void    OnPaint();
	void    Resize();

public:

	MainWindow() : pFactory(NULL), pRenderTarget(NULL), pBrush(NULL)
	{
	}

	PCWSTR  ClassName() const {
		//user define
		return L"Circle Window Class";
		//user define end
	}
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

void MainWindow::CalculateLayout()
{
	if (pRenderTarget != NULL)
	{
		D2D1_SIZE_F size = pRenderTarget->GetSize();
		//user define
		const float x = size.width / 2;
		const float y = size.height / 2;
		const float radius = min(x, y);
		ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);
		//user define end
	}
}

HRESULT MainWindow::CreateGraphicsResources()
{
	HRESULT hr = S_OK;
	if (pRenderTarget == NULL)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		hr = pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&pRenderTarget);

		if (SUCCEEDED(hr))
		{
			//user define
			const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0.0f);
			hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
			//user define end

			if (SUCCEEDED(hr))
			{
				CalculateLayout();
			}
		}
	}
	return hr;
}

void MainWindow::DiscardGraphicsResources()
{
	SafeRelease(&pRenderTarget);
	SafeRelease(&pBrush);
}

void MainWindow::OnPaint()
{
	HRESULT hr = CreateGraphicsResources();
	if (SUCCEEDED(hr))
	{
		PAINTSTRUCT ps;
		BeginPaint(m_hwnd, &ps);

		pRenderTarget->BeginDraw();

		//user define
		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
		pRenderTarget->FillEllipse(ellipse, pBrush);
		//user define end

		hr = pRenderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			DiscardGraphicsResources();
		}
		EndPaint(m_hwnd, &ps);
	}
}

void MainWindow::Resize()
{
	if (pRenderTarget != NULL)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		pRenderTarget->Resize(size);
		CalculateLayout();
		InvalidateRect(m_hwnd, NULL, FALSE);
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	MainWindow win;

	if (!win.Create(L"Circle", WS_OVERLAPPEDWINDOW))
	{
		return 0;
	}

	ShowWindow(win.Window(), nCmdShow);

	//set transparent background
	UpdateWindow(win.Window());

	LONG ret = GetWindowLong(win.Window(), GWL_EXSTYLE);
	ret = ret | WS_EX_LAYERED;
	SetWindowLong(win.Window(), GWL_STYLE, GetWindowLong(win.Window(), GWL_STYLE) & ~WS_CAPTION);
	//end

	// Run the message loop.

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		if (FAILED(D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
		{
			return -1;  // Fail CreateWindowEx.
		}
		RECT rcClient;
		GetWindowRect(m_hwnd, &rcClient);

		// Inform the application of the frame change.
		SetWindowPos(m_hwnd,
			NULL,
			rcClient.left, rcClient.top,
			rcClient.right, rcClient.bottom,
			SWP_FRAMECHANGED);
		return 0;

	case WM_DESTROY:
		DiscardGraphicsResources();
		SafeRelease(&pFactory);
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
		OnPaint();
		return 0;

	case WM_SIZE:
		Resize();
		return 0;

	case WM_NCCALCSIZE:
		wParam = 1;
		return 0;

		//case
	}
	// Handle the window activation.
	if (uMsg == WM_ACTIVATE)
	{
		// Extend the frame into the client area.
		MARGINS margins;

		margins.cxLeftWidth = -20;      // LEFTEXTENDWIDTH
		margins.cxRightWidth = -20;    // RIGHTEXTENDWIDTH
		margins.cyBottomHeight = -20; // BOTTOMEXTENDWIDTH
		margins.cyTopHeight = -20;       // TOPEXTENDWIDTH

		auto hr = DwmExtendFrameIntoClientArea(m_hwnd, &margins);

		if (!SUCCEEDED(hr))
		{
			// Handle the error.
		}
		return 0;
	}
	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
#endif