// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @nicegram, 2025
#pragma once

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QSet>
#include <QtCore/QString>

#include "ayu/features/translator/implementations/base.h"

namespace Ayu::Translator {

class DeepLTranslator final : public MultiThreadTranslator
{
	Q_OBJECT

public:
	static DeepLTranslator &instance();

	// all languages
	[[nodiscard]] QSet<QString> supportedLanguages() const override { return {}; }

	[[nodiscard]] QPointer<QNetworkReply> startSingleTranslation(
		const MultiThreadArgs &args
	) override;

private:
	explicit DeepLTranslator(QObject *parent = nullptr);

	QNetworkAccessManager _nam;
};

}
