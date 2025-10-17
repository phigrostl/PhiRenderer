#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include "PGR/Window/Window.h"
#include "PGR/Renderer/Texture.h"

#include <map>
#include <chrono>
#include <string>
#include <fstream>
#include <thread>
#include <direct.h>
#include <mmsystem.h>
#include <random>
#include <codecvt>
#include <locale>

#include "cJSON/cJSON.h"

#pragma comment(lib, "winmm.lib")

namespace PGR {

	constexpr float pgrw = 0.05625f;
	constexpr float pgrh = 0.6f;
	constexpr float pgrbeat = 60.0f / 32.0f;

	constexpr float linew = 0.0075f;
	constexpr float lineh = 5.76f;
	constexpr Vec3 pcolor = Vec3((float)0xff / 0xff, (float)0xec / 0xff, (float)0x9f / 0xff);
	constexpr float palpha = (float)0xe1 / 0xff;

	constexpr float noteSize = 0.1134375f;

	struct NoteImgs {
		Texture* click = new Texture("click.png");
		Texture* drag = new Texture("drag.png");
		Texture* hold = new Texture("hold.png");
		Texture* flick = new Texture("flick.png");

		Texture* holdBody = nullptr;
		Texture* holdHead = nullptr;
		Texture* holdTail = nullptr;

		Texture* clickMH = new Texture("clickMH.png");
		Texture* dragMH = new Texture("dragMH.png");
		Texture* holdMH = new Texture("holdMH.png");
		Texture* flickMH = new Texture("flickMH.png");

		Texture* holdMHBody = nullptr;
		Texture* holdMHHead = nullptr;
		Texture* holdMHTail = nullptr;

		Texture* hitFx = new Texture("hitFx.png");
	};

	enum NoteType {
		tap = 1,
		drag,
		hold,
		flick
	};

	struct Respack {
		Respack() = default;

		Vec2 hitFx;
		Vec2 holdAtlas;
		Vec2 holdAtlasMH;
	};

	struct ChartInfo {
		ChartInfo() = default;
		std::wstring level;
		std::wstring name;
		std::wstring song;
		std::wstring picture;
		std::wstring chart;
	};

	struct JudgeLineMoveEvent {
		JudgeLineMoveEvent() = default;
		float startTime, endTime;
		float start, end;
		float start2, end2;
	};

	struct JudgeLineRotateEvent {
		JudgeLineRotateEvent() = default;
		float startTime, endTime;
		float start, end;
	};

	struct JudgeLineDisappearEvent {
		JudgeLineDisappearEvent() = default;
		float startTime, endTime;
		float start, end;
	};

	struct SpeedEvent {
		SpeedEvent() = default;
		float startTime, endTime;
		float value;
		float floorPosition;
	};

	float linear(float t, float st, float et, float sv, float ev);

	Vec2 rotatePoint(float x, float y, float r, float deg);

	float randf(float a, float b);

	struct Particles {
		Particles() = default;
		Particles(float w, float h) {
			for (int i = 0; i < 4; i++) {
				float rotate = randf(0.0f, 360.0f);
				float size = 33.0f * 0.75f;
				float r = randf(185.0f, 265.0f);
				pars[i] = Vec3(rotate, size, r);
			}
		}
		Vec3 pars[4];
		Vec3* getClickEffect(float p) {
			Vec3 ret[4];
			for (int i = 0; i < 4; i++) {
				ret[i] = Vec3(pars[i].X, pars[i].Y, pars[i].Z * (9 * p / (8 * p + 1)));
			}
		}
	};

	struct EventsValue {
		EventsValue() = default;
		float rotate = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float alpha = 1.0f;
		float speed = 0.0f;
	};

	struct Note {
		Note() = default;
		int type = 0;
		float time = 0.0f;
		float floorPosition = 0.0f;
		float holdTime = 0.0f;
		float speed = 0.0f;
		float positionX = 0.0f;
		bool isAbove = false;
		float sect = 0.0f;
		float secht = 0.0f;
		float holdEndTime = 0.0f;
		float holdLength = 0.0f;
		bool isHold = false;
		bool clicked = false;
		int morebets = false;
		int line = 0;
		Vec2 getclickEffect(float w, float h, EventsValue ev) {
			Vec2 pos = rotatePoint(
				ev.x * w, ev.y * h,
				positionX * pgrw * w,
				ev.rotate
			);
			return pos;
		}
	};

	template<typename T>
	int findEvent(float t, std::vector<T> es) {
		size_t l = 0; size_t r = es.size() - 1;
		while (l <= r) {
			size_t m = (l + r) / 2;
			T e = es[m];

			if (e.startTime <= t && t <= e.endTime)
				return (int)m;
			else if (t < e.startTime)
				r = m - 1;
			else
				l = m + 1;
		}
		return -1;
	}
	template<typename T>
	float getEventValue(float t, std::vector<T> es) {
		const int i = findEvent(t, es);
		if (i == -1)
			return 0.0f;

		const T& e = es[i];

		return linear(t, e.startTime, e.endTime, e.start, e.end);
	}

	float getPosYEvent(float t, std::vector<JudgeLineMoveEvent> es);

	float getSpeedValue(float t, std::vector<SpeedEvent> es);

	struct NoteMap {
		NoteMap() = default;
		Note note;
		float sect;
		Particles particles;
	};


	struct JudgeLine {
		JudgeLine() = default;

		float bpm = 0.0f;
		std::vector<JudgeLineMoveEvent> moveEvents;
		std::vector<JudgeLineRotateEvent> rotateEvents;
		std::vector<JudgeLineDisappearEvent> disappearEvents;
		std::vector<SpeedEvent> speedEvents;

		std::vector<Note> notesAbove;
		std::vector<Note> notesBelow;

		std::vector<Note> notes;

		float sec2beat(float t) {
			return t / (pgrbeat / bpm);
		}

		float beat2sec(float t) {
			return t * (pgrbeat / bpm);
		}

		void initSpeedEvents() {
			float fp = 0.0f;

			for (auto& e : speedEvents) {
				e.floorPosition = fp;
				fp += (e.endTime - e.startTime) * e.value;
			}
		}

		void mergeNotes() {
			for (auto& n : notesAbove) {
				n.isAbove = true;
				this->notes.push_back(n);
			}
			for (auto& n : notesBelow) {
				n.isAbove = false;
				this->notes.push_back(n);
			}
		}

		float getFp(float t) {
			int i = findEvent(t, speedEvents);
			if (i == -1)
				return 0.0f;

			SpeedEvent e = speedEvents[i];
			return e.floorPosition + (t - e.startTime) * e.value;
		}

		void initNoteFp() {
			for (auto& n : notes)
				n.floorPosition = getFp(n.time);
		}

		EventsValue getState(float t) {
			float beatt = sec2beat(t);
			float rotate = getEventValue(beatt, rotateEvents);
			float x = getEventValue(beatt, moveEvents);
			float y = getPosYEvent(beatt, moveEvents);
			float alpha = getEventValue(beatt, disappearEvents);
			float speed = getSpeedValue(beatt, speedEvents);
			return { rotate, x, y, alpha, speed };
		}

	};

	struct ChartData {
		std::vector<JudgeLine> judgeLines;
		std::vector<NoteMap> clickEffectCollection;
		int noteCount = 0;
		float time = 0.0f;
	};

	struct Chart {
		Chart() = default;

		ChartInfo info;
		Texture* image = nullptr;
		Texture* blurImage = nullptr;
		cJSON* json = nullptr;
		ChartData data;
		std::string path;
		std::string wPath;

	};

	struct Camera {
		Camera() = default;
		Vec2 Pos = { 0.0f, 0.0f };
		float size = 1.0f;
	};

	struct C {
		NoteImgs noteImgs;
		std::vector<Texture*> hitFxImgs;
		Chart chart;
		Texture* noteHeadImgs[4][2] = { 0 };
		Texture* holdBodyImgs[2] = { 0 };
		Texture* holdTailImgs[2] = { 0 };
		Camera camera;
	};

	class Application {
	public:
		Application(
			int argc, char** argv,
			const std::string& name,
			const int width, const int height
		);

		~Application();

		void Run();

		bool __DEBUG__ = false;

	private:
		void Init();
		void Terminate();

		void OnUpdate();
		void Render(float size, float ox, float oy);

		void LoadFiles();

	private:
		void LoadImgs();
		void LoadFxImgs();
		void LoadJsons();
		void DrawTexture(Texture* texture, int x, int y, const float sx = 1.0f, const float sy = -1.0f, float angle = 0.0f);

	private:
		std::string m_Name;
		int m_Width, m_Height;

		Window* m_Window;
		Framebuffer* m_Framebuffer;

		std::chrono::steady_clock::time_point m_LastFrameTime;
		std::chrono::steady_clock::time_point m_StartFrameTime;

		Vec2 OriL;
		bool IsPress = false;
		bool IsPlaying = false;
		bool IsSpace = false;

		Respack m_Respack;

		float m_Time;
		float m_Accumulator = 0.0f;
		int m_FPSCounter = 0;

		int m_Line = -1;

		C m_C;
	};

}
