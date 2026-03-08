/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "nagramx/bookmarks/nagramx_bookmarks.h"

#include "ayu/data/ayu_database.h"
#include "ayu/data/entities.h"
#include "base/unixtime.h"
#include "data/data_document.h"
#include "data/data_peer.h"
#include "history/history.h"
#include "history/history_item.h"

namespace NagramX {
namespace {

[[nodiscard]] std::string MediaKind(not_null<HistoryItem*> item) {
	if (item->isService()) {
		return "service";
	}
	if (const auto media = item->media()) {
		if (media->photo()) {
			return "photo";
		}
		if (const auto document = media->document()) {
			if (document->isVoiceMessage()) {
				return "voice";
			} else if (document->isVideoFile()) {
				return "video";
			} else if (document->isAnimation()) {
				return "gif";
			} else if (document->sticker()) {
				return "sticker";
			}
			return "document";
		}
		if (media->poll()) {
			return "poll";
		}
		return "media";
	}
	return "text";
}

} // namespace

void AddBookmark(not_null<HistoryItem*> item) {
	const auto peer = item->history()->peer;
	auto bookmark = NagramxBookmark();
	bookmark.peerId = peer->id.value;
	bookmark.messageId = item->fullId().msg.bare;
	bookmark.peerName = peer->shortName().toStdString();
	bookmark.text = item->originalText().text.toStdString();
	bookmark.mediaKind = MediaKind(item);
	bookmark.date = item->date();
	bookmark.bookmarkedAt = base::unixtime::now();
	AyuDatabase::addBookmark(bookmark);
}

void RemoveBookmark(long long peerId, int messageId) {
	AyuDatabase::removeBookmark(peerId, messageId);
}

bool IsBookmarked(long long peerId, int messageId) {
	return AyuDatabase::isBookmarked(peerId, messageId);
}

} // namespace NagramX
