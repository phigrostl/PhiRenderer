#pragma once

#include "PGR/Window/Framebuffer.h"
#include "PGR/Base/Base.h"
#include "PGR/Window/InputCode.h"

#include <string>
#include <Windows.h>

namespace PGR {

	class Window {
	public:
		Window(const std::string title, int width, int height);
		~Window();

		static void Init();
		static void Terminate();
		static Window* Create(const std::string title, int width, int height);

		void Show();
		void DrawFramebuffer(Framebuffer* framebuffer);

		bool Closed() const { return m_Closed; }
		char GetKey(const uint32_t index) const { return m_Keys[index]; }

		int GetMouseX() const { return m_MouseX; }
		int GetMouseY() const { return m_MouseY; }
		int GetMsg() const { return m_Msg; }

		static void Register();
		static void Unregister();

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam);
		static void SetMsg(Window* window, UINT msgID, const WPARAM wParam, const LPARAM lParam);

		static void PollInputEvents();


	protected:
		std::string m_Title;
		int m_Width, m_Height;
		bool m_Closed = true;

		char m_Keys[PGR_KEY_MAX_COUNT];

		HWND m_Handle;
		HDC m_MemoryDC;
		unsigned char* m_Buffer;

		static bool s_Inited;

		int m_MouseX, m_MouseY;
		unsigned int m_Msg;

	};

}
