#include "Framebuffer.h"

namespace PGR {

	Framebuffer::Framebuffer(const int width, const int height)
		: m_Width(width), m_Height(height) {
		ASSERT(width > 0 && height > 0);
		m_PixelSize = m_Width * m_Height;
		m_ColorBuffer = new Vec3[m_PixelSize]();
		Clear();
	}

	Framebuffer::~Framebuffer() {
		delete[] m_ColorBuffer;
		m_ColorBuffer = nullptr;
	}

	void Framebuffer::SetColor(const int x, const int y, const Vec4& color) {
		if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) {
			ASSERT(false);
			return;
		}

		const int index = x + y * m_Width;

		const float alpha = color.W;
		if (alpha >= 1.0f) {
			m_ColorBuffer[index] = Vec3(color.X, color.Y, color.Z);
		}
		else if (alpha > 0.0f) {
			Vec3& target = m_ColorBuffer[index];
			target = target * (1.0f - alpha) + Vec3(color.X, color.Y, color.Z) * alpha;
		}
	}

	Vec3 Framebuffer::GetColor(const int x, const int y) const {
		if (x >= 0 && x < m_Width && y >= 0 && y < m_Height)
			return m_ColorBuffer[x + y * m_Width];
		else
			ASSERT(false);
		return Vec3(0.0f, 0.0f, 0.0f);
	}

	void Framebuffer::Clear(const Vec3& color) {
		for (int i = 0; i < m_PixelSize; i++)
			m_ColorBuffer[i] = color;
	}

	// short
	void Framebuffer::LoadFontTTF(const std::string& fontPath) {
		std::ifstream file(fontPath, std::ios::binary);
		if (!file) {
			file.open("C:\\Windows\\Fonts\\" + fontPath + ".ttf", std::ios::binary);
			if (!file) {
				file.open("C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\Windows\\Fonts" + fontPath + ".ttf", std::ios::binary);
				if (!file) {
					file.open(fontPath + ".ttf", std::ios::binary);
					if (!file) file.open("C:\\Windows\\Fonts\\arial.ttf", std::ios::binary);
				}
			}
		}
		m_fontBuffer = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		stbtt_InitFont(&m_FontInfo, m_fontBuffer.data(), 0);
	}

	void Framebuffer::DrawCharTTF(int x, int y, char c, const Vec4& color, float fontSize) {
		unsigned char* bitmap;
		int w, h, xoff, yoff;
		float scale = stbtt_ScaleForPixelHeight(&m_FontInfo, fontSize);

		bitmap = stbtt_GetCodepointBitmap(&m_FontInfo, 0, scale, c, &w, &h, &xoff, &yoff);

		int ascent, descent, lineGap;;
		stbtt_GetFontVMetrics(&m_FontInfo, &ascent, &descent, &lineGap);
		int baseline = int(ascent * scale);

		for (int j = 0; j < h; j++) {
			for (int i = 0; i < w; i++) {
				float alpha = bitmap[i + j * w] / 255.0f;
				if (alpha > 0.1f) {
					Vec4 col = color * alpha;
					SetColor(x + i + xoff, m_Height - (y + j + yoff + baseline), col);
				}
			}
		}
		stbtt_FreeBitmap(bitmap, nullptr);
	}

	void Framebuffer::DrawTextTTF(int x, int y, const std::string& text, const Vec4& color, float fontSize) {
		float scale = stbtt_ScaleForPixelHeight(&m_FontInfo, fontSize);
		int xpos = x;

		for (char c : text) {
			int ax;
			int lsb;
			stbtt_GetCodepointHMetrics(&m_FontInfo, c, &ax, &lsb);
			DrawCharTTF(xpos, y, c, color, fontSize);
			int kern = stbtt_GetCodepointKernAdvance(&m_FontInfo, 0, c);
			xpos += int(ax * scale) + kern;
		}
	}

	void Framebuffer::DrawCenterTextTTF(int x, int y, const std::string& text, const Vec4& color, float fontSize, float rotation) {
		float scale = stbtt_ScaleForPixelHeight(&m_FontInfo, fontSize);
		int width = 0;
		int maxH = 0;
		for (char c : text) {
			int ax, lsb, w, h, xoff, yoff;
			stbtt_GetCodepointHMetrics(&m_FontInfo, c, &ax, &lsb);
			stbtt_GetCodepointBitmapBox(&m_FontInfo, c, scale, scale, &xoff, &yoff, &w, &h);
			width += int(ax * scale);
			if (h > maxH) maxH = h;
		}

		float cx = x - width / 2.0f;
		float cy = y - maxH / 2.0f;

		float xpos = 0.0f;
		float rad = rotation * 3.14159265f / 180.0f;
		float cosA = cosf(rad);
		float sinA = sinf(rad);

		for (char c : text) {
			int ax, lsb, w, h, xoff, yoff;
			stbtt_GetCodepointHMetrics(&m_FontInfo, c, &ax, &lsb);
			unsigned char* bitmap = stbtt_GetCodepointBitmap(&m_FontInfo, 0, scale, c, &w, &h, &xoff, &yoff);

			for (int j = 0; j < h; ++j) {
				for (int i = 0; i < w; ++i) {
					float alpha = bitmap[i + j * w] / 255.0f;
					if (alpha > 0.1f) {
						
						float px = cx + xpos + i + xoff;
						float py = cy + j + yoff;
						
						float dx = px - x;
						float dy = py - y;
						int fx = int(x + dx * cosA - dy * sinA);
						int fy = int(y + dx * sinA + dy * cosA);
						if (fx >= 0 && fx < m_Width && fy >= 0 && fy < m_Height) {
							Vec4 col = alpha > 0.5f ? color : Vec4(0.0f, 0.0f);
							SetColor(fx, m_Height - fy, col);
						}
					}
				}
			}
			stbtt_FreeBitmap(bitmap, nullptr);
			xpos += int(ax * scale);
		}
	}

	// wide
	void Framebuffer::LoadWFontTTF(const std::wstring& fontPath) {
		std::wifstream file(fontPath, std::ios::binary);
		if (!file) {
			file.open(L"C:\\Windows\\Fonts\\" + fontPath + L".ttf", std::ios::binary);
			if (!file) return;
		}
		m_fontBuffer = std::vector<unsigned char>((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());
		file.close();
		stbtt_InitFont(&m_FontInfo, m_fontBuffer.data(), 0);
	}

	void Framebuffer::DrawWCharTTF(int x, int y, wchar_t c, const Vec4& color, float fontSize) {
		int w, h, xoff, yoff;
		float scale = stbtt_ScaleForPixelHeight(&m_FontInfo, fontSize);

		unsigned char* bitmap = stbtt_GetCodepointBitmap(
			&m_FontInfo, 0, scale, c, &w, &h, &xoff, &yoff);

		for (int j = 0; j < h; j++) {
			for (int i = 0; i < w; i++) {
				float alpha = bitmap[i + j * w] / 255.0f;
				if (alpha > 0.1f) {
					int px = x + i + xoff;
					int py = m_Height - (y + j + yoff);
					if (px >= 0 && px < m_Width && py >= 0 && py < m_Height) {
						Vec3 dst = GetColor(px, py);
						Vec4 blended = Vec4(color * alpha + dst * (1.0f - alpha), 1.0f);
						SetColor(px, py, blended);
					}
				}
			}
		}
		stbtt_FreeBitmap(bitmap, nullptr);
	}

	void Framebuffer::DrawWTextTTF(int x, int y, const std::wstring& text, const Vec4& color, float fontSize) {
		float scale = stbtt_ScaleForPixelHeight(&m_FontInfo, fontSize);
		int xpos = x;
		int ascent, descent, lineGap;;
		stbtt_GetFontVMetrics(&m_FontInfo, &ascent, &descent, &lineGap);
		int baseline = int(ascent * scale);
		for (wchar_t c : text) {
			int ax;
			int lsb;
			stbtt_GetCodepointHMetrics(&m_FontInfo, c, &ax, &lsb);
			DrawWCharTTF(xpos, y + baseline, c, color, fontSize);
			int kern = stbtt_GetCodepointKernAdvance(&m_FontInfo, 0, c);
			xpos += int(ax * scale) + kern;
		}
	}

	void Framebuffer::DrawLine(int x0, int y0, int x1, int y1, float w, const Vec4& color) {
		int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx + dy, e2;

		float halfW = w * 0.5f;
		while (true) {
			for (int wx = -int(halfW); wx <= int(halfW); ++wx) {
				for (int wy = -int(halfW); wy <= int(halfW); ++wy) {
					int px = x0 + wx;
					int py = y0 + wy;
					float dist = sqrtf((float)(wx * wx + wy * wy));
					if (dist <= halfW + 0.5f) {
						SetColor(px, py, color);
					}
				}
			}
			if (x0 == x1 && y0 == y1) break;
			e2 = 2 * err;
			if (e2 >= dy) { err += dy; x0 += sx; }
			if (e2 <= dx) { err += dx; y0 += sy; }
		}
	}

	void Framebuffer::FillRect(int x0, int y0, int x1, int y1, const Vec4& color) {
		if (x0 > x1) std::swap(x0, x1);
		if (y0 > y1) std::swap(y0, y1);
		for (int y = y0; y <= y1; ++y) {
			for (int x = x0; x <= x1; ++x) {
				SetColor(x, m_Height - y, color);
			}
		}
	}

	void Framebuffer::FillSizeRect(int x, int y, int w, int h, const Vec4& color) {
		FillRect(x - w / 2, y - h / 2, x + w / 2, y + h / 2, color);
	}

	void Framebuffer::Resize(int width, int height) {
		m_Width = width;
		m_Height = height;
		m_PixelSize = width * height;
		delete[] m_ColorBuffer;
		m_ColorBuffer = new Vec3[(size_t)width * (size_t)height];
	}

	Framebuffer* Framebuffer::Create(const int width, const int height) {
		return new Framebuffer(width, height);
	}

}
