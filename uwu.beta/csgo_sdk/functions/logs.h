#pragma once

#include "../sdk/sdk.hpp"
#include "../hooks.hpp"

#include <deque>

class Logs : public Singleton <Logs>
{
	friend class Singleton<Logs>;

public:

	void DrawLogs();
	void AddLog(std::string text, Color color);

private:

	struct loginfo_t {
		loginfo_t(const float log_time, std::string message, Color color)
		{
			this->log_time = log_time;
			this->message = message;
			this->color = color;

			this->x = 6.f;
			this->y = 0.f;
		}

		float log_time;
		std::string message;
		Color color;
		float x, y;
		float w, h;
		float alpha;
	};

	std::deque< loginfo_t > logs;

};