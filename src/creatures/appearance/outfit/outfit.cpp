/**
 * The Forgotten Server - a free and open-source MMORPG server emulator
 * Copyright (C) 2019  Mark Samman <mark.samman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "otpch.h"

#include "creatures/appearance/outfit/outfit.h"

#include "utils/tools.h"
#include "game/game.h"

#include <cctype>

bool Outfits::loadFromXml()
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("data/XML/outfits.xml");
	if (!result) {
		printXMLError("[Outfits::loadFromXml] - ", "data/XML/outfits.xml", result);
		return false;
	}

	for (auto outfitNode : doc.child("outfits").children()) {
		pugi::xml_attribute attr;
		if ((attr = outfitNode.attribute("enabled")) && !attr.as_bool()) {
			continue;
		}

		if (!(attr = outfitNode.attribute("type"))) {
			SPDLOG_WARN("[Outfits::loadFromXml] - Missing outfit type");
			continue;
		}

		auto type = static_cast<uint8_t>(outfitNode.attribute("type").as_uint());
		if (type > PLAYERSEX_LAST) {
			SPDLOG_WARN("[Outfits::loadFromXml] - Invalid outfit type {}", type);
			continue;
		}

		pugi::xml_attribute lookTypeAttribute = outfitNode.attribute("looktype");
		auto lookType = static_cast<uint16_t>(lookTypeAttribute.as_uint());
		const std::string outfitName = outfitNode.attribute("name").as_string();
		if (!lookTypeAttribute.empty()) {
			const std::string lookTypeString = lookTypeAttribute.as_string();
			if (lookTypeString.empty() || lookType == 0) {
				SPDLOG_WARN("[Outfits::loadFromXml] - Empty looktype on outfit with name {}", outfitName);
				continue;
			}

			if (!isNumber(lookTypeString)) {
				SPDLOG_WARN("[Outfits::loadFromXml] - Invalid looktype {} with name {}", lookTypeString, outfitName);
				continue;
			}

			if (pugi::xml_attribute nameAttribute = outfitNode.attribute("name");
			!nameAttribute || outfitName.empty())
			{
				SPDLOG_WARN("[Outfits::loadFromXml] - Missing or empty name on outfit with looktype {}", lookTypeString);
				continue;
			}
		} else {
			SPDLOG_WARN("[Outfits::loadFromXml] - "
						"Missing looktype id for outfit name: {}", outfitName);
		}

		if (uint16_t lookType = static_cast<uint16_t>(lookTypeAttribute.as_uint());
				g_configManager().getBoolean(WARN_UNSAFE_SCRIPTS) && lookType != 0
				&& !g_game().isLookTypeRegistered(lookType)
			)
		{
			SPDLOG_WARN("[Outfits::loadFromXml] An unregistered creature looktype type with id '{}' was blocked to prevent client crash.", lookType);
			return false;
		}

		outfits[type].emplace_back(
			outfitName,
			lookType,
			outfitNode.attribute("premium").as_bool(),
			outfitNode.attribute("unlocked").as_bool(true),
			outfitNode.attribute("from").as_string()
		);
	}
	for (uint8_t sex = PLAYERSEX_FEMALE; sex <= PLAYERSEX_LAST; ++sex) {
		outfits[sex].shrink_to_fit();
	}
	return true;
}

const Outfit* Outfits::getOutfitByLookType(PlayerSex_t sex, uint16_t lookType) const
{
	for (const Outfit& outfit : outfits[sex]) {
		if (outfit.lookType == lookType) {
			return &outfit;
		}
	}
	return nullptr;
}

/**
 * Get the oposite sex equivalent outfit
 * @param sex current sex
 * @param lookType current looktype
 * @return <b>const</b> pointer to the outfit or <b>nullptr</b> if it could not be found.
 */

const Outfit *Outfits::getOpositeSexOutfitByLookType(PlayerSex_t sex, uint16_t lookType)
{
	PlayerSex_t	searchSex = (sex == PLAYERSEX_MALE)?PLAYERSEX_FEMALE:PLAYERSEX_MALE;

	for(uint16_t i=0; i< outfits[sex].size(); i++) {
		if (outfits[sex].at(i).lookType == lookType) {
			if (outfits[searchSex].size()>i) {
				return &outfits[searchSex].at(i);
			} else { //looktype found but the oposite sex array doesn't have this index.
				return nullptr;
			}
		}
	}
	return nullptr;
}
