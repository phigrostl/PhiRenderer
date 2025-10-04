#pragma once

#include "PGR/Base/Maths.h"
#include "PGR/Renderer/Texture.h"

#include <Windows.h>
#include <fstream>
#include <stb_image/stb_truetype.h>

namespace PGR {

	class Framebuffer {
	public:
		Framebuffer(const int width, const int height);
		~Framebuffer();

		int GetWidth() const { return m_Width; }
		int GetHeight() const { return m_Height; }

		void SetColor(const int x, const int y, const Vec4& color);
		Vec3 GetColor(const int x, const int y) const;

		void Clear(const Vec3& color = Vec3(0.0f, 0.0f, 0.0f));

		// short
		void LoadFontTTF(const std::string& fontPath);
		void DrawCharTTF(int x, int y, char c, const Vec4& color, float fontSize);
		void DrawTextTTF(int x, int y, const std::string& text, const Vec4& color, float fontSize);

		// wide
		void LoadWFontTTF(const std::wstring& fontPath);
		void DrawWCharTTF(int x, int y, wchar_t c, const Vec4& color, float fontSize);
		void DrawWTextTTF(int x, int y, const std::wstring& text, const Vec4& color, float fontSize);

		void DrawLine(int x0, int y0, int x1, int y1, float w, const Vec4& color);
		void FillRect(int x0, int y0, int x1, int y1, const Vec4& color);
		void FillSizeRect(int x, int y, int w, int h, const Vec4& color);

		static Framebuffer* Create(const int width, const int height);

	private:
		int GetPixelIndex(const int x, const int y) const { return (y * m_Width + x) * 3; }

	private:
		int m_Width;
		int m_Height;
		int m_PixelSize;
		Vec3* m_ColorBuffer;

		stbtt_fontinfo m_FontInfo;
		std::vector<unsigned char> m_fontBuffer;
	};

}
