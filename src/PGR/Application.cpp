#include "Application.h"

namespace PGR {

	Application::Application(
		int argc, char** argv,
		const std::string& name,
		const int width, const int height)
		: m_Name(name), m_Width(width), m_Height(height) {

		Init();
	}

	Application::~Application() {
		Terminate();
	}

	void Application::Init() {
		if (_chdir("../../resources"))
			exit(1);
		Window::Init();
		m_Window = Window::Create(m_Name, m_Width, m_Height);

		m_Framebuffer = Framebuffer::Create(m_Width, m_Height);
		m_Framebuffer->LoadFontTTF("Exo-Regular");
	}

	void Application::Terminate() {
		delete m_Window;
		Window::Terminate();
	}

	void Application::Run() {
		while (!m_Window->Closed()) {
			m_Framebuffer->Clear(Vec3(0.09f, 0.10f, 0.14f));
			m_Window->PollInputEvents();

			float deltaTime = (std::chrono::steady_clock::now() - m_LastFrameTime).count() * 0.001f;

			m_LastFrameTime = std::chrono::steady_clock::now();

			OnUpdate(deltaTime);
			m_Window->DrawFramebuffer(m_Framebuffer);
		}
	}

	void Application::OnUpdate(float time) {
		float FPS = 1000000.0f / time;
		m_Framebuffer->DrawTextTTF(0, 0, std::to_string(FPS), Vec3(1.0f, 1.0f, 1.0f), 20.0f);
	}

}
