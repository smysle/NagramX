/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "base/basic_types.h"

#include <QtNetwork/QNetworkAccessManager>

namespace NagramX {

struct LlmMessage {
	QString role;
	QString content;
};

struct LlmRequest {
	QString systemPrompt;
	std::vector<LlmMessage> messages;
	float temperature = 0.7f;
};

struct LlmResult {
	QString text;
	QString error;
};

class LlmClient final {
public:
	void complete(LlmRequest request, Fn<void(LlmResult)> done);

private:
	QNetworkAccessManager _network;
};

} // namespace NagramX
