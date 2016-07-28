/*
* Copyright (C) 2016 Frederic Meyer
*
* This file is part of nanoboyadvance.
*
* nanoboyadvance is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* nanoboyadvance is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "arguments.h"

using namespace std;

void usage()
{
    cout << "Usage: ./nanoboyadvance [--bios bios_file] [--scale factor] [--speedup multiplier] rom" << endl;
}

Arguments* parse_args(int argc, char** argv)
{
    int i = 1;
    Arguments* args = (Arguments*)malloc(sizeof(Arguments));

    // Init cmdline
    args->bios_file = (char*)"bios.bin";
    args->use_bios = false;
    args->scale = 2;
    args->speedup = 1;

    // Process arguments
    while (i < argc)
    {
        bool no_switch = false;

        // Handle optional parameters
        if (strcmp(argv[i], "--bios") == 0)
        {
            args->use_bios = true;

            if (argc > i + 1)
                args->bios_file = argv[++i];
            else
                return nullptr;
        }
        else if (strcmp(argv[i], "--scale") == 0)
        {
            if (argc > i + 1)
            {
                args->scale = atoi(argv[++i]);
                if (args->scale == 0)
                    return nullptr;
            }
            else
            {
                return nullptr;
            }
        }
        else if (strcmp(argv[i], "--speedup") == 0)
        {
            if (argc > i + 1)
            {
                args->speedup = atoi(argv[++i]);
                if (args->speedup == 0)
                    return nullptr;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            no_switch = true;
        }

        // Get rom path
        if (i == argc - 1)
        {
            if (!no_switch)
                return nullptr;
            args->rom_file = argv[i];
        }
        
        i++;
    }

    return args;
}
