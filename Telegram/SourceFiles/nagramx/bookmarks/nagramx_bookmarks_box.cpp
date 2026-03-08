/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "nagramx/bookmarks/nagramx_bookmarks_box.h"

#include "ayu/data/ayu_database.h"
#include "ayu/data/entities.h"
#include "data/data_peer_id.h"
#include "lang/lang_keys.h"
#include "ui/layers/generic_box.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_section_common.h"
#include "window/window_session_controller.h"
#include "styles/style_layers.h"
#include "styles/style_settings.h"

namespace NagramX {

void ShowBookmarksBox(not_null<Window::SessionController*> controller) {
	controller->show(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(tr::lng_nagramx_bookmarks_title());

		const auto content = box->verticalLayout();
		const auto entries = AyuDatabase::getBookmarks();

		if (entries.empty()) {
			content->add(
				object_ptr<Ui::FlatLabel>(content, tr::lng_nagramx_bookmarks_empty(), st::defaultFlatLabel),
				st::boxRowPadding);
		} else {
			for (const auto &entry : entries) {
				const auto peerName = QString::fromStdString(entry.peerName);
				const auto text = QString::fromStdString(entry.text);
				auto preview = peerName + u": "_q + text.left(80);
				const auto button = content->add(
					object_ptr<Ui::SettingsButton>(content, rpl::single(preview), st::settingsButton));
				const auto peerId = entry.peerId;
				const auto msgId = entry.messageId;
				button->addClickHandler([=] {
					box->closeBox();
					controller->showPeerHistory(
						PeerId(peerId),
						Window::SectionShow::Way::Forward,
						MsgId(msgId));
				});
			}
		}

		box->addButton(tr::lng_close(), [=] { box->closeBox(); });
	}));
}

} // namespace NagramX
