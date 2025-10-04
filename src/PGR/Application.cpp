#include "Application.h"

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

	static void PlayAudio(const std::string& name) {
		mciSendString(("play " + name).c_str(), NULL, 0, NULL);
	}

	void Application::DrawTexture(Texture* texture, const int x, const int y, const float sx, float sy, float angle) {
		if (sy == -1)
			sy = sx;
		float srcW = (float)texture->GetWidth();
		float srcH = (float)texture->GetHeight();
		float w = srcW * sx;
		float h = srcH * sy;

		float rad = angle * 3.14159265f / 180.0f;
		float cosA = cosf(rad);
		float sinA = sinf(rad);

		float corners[4][2] = {
			{0, 0},
			{w, 0},
			{0, h},
			{w, h}
		};

		float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
		for (int k = 0; k < 4; ++k) {
			float rx = corners[k][0] * cosA - corners[k][1] * sinA;
			float ry = corners[k][0] * sinA + corners[k][1] * cosA;
			minX = Min(minX, rx);
			minY = Min(minY, ry);
			maxX = Max(maxX, rx);
			maxY = Max(maxY, ry);
		}

		int startY = static_cast<int>(std::floor(minY));
		int endY = static_cast<int>(std::ceil(maxY));
		int startX = static_cast<int>(std::floor(minX));
		int endX = static_cast<int>(std::ceil(maxX));

		float invSx = 1.0f / sx;
		float invSy = 1.0f / sy;

		for (int j = startY; j <= endY; ++j) {
			float jSinA = j * sinA;
			float jCosA = j * cosA;
			int dstY = y + j;
			
			if (dstY < 0 || dstY >= m_Height)
				continue;

			for (int i = startX; i <= endX; ++i) {
				float tx = (i * cosA + jSinA) * invSx;
				float ty = (-i * sinA + jCosA) * invSy;

				if (tx >= 0 && tx < srcW && ty >= 0 && ty < srcH) {
					int dstX = x + i;
					if (dstX >= 0 && dstX < m_Width) {
						int texX = static_cast<int>(tx);
						int texY = static_cast<int>(ty);

						Vec4 tColor = texture->GetColor(texX, texY);
						if (tColor.W < 1.0f && tColor.W > 0.0f) {
							Vec4 color = Lerp(Vec4(m_Framebuffer->GetColor(dstX, dstY), 1.0f), tColor, tColor.W);
							m_Framebuffer->SetColor(dstX, dstY, color);
						}
						else if (tColor.W >= 1.0f) {
							m_Framebuffer->SetColor(dstX, dstY, tColor);
						}
					}
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

		file.open("chart\\chart.json");
		json = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		root = cJSON_Parse(json.c_str());

		std::map<float, int> noteSectCounter;

		arrayExt = cJSON_GetObjectItem(root, "judgeLineList");
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
			}
			arrayInt = cJSON_GetObjectItem(line, "speedEvents");
			for (int j = 0; j < cJSON_GetArraySize(arrayInt); j++) {
				SpeedEvent e;
				obj = cJSON_GetArrayItem(arrayInt, j);
				e.startTime = (float)cJSON_GetObjectItem(obj, "startTime")->valuedouble;
				e.endTime = (float)cJSON_GetObjectItem(obj, "endTime")->valuedouble;
				e.value = (float)cJSON_GetObjectItem(obj, "value")->valuedouble;
				jline.speedEvents.push_back(e);
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

		}

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
	}

	void Application::LoadImgs() {
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

		m_C.chart.image = (new Texture("chart\\image-blur.png"))->ColorTexture(Vec4(0.6f));
	}

	void Application::LoadFxImgs() {
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
		mciSendString("open click.wav type waveaudio alias click", NULL, 0, NULL);
		mciSendString("open drag.wav type waveaudio alias drag", NULL, 0, NULL);
		mciSendString("open flick.wav type waveaudio alias flick", NULL, 0, NULL);
		
	}

	void Application::LoadFiles() {
		LoadJsons();
		LoadImgs();
		LoadFxImgs();
	}

	void Application::Init() {
		Window::Init();
		m_Window = Window::Create(m_Name, m_Width, m_Height);

		m_Framebuffer = Framebuffer::Create(m_Width, m_Height);
		m_Framebuffer->LoadFontTTF("Exo-Regular");

		LoadFiles();

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
		PlaySound("chart/music.wav", NULL, SND_ASYNC | SND_NODEFAULT);
		while (!m_Window->Closed()) {
			m_Framebuffer->Clear(Vec3(0.09f, 0.10f, 0.14f));
			m_Window->PollInputEvents();

			float deltaTime = (float)(std::chrono::steady_clock::now() - m_LastFrameTime).count();

			m_LastFrameTime = std::chrono::steady_clock::now();

			float startTime = (float)(std::chrono::steady_clock::now() - m_StartFrameTime).count();

			OnUpdate(startTime, deltaTime);
			m_Window->DrawFramebuffer(m_Framebuffer);
		}
	}

	void Application::Render(float t) {
		float noteW = noteSize * m_Width;
		for (size_t i = 0; i < m_C.chart.data.judgeLines.size(); i++) {

			JudgeLine line = m_C.chart.data.judgeLines[i];

			EventsValue ev = line.getState(t);
			ev.x *= m_Width, ev.y *= m_Height;
			Vec2 linePos[2] = {
				rotatePoint(ev.x, ev.y, m_Height * lineh, ev.rotate),
				rotatePoint(ev.x, ev.y, m_Height * lineh, ev.rotate + 180.0f)
			};

			m_Framebuffer->DrawLine(
				(int)linePos[0].X, (int)linePos[0].Y,
				(int)linePos[1].X, (int)linePos[1].Y,
				m_Height * linew, Vec4(pcolor, ev.alpha * 0.99f + 0.01f)
			);

			float beatt = line.sec2beat(t);
			float lineFp = line.getFp(beatt);

			JudgeLine& currentLine = m_C.chart.data.judgeLines[i];
			auto& notes = currentLine.notes;
			size_t notesCount = notes.size();
			float pgrwTimesWidth = pgrw * m_Width;
			float PI_OVER_180 = PI / 180.0f;
			float cosEvRotate = cos(ev.rotate * PI_OVER_180);
			float sinEvRotate = sin(ev.rotate * PI_OVER_180);

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
					continue;
				}

				float noteFp = (note.floorPosition - lineFp) * pgrh * (pgrbeat / line.bpm) * m_Height;

				if (!note.isHold) {
					noteFp *= note.speed;
					if (noteFp < -1e6) {
						continue;
					}
				}

				if (noteFp > m_Height * 2) {
					continue;
				}

				bool drawHead = note.sect > t;
				Texture* noteHeadImg = m_C.noteHeadImgs[note.type - 1][note.morebets];
				float thisNoteWidth = 1.0f * noteSize;

				if (note.morebets) {
					thisNoteWidth = noteSize * (
						(float)(m_C.noteHeadImgs[note.type - 1][1]->GetWidth())
						/ (float)(m_C.noteHeadImgs[note.type - 1][0]->GetWidth())
						);
				}

				float headImgWidth = (float)noteHeadImg->GetWidth();
				float headImgHeight = (float)noteHeadImg->GetHeight();
				float thisNoteHeadHeight = thisNoteWidth / headImgWidth * headImgHeight;
				float posX = note.positionX * pgrwTimesWidth;
				
				float noteAtlineX = ev.x + posX * cosEvRotate - 0 * sinEvRotate;
				float noteAtlineY = ev.y + posX * sinEvRotate + 0 * cosEvRotate;
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

				float size = m_Width * noteSize / headImgWidth;
				float W = size * headImgWidth * thisNoteWidth / noteSize / 2.0f;
				float H = size * headImgHeight / 2.0f;
				float w = W * cosDrawRad - H * sinDrawRad;
				float h = H * cosDrawRad + W * sinDrawRad;

				if (drawHead) {
					float texScale = thisNoteWidth * m_Width / headImgWidth;
					DrawTexture(
						noteHeadImg, (int)(noteHeadPos.X - w), (int)(noteHeadPos.Y - h),
						texScale, texScale,
						noteDrawRotate
					);
				}

				if (note.isHold) {
					Texture* noteBodyImg = m_C.holdBodyImgs[note.morebets];
					Texture* noteTailImg = m_C.holdTailImgs[note.morebets];
					float noteTillHeight = thisNoteWidth * m_Width / noteTailImg->GetWidth() * noteTailImg->GetHeight();
					float noteHeadHeight = thisNoteWidth * m_Width / headImgWidth * headImgHeight;

					float noteBodyHeight = Max(
						note.holdLength * m_Height +
						Min(0.0f, noteFp) +
						(note.clicked ? noteHeadHeight : 0.0f) -
						noteTillHeight, 0.0f
					);

					float tailPosBaseX = note.clicked ? noteAtlineX : headX;
					float tailPosBaseY = note.clicked ? noteAtlineY : headY;
					float tailOffset = (note.clicked ? 0 : noteHeadHeight) + noteBodyHeight;
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
						note.clicked ? (int)(noteAtlineX - w) : (int)x,
						note.clicked ? (int)(noteAtlineY - h) : (int)y,
						bodyTexScaleX, bodyTexScaleY,
						noteDrawRotate
					);

					float tailImgWidth = (float)noteTailImg->GetWidth();
					float tailImgHeight = (float)noteTailImg->GetHeight();
					float tailS = m_Width * noteSize / tailImgWidth;
					float tailW = tailS * tailImgWidth * thisNoteWidth / noteSize / 2.0f;
					float tailH = tailS * tailImgHeight / 2.0f;
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
		}

		float effectDur = 0.5f;
		float pgrwTimesWidthEffect = pgrw * m_Width;
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

			float posX = ev.x * m_Width;
			float posY = ev.y * m_Height;
			float rotRad = ev.rotate * PI_OVER_180_EFFECT;
			float cosRot = cos(rotRad);
			float sinRot = sin(rotRad);
			float offsetX = nm.note.positionX * pgrwTimesWidthEffect;
			
			float finalX = posX + offsetX * cosRot;
			float finalY = posY + offsetX * sinRot;
			Vec2 pos(finalX, finalY);

			float texScale = effectSize / img->GetWidth();
			DrawTexture(
				img,
				(int)(finalX - halfEffectSize), (int)(finalY - halfEffectSize),
				texScale
			);

			for (int parIdx = 0; parIdx < 4; parIdx++) {
				const Vec3& parItem = nm.particles.pars[parIdx];
				float parRad = parItem.X * PI_OVER_180_EFFECT;
				float cosParRad = cos(parRad);
				float sinParRad = sin(parRad);
				
				float parCenterX = parItem.Z * cosParRad;
				float parCenterY = parItem.Z * sinParRad;
				float scaledParSize = parItem.Y * p;
				
				float pScaledCenterX = parCenterX * (9.0f * p / (8.0f * p + 1.0f));
				float pScaledCenterY = parCenterY * (9.0f * p / (8.0f * p + 1.0f));
				float x1 = pScaledCenterX + finalX;
				float y1 = pScaledCenterY + finalY;
				
				m_Framebuffer->FillSizeRect((int)x1, (int)y1, (int)parItem.Y, (int)parItem.Y, Vec4(pcolor, alpha));
			}
		}
	}

	void Application::OnUpdate(float startTime, float deltaTime) {

		Texture* texture = m_C.chart.image;
		DrawTexture(
			texture, 0, 0,
			(float)m_Width / texture->GetWidth(),
			(float)m_Height / texture->GetHeight()
		);
		
		Render(startTime / 1000 / 1000 / 1000);

		float FPS = 1.0f / (deltaTime / 1000 / 1000 / 1000);
		m_Framebuffer->DrawTextTTF(0, 0, std::to_string(FPS), Vec4(1.0f), 20.0f);
		m_Framebuffer->DrawTextTTF(0, 20, std::to_string(startTime / 1000.0f / 1000.0f / 1000.0f), Vec4(1.0f), 20.0f);
	}

}
