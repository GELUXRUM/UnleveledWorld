#include <cctype>
#include "hooks.h"
#include "SimpleIni.h"

CSimpleIniA ini(true, false, false);
bool unlevelItems;
bool unlevelCharacters;
bool unlevelInjections;  // used to unlevel script-added things after the save is loaded

bool GetOptionValueBool(const char* a_optionValue)
{
	std::string lowercaseStr;

	size_t length = std::strlen(a_optionValue);

	for (size_t i = 0; i < length; ++i) {
		lowercaseStr += static_cast<char>(std::tolower(static_cast<unsigned char>(a_optionValue[i])));
	}

	if (lowercaseStr == "true") {
		return true;
	} else {
		return false;
	}
}

void LoadConfigs()
{
	ini.LoadFile("Data\\F4SE\\Plugins\\GLXRM_UnleveledWorld.ini");

	unlevelItems = GetOptionValueBool(ini.GetValue("General", "UnlevelItems", "false"));
	unlevelCharacters = GetOptionValueBool(ini.GetValue("General", "UnlevelCharacters", "false"));
	unlevelInjections = GetOptionValueBool(ini.GetValue("General", "UnlevelInjections", "false"));

	ini.Reset();
}

void UnlevelStuff()
{
	if (auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		if (unlevelItems) {
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

		if (unlevelCharacters) {
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

					if (obj) {
						obj->level = 1;
					}
				}
			}
		} else {
			logger::warn("Ignoring character lists");
		}
	}
}

void ListenerThing(F4SE::MessagingInterface::Message* a_thing)
{
	if (a_thing->type == F4SE::MessagingInterface::kGameDataReady) {
		logger::info("Game data finished loading. Beginning unleveling...");
		UnlevelStuff();
	}
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

	// hook into the function that injects stuff to LLs to force level to 1
	if (unlevelInjections) {
		auto& trampoline = F4SE::GetTrampoline();
		trampoline.create(20);
		HookLineAndSinker::RegisterHook(trampoline);
	}

	F4SE::GetMessagingInterface()->RegisterListener(ListenerThing);

	return true;
}
