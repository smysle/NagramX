// NagramX Settings for AyuGram Desktop
#include "settings_nagramx.h"

#include "lang_auto.h"
#include "ayu/ayu_settings.h"
#include "ayu/ui/settings/settings_ayu_utils.h"
#include "nagramx/bookmarks/nagramx_bookmarks_box.h"
#include "nagramx/llm/nagramx_llm_provider_presets.h"
#include "settings/settings_common.h"
#include "ui/layers/generic_box.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"

namespace Settings {
namespace {

[[nodiscard]] QString LabelOrDefault(
		const QString &value,
		const QString &fallback) {
	return value.trimmed().isEmpty() ? fallback : value.trimmed();
}

[[nodiscard]] QString ApiKeyLabel(const QString &value) {
	return value.trimmed().isEmpty() ? u"Not set"_q : u"Configured"_q;
}

[[nodiscard]] QString NxTranslationProviderName(int id) {
	switch (id) {
	case 0: return u"Auto (AyuGram)"_q;
	case 1: return u"DeepL"_q;
	case 2: return u"Microsoft"_q;
	case 3: return u"Lingo (Caiyun)"_q;
	case 4: return u"TranSmart (Tencent)"_q;
	case 5: return u"LLM"_q;
	}
	return u"Auto (AyuGram)"_q;
}

void ShowTextSettingBox(
		not_null<Window::SessionController*> controller,
		QString title,
		QString placeholder,
		QString current,
		Fn<void(QString)> save) {
	controller->show(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(title));

		const auto skip = st::defaultSubsectionTitlePadding.top();
		box->addRow(
			object_ptr<Ui::FlatLabel>(
				box,
				rpl::single(placeholder),
				st::defaultSubsectionTitle),
			st::boxRowPadding + style::margins(0, skip, 0, 0));

		const auto field = box->addRow(
			object_ptr<Ui::InputField>(
				box,
				st::settingsDeviceName,
				rpl::single(placeholder),
				current),
			st::boxRowPadding - style::margins(
				st::settingsDeviceName.textMargins.left(),
				0,
				st::settingsDeviceName.textMargins.right(),
				0));

		field->setMaxLength(4096);
		box->setFocusCallback([=] {
			field->setFocusFast();
		});

		const auto submit = [=] {
			box->closeBox();
			save(field->getLastText().trimmed());
		};

		field->submits() | rpl::on_next(submit, field->lifetime());
		box->addButton(tr::lng_settings_save(), submit);
		box->addButton(tr::lng_cancel(), [=] {
			box->closeBox();
		});
	}));
}

void ShowPresetPickerBox(
		not_null<Window::SessionController*> controller) {
	controller->show(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(u"LLM Provider Preset"_q));
		const auto content = box->verticalLayout();

		for (auto i = 0; i < NagramX::kLlmPresetCount; ++i) {
			const auto info = NagramX::LlmPresetInfoFor(i);
			const auto presetId = i;
			const auto button = content->add(
				object_ptr<Ui::SettingsButton>(
					content,
					rpl::single(info.name),
					st::settingsButton));
			button->addClickHandler([=] {
				AyuSettings::set_nagramxLlmProviderPreset(presetId);
				AyuSettings::set_nagramxLlmProvider(info.name);
				if (!info.defaultUrl.isEmpty()) {
					AyuSettings::set_nagramxLlmApiUrl(info.defaultUrl);
				}
				if (!info.defaultModel.isEmpty()) {
					AyuSettings::set_nagramxLlmModel(info.defaultModel);
				}
				AyuSettings::save();
				box->closeBox();
			});
		}

		box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });
	}));
}

void ShowNxTranslationProviderBox(
		not_null<Window::SessionController*> controller) {
	controller->show(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(u"NagramX Translation Provider"_q));
		const auto content = box->verticalLayout();

		for (auto i = 0; i <= 5; ++i) {
			const auto providerId = i;
			const auto button = content->add(
				object_ptr<Ui::SettingsButton>(
					content,
					rpl::single(NxTranslationProviderName(i)),
					st::settingsButton));
			button->addClickHandler([=] {
				AyuSettings::set_nagramxTranslationProvider(providerId);
				AyuSettings::save();
				box->closeBox();
			});
		}

		box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });
	}));
}

} // namespace

NagramXSettings::NagramXSettings(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent) {
	setupContent(controller);
}

rpl::producer<QString> NagramXSettings::title() {
	return rpl::single(u"NagramX"_q);
}

void NagramXSettings::setupContent(not_null<Window::SessionController*> controller) {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	auto &settings = AyuSettings::getInstance();

	AddSkip(content);
	AddDividerText(
		content,
		rpl::single(u"NagramX settings for this desktop fork."_q));

	// --- Live Features ---
	AddSubsectionTitle(content, rpl::single(u"Live Features"_q));

	const auto addToggle = [&](const QString &label, bool checked, auto &&save) {
		const auto checkbox = content->add(
			object_ptr<Ui::Checkbox>(content, label, checked, st::settingsCheckbox),
			st::settingsCheckboxPadding);
		checkbox->checkedChanges() | rpl::on_next(
			save,
			content->lifetime());
		return checkbox;
	};

	addToggle(
		u"Always confirm external links"_q,
		settings.nagramxConfirmExternalLinks,
		[](bool v) { AyuSettings::set_nagramxConfirmExternalLinks(v); AyuSettings::save(); });

	addToggle(
		u"Hide stories row in dialogs list"_q,
		settings.nagramxHideStories,
		[](bool v) { AyuSettings::set_nagramxHideStories(v); AyuSettings::save(); });

	addToggle(
		u"Always show downloads entry"_q,
		settings.nagramxAlwaysShowDownloadIcon,
		[](bool v) { AyuSettings::set_nagramxAlwaysShowDownloadIcon(v); AyuSettings::save(); });

	// --- Context Menu ---
	AddSubsectionTitle(content, rpl::single(u"Context Menu"_q));

	addToggle(
		u"Show 'Add to Bookmarks'"_q,
		settings.nagramxShowBookmarkAction,
		[](bool v) { AyuSettings::set_nagramxShowBookmarkAction(v); AyuSettings::save(); });

	addToggle(
		u"Forward without quote"_q,
		settings.nagramxNoQuoteForward,
		[](bool v) { AyuSettings::set_nagramxNoQuoteForward(v); AyuSettings::save(); });

	addToggle(
		u"Repeat as copy"_q,
		settings.nagramxRepeatAsCopy,
		[](bool v) { AyuSettings::set_nagramxRepeatAsCopy(v); AyuSettings::save(); });

	addToggle(
		u"High quality photos by default"_q,
		settings.nagramxSendHighQualityPhoto,
		[](bool v) { AyuSettings::set_nagramxSendHighQualityPhoto(v); AyuSettings::save(); });

	// --- Bookmarks ---
	AddSubsectionTitle(content, rpl::single(u"Bookmarks"_q));

	const auto viewBookmarks = AddButtonWithLabel(
		content,
		rpl::single(u"View Bookmarks"_q),
		rpl::single(QString()),
		st::settingsButton,
		{ &st::menuIconFave });
	viewBookmarks->addClickHandler([=] {
		NagramX::ShowBookmarksBox(controller);
	});

	// --- Branding ---
	AddSubsectionTitle(content, rpl::single(u"Branding"_q));

	const auto customTitle = AddButtonWithLabel(
		content,
		rpl::single(u"Custom title"_q),
		rpl::single(LabelOrDefault(settings.nagramxCustomTitle, u"NagramX"_q)),
		st::settingsButton,
		{ &st::menuIconPalette });
	customTitle->addClickHandler([=] {
		ShowTextSettingBox(
			controller,
			u"NagramX custom title"_q,
			u"Title"_q,
			AyuSettings::getInstance().nagramxCustomTitle,
			[](const QString &value) {
				AyuSettings::set_nagramxCustomTitle(value);
				AyuSettings::save();
			});
	});

	// --- Translation ---
	AddSubsectionTitle(content, rpl::single(u"NagramX Translation"_q));

	const auto translationProvider = AddButtonWithLabel(
		content,
		rpl::single(u"NagramX translation provider"_q),
		rpl::single(NxTranslationProviderName(settings.nagramxTranslationProvider)),
		st::settingsButton,
		{ &st::menuIconTranslate });
	translationProvider->addClickHandler([=] {
		ShowNxTranslationProviderBox(controller);
	});

	const auto deeplKey = AddButtonWithLabel(
		content,
		rpl::single(u"DeepL API key"_q),
		rpl::single(ApiKeyLabel(settings.nagramxDeepLApiKey)),
		st::settingsButton,
		{ &st::menuIconLock });
	deeplKey->addClickHandler([=] {
		ShowTextSettingBox(
			controller,
			u"DeepL API Key"_q,
			u"API Key"_q,
			AyuSettings::getInstance().nagramxDeepLApiKey,
			[](const QString &value) {
				AyuSettings::set_nagramxDeepLApiKey(value);
				AyuSettings::save();
			});
	});

	const auto msKey = AddButtonWithLabel(
		content,
		rpl::single(u"Microsoft Translate key"_q),
		rpl::single(ApiKeyLabel(settings.nagramxMicrosoftTranslateKey)),
		st::settingsButton,
		{ &st::menuIconLock });
	msKey->addClickHandler([=] {
		ShowTextSettingBox(
			controller,
			u"Microsoft Translate Key"_q,
			u"API Key"_q,
			AyuSettings::getInstance().nagramxMicrosoftTranslateKey,
			[](const QString &value) {
				AyuSettings::set_nagramxMicrosoftTranslateKey(value);
				AyuSettings::save();
			});
	});

	const auto lingoKey = AddButtonWithLabel(
		content,
		rpl::single(u"Lingo (Caiyun) token"_q),
		rpl::single(ApiKeyLabel(settings.nagramxLingoApiToken)),
		st::settingsButton,
		{ &st::menuIconLock });
	lingoKey->addClickHandler([=] {
		ShowTextSettingBox(
			controller,
			u"Lingo API Token"_q,
			u"API Token"_q,
			AyuSettings::getInstance().nagramxLingoApiToken,
			[](const QString &value) {
				AyuSettings::set_nagramxLingoApiToken(value);
				AyuSettings::save();
			});
	});

	AddDividerText(
		content,
		rpl::single(u"TranSmart (Tencent) does not require an API key."_q));

	// --- LLM Settings ---
	AddSubsectionTitle(content, rpl::single(u"LLM Settings"_q));

	const auto presetButton = AddButtonWithLabel(
		content,
		rpl::single(u"Provider preset"_q),
		rpl::single(LabelOrDefault(settings.nagramxLlmProvider, u"Custom"_q)),
		st::settingsButton,
		{ &st::menuIconTranslate });
	presetButton->addClickHandler([=] {
		ShowPresetPickerBox(controller);
	});

	const auto apiUrl = AddButtonWithLabel(
		content,
		rpl::single(u"API URL"_q),
		rpl::single(LabelOrDefault(settings.nagramxLlmApiUrl, u"https://api.openai.com/v1"_q)),
		st::settingsButton,
		{ &st::menuIconManage });
	apiUrl->addClickHandler([=] {
		ShowTextSettingBox(
			controller,
			u"NagramX LLM API URL"_q,
			u"API URL"_q,
			AyuSettings::getInstance().nagramxLlmApiUrl,
			[](const QString &value) {
				AyuSettings::set_nagramxLlmApiUrl(value);
				AyuSettings::save();
			});
	});

	const auto model = AddButtonWithLabel(
		content,
		rpl::single(u"Model"_q),
		rpl::single(LabelOrDefault(settings.nagramxLlmModel, u"gpt-4.1-mini"_q)),
		st::settingsButton,
		{ &st::menuIconChatBubble });
	model->addClickHandler([=] {
		ShowTextSettingBox(
			controller,
			u"NagramX LLM model"_q,
			u"Model"_q,
			AyuSettings::getInstance().nagramxLlmModel,
			[](const QString &value) {
				AyuSettings::set_nagramxLlmModel(value);
				AyuSettings::save();
			});
	});

	const auto apiKey = AddButtonWithLabel(
		content,
		rpl::single(u"API Key"_q),
		rpl::single(ApiKeyLabel(settings.nagramxLlmApiKey)),
		st::settingsButton,
		{ &st::menuIconLock });
	apiKey->addClickHandler([=] {
		ShowTextSettingBox(
			controller,
			u"NagramX LLM API key"_q,
			u"API Key"_q,
			AyuSettings::getInstance().nagramxLlmApiKey,
			[](const QString &value) {
				AyuSettings::set_nagramxLlmApiKey(value);
				AyuSettings::save();
			});
	});

	const auto systemPrompt = AddButtonWithLabel(
		content,
		rpl::single(u"System prompt"_q),
		rpl::single(LabelOrDefault(settings.nagramxLlmSystemPrompt, u"Default"_q)),
		st::settingsButton,
		{ &st::menuIconChatBubble });
	systemPrompt->addClickHandler([=] {
		ShowTextSettingBox(
			controller,
			u"LLM System Prompt"_q,
			u"System prompt"_q,
			AyuSettings::getInstance().nagramxLlmSystemPrompt,
			[](const QString &value) {
				AyuSettings::set_nagramxLlmSystemPrompt(value);
				AyuSettings::save();
			});
	});

	addToggle(
		u"Use LLM for translation"_q,
		settings.nagramxTranslateWithLlm,
		[](bool v) { AyuSettings::set_nagramxTranslateWithLlm(v); AyuSettings::save(); });

	AddDividerText(
		content,
		rpl::single(u"All LLM providers use the OpenAI-compatible API format."_q));

	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
