/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "nagramx/llm/nagramx_llm_provider_presets.h"

namespace NagramX {
namespace {

const LlmPresetInfo kPresets[] = {
	{
		u"Custom"_q,
		QString(),
		QString(),
	},
	{
		u"OpenAI"_q,
		u"https://api.openai.com/v1"_q,
		u"gpt-4.1-mini"_q,
	},
	{
		u"Gemini"_q,
		u"https://generativelanguage.googleapis.com/v1beta/openai"_q,
		u"gemini-2.0-flash"_q,
	},
	{
		u"Groq"_q,
		u"https://api.groq.com/openai/v1"_q,
		u"llama-3.3-70b-versatile"_q,
	},
	{
		u"DeepSeek"_q,
		u"https://api.deepseek.com/v1"_q,
		u"deepseek-chat"_q,
	},
	{
		u"xAI"_q,
		u"https://api.x.ai/v1"_q,
		u"grok-3-mini"_q,
	},
	{
		u"Cerebras"_q,
		u"https://api.cerebras.ai/v1"_q,
		u"llama-3.3-70b"_q,
	},
	{
		u"Ollama"_q,
		u"http://localhost:11434/v1"_q,
		u"llama3"_q,
	},
	{
		u"OpenRouter"_q,
		u"https://openrouter.ai/api/v1"_q,
		u"openai/gpt-4.1-mini"_q,
	},
};

static_assert(
	std::size(kPresets) == kLlmPresetCount,
	"kPresets size must match kLlmPresetCount");

} // namespace

LlmPresetInfo LlmPresetInfoFor(int id) {
	if (id < 0 || id >= kLlmPresetCount) {
		return kPresets[0];
	}
	return kPresets[id];
}

LlmPresetInfo LlmPresetInfoFor(LlmPreset preset) {
	return LlmPresetInfoFor(static_cast<int>(preset));
}

} // namespace NagramX
