// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @nicegram, 2025
#include "nagramx/translate/nagramx_translate_lingo.h"

#include "ayu/ayu_settings.h"

#include <memory>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace Ayu::Translator {

LingoTranslator &LingoTranslator::instance() {
	static LingoTranslator inst;
	return inst;
}

LingoTranslator::LingoTranslator(QObject *parent)
	: MultiThreadTranslator(parent) {
}

QPointer<QNetworkReply> LingoTranslator::startSingleTranslation(
	const MultiThreadArgs &args
) {
	const auto &text = args.parsedData.text;
	const auto &toLang = args.parsedData.toLang;
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;

	if (text.empty() || toLang.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}

	const auto &settings = AyuSettings::getInstance();
	const auto token = settings.nagramxLingoApiToken;
	if (token.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}

	const auto lang = toLang.trimmed();
	const auto transType = QStringLiteral("auto2%1").arg(lang);

	auto sourceArray = QJsonArray();
	sourceArray.append(text.text);

	auto body = QJsonObject();
	body.insert(QStringLiteral("source"), sourceArray);
	body.insert(QStringLiteral("trans_type"), transType);
	body.insert(QStringLiteral("request_id"), QStringLiteral("nagramx"));
	body.insert(QStringLiteral("detect"), true);

	auto req = QNetworkRequest(QUrl(QStringLiteral("https://api.interpreter.caiyunai.com/v1/translator")));
	req.setHeader(QNetworkRequest::ContentTypeHeader,
				  QStringLiteral("application/json"));
	req.setRawHeader(QByteArrayLiteral("X-Authorization"),
					 QStringLiteral("token %1").arg(token).toUtf8());
	req.setHeader(QNetworkRequest::UserAgentHeader, randomDesktopUserAgent());

	QPointer<QNetworkReply> reply = _nam.post(
		req,
		QJsonDocument(body).toJson(QJsonDocument::Compact));

	auto timer = new QTimer(reply);
	timer->setSingleShot(true);
	timer->setInterval(15000);
	QObject::connect(timer,
					 &QTimer::timeout,
					 reply,
					 [reply]
					 {
						 if (!reply) return;
						 if (reply->isRunning()) reply->abort();
					 });
	timer->start();

	QObject::connect(reply,
					 &QNetworkReply::finished,
					 reply,
					 [reply, onSuccess = onSuccess, onFail = onFail, timer]
					 {
						 if (!reply) return;
						 timer->stop();
						 const auto guard = std::unique_ptr<QNetworkReply, void(*)(QNetworkReply *)>(
							 reply,
							 [](QNetworkReply *r) { r->deleteLater(); });
						 if (reply->error() != QNetworkReply::NoError) {
							 if (onFail) onFail();
							 return;
						 }
						 const auto body = reply->readAll();
						 QJsonParseError parseError{};
						 const auto doc = QJsonDocument::fromJson(body, &parseError);
						 if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
							 if (onFail) onFail();
							 return;
						 }
						 const auto target = doc.object()
							 .value(QStringLiteral("target")).toArray();
						 if (target.isEmpty()) {
							 if (onFail) onFail();
							 return;
						 }
						 const auto translatedText = target[0].toString();
						 if (translatedText.trimmed().isEmpty()) {
							 if (onFail) onFail();
							 return;
						 }
						 if (onSuccess) onSuccess(TextWithEntities{translatedText});
					 });

	return reply;
}

}
