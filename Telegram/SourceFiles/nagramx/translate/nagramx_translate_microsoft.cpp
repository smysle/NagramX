// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @nicegram, 2025
#include "nagramx/translate/nagramx_translate_microsoft.h"

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

MicrosoftTranslator &MicrosoftTranslator::instance() {
	static MicrosoftTranslator inst;
	return inst;
}

MicrosoftTranslator::MicrosoftTranslator(QObject *parent)
	: MultiThreadTranslator(parent) {
}

QPointer<QNetworkReply> MicrosoftTranslator::startSingleTranslation(
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
	const auto key = settings.nagramxMicrosoftTranslateKey;
	if (key.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}

	const auto lang = toLang.trimmed();
	const auto url = QStringLiteral(
		"https://api.cognitive.microsofttranslator.com/translate?api-version=3.0&to=%1"
	).arg(lang);

	auto bodyArray = QJsonArray();
	auto entry = QJsonObject();
	entry.insert(QStringLiteral("Text"), text.text);
	bodyArray.append(entry);

	auto req = QNetworkRequest(QUrl(url));
	req.setHeader(QNetworkRequest::ContentTypeHeader,
				  QStringLiteral("application/json"));
	req.setRawHeader(QByteArrayLiteral("Ocp-Apim-Subscription-Key"),
					 key.toUtf8());
	req.setHeader(QNetworkRequest::UserAgentHeader, randomDesktopUserAgent());

	QPointer<QNetworkReply> reply = _nam.post(
		req,
		QJsonDocument(bodyArray).toJson(QJsonDocument::Compact));

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
						 if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
							 if (onFail) onFail();
							 return;
						 }
						 const auto root = doc.array();
						 if (root.isEmpty()) {
							 if (onFail) onFail();
							 return;
						 }
						 const auto translations = root[0]
							 .toObject().value(QStringLiteral("translations")).toArray();
						 if (translations.isEmpty()) {
							 if (onFail) onFail();
							 return;
						 }
						 const auto translatedText = translations[0]
							 .toObject().value(QStringLiteral("text")).toString();
						 if (translatedText.trimmed().isEmpty()) {
							 if (onFail) onFail();
							 return;
						 }
						 if (onSuccess) onSuccess(TextWithEntities{translatedText});
					 });

	return reply;
}

}
