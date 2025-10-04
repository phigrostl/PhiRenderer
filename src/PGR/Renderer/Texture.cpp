#include "Texture.h"

namespace PGR {

	Texture::Texture(const std::string& path)
		: m_Path(path) {
		Init();
	}

	Texture::Texture(const float value) {
		m_Width = 1;
		m_Height = 1;
		m_Channels = 4;
		m_Data = new Vec4[1];
		m_Data[0] = Vec4(value, value, value, value);
	}

	Texture::Texture(const Vec4& value) {
		m_Width = 1;
		m_Height = 1;
		m_Channels = 4;
		m_Data = new Vec4[1];
		m_Data[0] = value;
	}

	Texture::~Texture() {
		if (m_Data)
			delete[] m_Data;
		m_Data = nullptr;
	}

	void Texture::Init() {
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		data = stbi_load(m_Path.c_str(), &width, &height, &channels, 0);
		ASSERT(data);

		m_Height = height;
		m_Width = width;
		m_Channels = channels;
		int size = width * height;
		m_Data = new Vec4[size];

		switch (channels) {
		case 4:
			for (int i = 0; i < size; i++) {
				m_Data[i] = Vec4(
					UChar2Float(data[i * 4]),
					UChar2Float(data[i * 4 + 1]),
					UChar2Float(data[i * 4 + 2]),
					UChar2Float(data[i * 4 + 3])
				);
			}
			break;

		case 3:
			for (int i = 0; i < size; i++) {
				m_Data[i] = Vec4(
					UChar2Float(data[i * 3]),
					UChar2Float(data[i * 3 + 1]),
					UChar2Float(data[i * 3 + 2]),
					0.0f
				);
			}
			break;

		case 2:
			for (int i = 0; i < size; i++) {
				m_Data[i] = Vec4(
					UChar2Float(data[i * 2]),
					UChar2Float(data[i * 2 + 1]),
					0.0f,
					0.0f
				);
			}
			break;

		case 1:
			for (int i = 0; i < size; i++) {
				m_Data[i] = Vec4(
					UChar2Float(data[i]),
					0.0f,
					0.0f,
					0.0f
				);
			}
			break;

		default:
            ASSERT(false);
			break;
		}
	}

	Vec4 Texture::Sample(Vec2 texCoords, bool enableLerp, Vec4 defaultValue) const {
		if (this == nullptr)
			return defaultValue;
		if (m_Data == nullptr)
			return defaultValue;
		if (!enableLerp) {
			float vx = Clamp(texCoords.X, 0.0f, 1.0f);
			float vy = Clamp(texCoords.Y, 0.0f, 1.0f);

			int x = (int)(vx * (m_Width - 1) + 0.5f);
			int y = (int)(vy * (m_Height - 1) + 0.5f);

			int index = x + y * m_Width;
			return m_Data[index];
		}
		else {
			float vx = Clamp(texCoords.X, 0.0f, 1.0f);
			float vy = Clamp(texCoords.Y, 0.0f, 1.0f);

			float fx = vx * (m_Width - 1);
			float fy = vy * (m_Height - 1);

			int x0 = (int)fx;
			int y0 = (int)fy;
			int x1 = (int)Clamp((float)x0 + 1.0f, 0.0f, (float)m_Width - 1.0f);
			int y1 = (int)Clamp((float)y0 + 1.0f, 0.0f, (float)m_Height - 1.0f);

			float dx = fx - x0;
			float dy = fy - y0;

			Vec4 c00 = m_Data[x0 + y0 * m_Width];
			Vec4 c10 = m_Data[x1 + y0 * m_Width];
			Vec4 c01 = m_Data[x0 + y1 * m_Width];
			Vec4 c11 = m_Data[x1 + y1 * m_Width];

			Vec4 c0 = c00 * (1 - dx) + c10 * dx;
			Vec4 c1 = c01 * (1 - dx) + c11 * dx;
			Vec4 c = c0 * (1 - dy) + c1 * dy;

			return c;
		}
	}

	float Texture::SampleFloat(Vec2 texCoords, bool enableLerp, float defaultValue) const {
		if (this == nullptr)
			return defaultValue;
		Vec4 c;
		switch (m_Channels) {
		case 1:
			return Sample(texCoords, enableLerp, Vec4(defaultValue, 0.0f, 0.0f, 0.0f)).X;
		case 2:
			c = Sample(texCoords, enableLerp, Vec4(defaultValue, defaultValue, 0.0f, 0.0f));
			return (c.X + c.Y) / 2.0f;
		case 3:
			c = Sample(texCoords, enableLerp, Vec4(defaultValue, defaultValue, defaultValue, 0.0f));
			return (c.X + c.Y + c.Z) / 3.0f;
		case 4:
			c = Sample(texCoords, enableLerp, Vec4(defaultValue, defaultValue, defaultValue, defaultValue));
			return (c.X + c.Y + c.Z + c.W) / 4.0f;
		default:
			return defaultValue;
		}
	}

	Texture* Texture::ClipImg(int y0, int y1) {
		if (!this || y0 < 0 || y1 <= y0 || y1 > this->GetHeight())
			return new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));

		Texture* newTexture = new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
		newTexture->m_Width = this->GetWidth();
		newTexture->m_Height = y1 - y0;
		newTexture->m_Channels = this->m_Channels;
		newTexture->m_Path = this->GetPath() + "_clipped";

		int newSize = newTexture->m_Width * newTexture->m_Height;
		delete[] newTexture->m_Data;
		newTexture->m_Data = new Vec4[newSize];

		for (int y = 0; y < newTexture->m_Height; y++) {
			for (int x = 0; x < newTexture->m_Width; x++) {
				int srcIndex = x + (y + y0) * this->GetWidth();
				int dstIndex = x + y * newTexture->m_Width;
				newTexture->m_Data[dstIndex] = this->m_Data[srcIndex];
			}
		}

		return newTexture;
	}

	Texture* Texture::ClipBlockImg(int x0, int y0, int x1, int y1) {
		if (!this || x0 < 0 || y0 < 0 || x1 <= x0 || y1 <= y0 || x1 > this->GetWidth() || y1 > this->GetHeight()) {
			return new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
		}

		Texture* newTexture = new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
		newTexture->m_Width = x1 - x0;
		newTexture->m_Height = y1 - y0;
		newTexture->m_Channels = this->m_Channels;
		newTexture->m_Path = this->GetPath() + "_blockclipped";

		int newSize = newTexture->m_Width * newTexture->m_Height;
		newTexture->m_Data = new Vec4[newSize];

		for (int y = 0; y < newTexture->m_Height; y++) {
			for (int x = 0; x < newTexture->m_Width; x++) {
				Vec4 color = this->GetColor(x + x0, y + y0);
				newTexture->SetColor(x, y, color);
			}
		}

		return newTexture;
	}

	Texture* Texture::ColorTexture(Vec4 color) {
		Texture* newTexture = new Texture(color);
		newTexture->m_Width = this->GetWidth();
		newTexture->m_Height = this->GetHeight();
		newTexture->m_Channels = this->m_Channels;
		newTexture->m_Path = this->GetPath() + "_colored";

		int newSize = newTexture->m_Width * newTexture->m_Height;
		delete[] newTexture->m_Data;
		newTexture->m_Data = new Vec4[newSize];

		for (int j = 0; j < this->GetHeight(); j++) {
			for (int i = 0; i < this->GetWidth(); i++) {
				newTexture->SetColor(i, j, color * this->GetColor(i, j));
			}
		}

		return newTexture;
	}

	Texture* Texture::GetBlurImg(int radius) {
		if (!this || radius <= 0 || this->GetWidth() <= 0 || this->GetHeight() <= 0) {
			Texture* copyTexture = new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
			copyTexture->m_Width = this->GetWidth();
			copyTexture->m_Height = this->GetHeight();
			copyTexture->m_Channels = this->m_Channels;
			copyTexture->m_Path = this->GetPath() + "_copy";

			int size = copyTexture->m_Width * copyTexture->m_Height;
			delete[] copyTexture->m_Data;
			copyTexture->m_Data = new Vec4[size];

			for (int y = 0; y < copyTexture->m_Height; y++) {
				for (int x = 0; x < copyTexture->m_Width; x++) {
					copyTexture->SetColor(x, y, this->GetColor(x, y));
				}
			}

			return copyTexture;
		}

		Texture* tempTexture = new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
		Texture* blurTexture = new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
		
		for (auto tex : {tempTexture, blurTexture}) {
			tex->m_Width = this->GetWidth();
			tex->m_Height = this->GetHeight();
			tex->m_Channels = this->m_Channels;
			tex->m_Path = this->GetPath() + "_blur";
			int size = tex->m_Width * tex->m_Height;
			delete[] tex->m_Data;
			tex->m_Data = new Vec4[size];
		}

		std::vector<float> weights(radius * 2 + 1);
		float sigma = radius / 2.0f;
		float twoSigmaSquare = 2.0f * sigma * sigma;
		float sum = 0.0f;
		
		for (int i = -radius; i <= radius; i++) {
			int index = i + radius;
			weights[index] = exp(-(i * i) / twoSigmaSquare);
			sum += weights[index];
		}
		
		for (float& weight : weights) {
			weight /= sum;
		}

		for (int y = 0; y < this->GetHeight(); y++) {
			for (int x = 0; x < this->GetWidth(); x++) {
				Vec4 blurredColor(0.0f, 0.0f, 0.0f, 0.0f);
				for (int i = -radius; i <= radius; i++) {
					int nx = x + i;
					if (nx >= 0 && nx < this->GetWidth()) {
						int weightIndex = i + radius;
						blurredColor += this->GetColor(nx, y) * weights[weightIndex];
					}
				}
				tempTexture->SetColor(x, y, blurredColor);
			}
		}

		for (int x = 0; x < this->GetWidth(); x++) {
			for (int y = 0; y < this->GetHeight(); y++) {
				Vec4 blurredColor(0.0f, 0.0f, 0.0f, 0.0f);
				for (int j = -radius; j <= radius; j++) {
					int ny = y + j;
					if (ny >= 0 && ny < this->GetHeight()) {
						int weightIndex = j + radius;
						blurredColor += tempTexture->GetColor(x, ny) * weights[weightIndex];
					}
				}
				blurTexture->SetColor(x, y, blurredColor);
			}
		}

		delete tempTexture;

		return blurTexture;
	}

}
