/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <platform/game_db.hpp>

namespace nba {

/*
 * Adapted from VisualBoyAdvance-M's vba-over.ini:
 * https://github.com/visualboyadvance-m/visualboyadvance-m/blob/master/src/vba-over.ini
 *
 * TODO: it is unclear how accurate the EEPROM sizes are.
 * Since VBA guesses EEPROM sizes, the vba-over.ini did not contain the sizes.
 */
const std::map<std::string, GameInfo> g_game_db {
  { "ALFP", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Dragon Ball Z - The Legacy of Goku II (Europe)(En,Fr,De,Es,It) */
  { "ALGP", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Dragon Ball Z - The Legacy of Goku (Europe)(En,Fr,De,Es,It) */
  { "AROP", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Rocky (Europe)(En,Fr,De,Es,It) */
  { "AR8e", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Rocky (USA)(En,Fr,De,Es,It) */
  { "AXVE", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Ruby Version (USA, Europe) */
  { "AXPE", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Sapphire Version (USA, Europe) */
  { "AX4P", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Super Mario Advance 4 - Super Mario Bros. 3 (Europe)(En,Fr,De,Es,It) */
  { "A2YE", { Config::BackupType::None, GPIODeviceType::None, false } },      /* Top Gun - Combat Zones (USA)(En,Fr,De,Es,It) */
  { "BDBP", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Dragon Ball Z - Taiketsu (Europe)(En,Fr,De,Es,It) */
  { "BM5P", { Config::BackupType::FLASH_64, GPIODeviceType::None, false } },  /* Mario vs. Donkey Kong (Europe) */
  { "BPEE", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Emerald Version (USA, Europe) */
  { "BY6P", { Config::BackupType::SRAM, GPIODeviceType::None, false } },      /* Yu-Gi-Oh! - Ultimate Masters - World Championship Tournament 2006 (Europe)(En,Jp,Fr,De,Es,It) */
  { "B24E", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pokemon Mystery Dungeon - Red Rescue Team (USA, Australia) */
  { "FADE", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Classic NES Series - Castlevania (USA, Europe) */
  { "FBME", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Classic NES Series - Bomberman (USA, Europe) */
  { "FDKE", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Classic NES Series - Donkey Kong (USA, Europe) */
  { "FDME", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Classic NES Series - Dr. Mario (USA, Europe) */
  { "FEBE", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Classic NES Series - Excitebike (USA, Europe) */
  { "FICE", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Classic NES Series - Ice Climber (USA, Europe) */
  { "FLBE", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Classic NES Series - Zelda II - The Adventure of Link (USA, Europe) */
  { "FMRE", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Classic NES Series - Metroid (USA, Europe) */
  { "FP7E", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Classic NES Series - Pac-Man (USA, Europe) */
  { "FSME", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Classic NES Series - Super Mario Bros. (USA, Europe) */
  { "FXVE", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Classic NES Series - Xevious (USA, Europe) */
  { "FZLE", { Config::BackupType::EEPROM_64, GPIODeviceType::None, true } },   /* Classic NES Series - Legend of Zelda (USA, Europe) */
  { "KYGP", { Config::BackupType::EEPROM_64/*_SENSOR*/, GPIODeviceType::None, false } }, /* Yoshi's Universal Gravitation (Europe)(En,Fr,De,Es,It) */
  { "U3IP", { Config::BackupType::Detect, GPIODeviceType::RTC, false } },     /* Boktai - The Sun Is in Your Hand (Europe)(En,Fr,De,Es,It) */
  { "U32P", { Config::BackupType::Detect, GPIODeviceType::RTC, false } },     /* Boktai 2 - Solar Boy Django (Europe)(En,Fr,De,Es,It) */
  { "AGFE", { Config::BackupType::FLASH_64, GPIODeviceType::RTC, false } },   /* Golden Sun - The Lost Age (USA) */
  { "AGSE", { Config::BackupType::FLASH_64, GPIODeviceType::RTC, false } },   /* Golden Sun (USA) */
  { "ALFE", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Dragon Ball Z - The Legacy of Goku II (USA) */
  { "ALGE", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Dragon Ball Z - The Legacy of Goku (USA) */
  { "AX4E", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Super Mario Advance 4 - Super Mario Bros 3 - Super Mario Advance 4 v1.1 (USA) */
  { "BDBE", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Dragon Ball Z - Taiketsu (USA) */
  { "BG3E", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Dragon Ball Z - Buu's Fury (USA) */
  { "BLFE", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* 2 Games in 1 - Dragon Ball Z - The Legacy of Goku I & II (USA) */
  { "BPRE", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pokemon - Fire Red Version (USA, Europe) */
  { "BPGE", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pokemon - Leaf Green Version (USA, Europe) */
  { "BT4E", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Dragon Ball GT - Transformation (USA) */
  { "BUFE", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* 2 Games in 1 - Dragon Ball Z - Buu's Fury + Dragon Ball GT - Transformation (USA) */
  { "BYGE", { Config::BackupType::SRAM, GPIODeviceType::None, false } },      /* Yu-Gi-Oh! GX - Duel Academy (USA) */
  { "KYGE", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Yoshi - Topsy-Turvy (USA) */
  { "PSAE", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* e-Reader (USA) */
  { "U3IE", { Config::BackupType::Detect, GPIODeviceType::RTC, false } },     /* Boktai - The Sun Is in Your Hand (USA) */
  { "U32E", { Config::BackupType::Detect, GPIODeviceType::RTC, false } },     /* Boktai 2 - Solar Boy Django (USA) */
  { "ALFJ", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Dragon Ball Z - The Legacy of Goku II International (Japan) */
  { "AXPJ", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pocket Monsters - Sapphire (Japan) */
  { "AXVJ", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pocket Monsters - Ruby (Japan) */
  { "AX4J", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Super Mario Advance 4 (Japan) */
  { "BFTJ", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* F-Zero - Climax (Japan) */
  { "BGWJ", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Game Boy Wars Advance 1+2 (Japan) */
  { "BKAJ", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Sennen Kazoku (Japan) */
  { "BPEJ", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pocket Monsters - Emerald (Japan) */
  { "BPGJ", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pocket Monsters - Leaf Green (Japan) */
  { "BPRJ", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pocket Monsters - Fire Red (Japan) */
  { "BDKJ", { Config::BackupType::EEPROM_64, GPIODeviceType::None, false } }, /* Digi Communication 2 - Datou! Black Gemagema Dan (Japan) */
  { "BR4J", { Config::BackupType::Detect, GPIODeviceType::RTC, false } },     /* Rockman EXE 4.5 - Real Operation (Japan) */
  { "FMBJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 01 - Super Mario Bros. (Japan) */
  { "FCLJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 12 - Clu Clu Land (Japan) */
  { "FBFJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 13 - Balloon Fight (Japan) */
  { "FWCJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 14 - Wrecking Crew (Japan) */
  { "FDMJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 15 - Dr. Mario (Japan) */
  { "FDDJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 16 - Dig Dug (Japan) */
  { "FTBJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 17 - Takahashi Meijin no Boukenjima (Japan) */
  { "FMKJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 18 - Makaimura (Japan) */
  { "FTWJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 19 - Twin Bee (Japan) */
  { "FGGJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 20 - Ganbare Goemon! Karakuri Douchuu (Japan) */
  { "FM2J", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 21 - Super Mario Bros. 2 (Japan) */
  { "FNMJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 22 - Nazo no Murasame Jou (Japan) */
  { "FMRJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 23 - Metroid (Japan) */
  { "FPTJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 24 - Hikari Shinwa - Palthena no Kagami (Japan) */
  { "FLBJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 25 - The Legend of Zelda 2 - Link no Bouken (Japan) */
  { "FFMJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 26 - Famicom Mukashi Banashi - Shin Onigashima - Zen Kou Hen (Japan) */
  { "FTKJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 27 - Famicom Tantei Club - Kieta Koukeisha - Zen Kou Hen (Japan) */
  { "FTUJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 28 - Famicom Tantei Club Part II - Ushiro ni Tatsu Shoujo - Zen Kou Hen (Japan) */
  { "FADJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 29 - Akumajou Dracula (Japan) */
  { "FSDJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, true } },   /* Famicom Mini Vol. 30 - SD Gundam World - Gachapon Senshi Scramble Wars (Japan) */
  { "KHPJ", { Config::BackupType::EEPROM_64/*_SENSOR*/, GPIODeviceType::None, false } }, /* Koro Koro Puzzle - Happy Panechu! (Japan) */
  { "KYGJ", { Config::BackupType::EEPROM_64/*_SENSOR*/, GPIODeviceType::None, false } }, /* Yoshi no Banyuuinryoku (Japan) */
  { "PSAJ", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Card e-Reader+ (Japan) */
  { "U3IJ", { Config::BackupType::Detect, GPIODeviceType::RTC, false } },     /* Bokura no Taiyou - Taiyou Action RPG (Japan) */
  { "U32J", { Config::BackupType::Detect, GPIODeviceType::RTC, false } },     /* Zoku Bokura no Taiyou - Taiyou Shounen Django (Japan) */
  { "U33J", { Config::BackupType::Detect, GPIODeviceType::RTC, false } },     /* Shin Bokura no Taiyou - Gyakushuu no Sabata (Japan) */
  { "AXPF", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Version Saphir (France) */
  { "AXVF", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Version Rubis (France) */
  { "BPEF", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Version Emeraude (France) */
  { "BPGF", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pokemon - Version Vert Feuille (France) */
  { "BPRF", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pokemon - Version Rouge Feu (France) */
  { "AXPI", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Versione Zaffiro (Italy) */
  { "AXVI", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Versione Rubino (Italy) */
  { "BPEI", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Versione Smeraldo (Italy) */
  { "BPGI", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pokemon - Versione Verde Foglia (Italy) */
  { "BPRI", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pokemon - Versione Rosso Fuoco (Italy) */
  { "AXPD", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Saphir-Edition (Germany) */
  { "AXVD", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Rubin-Edition (Germany) */
  { "BPED", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Smaragd-Edition (Germany) */
  { "BPGD", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pokemon - Blattgruene Edition (Germany) */
  { "BPRD", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pokemon - Feuerrote Edition (Germany) */
  { "AXPS", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Edicion Zafiro (Spain) */
  { "AXVS", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Edicion Rubi (Spain) */
  { "BPES", { Config::BackupType::FLASH_128, GPIODeviceType::RTC, false } },  /* Pokemon - Edicion Esmeralda (Spain) */
  { "BPGS", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }, /* Pokemon - Edicion Verde Hoja (Spain) */
  { "BPRS", { Config::BackupType::FLASH_128, GPIODeviceType::None, false } }  /* Pokemon - Edicion Rojo Fuego (Spain) */,
  { "A9DP", { Config::BackupType::EEPROM_4, GPIODeviceType::None, false } },  /* DOOM II */
  { "AAOJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, false } },  /* Acrobat Kid (Japan) */
  { "BGDP", { Config::BackupType::EEPROM_4, GPIODeviceType::None, false } },  /* Baldur's Gate - Dark Alliance (Europe) */
  { "BGDE", { Config::BackupType::EEPROM_4, GPIODeviceType::None, false } },  /* Baldur's Gate - Dark Alliance (USA) */
  { "BJBE", { Config::BackupType::EEPROM_4, GPIODeviceType::None, false } },  /* 007 - Everything or Nothing (USA, Europe) (En,Fr,De) */
  { "BJBJ", { Config::BackupType::EEPROM_4, GPIODeviceType::None, false } },  /* 007 - Everything or Nothing (Japan) */
  { "ALUP", { Config::BackupType::EEPROM_4, GPIODeviceType::None, false } },  /* 0937 - Super Monkey Ball Jr. (Europe) */
  { "ALUE", { Config::BackupType::EEPROM_4, GPIODeviceType::None, false } },  /* 0763 - Super Monkey Ball Jr. (USA) */
  { "BL8E", { Config::BackupType::EEPROM_4, GPIODeviceType::None, false } }   /* 2561 - Tomb Raider - Legend  */
};

} // namespace nba
