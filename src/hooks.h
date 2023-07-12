#pragma once

namespace HookLineAndSinker
{
	int8_t HookedAddLeveledObject(RE::TESLeveledList* a_leveledList, uint16_t a_level, int16_t a_count, int8_t a_chanceNone, RE::TESForm* a_item, RE::ContainerItemExtra* a_containerExta);

	void RegisterHook(F4SE::Trampoline& trampoline);
}
