/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QString>

namespace NagramX {

struct LlmPresetInfo {
	QString name;
	QString defaultUrl;
	QString defaultModel;
};

enum class LlmPreset : int {
	Custom = 0,
	OpenAI = 1,
	Gemini = 2,
	Groq = 3,
	DeepSeek = 4,
	XAI = 5,
	Cerebras = 6,
	Ollama = 7,
	OpenRouter = 8,
};

inline constexpr auto kLlmPresetCount = 9;

[[nodiscard]] LlmPresetInfo LlmPresetInfoFor(int id);
[[nodiscard]] LlmPresetInfo LlmPresetInfoFor(LlmPreset preset);

} // namespace NagramX
