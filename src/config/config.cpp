///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////


#include "config.h"
#include "util/log.h"
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <stack>

using namespace std;

namespace NanoboyAdvance
{
    Config::Config(string path)
    {
        m_File = path;
    }

    int Config::ReadInt(string category, string key)
    {
        // TODO: catch exception
        return stoi(Read(category, key));
    }

    bool Config::ReadBool(string category, string key)
    {
        string result = Read(category, key);

        if (result == "true" || result == "1") return true;
        if (result == "false" || result == "0") return false;

        LOG(LOG_ERROR, "%s::%s: Invalid format. Expected bool.", category.c_str(), key.c_str());

        return false;
    }

    string Config::Read(string category, string key)
    {
        string line;
        string current_category = "";
        ifstream stream(m_File, ios::in);
        stack<string> category_stack;

        // Read config file line by line.
        while (std::getline(stream, line))
        {
            int seperator = 0;
            char first_char;

            // Remove any intendation
            boost::trim_left(line);

            // Don't parse empty lines...
            if (line.size() == 0) continue;

            first_char = line.at(0);

            // Skip any comments
            if (first_char == '#') continue;

            // Check for new category
            if (first_char == '~')
            {
                current_category = line.substr(1, line.size() - 1);
                continue;
            }

            // Check for new *sub*category
            if (first_char == '+')
            {
                category_stack.push(current_category);
                current_category += "::" + line.substr(1, line.size() - 1);
                continue;
            }

            // Check for closing a *sub*category
            if (first_char == '-')
            {
                if (category_stack.empty())
                {
                    LOG(LOG_ERROR, "Closing sub-category, but no such.");
                    return "{error}";
                }

                current_category = category_stack.top();
                category_stack.pop();
                continue;
            }

            // Check for an actual key
            if ((seperator = line.find(':')) != string::npos)
            {
                if (current_category == "")
                {
                    LOG(LOG_ERROR, "Not in a namespace/node yet");
                    return "{error}";
                }

                string part1 = line.substr(0, seperator);
                string part2 = line.substr(seperator + 2, line.size() - seperator - 2);

                // Test if key and category match.
                if (part1 == key && current_category == category)
                {
                    stream.close();
                    boost::trim_right(part2);
                    return part2;
                }

                continue;
            }

            LOG(LOG_ERROR, "Config parse error in line: %s", line.c_str());
            stream.close();
            return "{error}";
        }

        return "{not found}";
    }

    void Config::Write(string category, string key, string value)
    {

    }
}
