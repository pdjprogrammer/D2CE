/*
    Diablo II Character Editor
    Copyright (C) 2021 Walter Couto

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------

#pragma once

#include "CharacterStatsConstants.h"
#include "SkillConstants.h"
#include "DataTypes.h"
#include "sstream"

namespace d2ce
{
    //---------------------------------------------------------------------------
    class CharacterStats
    {
        friend class Character;

    private:
        mutable std::vector<std::uint8_t> data;
        std::uint32_t stats_location = 0,
            skills_location = 0;
        bool update_locations = true;

        EnumCharVersion Version = APP_CHAR_VERSION;  // Version for Character file
        EnumCharClass Class = EnumCharClass::Amazon; // Class for Character file

        /*
           The next short field is for character file version prior to 1.09.
           It is bit field that indicates the presence or absence of each particular statistic.
           If the bit is 0, the corresponding statistic is zero and not stored in the file.
           If the bit is 1, the statistic has a long value stored in the file.
           Note that many of the statistics are never zero, so they will always be present; but there is a bit assigned to them nonetheless.

           The bits are assigned as follows:
           LSB:  0 Strength
                 1 Energy
                 2 Dexterity
                 3 Vitality
                 4 Stat Points Remaining
                 5 Skill Choices Remaining
                 6 Life (current)
                 7 Life (max)
                 8 Mana (current)
                 9 Mana (max)
                 10 Stamina (current)
                 11 Stamina (max)
                 12 Level
                 13 Experience
                 14 Gold in Inventory
           MSB:  15 Gold in Stash
        */
        bitmask::bitmask<EnumCharStatInfo> StatInfo = EnumCharStatInfo::All; // pos 562 (pre-1.09)

        std::uint8_t nullByte = 0;

        std::uint32_t min_hit_points = 0x100, min_stamina = 0x100, min_mana = 0x100;
        void updateMinStats();

        std::uint32_t min_vitality = 1, min_energy = 1, min_dexterity = 1, min_strength = 1;
        void updateStartStats();
        
        void updateSkillChoices(std::uint16_t skillPointsEarned);

        void updateLifePointsEarned(std::uint16_t lifePointsEarned);
        void updatePointsEarned(std::uint16_t lifePointsEarned, std::uint16_t statPointEarned, std::uint16_t skillPointsEarne);

        void checkStatInfo();
        EnumCharStatInfo GetStatInfoMask(std::uint16_t stat) const;
        std::uint32_t* GetStatBuffer(std::uint16_t stat);

        std::uint64_t readBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits);
        bool skipBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits);
        size_t readNextStat(std::FILE* charfile, size_t& current_bit_offset, std::uint16_t& stat);
        size_t readStatBits(std::FILE* charfile, size_t& current_bit_offset, std::uint16_t stat);
        bool readStats(std::FILE* charfile);
        bool readStats_109(std::FILE* charfile);
        bool readSkills(std::FILE* charfile);

        size_t updateBits(size_t& current_bit_offset, size_t size, std::uint32_t value);
        size_t updateStat(std::FILE* charfile, size_t& current_bit_offset, std::uint16_t stat);
        size_t updateStatBits(size_t& current_bit_offset, std::uint16_t stat);
        size_t writeBufferBits(std::FILE* charfile);
        bool writeStats_109(std::FILE* charfile);
        bool writeSkills(std::FILE* charfile);

    protected:
        CharStats Cs;                 // Strength:  pos 565 (pre-1.09)
                                      // Energy:    pos 569 (pre-1.09)
                                      // Dexterity: pos 573 (pre-1.09)
                                      // Vitality:  pos 577 (pre-1.09)

        std::uint8_t Skills[NUM_OF_SKILLS] = { 0 };

    protected:
        bool readStats(EnumCharVersion version, EnumCharClass charClass, std::FILE* charfile);
        bool writeStats(std::FILE* charfile);
        std::uint32_t getHeaderLocation();

        void resetStats(std::uint16_t lifePointsEarned, std::uint16_t statPointEarned, std::uint16_t skillPointsEarned);
        void updateSkills(const std::uint8_t(&updated_skills)[NUM_OF_SKILLS], std::uint16_t skillPointsEarned);
        void resetSkills(std::uint16_t skillPointsEarned);

        void updateClass(EnumCharClass charClass);

        std::string getAttributeJsonName(std::uint16_t stat) const;
        void attributesAsJson(std::stringstream& ss, const std::string& parentIndent) const;
        void skillsAsJson(std::stringstream& ss, const std::string& parentIndent) const;
        void asJson(std::stringstream& ss, const std::string& parentIndent) const;

    public:
        CharacterStats();
        ~CharacterStats();

        void clear();

        void fillCharacterStats(CharStats& cs);
        void updateCharacterStats(const CharStats& cs);

        std::uint32_t getLevel() const;
        std::uint32_t getExperience() const;
        std::uint32_t getMaxGoldInBelt() const;
        std::uint32_t getMaxGoldInStash() const;
        std::uint32_t getMinExperienceRequired() const;
        std::uint32_t getNextExperienceLevel() const;
        std::uint32_t getMinStrength() const;
        std::uint32_t getMinEnergy() const;
        std::uint32_t getMinDexterity() const;
        std::uint32_t getMinVitality() const;
        std::uint32_t getMaxHitPoints() const;
        std::uint32_t getMaxStamina() const;
        std::uint32_t getMaxMana() const;

        std::uint32_t getTotalStartStatPoints() const;
        std::uint32_t getTotalStatPoints() const;
        std::uint32_t getStatPointsUsed() const;
        std::uint32_t getStatLeft() const;

        // Skills
        std::uint8_t getSkillId(std::uint32_t skill) const;
        std::string getSkillNameById(std::uint32_t id) const;
        std::string getSkillName(std::uint32_t skill) const;
        std::uint8_t getSkillPoints(std::uint32_t skill) const;
        std::uint8_t(&getSkills())[NUM_OF_SKILLS];

        std::uint32_t getTotalSkillPoints() const;
        std::uint32_t getSkillPointsUsed() const;
        std::uint32_t getSkillChoices() const;

        bool areSkillsMaxed() const;
        void maxSkills();
        void clearSkillChoices();
    };
    //---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------

