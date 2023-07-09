#include "SimpleIni.h"
#include <cctype>

CSimpleIniA ini(true, false, false);
bool unscaleItems{ false };
bool unscaleCharacters{ false };

void GameDataListener(F4SE::MessagingInterface::Message* thing)
{
	if (thing->type == F4SE::MessagingInterface::kGameDataReady) {
		if (auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
			if (unscaleItems) {
				logger::warn("Unleveling item lists");
				for (auto currentItemList : dataHandler->GetFormArray<RE::TESLevItem>()) {
					std::int32_t total = (currentItemList->baseListCount + currentItemList->scriptListCount);

					for (int i = 0; i < total; i++) {
						RE::LEVELED_OBJECT* obj;
						if (i < currentItemList->baseListCount) {
							obj = &currentItemList->leveledLists[i];
						} else {
							obj = currentItemList->scriptAddedLists[i - (currentItemList->baseListCount - 1)];
						}

						obj->level = 1;
					}
				}
			} else {
				logger::warn("Ignoring item lists");
			}

			if (unscaleCharacters) {
				logger::warn("Unleveling character lists");
				for (auto currentCharList : dataHandler->GetFormArray<RE::TESLevCharacter>()) {
					std::int32_t total = (currentCharList->baseListCount + currentCharList->scriptListCount);

					for (int i = 0; i < total; i++) {
						RE::LEVELED_OBJECT* obj;
						if (i < currentCharList->baseListCount) {
							obj = &currentCharList->leveledLists[i];
						} else {
							obj = currentCharList->scriptAddedLists[i - (currentCharList->baseListCount - 1)];
						}

						obj->level = 1;
					}
				}
			} else {
				logger::warn("Ignoring character lists");
			}
		}
	}

	return;
}

std::string toLowercase(const char* str)
{
	std::string lowercaseStr;
	size_t length = std::strlen(str);
	for (size_t i = 0; i < length; ++i) {
		lowercaseStr += static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
	}
	return lowercaseStr;
}

void LoadConfigs()
{
	ini.LoadFile("Data\\F4SE\\Plugins\\GLXRM_UnleveledWorld.ini");

	auto itemValue = ini.GetValue("General", "UnscaleItems", "true");
	std::string itemAnswer = toLowercase(itemValue);

	if (itemAnswer == "true") {
		unscaleItems = true;
	} else if (itemAnswer == "false") {
		unscaleItems = false;
	} else {
		logger::warn("Invalid value passed to UnscaleItems. Defaulting to false");
	}

	auto characterValue = ini.GetValue("General", "UnscaleCharacters", "true");
	std::string characterAnswer = toLowercase(characterValue);

	if (characterAnswer == "true") {
		unscaleCharacters = true;
	} else if (characterAnswer == "false") {
		unscaleCharacters = false;
	} else {
		logger::warn("Invalid value passed to UnscaleCharacters. Defaulting to false");
	}

	ini.Reset();
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::warn);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = "GLXRM_UnleveledWorld";
	a_info->version = 69;

	if (a_f4se->IsEditor()) {
		logger::critical("loaded in editor");
		return false;
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_162) {
		logger::critical(FMT_STRING("unsupported runtime v{}"), ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);
	
	LoadConfigs();

	F4SE::GetMessagingInterface()->RegisterListener(GameDataListener);

	return true;
}
