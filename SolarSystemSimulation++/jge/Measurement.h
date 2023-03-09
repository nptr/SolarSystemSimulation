#pragma once

namespace jge
{
	class FrameCounter
	{
	public:
		void Tick();
		int GetFPS() const;
		float GetTimeForFrame() const;

	private:
		double begin;
		double current;
		int frames;
		int fps;
	};

	class Stopwatch
	{
	public:
		void Start();
		void Stop();
		double GetElapsedTime() const;

	private:
		double beginTime;
		double endTime;
	};

	class OpenGlTimer
	{
	public:
		OpenGlTimer();
		~OpenGlTimer();

		void Start();
		void Stop();
		void Swap();

		unsigned int GetElapsedTime() const;

	private:
		// [front & backbuffer][query]
		int queryIds[2][2];
		int front;
		int back;
	};

	void jgeSleep(int sleepMs);
}

