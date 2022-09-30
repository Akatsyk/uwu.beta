#include "logs.h"
#include "../render/engine/engine_draw.h"

#include "../gui/gui.h"
#include "../config/config.h"

int ETime() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void Logs::DrawLogs()
{
	if (logs.empty())
		return;

	auto last_y = 0;

	int m_Total = 0;
	int m_Alpha = 0;
	int m_Rect = 0;
	int m_LastX = 0;

	const auto font = g_Options.CustomVisualsFont.LogsFont == 0 ? EngineDraw::Get().fonts.indicators_font : 
		g_Options.CustomFonts.m_Font[g_Options.CustomVisualsFont.LogsFont - 1];

	for (unsigned int i = 0; i < logs.size(); i++) {
		auto& log = logs.at(i);

		if (ETime() - log.log_time > 4700) {
			float factor = (log.log_time + 5100) - ETime();
			factor /= 1000;

			auto opacity = int(255 * factor);

			if (opacity < 2) {
				logs.erase(logs.begin() + i);
				continue;
			}

			log.color = Color(log.color.r(), log.color.g(),
				log.color.b(), opacity);

			log.x -= 20 * (factor * 1.25);
			log.y -= 0 * (factor * 1.25);
		}

		const auto text = log.message.c_str();
		auto m_Net = EngineDraw::Get().GetTextSize(font, text);

		m_LastX = log.x;
		m_Alpha = log.color.a();
		m_Total = i;
		m_Rect = m_Net.bottom;

		EngineDraw::Get().Gradient(log.x - 6, last_y + log.y, 400, m_Net.bottom, Color(0, 0, 0, m_Alpha), Color(0, 0, 0, 0), GradientType::GRADIENT_HORIZONTAL);
		EngineDraw::Get().DrawTextA(log.x - 3, last_y + log.y, log.color, font, false, text);
		
		last_y += m_Net.bottom;
	}

	EngineDraw::Get().Gradient(m_LastX - 6, (m_Rect * ( m_Total + 1 )), 400, 1, Color( Gui::m_Details.m_DefaultColor.r(), Gui::m_Details.m_DefaultColor.g(),
		Gui::m_Details.m_DefaultColor.b(), m_Alpha) , Color(0, 0, 0, 0), GradientType::GRADIENT_HORIZONTAL);
}

void Logs::AddLog(std::string text, Color color)
{
	logs.push_front(loginfo_t(ETime(), text, color));
}