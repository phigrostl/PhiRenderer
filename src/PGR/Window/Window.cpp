#include "Window.h"

#define RTL_WINDOW_ENTRY_NAME "Entry"
#define RTL_WINDOW_CLASS_NAME "Class"

namespace PGR {

	bool Window::s_Inited = false;
	bool Window::s_IsWheel = false;

	Window::Window(const std::string title, int width, int height)
		: m_Title(title), m_Width(width), m_Height(height) {
		DWORD style = WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_THICKFRAME;
		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.bottom = (long)height;
		rect.right = (long)width;
		AdjustWindowRect(&rect, style, false);
		m_Handle = CreateWindow(RTL_WINDOW_CLASS_NAME, m_Title.c_str(), style,
			CW_USEDEFAULT, 0, rect.right - rect.left, rect.bottom - rect.top,
			NULL, NULL, GetModuleHandle(NULL), NULL);
		m_Closed = false;
		SetProp(m_Handle, RTL_WINDOW_ENTRY_NAME, this);

		HDC windowDC = GetDC(m_Handle);
		m_MemoryDC = CreateCompatibleDC(windowDC);

		BITMAPINFOHEADER biHeader = {};
		HBITMAP newBitmap;
		HBITMAP oldBitmap;

		biHeader.biSize = sizeof(BITMAPINFOHEADER);
		biHeader.biWidth = ((long)m_Width);
		biHeader.biHeight = -((long)m_Height);
		biHeader.biPlanes = 1;
		biHeader.biBitCount = 24;
		biHeader.biCompression = BI_RGB;

		newBitmap = CreateDIBSection(m_MemoryDC, (BITMAPINFO*)&biHeader, DIB_RGB_COLORS, (void**)&m_Buffer, nullptr, 0);
		ASSERT(newBitmap != NULL);
		constexpr int channelCount = 3;
		int size = m_Width * m_Height * channelCount * sizeof(unsigned char);
		memset(m_Buffer, 0, size);
		oldBitmap = (HBITMAP)SelectObject(m_MemoryDC, newBitmap);

		DeleteObject(oldBitmap);
		ReleaseDC(m_Handle, windowDC);

		Show();

		memset(m_Keys, PGR_RELEASE, PGR_KEY_MAX_COUNT);
	}

	Window::~Window() {
		ShowWindow(m_Handle, SW_HIDE);
		RemoveProp(m_Handle, RTL_WINDOW_ENTRY_NAME);
		DeleteDC(m_MemoryDC);
		DestroyWindow(m_Handle);
	}

	void Window::Init() {
		Register();
		s_Inited = true;
	}

	void Window::Terminate() {
		Unregister();
		s_Inited = false;
	}

	void Window::Register() {
		ATOM atom;
		WNDCLASS wc = { 0 };
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hbrBackground = (HBRUSH)(WHITE_BRUSH);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = NULL;
		wc.hInstance = GetModuleHandle(NULL);
		wc.lpfnWndProc = WndProc;
		wc.lpszClassName = RTL_WINDOW_CLASS_NAME;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpszMenuName = NULL;
		atom = RegisterClass(&wc);
	}

	void Window::Unregister() {
		UnregisterClass(RTL_WINDOW_CLASS_NAME, GetModuleHandle(NULL));
	}

	void Window::Show() {
		HDC windowDC = GetDC(m_Handle);
		BitBlt(windowDC, 0, 0, m_Width, m_Height, m_MemoryDC, 0, 0, SRCCOPY);
		ShowWindow(m_Handle, SW_SHOW);
		ReleaseDC(m_Handle, windowDC);
		PollInputEvents();
	}

	void Window::DrawFramebuffer(Framebuffer* framebuffer) {
		const int fWidth = framebuffer->GetWidth();
		const int fHeight = framebuffer->GetHeight();
		const int width = m_Width < fWidth ? m_Width : fWidth;
		const int height = m_Height < fHeight ? m_Height : fHeight;
		constexpr int channelCount = 3;

		int rowSize = m_Width * channelCount;
		if (rowSize % 4 != 0) {
			rowSize += 4 - (rowSize % 4);
		}

		unsigned char* buffer = m_Buffer;
		const int fHeightMinusOne = fHeight - 1;

		for (int i = 0; i < height; i++) {
			unsigned char* rowStart = buffer + i * rowSize;
			int framebufferY = fHeightMinusOne - i;

			for (int j = 0; j < width; j++) {
				Vec3 color = framebuffer->GetColor(j, framebufferY);

				rowStart[j * channelCount + 2] = Float2UChar(color.X); // R
				rowStart[j * channelCount + 1] = Float2UChar(color.Y); // G
				rowStart[j * channelCount + 0] = Float2UChar(color.Z); // B
			}
		}
		Show();
	}

	void Window::SetMsg(Window* window, UINT msgID, const WPARAM wParam, const LPARAM lParam) {
		window->m_Msg = msgID;
		if (msgID == WM_MOUSEMOVE) {
			window->m_MouseX = LOWORD(lParam);
			window->m_MouseY = HIWORD(lParam);
		}
	}

	LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam) {
		Window* window = (Window*)GetProp(hWnd, RTL_WINDOW_ENTRY_NAME);
		if (window == nullptr)
			return DefWindowProc(hWnd, msgID, wParam, lParam);
		SetMsg(window, msgID, wParam, lParam);
		switch (msgID) {
		case WM_DESTROY:
			window->m_Closed = true;
			return 0;
		case WM_KEYDOWN:
			window->m_Keys[wParam] = PGR_PRESS;
			return 0;
		case WM_KEYUP:
			window->m_Keys[wParam] = PGR_RELEASE;
			return 0;
		case WM_LBUTTONUP:
			window->m_Keys[PGR_BUTTON_LEFT] = PGR_RELEASE;
			return 0;
		case WM_LBUTTONDOWN:
			window->m_Keys[PGR_BUTTON_LEFT] = PGR_PRESS;
			return 0;
		case WM_RBUTTONUP:
			window->m_Keys[PGR_BUTTON_RIGHT] = PGR_RELEASE;
			return 0;
		case WM_RBUTTONDOWN:
			window->m_Keys[PGR_BUTTON_RIGHT] = PGR_PRESS;
			return 0;
		case WM_SIZE:
			window->Resize(LOWORD(lParam), HIWORD(lParam));
			return 0;
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE) {
				window->m_Active = false;
				puts("Inactive");
			}
			else {
				window->m_Active = true;
				puts("Active");
			}
			return 0;
		}
		if (msgID == WM_MOUSEWHEEL && !s_IsWheel) {
			window->m_MouseWheel = GET_WHEEL_DELTA_WPARAM(wParam);
			s_IsWheel = true;
			return 0;
		}
		else if (s_IsWheel) {
			window->m_MouseWheel = 0;
			s_IsWheel = false;
			return 0;
		}

		return DefWindowProc(hWnd, msgID, wParam, lParam);
	}

	void Window::PollInputEvents() {
		MSG message;
		PeekMessage(&message, NULL, 0, 0, PM_REMOVE);
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	void Window::Resize(int width, int height) {
		HDC windowDC = GetDC(m_Handle);
		long style = GetWindowLong(m_Handle, GWL_STYLE);

		HBITMAP oldBitmap = (HBITMAP)GetCurrentObject(m_MemoryDC, OBJ_BITMAP);
		HDC oldMemoryDC = m_MemoryDC;

		m_Width = width;
		m_Height = height;

		HDC newMemoryDC = CreateCompatibleDC(windowDC);

		BITMAPINFOHEADER biHeader = {};
		biHeader.biSize = sizeof(BITMAPINFOHEADER);
		biHeader.biWidth = static_cast<long>(width);
		biHeader.biHeight = -static_cast<long>(height);
		biHeader.biPlanes = 1;
		biHeader.biBitCount = 24;
		biHeader.biCompression = BI_RGB;

		void* newBuffer = nullptr;
		HBITMAP newBitmap = CreateDIBSection(newMemoryDC, (BITMAPINFO*)&biHeader, DIB_RGB_COLORS, &newBuffer, nullptr, 0);
		ASSERT(newBitmap != NULL);

		constexpr int channelCount = 3;

		int rowSize = width * channelCount;
		if (rowSize % 4 != 0) {
			rowSize += 4 - (rowSize % 4);
		}
		int bufferSize = rowSize * height;
		memset(newBuffer, 0, bufferSize);

		HBITMAP tempBitmap = (HBITMAP)SelectObject(newMemoryDC, newBitmap);

		m_Buffer = static_cast<unsigned char*>(newBuffer);
		m_MemoryDC = newMemoryDC;

		DeleteDC(oldMemoryDC);
		DeleteObject(oldBitmap);
		DeleteObject(tempBitmap);

		RECT rect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
		AdjustWindowRect(&rect, style, FALSE);
		SetWindowPos(m_Handle, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE);

		ReleaseDC(m_Handle, windowDC);
	}

	Window* Window::Create(const std::string title, int width, int height) {
		return new Window(title, width, height);
	}

}
