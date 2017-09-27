/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

#include <iostream>

namespace Util {
    struct INILine {
        int line;
        std::string text;

        std::string s_trim; // start trimmed
        std::string e_trim; // end trimmed
        std::string comment;
        bool has_comment;
    };

    struct INIValue {
        //std::string key;
        std::string value;
        INILine* source_line;
    };

    struct INIParseError : std::runtime_error {
        INILine* line;
        INIParseError(INILine* line, std::string message) : std::runtime_error(message) {}
    };

    struct INISectionNotFoundError : std::runtime_error {
        std::string section;
        INISectionNotFoundError(std::string section, std::string message) : std::runtime_error(message), section(section) {}
    };

    struct INIKeyNotFoundError     : std::runtime_error {
        std::string section, key;
        INIKeyNotFoundError(std::string section, std::string key, std::string message) : std::runtime_error(message), section(section), key(key) {}
    };

    struct INITypeConversionError  : std::runtime_error {
        std::string section, key;
        INITypeConversionError(std::string section, std::string key, std::string message) : std::runtime_error(message), section(section), key(key) {}
    };

    typedef std::map<std::string, INIValue*> INISection;

    class INI {
    private:
        // Hold information for each single INI line
        std::vector<INILine*> lines;

        // Maps section names to section key-value stores
        std::map<std::string, INISection*> sections;

        // Global value context
        INISection global;

        std::string ini_path;

        bool write_on_change;

        auto readINI() -> void {
            std::ifstream ini(ini_path);

            int nr = 1;
            INISection* current_section = &global;

            for (std::string line; std::getline(ini, line); ) {
                // Do not insert any data for empty lines
                if (line.size() == 0) {
                    nr++;
                    continue;
                }

                auto ini_line = new INILine();

                // - Detect and trim comment -
                auto c = line.find_first_of("#");

                if (c != std::string::npos) {
                    // Store comment information in line structure
                    ini_line->has_comment = true;
                    ini_line->comment     = line.substr(c + 1);

                    // Remove it from line string
                    line = line.substr(0, c);
                } else {
                    ini_line->has_comment = false;
                }

                // - Remove leading or trailing whitespaces / tabs -
                auto s = line.find_first_not_of(" \t");
                auto e = line.find_last_not_of (" \t");

                /*if (s != std::string::npos) {
                    ini_line->s_trim = line.substr(0, line.size() - s);
                    line = line.substr(s);
                }
                if (e != std::string::npos) {
                    line = line.substr(0, e + 1);
                }*/

                if (s != std::string::npos && e != std::string::npos) {
                    //ini_line->s_trim = line.substr(0, s - 1) : "";
                    //ini_line->e_trim = (e != line.size()-1) ? line.substr(e + 1)    : "";

                    line = line.substr(s, (e - s) + 1);
                }

                // - Insert new line -
                ini_line->line = nr++;
                ini_line->text = line;
                lines.push_back(ini_line);

                // - Parse line, if anything left -
                int line_size = line.size();

                if (line_size != 0) {
                    if (line[0] == '[') {
                        // Try parse section begin / definition
                        if (line_size == 1 || line[line_size-1] != ']') {
                            throw INIParseError(ini_line, "missing ']'");
                        }

                        // New section, create object for it, or use existing
                        auto section_name = line.substr(1, line_size - 2);
                        auto existing_it  = sections.find(section_name);

                        if (existing_it != sections.end()) {
                            current_section = existing_it->second;
                        } else {
                            current_section = new INISection();
                            sections[section_name] = current_section;
                        }
                    } else {
                        // Try parse key-value pair
                        // TODO: define valid/invalid characters?

                        auto first = line.find_first_of("=");
                        auto last  = line.find_last_of ("=");

                        if (first == std::string::npos) {
                            throw INIParseError(ini_line, "expected '='");
                        }
                        if (first != last) {
                            throw INIParseError(ini_line, "too many '='. expected one.");
                        }

                        auto key   = line.substr(0, first);
                        auto value = line.substr(first + 1);

                        auto a =   key.find_last_not_of (" \t");
                        auto b = value.find_first_not_of(" \t");

                        if (a != std::string::npos) {
                            key = key.substr(0, a + 1);
                        }
                        if (b != std::string::npos) {
                            value = value.substr(b);
                        }

                        // Store key-value pair in namespace/section
                        auto ini_value = new INIValue();

                        ini_value->value        = value;
                        ini_value->source_line  = ini_line;
                        (*current_section)[key] = ini_value;
                    }
                }
                /*else {
                    // Empty line, switch back to global namespace
                    current_section = &global;
                }*/
            }
            ini.close();
        }

        auto findValueEntry(std::string section, std::string key) -> INIValue* {
            auto section_it  = sections.find(section);
            auto section_str = section;

            if (section_it != sections.end()) {
                auto section    = section_it->second;
                auto keypair_it = section->find(key);

                if (keypair_it != section->end()) {
                    return keypair_it->second;
                } else {
                    throw INIKeyNotFoundError(section_str, key, "unknown key: " + key + " [" + section_str + "]");
                }
            } else {
                throw INISectionNotFoundError(section_str, "unknown section: " + section_str);
            }
        }
    public:
        INI(std::string ini_path, bool write_on_change = false) : ini_path(ini_path), write_on_change(write_on_change) {
            readINI();
        }

        auto getString(std::string section, std::string key) -> std::string {
            return findValueEntry(section, key)->value;
        }

        auto setString(std::string section, std::string key, std::string value) -> void {
            auto entry = findValueEntry(section, key);
            auto line  = entry->source_line;

            entry->value = value;
            line->text   = key + " = " + value;

            if (write_on_change) {
                writeINI();
            }
        }

        auto getInteger(std::string section, std::string key) -> int {
            auto value = getString(section, key);

            try {
                // TODO: support hexadecimal and binary
                return std::stoi(value);
            }
            catch (const std::invalid_argument& ex) {
                throw INITypeConversionError(section, key, "cannot convert '" + value + "' to int.");
            }
        }

        auto setInteger(std::string section, std::string key, int value) -> void {
            setString(section, key, std::to_string(value));
        }

        auto writeINI() -> void {
            int nr = 1;
            std::ofstream ini(ini_path, std::ofstream::out);
            for (auto ini_line : lines) {
                // catchup
                while (nr != ini_line->line) {
                    ini << std::endl;
                    nr++;
                }
                std::string str_line = ini_line->s_trim + ini_line->text + ini_line->e_trim;
                if (ini_line->has_comment) {
                    str_line += "#" + ini_line->comment;
                }
                ini << str_line << std::endl;
                nr++;
            }
            ini.close();
        }
    };
}
