// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @nicegram, 2025
#include "nagramx/translate/nagramx_translate_llm.h"

#include "ayu/ayu_settings.h"
#include "nagramx/llm/nagramx_llm_client.h"

namespace Ayu::Translator {

LlmTranslator &LlmTranslator::instance() {
	static LlmTranslator inst;
	return inst;
}

LlmTranslator::LlmTranslator(QObject *parent)
	: BaseTranslator(parent) {
}

CallbackCancel LlmTranslator::startTranslation(
	const StartTranslationArgs &args
) {
	const auto &parsedData = args.parsedData;
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;

	if (parsedData.texts.empty() || parsedData.toLang.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}

	// Combine all texts for translation.
	auto combinedText = QString();
	for (const auto &t : parsedData.texts) {
		if (!combinedText.isEmpty()) {
			combinedText += QStringLiteral("\n");
		}
		combinedText += t.text;
	}

	if (combinedText.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}

	const auto lang = parsedData.toLang.trimmed();
	const auto systemPrompt = QStringLiteral(
		"You are a translator. Translate the following text to %1. "
		"Return only the translation, no explanations."
	).arg(lang);

	// Temperature: use a sensible default for translation.
	constexpr auto kDefaultTemperature = 0.3f;

	auto llmRequest = NagramX::LlmRequest{
		.systemPrompt = systemPrompt,
		.messages = { NagramX::LlmMessage{
			.role = QStringLiteral("user"),
			.content = combinedText,
		} },
		.temperature = kDefaultTemperature,
	};

	auto cancelled = std::make_shared<bool>(false);

	_client.complete(std::move(llmRequest), [=](NagramX::LlmResult llmResult) {
		if (*cancelled) {
			return;
		}
		if (llmResult.error.isEmpty()) {
			// Return the result as a single-element vector to match the
			// CallbackSuccess signature.
			std::vector<TextWithEntities> results;
			results.push_back(TextWithEntities{llmResult.text});
			if (onSuccess) onSuccess(results);
		} else {
			if (onFail) onFail();
		}
	});

	return [cancelled] {
		*cancelled = true;
	};
}

}
