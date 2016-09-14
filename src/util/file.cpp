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


#include <iostream>
#include <fstream>
#include "file.h"


using namespace std;


namespace File
{
    bool Exists(string filename)
    {
        ifstream ifs(filename);
        bool exists = ifs.is_open();

        ifs.close();

        return exists;
    }

    int GetFileSize(string filename)
    {
        ifstream ifs(filename, ios::in | ios::binary | ios::ate);

        if (ifs.is_open())
        {
            ifs.seekg(0, ios::end);
            return ifs.tellg();
        }

        return 0;
    }

    u8* ReadFile(string filename)
    {
        ifstream ifs(filename, ios::in | ios::binary | ios::ate);
        size_t filesize;
        u8* data = 0;

        if (ifs.is_open())
        {
            ifs.seekg(0, ios::end);
            filesize = ifs.tellg();
            ifs.seekg(0, ios::beg);
            data = new u8[filesize];
            ifs.read((char*)data, filesize);
            ifs.close();
        }
        else
        {
            cout << "Cannot open file " << filename.c_str();
            return NULL;
        }

        return data;
    }

    void WriteFile(string filename, u8* data, int size)
    {
        ofstream ofs(filename, ios::out | ios::binary);

        if (ofs.is_open())
        {
            ofs.write((char*)data, size);
            ofs.close();
        }
        else
        {
            cout << "Cannot write file " << filename.c_str();
        }
    }
}
