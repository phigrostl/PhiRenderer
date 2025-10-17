#pragma once
#include "PGR/Base/Maths.h"
#include "PGR/Window/Framebuffer.h"

#include <string>
#include <stb_image/stb_image.h>
#include <stb_image/stb_image_resize2.h>

namespace PGR {

	class Texture {
	public:
		Texture(const std::string& path);
		Texture(const float value);
		Texture(const Vec4& value);
		~Texture();

		Vec4 Sample(Vec2 texCoords, bool enableLerp = true, Vec4 defaultValue = Vec4(0.0f)) const;
		float SampleFloat(Vec2 texCoords, bool enableLerp = true, float defaultValue = 0.0f) const;
		Vec4 GetColor(int x, int y) const { return m_Data[x + y * m_Width]; }

		int GetWidth() const { return m_Width; }
		int GetHeight() const { return m_Height; }
		std::string GetPath() const { return m_Path; }

		Texture* ClipImg(int y0, int y1, bool reserve = true);
		Texture* ClipBlockImg(int x0, int y0, int x1, int y1, bool reserve = true);
		Texture* ColorTexture(Vec4 color, bool reserve = true);
		Texture* GetBlurImg(float radius, bool reserve = true);

		void SetColor(int x, int y, Vec4 color) { m_Data[x + y * m_Width] = color; }

	private:
		void Init();

	private:
		int m_Width, m_Height, m_Channels;
		std::string m_Path;
		Vec4* m_Data;
	};

}
