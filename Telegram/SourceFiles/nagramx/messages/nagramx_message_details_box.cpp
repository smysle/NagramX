/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "nagramx/messages/nagramx_message_details_box.h"

#include "data/data_document.h"
#include "data/data_peer.h"
#include "history/history.h"
#include "history/history_item.h"
#include "history/history_item_components.h"
#include "lang/lang_keys.h"
#include "ui/layers/generic_box.h"
#include "ui/vertical_list.h"
#include "ui/widgets/labels.h"
#include "ui/text/text_utilities.h"
#include "window/window_session_controller.h"
#include "styles/style_layers.h"
#include "styles/style_settings.h"
#include <QtCore/QDateTime>
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>

namespace NagramX {

void ShowMessageDetailsBox(
		not_null<Window::SessionController*> controller,
		not_null<HistoryItem*> item) {
	controller->show(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(u"Message Details"_q));

		const auto content = box->verticalLayout();

		const auto addRow = [&](const QString &label, const QString &value) {
			const auto text = Ui::Text::Bold(label + u": "_q).append(value);
			content->add(
				object_ptr<Ui::FlatLabel>(content, rpl::single(text), st::defaultFlatLabel),
				st::boxRowPadding);
		};

		const auto peer = item->history()->peer;
		const auto author = item->from();

		addRow(u"Message ID"_q, QString::number(item->fullId().msg.bare));
		addRow(u"Chat"_q, peer->name() + u" ("_q + QString::number(peer->id.value) + u")"_q);
		addRow(u"From"_q, author->name() + u" ("_q + QString::number(author->id.value) + u")"_q);
		addRow(u"Date"_q, QDateTime::fromSecsSinceEpoch(item->date()).toString(u"yyyy-MM-dd hh:mm:ss"_q));

		if (const auto edited = item->Get<HistoryMessageEdited>()) {
			addRow(u"Edited"_q, QDateTime::fromSecsSinceEpoch(edited->date).toString(u"yyyy-MM-dd hh:mm:ss"_q));
		}

		if (const auto forwarded = item->Get<HistoryMessageForwarded>()) {
			auto fromName = forwarded->originalSender
				? forwarded->originalSender->name()
				: forwarded->originalHiddenSenderInfo
					? forwarded->originalHiddenSenderInfo->name
					: u"Unknown"_q;
			addRow(u"Forwarded from"_q, fromName);
		}

		if (item->hasViews()) {
			addRow(u"Views"_q, QString::number(item->viewsCount()));
		}

		if (const auto media = item->media()) {
			auto kind = u"media"_q;
			if (media->photo()) {
				kind = u"photo"_q;
			} else if (const auto doc = media->document()) {
				if (doc->isVoiceMessage()) {
					kind = u"voice"_q;
				} else if (doc->isVideoFile()) {
					kind = u"video"_q;
				} else if (doc->isAnimation()) {
					kind = u"gif"_q;
				} else if (doc->sticker()) {
					kind = u"sticker"_q;
				} else {
					kind = u"document"_q;
				}
			} else if (media->poll()) {
				kind = u"poll"_q;
			}
			addRow(u"Media"_q, kind);
		}

		box->addButton(tr::lng_close(), [=] { box->closeBox(); });
		box->addLeftButton(rpl::single(u"Copy ID"_q), [=] {
			QGuiApplication::clipboard()->setText(QString::number(item->fullId().msg.bare));
		});
	}));
}

} // namespace NagramX
