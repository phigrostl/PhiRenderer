#pragma once

#include "PGR/Window/Window.h"

#include <chrono>
#include <string>
#include <fstream>
#include <thread>
#include <direct.h>

namespace PGR {

	class Application {
	public:
		Application(
			int argc, char** argv,
			const std::string& name,
			const int width, const int height
		);

		~Application();

		void Run();

	private:
		void Init();
		void Terminate();

		void OnUpdate(float time);

	private:
		std::string m_Name;
		int m_Width, m_Height;

		Window* m_Window;
		Framebuffer* m_Framebuffer;

		std::chrono::steady_clock::time_point m_LastFrameTime;
	};

}
