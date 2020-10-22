#include "Measurement.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "../gl_core_3_3.h"
#include <glfw/glfw3.h>


namespace jge
{
	void FrameCounter::Tick()
	{
		current = glfwGetTime();
		if (current - begin >= 1.0)
		{
			fps = frames;
			frames = 0;
			begin = glfwGetTime();
		}
		else
		{
			frames++;
		}
	}

	int FrameCounter::GetFPS() const
	{
		return fps;
	}

	float FrameCounter::GetTimeForFrame() const
	{
		return (1.0f / fps) * 1000.0f;
	}

	void Stopwatch::Start()
	{
		beginTime = glfwGetTime();
	}

	void Stopwatch::Stop()
	{
		endTime = glfwGetTime();
	}

	double Stopwatch::GetElapsedTime() const
	{
		return endTime - beginTime;
	}

	void jgeSleep(int sleepMs)
	{
		#ifdef __unix__
				usleep(sleepMs * 1000); 
		#endif
		#ifdef _WIN32
				Sleep(sleepMs);
		#endif
	}


#define FRONT_BUFFER 0
#define BACK_BUFFER 1

	OpenGlTimer::OpenGlTimer()
		: front(FRONT_BUFFER)
		, back(BACK_BUFFER)
	{
		glGenQueries(2, (GLuint*)queryIds[FRONT_BUFFER]);
		glGenQueries(2, (GLuint*)queryIds[BACK_BUFFER]);

		glQueryCounter(queryIds[FRONT_BUFFER][0], GL_TIMESTAMP);
	}

	OpenGlTimer::~OpenGlTimer()
	{
		glDeleteQueries(2, (GLuint*)queryIds[FRONT_BUFFER]);
		glDeleteQueries(2, (GLuint*)queryIds[BACK_BUFFER]);
	}

	void OpenGlTimer::Start()
	{
		//glBeginQuery(GL_TIME_ELAPSED, (GLuint)queryIds[back][0]);
		glQueryCounter(queryIds[back][0], GL_TIMESTAMP);
	}

	void OpenGlTimer::Stop()
	{
		//glEndQuery(GL_TIME_ELAPSED);
		glQueryCounter(queryIds[back][1], GL_TIMESTAMP);
	}

	void OpenGlTimer::Swap()
	{
		if (back)
		{
			front = BACK_BUFFER;
			back = FRONT_BUFFER;
		}
		else
		{
			front = FRONT_BUFFER;
			back = BACK_BUFFER;
		}
	}

	unsigned int OpenGlTimer::GetElapsedTime() const
	{
		GLuint64 start, stop;
		glGetQueryObjectui64v((GLuint)queryIds[front][0], GL_QUERY_RESULT, &start);
		glGetQueryObjectui64v((GLuint)queryIds[front][1], GL_QUERY_RESULT, &stop);
		return stop - start;
	}
}