#include "Application.h"
#include <Windows.h>

namespace PGR {

	float linear(float t, float st, float et, float sv, float ev) {
		return sv + (t - st) / (et - st) * (ev - sv);
	}

	Vec2 rotatePoint(float x, float y, float r, float deg) {
		return Vec2(
			x + r * cos(deg * PI / 180.0f),
			y + r * sin(deg * PI / 180.0f)
		);
	}

	float randf(float a, float b) {
		static std::mt19937 rng{ std::random_device{}() };
		std::uniform_real_distribution<float> dist(a, b);
		return dist(rng);
	}

	float getPosYEvent(float t, std::vector<JudgeLineMoveEvent> es) {
		const int i = findEvent(t, es);
		if (i == -1)
			return 0.0f;
		const JudgeLineMoveEvent& e = es[i];

		return linear(t, e.startTime, e.endTime, e.start2, e.end2);
	}

	float getSpeedValue(float t, std::vector<SpeedEvent> es) {
		const int i = findEvent(t, es);
		if (i == -1)
			return 0.0f;
		const SpeedEvent& e = es[i];

		return e.value;
	}

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

	void Application::DrawTexture(Texture* texture, const int x, const int y, const float sx, float sy, float angle) {
		if (!texture) return;

		if (sy == -1)
			sy = sx;

		const int texWidth = texture->GetWidth();
		const int texHeight = texture->GetHeight();
		const float srcW = static_cast<float>(texWidth);
		const float srcH = static_cast<float>(texHeight);
		const float w = srcW * sx;
		const float h = srcH * sy;

		const float rad = angle * 3.14159265f / 180.0f;
		const float cosA = cosf(rad);
		const float sinA = sinf(rad);

		float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
		float corners[4][2] = { {0, 0}, {w, 0}, {0, h}, {w, h} };
		for (int k = 0; k < 4; ++k) {
			float rx = corners[k][0] * cosA - corners[k][1] * sinA;
			float ry = corners[k][0] * sinA + corners[k][1] * cosA;
			minX = Min(minX, rx);
			minY = Min(minY, ry);
			maxX = Max(maxX, rx);
			maxY = Max(maxY, ry);
		}

		const int startY = static_cast<int>(std::floor(minY));
		const int endY = static_cast<int>(std::ceil(maxY));
		const int startX = static_cast<int>(std::floor(minX));
		const int endX = static_cast<int>(std::ceil(maxX));

		const float invSx = 1.0f / sx;
		const float invSy = 1.0f / sy;

		const int windowWidth = m_Width;
		const int windowHeight = m_Height;

		Framebuffer* framebuffer = m_Framebuffer;

		for (int j = startY; j <= endY; ++j) {
			const float jSinA = j * sinA;
			const float jCosA = j * cosA;
			const int dstY = y + j;

			if (dstY < 0 || dstY >= windowHeight)
				continue;

			for (int i = startX; i <= endX; ++i) {
				const int dstX = x + i;
				if (dstX < 0 || dstX >= windowWidth)
					continue;

				const float tx = (i * cosA + jSinA) * invSx;
				const float ty = (-i * sinA + jCosA) * invSy;

				if (tx >= 0 && tx < srcW && ty >= 0 && ty < srcH) {
					const int texX = static_cast<int>(tx);
					const int texY = static_cast<int>(ty);

					Vec4 tColor = texture->GetColor(texX, texY);
					framebuffer->SetColor(dstX, dstY, tColor);
				}
			}
		}
	}

	void Application::LoadJsons() {
		cJSON* root;
		cJSON* arrayInt;
		cJSON* arrayExt;
		cJSON* line;
		cJSON* obj;

		puts("Reading render info...\n");

		std::ifstream file("respack.json");
		std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		root = cJSON_Parse(json.c_str());

		arrayExt = cJSON_GetObjectItem(root, "hitFX");
		m_Respack.hitFx.X = (float)cJSON_GetArrayItem(arrayExt, 0)->valueint;
		m_Respack.hitFx.Y = (float)cJSON_GetArrayItem(arrayExt, 1)->valueint;

		arrayExt = cJSON_GetObjectItem(root, "holdAtlas");
		m_Respack.holdAtlas.X = (float)cJSON_GetArrayItem(arrayExt, 0)->valueint;
		m_Respack.holdAtlas.Y = (float)cJSON_GetArrayItem(arrayExt, 1)->valueint;

		arrayExt = cJSON_GetObjectItem(root, "holdAtlasMH");
		m_Respack.holdAtlasMH.X = (float)cJSON_GetArrayItem(arrayExt, 0)->valueint;
		m_Respack.holdAtlasMH.Y = (float)cJSON_GetArrayItem(arrayExt, 1)->valueint;

		cJSON_Delete(root);
		std::cout << "HitFX frame num: " << m_Respack.hitFx.X * m_Respack.hitFx.Y << std::endl << "Hold head&tail Len.: " << m_Respack.holdAtlas.X << " " <<m_Respack.holdAtlas.Y << std::endl << "HoldMH head&tail Len.: " << m_Respack.holdAtlasMH.X << " " << m_Respack.holdAtlasMH.Y << std::endl;

		puts("Reading chart info...\n");

		OPENFILENAMEW ofn;
		wchar_t szFile[260] = L"";

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
		ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrInitialDir = L"chart\\";
		ofn.lpstrTitle = L"Choose chartInfo file";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		m_C.chart.wPath = _getcwd(NULL, 0);

		if (GetOpenFileNameW(&ofn))
			file.open(ofn.lpstrFile);

		m_C.chart.path = _getcwd(NULL, 0);

		std::wstring chartInfo = ofn.lpstrFile;

		if (!file.is_open()) {
			m_C.chart.info.name = L"";
			m_C.chart.info.level = L"";
			m_C.chart.info.song = L"chart\\music.wav";
			m_C.chart.info.picture = L"chart\\image.png";
			m_C.chart.info.chart = L"chart\\chart.json";
		}
		else {
			std::wstring path;

			size_t lastSlash = chartInfo.find_last_of(L"\\/");
			if (lastSlash == std::string::npos) path = L".\\";
			else path = chartInfo.substr(0, lastSlash + 1);

			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
			while (!file.eof()) {
				std::string line;
				std::getline(file, line);
				if (line.find("Name:") == 0) {
                    m_C.chart.info.name = converter.from_bytes(line.substr(6));
				}
				else if (line.find("Level:") == 0) {
					m_C.chart.info.level = converter.from_bytes(line.substr(7));
				}
				else if (line.find("Song:") == 0) {
					m_C.chart.info.song = converter.from_bytes(line.substr(6));
				}
				else if (line.find("Picture:") == 0) {
					m_C.chart.info.picture = converter.from_bytes(line.substr(9));
				}
				else if (line.find("Chart:") == 0) {
					m_C.chart.info.chart = converter.from_bytes(line.substr(7));
				}

			}
			file.close();
		}

		printf("Name: %ls\nLevel: %ls\nSong: %ls\nPicture: %ls\nChart: %ls\n", m_C.chart.info.name.c_str(), m_C.chart.info.level.c_str(), m_C.chart.info.song.c_str(), m_C.chart.info.picture.c_str(), m_C.chart.info.chart.c_str());

		puts("\nReading chart\n");
		file.open(m_C.chart.info.chart);
		json = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		root = cJSON_Parse(json.c_str());

		std::map<float, int> noteSectCounter;

		arrayExt = cJSON_GetObjectItem(root, "judgeLineList");

		printf("Line   Notes    Move  Rotate   Alpha   Speed\n");

		for (int i = 0; i < cJSON_GetArraySize(arrayExt); i++) {
			JudgeLine jline;
			line = cJSON_GetArrayItem(arrayExt, i);
			jline.bpm = (float)cJSON_GetObjectItem(line, "bpm")->valuedouble;
			arrayInt = cJSON_GetObjectItem(line, "judgeLineMoveEvents");
			for (int j = 0; j < cJSON_GetArraySize(arrayInt); j++) {
				JudgeLineMoveEvent e;
				obj = cJSON_GetArrayItem(arrayInt, j);
				e.startTime = (float)cJSON_GetObjectItem(obj, "startTime")->valuedouble;
				e.endTime = (float)cJSON_GetObjectItem(obj, "endTime")->valuedouble;
				e.start = (float)cJSON_GetObjectItem(obj, "start")->valuedouble;
				e.end = (float)cJSON_GetObjectItem(obj, "end")->valuedouble;
				e.start2 = (float)cJSON_GetObjectItem(obj, "start2")->valuedouble;
				e.end2 = (float)cJSON_GetObjectItem(obj, "end2")->valuedouble;
				jline.moveEvents.push_back(e);
				float t = j == cJSON_GetArraySize(arrayInt) - 1 ? jline.beat2sec(e.startTime) : jline.beat2sec(e.endTime);
				if (t > m_C.chart.data.time) {
					m_C.chart.data.time = t;
				}
			}
			arrayInt = cJSON_GetObjectItem(line, "judgeLineRotateEvents");
			for (int j = 0; j < cJSON_GetArraySize(arrayInt); j++) {
				JudgeLineRotateEvent e;
				obj = cJSON_GetArrayItem(arrayInt, j);
				e.startTime = (float)cJSON_GetObjectItem(obj, "startTime")->valuedouble;
				e.endTime = (float)cJSON_GetObjectItem(obj, "endTime")->valuedouble;
				e.start = (float)cJSON_GetObjectItem(obj, "start")->valuedouble;
				e.end = (float)cJSON_GetObjectItem(obj, "end")->valuedouble;
				jline.rotateEvents.push_back(e);
				float t = j == cJSON_GetArraySize(arrayInt) - 1 ? jline.beat2sec(e.startTime) : jline.beat2sec(e.endTime);
				if (t > m_C.chart.data.time) {
					m_C.chart.data.time = t;
				}
			}
			arrayInt = cJSON_GetObjectItem(line, "judgeLineDisappearEvents");
			for (int j = 0; j < cJSON_GetArraySize(arrayInt); j++) {
				JudgeLineDisappearEvent e;
				obj = cJSON_GetArrayItem(arrayInt, j);
				e.startTime = (float)cJSON_GetObjectItem(obj, "startTime")->valuedouble;
				e.endTime = (float)cJSON_GetObjectItem(obj, "endTime")->valuedouble;
				e.start = (float)cJSON_GetObjectItem(obj, "start")->valuedouble;
				e.end = (float)cJSON_GetObjectItem(obj, "end")->valuedouble;
				jline.disappearEvents.push_back(e);
				float t = j == cJSON_GetArraySize(arrayInt) - 1 ? jline.beat2sec(e.startTime) : jline.beat2sec(e.endTime);
				if (t > m_C.chart.data.time) {
					m_C.chart.data.time = t;
				}
			}
			arrayInt = cJSON_GetObjectItem(line, "speedEvents");
			for (int j = 0; j < cJSON_GetArraySize(arrayInt); j++) {
				SpeedEvent e;
				obj = cJSON_GetArrayItem(arrayInt, j);
				e.startTime = (float)cJSON_GetObjectItem(obj, "startTime")->valuedouble;
				e.endTime = (float)cJSON_GetObjectItem(obj, "endTime")->valuedouble;
				e.value = (float)cJSON_GetObjectItem(obj, "value")->valuedouble;
				jline.speedEvents.push_back(e);
				float t = j == cJSON_GetArraySize(arrayInt) - 1 ? jline.beat2sec(e.startTime) : jline.beat2sec(e.endTime);
				if (t > m_C.chart.data.time) {
					m_C.chart.data.time = t;
				}
			}
			arrayInt = cJSON_GetObjectItem(line, "notesAbove");
			for (int j = 0; j < cJSON_GetArraySize(arrayInt); j++) {
				Note n;
				obj = cJSON_GetArrayItem(arrayInt, j);
				n.type = (int)cJSON_GetObjectItem(obj, "type")->valueint;
				n.time = (float)cJSON_GetObjectItem(obj, "time")->valuedouble;
				n.floorPosition = (float)cJSON_GetObjectItem(obj, "floorPosition")->valuedouble;
				n.holdTime = (float)cJSON_GetObjectItem(obj, "holdTime")->valuedouble;
				n.speed = (float)cJSON_GetObjectItem(obj, "speed")->valuedouble;
				n.positionX = (float)cJSON_GetObjectItem(obj, "positionX")->valuedouble;
				jline.notesAbove.push_back(n);
				float t = n.type != 3 ? jline.beat2sec(n.time) : jline.beat2sec(n.holdTime);
				if (t > m_C.chart.data.time) {
					m_C.chart.data.time = t;
				}
			}
			arrayInt = cJSON_GetObjectItem(line, "notesBelow");
			for (int j = 0; j < cJSON_GetArraySize(arrayInt); j++) {
				Note n;
				obj = cJSON_GetArrayItem(arrayInt, j);
				n.type = (int)cJSON_GetObjectItem(obj, "type")->valueint;
				n.time = (float)cJSON_GetObjectItem(obj, "time")->valuedouble;
				n.floorPosition = (float)cJSON_GetObjectItem(obj, "floorPosition")->valuedouble;
				n.holdTime = (float)cJSON_GetObjectItem(obj, "holdTime")->valuedouble;
				n.speed = (float)cJSON_GetObjectItem(obj, "speed")->valuedouble;
				n.positionX = (float)cJSON_GetObjectItem(obj, "positionX")->valuedouble;
				jline.notesBelow.push_back(n);
				float t = n.type != 3 ? jline.beat2sec(n.time) : jline.beat2sec(n.holdTime);
				if (t > m_C.chart.data.time) {
					m_C.chart.data.time = t;
				}
			}

			std::sort(jline.notesAbove.begin(), jline.notesAbove.end(), [](Note a, Note b) { return a.time < b.time; });
			std::sort(jline.notesBelow.begin(), jline.notesBelow.end(), [](Note a, Note b) { return a.time < b.time; });
			std::sort(jline.speedEvents.begin(), jline.speedEvents.end(), [](SpeedEvent a, SpeedEvent b) { return a.startTime < b.startTime; });
			std::sort(jline.moveEvents.begin(), jline.moveEvents.end(), [](JudgeLineMoveEvent a, JudgeLineMoveEvent b) { return a.startTime < b.startTime; });
			std::sort(jline.rotateEvents.begin(), jline.rotateEvents.end(), [](JudgeLineRotateEvent a, JudgeLineRotateEvent b) { return a.startTime < b.startTime; });
			std::sort(jline.disappearEvents.begin(), jline.disappearEvents.end(), [](JudgeLineDisappearEvent a, JudgeLineDisappearEvent b) { return a.startTime < b.startTime; });

			jline.initSpeedEvents();
			jline.mergeNotes();
			jline.initNoteFp();

			for (auto& n : jline.notes) {
				n.sect = jline.beat2sec(n.time);
				n.secht = jline.beat2sec(n.holdTime);
				n.holdEndTime = n.sect + n.secht;
				n.holdLength = n.secht * n.speed * pgrh;
				n.isHold = n.type == 3;
				n.clicked = false;
				n.line = i;
				if (noteSectCounter.find(n.sect) == noteSectCounter.end()) {
					noteSectCounter[n.sect] = 0;
				}

				noteSectCounter[n.sect]++;

			}

			jline.notesAbove.clear();
			jline.notesBelow.clear();

			m_C.chart.data.judgeLines.push_back(jline);

			m_C.chart.data.noteCount += (int)jline.notes.size();

			printf("%4d\t%4zu\t%4zu\t%4zu\t%4zu\t%4zu\n", i, jline.notes.size(), jline.moveEvents.size(), jline.rotateEvents.size(), jline.disappearEvents.size(), jline.speedEvents.size());

		}

		puts("\nEnd.\n");

		puts("Parsing Note...\n");
		for (auto& line : m_C.chart.data.judgeLines) {
			for (auto& n : line.notes) {
				n.morebets = noteSectCounter[n.sect] > 1;
				EventsValue ev = line.getState(n.time);
				Vec2 pos = rotatePoint(
					ev.x * m_Width, ev.y * m_Height,
					n.positionX * m_Width * pgrw, ev.rotate
				);
				m_C.chart.data.clickEffectCollection.push_back({ n, n.sect, Particles((float)m_Width, (float)m_Height) });
				if (n.isHold) {
					float dt = 30 / line.bpm;
					float st = n.sect + dt;
					while (st < n.holdEndTime) {
						ev = line.getState(st);
						pos = rotatePoint(
							ev.x * m_Width, ev.y * m_Height,
							n.positionX * m_Width * pgrw, ev.rotate
						);
						m_C.chart.data.clickEffectCollection.push_back({ n, st, Particles((float)m_Width, (float)m_Height) });
						st += dt;
					}
				}
			}
		}

		std::sort(
			m_C.chart.data.clickEffectCollection.begin(),
			m_C.chart.data.clickEffectCollection.end(),
			[](NoteMap a, NoteMap b) { return a.sect < b.sect; }
		);

		printf("\nHitFX num: %zd\n", m_C.chart.data.clickEffectCollection.size());

		puts("End.\n");

		if (_chdir(m_C.chart.wPath.c_str()))
			exit(1);
	}

	void Application::LoadImgs() {

		puts("\nLoading Imgs...\n");

		Texture* hold = m_C.noteImgs.hold;
		m_C.noteImgs.holdHead = hold->ClipImg(0, (int)m_Respack.holdAtlas.X);
		m_C.noteImgs.holdBody = hold->ClipImg((int)m_Respack.holdAtlas.X, hold->GetHeight() - (int)m_Respack.holdAtlas.Y);
		m_C.noteImgs.holdTail = hold->ClipImg(hold->GetHeight() - (int)m_Respack.holdAtlas.Y, hold->GetHeight());

		Texture* holdMH = m_C.noteImgs.holdMH;
		m_C.noteImgs.holdMHHead = holdMH->ClipImg(0, (int)m_Respack.holdAtlasMH.X);
		m_C.noteImgs.holdMHBody = holdMH->ClipImg((int)m_Respack.holdAtlasMH.X, holdMH->GetHeight() - (int)m_Respack.holdAtlasMH.Y);
		m_C.noteImgs.holdMHTail = holdMH->ClipImg(holdMH->GetHeight() - (int)m_Respack.holdAtlasMH.Y, holdMH->GetHeight());

		m_C.holdBodyImgs[0] = m_C.noteImgs.holdBody;
		m_C.holdBodyImgs[1] = m_C.noteImgs.holdMHBody;
		m_C.holdTailImgs[0] = m_C.noteImgs.holdTail;
		m_C.holdTailImgs[1] = m_C.noteImgs.holdMHTail;

		m_C.noteHeadImgs[0][0] = m_C.noteImgs.click;
		m_C.noteHeadImgs[0][1] = m_C.noteImgs.clickMH;
		m_C.noteHeadImgs[1][0] = m_C.noteImgs.drag;
		m_C.noteHeadImgs[1][1] = m_C.noteImgs.dragMH;
		m_C.noteHeadImgs[2][0] = m_C.noteImgs.holdHead;
		m_C.noteHeadImgs[2][1] = m_C.noteImgs.holdMHHead;
		m_C.noteHeadImgs[3][0] = m_C.noteImgs.flick;
		m_C.noteHeadImgs[3][1] = m_C.noteImgs.flickMH;

		if (_chdir(m_C.chart.path.c_str()))
			exit(1);

		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		m_C.chart.image = new Texture(converter.to_bytes(m_C.chart.info.picture).c_str());
		m_C.chart.blurImage = m_C.chart.image->GetBlurImg(0.0f);

		puts("End.\n");

		if (_chdir(m_C.chart.wPath.c_str()))
			exit(1);

	}

	void Application::LoadFxImgs() {
		puts("Loading hitFX...\n");
		for (int j = (int)m_Respack.hitFx.Y - 1; j >= 0; j--) {
			for (int i = 0; i < (int)m_Respack.hitFx.X; i++) {
				m_C.hitFxImgs.push_back(
					m_C.noteImgs.hitFx->ClipBlockImg(
						(int)((i / m_Respack.hitFx.X) * m_C.noteImgs.hitFx->GetWidth()),
						(int)((j / m_Respack.hitFx.Y) * m_C.noteImgs.hitFx->GetHeight()),
						(int)(((i + 1) / m_Respack.hitFx.X) * m_C.noteImgs.hitFx->GetWidth()),
						(int)(((j + 1) / m_Respack.hitFx.Y) * m_C.noteImgs.hitFx->GetHeight())
					)->ColorTexture(Vec4(pcolor, 1.0f))
				);
			}
		}
		puts("End.\n");
		puts("Loading audio...\n");
		mciSendString("open click.wav type waveaudio alias click", NULL, 0, NULL);
		mciSendString("open drag.wav type waveaudio alias drag", NULL, 0, NULL);
		mciSendString("open flick.wav type waveaudio alias flick", NULL, 0, NULL);

		if (_chdir(m_C.chart.path.c_str()))
			exit(1);

		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		std::string music = converter.to_bytes(m_C.chart.info.song);
		std::string str = "open " + music + " alias music";
		mciSendString(str.c_str(), NULL, 0, NULL);

		if (_chdir(m_C.chart.wPath.c_str()))
			exit(1);

		puts("End.\n");
	}

	void Application::LoadFiles() {
		LoadJsons();
		LoadImgs();
		LoadFxImgs();
	}

	void Application::Init() {

		HWND Cmd = GetConsoleWindow();
		ShowWindow(Cmd, SW_SHOW);

		LoadFiles();

		ShowWindow(Cmd, SW_HIDE);

		Window::Init();
		m_Window = Window::Create(m_Name, m_Width, m_Height);

		m_Framebuffer = Framebuffer::Create(m_Width, m_Height);
		m_Framebuffer->LoadFontTTF("font.ttf");

		m_Window->DrawFramebuffer(m_Framebuffer);
		m_StartFrameTime = std::chrono::steady_clock::now();

	}

	void Application::Terminate() {
		delete m_Window;
		Window::Terminate();
		delete m_Framebuffer;
		delete m_C.chart.image;
		delete m_C.noteImgs.click;
		delete m_C.noteImgs.drag;
		delete m_C.noteImgs.hold;
		delete m_C.noteImgs.flick;
		delete m_C.noteImgs.hitFx;
		delete m_C.noteImgs.clickMH;
		delete m_C.noteImgs.dragMH;
		delete m_C.noteImgs.holdMH;
		delete m_C.noteImgs.flickMH;
		delete m_C.noteImgs.holdMHHead;
		delete m_C.noteImgs.holdMHBody;
		delete m_C.noteImgs.holdMHTail;
		delete m_C.noteImgs.holdHead;
		delete m_C.noteImgs.holdBody;
		delete m_C.noteImgs.holdTail;
		m_C.holdBodyImgs[0] = nullptr;
		m_C.holdBodyImgs[1] = nullptr;
		m_C.holdTailImgs[0] = nullptr;
		m_C.holdTailImgs[1] = nullptr;
		delete m_C.holdBodyImgs[0];
		delete m_C.holdBodyImgs[1];
		delete m_C.holdTailImgs[0];
		delete m_C.holdTailImgs[1];
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 2; j++) {
				m_C.noteHeadImgs[i][j] = nullptr;
				delete m_C.noteHeadImgs[i][j];
			}
		for (size_t i = 0; i < m_C.hitFxImgs.size(); i++) {
			m_C.hitFxImgs[i] = nullptr;
			delete m_C.hitFxImgs[i];
		}
		mciSendString("close click", NULL, 0, NULL);
		mciSendString("close drag", NULL, 0, NULL);
		mciSendString("close flick", NULL, 0, NULL);
	}

	void Application::Run() {
		constexpr float TARGET_FPS = 60.0f;
		constexpr float FRAME_TIME_MS = 1000.0f / TARGET_FPS;
		
		while (!m_Window->Closed()) {
			const int currentWidth = m_Window->GetWidth();
			const int currentHeight = m_Window->GetHeight();
			if (m_Width != currentWidth || m_Height != currentHeight) {
				m_Width = currentWidth;
				m_Height = currentHeight;
				if (m_Width > 0 && m_Height > 0) {
					m_Framebuffer->Resize(m_Width, m_Height);
				}
			}

			m_Window->PollInputEvents();
			
			if (m_Width > 0 && m_Height > 0) {
				m_Framebuffer->Clear(Vec3(0.09f, 0.10f, 0.14f));
				OnUpdate();
				m_Window->DrawFramebuffer(m_Framebuffer);
			}

			const auto currentTime = std::chrono::steady_clock::now();
			m_LastFrameTime = currentTime;

			constexpr double NANOSECONDS_TO_SECONDS = 1.0 / 1000000000.0;
			if (IsPlaying) {
				m_Time = static_cast<float>((currentTime - m_StartFrameTime).count() * NANOSECONDS_TO_SECONDS);
			} else {
				m_StartFrameTime = currentTime - std::chrono::milliseconds(static_cast<int>(m_Time * 1000));
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	void Application::Render(float size, float ox, float oy) {

		std::chrono::duration Time = std::chrono::steady_clock::now() - m_StartFrameTime;
        float t = std::chrono::duration_cast<std::chrono::milliseconds>(Time).count() / 1000.0f;

		DrawTexture(
			m_C.chart.blurImage, 0, 0,
			(float)m_Width / m_C.chart.blurImage->GetWidth(),
			(float)m_Height / m_C.chart.blurImage->GetHeight()
		);

		m_Framebuffer->FillRect(0, 0, m_Width, m_Height, Vec4(0.0f, 0.0f, 0.0f, 0.2f));

		Texture* texture = m_C.chart.image;
		DrawTexture(
			texture, (int)(m_Width / 2 - m_Width * size / 2 + ox), (int)(m_Height / 2 - m_Height * size / 2 + oy),
			(float)m_Width / texture->GetWidth() * size,
			(float)m_Height / texture->GetHeight() * size
		);

		m_Framebuffer->FillRect(
			(int)(m_Width / 2.0f - m_Width / 2.0f * m_C.camera.size + m_C.camera.Pos.X),
			m_Height - (int)(m_Height / 2.0f - m_Height / 2.0f * m_C.camera.size + m_C.camera.Pos.Y),
			(int)(m_Width / 2.0f + m_Width / 2.0f * m_C.camera.size + m_C.camera.Pos.X),
			m_Height - (int)(m_Height / 2.0f + m_Height / 2.0f * m_C.camera.size + m_C.camera.Pos.Y),
			Vec4(0.0f, 0.0f, 0.0f, 0.6f)
		);

		float noteW = noteSize * m_Width * size;

		int combo = 0;

		for (size_t i = 0; i < m_C.chart.data.judgeLines.size(); i++) {

			JudgeLine line = m_C.chart.data.judgeLines[i];

			EventsValue e = line.getState(t);
			EventsValue ev = e;
			ev.x *= m_Width, ev.y *= m_Height;
			ev.x = (ev.x - m_Width / 2) * size + m_Width / 2 + ox;
			ev.y = (ev.y - m_Height / 2) * size + m_Height / 2 + oy;
			Vec2 linePos[2] = {
				rotatePoint(ev.x, ev.y, m_Height * lineh * size, ev.rotate),
				rotatePoint(ev.x, ev.y, m_Height * lineh * size, ev.rotate + 180.0f)
			};

			m_Framebuffer->DrawLine(
				(int)linePos[0].X, (int)linePos[0].Y,
				(int)linePos[1].X, (int)linePos[1].Y,
				m_Height * linew * size,
				Vec4(i == m_Line ? Vec4(0.0f, 1.0f, 0.0f, 1.0f) : pcolor, __DEBUG__ ? ev.alpha * 0.99f + 0.01f : ev.alpha)
			);

			Vec2 lineAPos = rotatePoint(ev.x, ev.y, m_Height * 0.025f, ev.rotate + 90.0f);

			if (__DEBUG__) {

				m_Framebuffer->DrawLine(
					(int)ev.x, (int)ev.y,
					(int)lineAPos.X, (int)lineAPos.Y,
					m_Height * linew * size, Vec4(i == m_Line ? Vec4(0.0f, 1.0f, 0.0f, 1.0f) : pcolor, ev.alpha * 0.99f + 0.01f)
				);

			}

			float beatt = line.sec2beat(t);
			float lineFp = line.getFp(beatt);

			JudgeLine& currentLine = m_C.chart.data.judgeLines[i];
			float pgrwTimesWidth = pgrw * m_Width * size;
			float PI_OVER_180 = PI / 180.0f;
			float cosEvRotate = cos(ev.rotate * PI_OVER_180);
			float sinEvRotate = sin(ev.rotate * PI_OVER_180);


			if (__DEBUG__) {
				char speedBuf[32];
				sprintf(speedBuf, "%.2f", ev.speed);

				char PosXBuf[32];
				sprintf(PosXBuf, "%.2f", (double)e.x - (double)0.5f);

				char PosYBuf[32];
				sprintf(PosYBuf, "%.2f", (double)e.y - (double)0.5f);

				std::string lineStr =
					"[" + std::to_string(i) +
					"] (" + PosXBuf +
					", " + PosYBuf +
					") " + std::to_string((int)ev.rotate) +
					"d " + std::to_string((int)(ev.alpha * 100.0f)) +
					": " + speedBuf;

				m_Framebuffer->DrawCenterTextTTF(
					(int)(ev.x + sin(ev.rotate * PI_OVER_180) * m_Width * 0.04f),
					(int)(m_Height - ev.y + cos(ev.rotate * PI_OVER_180) * m_Width * 0.04f),
					lineStr,
					Vec4(i == m_Line ? Vec4(0.0f, 1.0f, 0.0f, 1.0f) : pcolor, ev.alpha * 0.9f + 0.1f), m_Width * 0.04f, -ev.rotate
				);

				if (m_Line == i) {
					std::string lineStr =
						"[" + std::to_string(m_Line) +
						"] (" + PosXBuf +
						", " + PosYBuf +
						") " + std::to_string((int)ev.rotate) +
						"d " + std::to_string((int)(ev.alpha * 100.0f)) +
						": " + speedBuf;

					m_Framebuffer->DrawCenterTextTTF(
						(int)(m_Width / 2.0f),
						(int)(m_Height - m_Height * 0.03f),
						lineStr,
						Vec4(1.0f), m_Width * 0.04f, 0.0f
					);
				}

			}
		}

		for (int i = 0; i < m_C.chart.data.judgeLines.size(); i++) {
			JudgeLine line = m_C.chart.data.judgeLines[i];

			EventsValue e = line.getState(t);
			EventsValue ev = e;
			ev.x *= m_Width, ev.y *= m_Height;
			ev.x = (ev.x - m_Width / 2) * size + m_Width / 2 + ox;
			ev.y = (ev.y - m_Height / 2) * size + m_Height / 2 + oy;

			float beatt = line.sec2beat(t);
			float lineFp = line.getFp(beatt);

			JudgeLine& currentLine = m_C.chart.data.judgeLines[i];
			float pgrwTimesWidth = pgrw * m_Width * size;
			float PI_OVER_180 = PI / 180.0f;
			float cosEvRotate = cos(ev.rotate * PI_OVER_180);
			float sinEvRotate = sin(ev.rotate * PI_OVER_180);

			auto& notes = currentLine.notes;
			size_t notesCount = notes.size();
			for (size_t j = 0; j < notesCount; j++) {
				if (notes[j].sect < t && !notes[j].clicked) {
					switch (notes[j].type) {
					case 1:
					case 3:
						mciSendString("play click from 0", NULL, 0, NULL);
						break;
					case 2:
						mciSendString("play drag from 0", NULL, 0, NULL);
						break;
					case 4:
						mciSendString("play flick from 0", NULL, 0, NULL);
						break;
					}
					notes[j].clicked = true;
				}

				const Note& note = notes[j];

				if ((!note.isHold && note.sect < t) || (note.isHold && note.holdEndTime < t)) {
					combo++;
					continue;
				}

				float noteFp = (note.floorPosition - lineFp) * pgrh * (pgrbeat / line.bpm) * m_Height * size;

				if (!note.isHold) {
					noteFp *= note.speed;
					if (noteFp < -1e6) {
						continue;
					}
				}

				if ((__DEBUG__ ? noteFp : noteFp / size) > m_Height * 2) {
					continue;
				}

				if ((!note.isHold && noteFp < 0) || (note.isHold && noteFp < 0 && !note.clicked)) {
					continue;
				}

				bool drawHead = note.sect > t;
				Texture* noteHeadImg = m_C.noteHeadImgs[note.type - 1][note.morebets];

				float thisNoteWidth = 1.0f * noteSize * size;

				if (note.morebets) {
					thisNoteWidth = noteSize * size * (
						(float)(m_C.noteHeadImgs[note.type - 1][1]->GetWidth())
						/ (float)(m_C.noteHeadImgs[note.type - 1][0]->GetWidth())
						);
				}

				float headImgWidth = (float)noteHeadImg->GetWidth();
				float headImgHeight = (float)noteHeadImg->GetHeight();
				float thisNoteHeadHeight = thisNoteWidth / headImgWidth * headImgHeight;
				float posX = note.positionX * pgrwTimesWidth;

				float noteAtlineX = ev.x + posX * cosEvRotate;
				float noteAtlineY = ev.y + posX * sinEvRotate;
				Vec2 noteAtlinePos(noteAtlineX, noteAtlineY);

				float l2nRotate = ev.rotate - (note.isAbove ? -90 : 90);
				float l2nRad = l2nRotate * PI_OVER_180;
				float cosL2n = cos(l2nRad);
				float sinL2n = sin(l2nRad);

				float headX = noteAtlineX + noteFp * cosL2n;
				float headY = noteAtlineY + noteFp * sinL2n;
				Vec2 noteHeadPos(headX, headY);

				float noteDrawRotate = ev.rotate - (note.isAbove ? 0 : 180);
				float drawRad = noteDrawRotate * PI_OVER_180;
				float cosDrawRad = cos(drawRad);
				float sinDrawRad = sin(drawRad);

				float texScale = thisNoteWidth * m_Width / headImgWidth;
				float Size = m_Width * noteSize / headImgWidth * size;
				float W = texScale * headImgWidth / 2.0f;
				float H = texScale * headImgHeight / 2.0f;
				float w = W * cosDrawRad - H * sinDrawRad;
				float h = H * cosDrawRad + W * sinDrawRad;

				float noteBodyHeight = 0.0f;

				if (note.isHold) {
					Texture* noteBodyImg = m_C.holdBodyImgs[note.morebets];
					Texture* noteTailImg = m_C.holdTailImgs[note.morebets];

					float noteTillHeight = thisNoteWidth * m_Width / noteTailImg->GetWidth() * noteTailImg->GetHeight();
					float noteHeadHeight = thisNoteWidth * m_Width / headImgWidth * headImgHeight;

					noteBodyHeight = Max(
						note.holdLength * size * m_Height +
						Min(0.0f, noteFp) +
						(note.clicked ? noteHeadHeight : 0.0f) -
						noteTillHeight * 1.5f,
						0.0f
					);

					if (noteBodyHeight > 0.0f) {

						float tailPosBaseX = note.clicked ? noteAtlineX : headX;
						float tailPosBaseY = note.clicked ? noteAtlineY : headY;
						float tailOffset = (note.clicked ? noteHeadHeight / 2.0f : noteHeadHeight) + noteBodyHeight;
						float tailX = tailPosBaseX + tailOffset * cosL2n;
						float tailY = tailPosBaseY + tailOffset * sinL2n;
						Vec2 noteTailPos(tailX, tailY);

						float headH = thisNoteWidth * m_Width / headImgWidth * headImgHeight;
						float x = headX - w - headH * sinDrawRad;
						float y = headY - h + headH * cosDrawRad;

						float bodyTexScaleX = thisNoteWidth * m_Width / headImgWidth;
						float bodyTexScaleY = noteBodyHeight / noteBodyImg->GetHeight();
						DrawTexture(
							noteBodyImg,
							note.clicked ? (int)(noteAtlineX - w - headH / 2 * sinDrawRad) : (int)x,
							note.clicked ? (int)(noteAtlineY - h + headH / 2 * cosDrawRad) : (int)y,
							bodyTexScaleX, bodyTexScaleY,
							noteDrawRotate
						);

						float tailImgWidth = (float)noteTailImg->GetWidth();
						float tailImgHeight = (float)noteTailImg->GetHeight();
						float tailS = m_Width * noteSize / tailImgWidth;
						float tailW = tailS * tailImgWidth * thisNoteWidth / noteSize / 2.0f;
						float tailH = tailS * tailImgHeight / 2.0f * size;
						float tailw = tailW * cosDrawRad - tailH * sinDrawRad;
						float tailh = tailH * cosDrawRad + tailW * sinDrawRad;

						float tailTexScale = thisNoteWidth * m_Width / headImgWidth;
						DrawTexture(
							noteTailImg,
							(int)(noteTailPos.X - tailw), (int)(noteTailPos.Y - tailh),
							tailTexScale, tailTexScale,
							noteDrawRotate
						);

					}
				}

				if (drawHead && ((note.isHold && noteBodyHeight > 0.0f) || !note.isHold)) {
					DrawTexture(
						noteHeadImg, (int)(noteHeadPos.X - w), (int)(noteHeadPos.Y - h),
						texScale, texScale,
						noteDrawRotate
					);

					if (__DEBUG__) {

						const char* typeStr = "";

						switch (note.type) {
						case 1:
							typeStr = "click";
							break;
						case 2:
							typeStr = "drag";
							break;
						case 3:
							typeStr = "hold";
							break;
						case 4:
							typeStr = "flick";
							break;
						}

						char PosXBuf[32], PosYBuf[32];

						sprintf(PosXBuf, "%.2f", note.positionX);
						sprintf(PosYBuf, "%.2f", noteFp / (pgrh * m_Height * size));

						std::string noteStr =
							"[" + std::to_string(i) +
							"]" + std::to_string(j) +
							"( " + PosXBuf +
							"," + PosYBuf +
							" ): " + typeStr +
							": " + std::to_string((int)note.sect);

						if (note.isHold)
							noteStr += "/ " + std::to_string((int)note.holdEndTime);

						m_Framebuffer->DrawCenterTextTTF(
							(int)(noteHeadPos.X + sin(noteDrawRotate * PI_OVER_180) * m_Width * 0.03f),
							(int)(m_Height - noteHeadPos.Y + cos(noteDrawRotate * PI_OVER_180) * m_Width * 0.03f),
							noteStr, i == m_Line ? Vec4(0.0f, 1.0f, 0.0f, 1.0f) : Vec4(1.0f, 0.5f), m_Width * 0.025f, -noteDrawRotate
						);

					}
				}
			}
		}

		float effectDur = 0.5f;
		float pgrwTimesWidthEffect = pgrw * m_Width * size;
		float PI_OVER_180_EFFECT = PI / 180.0f;
		size_t effectCount = m_C.chart.data.clickEffectCollection.size();
		const auto& hitFxImgs = m_C.hitFxImgs;
		size_t hitFxImgsCount = hitFxImgs.size();

		for (size_t effectIdx = 0; effectIdx < effectCount; effectIdx++) {
			const NoteMap& nm = m_C.chart.data.clickEffectCollection[effectIdx];
			if (nm.sect > t) break;
			if (nm.sect + effectDur < t) continue;

			float p = (t - nm.sect) / effectDur;
			float alpha = 1.0f - p;

			size_t imgIndex = static_cast<size_t>(Max(0.0f, Min(static_cast<float>(hitFxImgsCount - 1), floor(p * hitFxImgsCount))));
			Texture* img = hitFxImgs[imgIndex];
			float effectSize = noteW * 1.375f * 1.12f;
			float halfEffectSize = effectSize * 0.5f;

			JudgeLine& line = m_C.chart.data.judgeLines[nm.note.line];
			EventsValue ev = line.getState(nm.sect);

			ev.x *= m_Width; ev.y *= m_Height;
			ev.x = (ev.x - m_Width / 2) * size + m_Width / 2;
			ev.y = (ev.y - m_Height / 2) * size + m_Height / 2;

			float posX = ev.x;
			float posY = ev.y;
			float rotRad = ev.rotate * PI_OVER_180_EFFECT;
			float cosRot = cos(rotRad);
			float sinRot = sin(rotRad);
			float offsetX = nm.note.positionX * pgrwTimesWidthEffect;

			float finalX = posX + offsetX * cosRot + ox;
			float finalY = posY + offsetX * sinRot + oy;
			Vec2 pos(finalX, finalY);

			float texScale = effectSize / img->GetWidth();
			DrawTexture(
				img,
				(int)(finalX - halfEffectSize), (int)(finalY - halfEffectSize),
				texScale
			);

			for (int parIdx = 0; parIdx < 4; parIdx++) {
				const Vec3& parItem = nm.particles.pars[parIdx];

				float s = m_Width / 4040.0f * 3.0f;

				float parRad = parItem.X * PI_OVER_180_EFFECT;
				float cosParRad = cos(parRad);
				float sinParRad = sin(parRad);

				float parCenterX = parItem.Z * s * cosParRad * size;
				float parCenterY = parItem.Z * s * sinParRad * size;
				float scaledParSize = parItem.Y * p * size;

				float pScaledCenterX = parCenterX * (9.0f * p / (8.0f * p + 1.0f));
				float pScaledCenterY = parCenterY * (9.0f * p / (8.0f * p + 1.0f));
				float x1 = pScaledCenterX + finalX;
				float y1 = m_Height - (pScaledCenterY + finalY);

				m_Framebuffer->FillSizeRect((int)x1, (int)y1, (int)(parItem.Y * size * s), (int)(parItem.Y * size * s), Vec4(pcolor, alpha));
			}
		}

		float endTime = m_C.chart.data.time;

		m_Framebuffer->FillRect(
			0, 0,
			(int)(m_Width * (Min(t, endTime) / endTime)), (int)(m_Height * 12.0f / 1080.0f),
			Vec4(0.45f, 1.0f)
		);

		m_Framebuffer->FillRect(
			(int)(m_Width * (Min(t, endTime) / endTime) - 0.5f), 0,
			(int)(m_Width * (Min(t, endTime) / endTime) + 0.5f), (int)(m_Height * 12.0f / 1080.0f),
			Vec4(1.0f, 1.0f)
		);

		m_Framebuffer->FillRect(
			(int)(m_Width * 33.0f / 1920.0f), (int)(m_Height * 39.0f / 1080.0f),
			(int)(m_Width * 45.0f / 1920.0f), (int)(m_Height * 39.0f / 1080.0f + m_Width * 39.0f / 1920.0f),
			Vec4(1.0f)
		);

		m_Framebuffer->FillRect(
			(int)(m_Width * 56.0f / 1920.0f), (int)(m_Height * 39.0f / 1080.0f),
			(int)(m_Width * 68.0f / 1920.0f), (int)(m_Height * 39.0f / 1080.0f + m_Width * 39.0f / 1920.0f),
			Vec4(1.0f)
		);

		if (combo >= 3) {
			m_Framebuffer->DrawCenterTextTTF(
				(int)(m_Width * 0.5f), (int)(m_Height * 39.0f / 1080.0f + m_Width * 60.0f / 1920.0f),
				std::to_string(combo), Vec4(1.0f), (m_Width * 86.0f / 1920.0f), 0.0f
			);

			m_Framebuffer->DrawCenterTextTTF(
				(int)(m_Width * 0.5f), (int)((m_Height * 39.0f / 1080.0f + m_Width * 60.0f / 1920.0f) * 84.0f / 63.0f),
				"AUTOPLAY", Vec4(1.0f), (m_Width * 34.0f / 1920.0f), 0.0f
			);
		}

		float score = (float)combo / m_C.chart.data.noteCount * 1000000.0f;
		char scoreStr[10];
		sprintf(scoreStr, "%07d", (int)score);
		m_Framebuffer->DrawTextTTF(
			(int)(m_Width - (m_Width * 365.0f / 1920.0f)), (int)(m_Height * 39.0f / 1080.0f),
			scoreStr, Vec4(1.0f), m_Width * 70.0f / 1920.0f
		);

		m_Framebuffer->DrawWTextTTF(
			(int)(m_Width * 48.0f / 1920.0f), (int)(m_Height * 980.0f / 1080.0f),
			m_C.chart.info.name, Vec4(1.0f), m_Width * 60.0f / 1920.0f
		);

		m_Framebuffer->DrawWTextTTF(
			(int)(m_Width - m_Width * 35.0f / 1920.0f * m_C.chart.info.level.length()), (int)(m_Height * 980.0f / 1080.0f),
			m_C.chart.info.level, Vec4(1.0f), m_Width * 60.0f / 1920.0f
		);

		if (IsPlaying) {
			static float lastFPSTime = 0.0f;
			static int frameCount = 0;
			frameCount++;
			float currentTime = static_cast<float>((std::chrono::steady_clock::now() - m_StartFrameTime).count() * 0.000000001f);
			float fps = frameCount / (currentTime - lastFPSTime);
			m_Framebuffer->DrawTextTTF(
				0, (int)(m_Height * 12.0f / 1080.0f), "FPS: " + std::to_string((int)fps), Vec4(1.0f), m_Height * 30.0f / 1080.0f
			);
			lastFPSTime = currentTime;
			frameCount = 0;
		}
		else {
			m_Framebuffer->DrawTextTTF(
				0, (int)(m_Height * 12.0f / 1080.0f), "FPS: " + std::to_string(0), Vec4(1.0f, 0.0f, 0.0f, 1.0f), m_Height * 30.0f / 1080.0f
			);
		}

		if (m_Line != -1 && __DEBUG__) {
			JudgeLine line = m_C.chart.data.judgeLines[m_Line];

			EventsValue e = line.getState(t);
			EventsValue ev = e;
			ev.x *= m_Width, ev.y *= m_Height;
			ev.x = (ev.x - m_Width / 2) * size + m_Width / 2 + ox;
			ev.y = (ev.y - m_Height / 2) * size + m_Height / 2 + oy;
			Vec2 linePos[2] = {
				rotatePoint(ev.x, ev.y, m_Height * lineh * size, ev.rotate),
				rotatePoint(ev.x, ev.y, m_Height * lineh * size, ev.rotate + 180.0f)
			};

			m_Framebuffer->DrawLine(
				(int)linePos[0].X, (int)linePos[0].Y,
				(int)linePos[1].X, (int)linePos[1].Y,
				m_Height* linew* size,
				Vec4(0.0f, 1.0f, 0.0f, 1.0f)
			);

			Vec2 lineAPos = rotatePoint(ev.x, ev.y, m_Height * 0.025f, ev.rotate + 90.0f);

			m_Framebuffer->DrawLine(
				(int)ev.x, (int)ev.y,
				(int)lineAPos.X, (int)lineAPos.Y,
				m_Height * linew * size, Vec4(0.0f, 1.0f, 0.0f, 1.0f)
			);

			float beatt = line.sec2beat(t);
			float lineFp = line.getFp(beatt);

			JudgeLine& currentLine = m_C.chart.data.judgeLines[m_Line];
			auto& notes = currentLine.notes;
			size_t notesCount = notes.size();
			float pgrwTimesWidth = pgrw * m_Width * size;
			float PI_OVER_180 = PI / 180.0f;
			float cosEvRotate = cos(ev.rotate * PI_OVER_180);
			float sinEvRotate = sin(ev.rotate * PI_OVER_180);

			char speedBuf[32];
			sprintf(speedBuf, "%.2f", ev.speed);

			char PosXBuf[32];
			sprintf(PosXBuf, "%.2f", (double)e.x - (double)0.5f);

			char PosYBuf[32];
			sprintf(PosYBuf, "%.2f", (double)e.y - (double)0.5f);

			std::string lStr =
				"[" + std::to_string(m_Line) +
				"] (" + PosXBuf +
				", " + PosYBuf +
				") " + std::to_string((int)ev.rotate) +
				"d " + std::to_string((int)(ev.alpha * 100.0f)) +
				": " + speedBuf;

			m_Framebuffer->DrawCenterTextTTF(
				(int)(ev.x + sin(ev.rotate * PI_OVER_180) * m_Width * 0.04f),
				(int)(m_Height - ev.y + cos(ev.rotate * PI_OVER_180) * m_Width * 0.04f),
				lStr,
				Vec4(0.0f, 1.0f, 0.0f, 1.0f), m_Width * 0.04f, -ev.rotate
			);

			std::string lineStr =
				"[" + std::to_string(m_Line) +
				"] (" + PosXBuf +
				", " + PosYBuf +
				") " + std::to_string((int)ev.rotate) +
				"d " + std::to_string((int)(ev.alpha * 100.0f)) +
				": " + speedBuf;

			m_Framebuffer->DrawCenterTextTTF(
				(int)(m_Width / 2.0f),
				(int)(m_Height - m_Height * 0.03f),
				lineStr,
				Vec4(1.0f), m_Width * 0.04f, 0.0f
			);

		}
	}

	void Application::OnUpdate() {

		if (m_Window->GetKey(PGR_KEY_W))
			m_C.camera.Pos.Y -= 2.0f;
		if (m_Window->GetKey(PGR_KEY_S))
			m_C.camera.Pos.Y += 2.0f;
		if (m_Window->GetKey(PGR_KEY_A))
			m_C.camera.Pos.X += 2.0f;
		if (m_Window->GetKey(PGR_KEY_D))
			m_C.camera.Pos.X -= 2.0f;

		if (m_Window->GetWhell() < 0) {
			float nSize = m_C.camera.size - m_C.camera.size * 0.01f;
			float mx = (float)m_Window->GetMouseX() - (float)m_Width / 2.0f;
			float my = -((float)m_Window->GetMouseY() - (float)m_Height / 2.0f);
			m_C.camera.Pos.X = nSize / m_C.camera.size * m_C.camera.Pos.X - mx * (nSize / m_C.camera.size - 1.0f);
			m_C.camera.Pos.Y = nSize / m_C.camera.size * m_C.camera.Pos.Y - my * (nSize / m_C.camera.size - 1.0f);
			m_C.camera.size = nSize;
		}

		if (m_Window->GetWhell() > 0) {
			float nSize = m_C.camera.size + 0.01f;
			float mx = (float)m_Window->GetMouseX() - (float)m_Width / 2.0f;
			float my = -((float)m_Window->GetMouseY() - (float)m_Height / 2.0f);
			m_C.camera.Pos.X = nSize / m_C.camera.size * m_C.camera.Pos.X - mx * (nSize / m_C.camera.size - 1.0f);
			m_C.camera.Pos.Y = nSize / m_C.camera.size * m_C.camera.Pos.Y - my * (nSize / m_C.camera.size - 1.0f);
			m_C.camera.size = nSize;
		}

		if (m_Window->GetKey(PGR_KEY_SPACE) && !IsSpace) {
			IsPlaying = !IsPlaying;
			IsSpace = true;

			if (IsPlaying) {
				std::string str = "play music";
				mciSendString(str.c_str(), NULL, 0, NULL);
			}
			else
				mciSendString("pause music", NULL, 0, NULL);

		}

		if (m_Window->GetKey(PGR_KEY_SPACE) == PGR_RELEASE) {
			IsSpace = false;
		}

		if (!IsPress && m_Window->GetKey(PGR_BUTTON_LEFT)) {
			OriL = Vec2((float)(m_Window->GetMouseX()), (float)(m_Window->GetMouseY()));
			IsPress = true;
		}
		else if (IsPress && m_Window->GetKey(PGR_BUTTON_LEFT)) {
			Vec2 nowL = Vec2((float)(m_Window->GetMouseX()), (float)(m_Window->GetMouseY()));
			Vec2 delta = nowL - OriL;
			m_C.camera.Pos.X += delta.X;
			m_C.camera.Pos.Y -= delta.Y;
			OriL = nowL;
		}
		else if (IsPress)
			IsPress = false;

		if (m_Window->GetKey(PGR_KEY_F))
			m_Line = m_Line == (int)m_C.chart.data.judgeLines.size() - 1 ? -1 : m_Line + 1;
		if (m_Window->GetKey(PGR_KEY_R))
			m_Line = m_Line == -1 ? (int)m_C.chart.data.judgeLines.size() - 1 : m_Line - 1;

		if (m_Window->GetKey(PGR_KEY_C))
			__DEBUG__ = true;
		if (m_Window->GetKey(PGR_KEY_V))
			__DEBUG__ = false;

		Render(m_C.camera.size, m_C.camera.Pos.X, m_C.camera.Pos.Y);
	}

}
