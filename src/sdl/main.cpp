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

#include <string>
#include <SDL2/SDL.h>
#include "gba/cpu.hpp"
#include "util/file.hpp"

#include "argument_parser.hpp"

#undef main

using namespace std;
using namespace Util;
using namespace GameBoyAdvance;

struct GBAConfig {
    std::string rom_path;
    std::string save_path;
    std::string bios_path;
    
    int scale = 1;
};

int main(int argc, char** argv) {
    CPU emu;
    
    ArgumentParser parser;
    shared_ptr<ParsedArguments> args;
    shared_ptr<GBAConfig> config(new GBAConfig());
    
    Option bios_opt("bios", "path", false);
    Option scale_opt("scale", "factor", true);
    
    parser.add_option(bios_opt);
    parser.add_option(scale_opt);
    parser.set_file_limit(1, 1);
    
    args = parser.parse(argc, argv);
    
    if (args->error) {
        return -1;
    }
    
    try {
        config->rom_path = args->files[0];
        
        args->get_option_string("bios", config->bios_path);
        
        if (args->option_exists("scale")) {
            args->get_option_int("scale", config->scale);
            
            if (config->scale == 0) {
                config->scale = 1; // silently adjust
            }
        }
        
        std::cout << config->bios_path << std::endl;
    } catch (std::exception& e) {
        parser.print_usage(argv[0]);
    }
    
    return 0;
}