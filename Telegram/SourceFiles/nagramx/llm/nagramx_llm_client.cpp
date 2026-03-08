/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "nagramx/llm/nagramx_llm_client.h"

#include "ayu/ayu_settings.h"
#include "nagramx/llm/nagramx_llm_provider_presets.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace NagramX {
namespace {

[[nodiscard]] QString EffectiveApiUrl() {
	const auto &settings = AyuSettings::getInstance();
	auto url = settings.nagramxLlmApiUrl;
	if (url.isEmpty()) {
		const auto preset = settings.nagramxLlmProviderPreset;
		url = LlmPresetInfoFor(preset).defaultUrl;
	}
	if (url.isEmpty()) {
		url = u"https://api.openai.com/v1"_q;
	}
	while (url.endsWith('/')) {
		url.chop(1);
	}
	return url;
}

[[nodiscard]] QString EffectiveModel() {
	const auto &settings = AyuSettings::getInstance();
	auto model = settings.nagramxLlmModel;
	if (model.isEmpty()) {
		const auto preset = settings.nagramxLlmProviderPreset;
		model = LlmPresetInfoFor(preset).defaultModel;
	}
	if (model.isEmpty()) {
		model = u"gpt-4.1-mini"_q;
	}
	return model;
}

} // namespace

void LlmClient::complete(LlmRequest request, Fn<void(LlmResult)> done) {
	const auto apiUrl = EffectiveApiUrl();
	const auto model = EffectiveModel();
	const auto apiKey = AyuSettings::getInstance().nagramxLlmApiKey;

	auto messages = QJsonArray();
	if (!request.systemPrompt.isEmpty()) {
		messages.append(QJsonObject{
			{ u"role"_q, u"system"_q },
			{ u"content"_q, request.systemPrompt },
		});
	}
	for (const auto &msg : request.messages) {
		messages.append(QJsonObject{
			{ u"role"_q, msg.role },
			{ u"content"_q, msg.content },
		});
	}

	auto body = QJsonObject{
		{ u"model"_q, model },
		{ u"messages"_q, messages },
		{ u"temperature"_q, double(request.temperature) },
	};

	const auto url = QUrl(apiUrl + u"/chat/completions"_q);
	auto networkRequest = QNetworkRequest(url);
	networkRequest.setHeader(
		QNetworkRequest::ContentTypeHeader,
		u"application/json"_q);
	if (!apiKey.isEmpty()) {
		networkRequest.setRawHeader(
			"Authorization",
			("Bearer " + apiKey).toUtf8());
	}

	const auto payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
	const auto reply = _network.post(networkRequest, payload);
	QObject::connect(reply, &QNetworkReply::finished, [=] {
		auto result = LlmResult();
		if (reply->error() != QNetworkReply::NoError) {
			result.error = reply->errorString();
			const auto responseBody = reply->readAll();
			if (!responseBody.isEmpty()) {
				auto parseError = QJsonParseError();
				const auto doc = QJsonDocument::fromJson(
					responseBody,
					&parseError);
				if (parseError.error == QJsonParseError::NoError
					&& doc.isObject()) {
					const auto errorObj = doc.object()
						.value(u"error"_q).toObject();
					const auto message = errorObj
						.value(u"message"_q).toString();
					if (!message.isEmpty()) {
						result.error = message;
					}
				}
			}
		} else {
			const auto responseBody = reply->readAll();
			auto parseError = QJsonParseError();
			const auto doc = QJsonDocument::fromJson(
				responseBody,
				&parseError);
			if (parseError.error != QJsonParseError::NoError) {
				result.error = u"Failed to parse response JSON"_q;
			} else {
				const auto choices = doc.object()
					.value(u"choices"_q).toArray();
				if (choices.isEmpty()) {
					result.error = u"No choices in response"_q;
				} else {
					result.text = choices[0].toObject()
						.value(u"message"_q).toObject()
						.value(u"content"_q).toString();
				}
			}
		}
		done(std::move(result));
		reply->deleteLater();
	});
}

} // namespace NagramX
