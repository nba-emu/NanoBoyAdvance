/*
* Copyright (C) 2015 Frederic Meyer
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

#include <iostream>
#include <iomanip>
#include <vector>
#include "debugger.h"

using namespace std;
using namespace NanoboyAdvance;

#define display_word setfill('0') << setw(8) << hex
#define display_hword setfill('0') << setw(4) << hex
#define display_byte setfill('0') << setw(2) << hex

#define wrong_param_amount cout << "Too many or few parameters. See \"help\" for help." << endl;
#define wrong_param(n) cout << "Cannot parse parameter #" << (n) << "\"" << tokens[n] << "\"" << endl;
#define debugger_try(code) try\
{\
code\
}\
catch (exception& e)\
{\
    cout << e.what() << endl;\
}

typedef struct
{
    u32 address;
    bool thumb;
} GBAExecuteBreakpoint;

ARM7* arm_cpu;
vector<GBAExecuteBreakpoint*> code_breakpoints;

static u8 debugger_take_byte(string parameter)
{
    u8 result = static_cast<u8>(stoul(parameter, nullptr, 0));
    if (result > 0xFF)
    {
        //throw new runtime_error("Parameters exceeds max. limit of byte.");
    }
    return result;
}

static u16 debugger_take_hword(string parameter)
{
    u16 result = static_cast<u16>(stoul(parameter, nullptr, 0));
    if (result > 0xFFFF)
    {
        //throw new runtime_error("Parameters exceeds max. limit of byte.");
    }
    return result;
}

static u32 debugger_take_word(string parameter)
{
    return stoul(parameter, nullptr, 0);
}

static void debugger_help()
{
    cout << "help: displays this help text" << endl;
}

static void debugger_bp_code(vector<string> tokens, bool thumb)
{
    GBAExecuteBreakpoint* breakpoint = (GBAExecuteBreakpoint*)malloc(sizeof(GBAExecuteBreakpoint));
    
    // Catch too many / few parameters
    if (tokens.size() != 2)
    {
        wrong_param_amount;
        return;
    }
    
    // Try to add breakpoint
    debugger_try(
        breakpoint->address = debugger_take_word(tokens[1]);
        breakpoint->thumb = thumb;
        code_breakpoints.push_back(breakpoint);
    )
}

static void debugger_showregs(vector<string> tokens)
{
    // Catch too many / few parameters
    if (tokens.size() > 2)
    {
        wrong_param_amount;
        return;
    }

    // Parse and display registers
    debugger_try(
        if (tokens.size() == 2) 
        {
            ARM7::ARM7Mode mode;
            if (tokens[1] == "fiq") 
            {
                for (int i = 0; i < 8; i++) cout << "r" << i << ": " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::User, i) << dec << endl;
                for (int i = 8; i < 15; i++) cout << "r" << i << "_<fiq>: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::FIQ, i) << dec << endl;
                cout << "r15: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::User, 15) << dec << endl;
                return;
            }
            else if (tokens[1] == "svc")
            {
                for (int i = 0; i < 13; i++) cout << "r" << i << ": " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::User, i) << dec << endl;
                for (int i = 13; i < 15; i++) cout << "r" << i << "_<svc>: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::SVC, i) << dec << endl;
                cout << "r15: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::User, 15) << dec << endl;
                return;
            }
            else if (tokens[1] == "abt")
            {
                for (int i = 0; i < 13; i++) cout << "r" << i << ": " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::User, i) << dec << endl;
                for (int i = 13; i < 15; i++) cout << "r" << i << "_<abt>: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::Abort, i) << dec << endl;
                cout << "r15: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::User, 15) << dec << endl;
                return;
            }
            else if (tokens[1] == "irq")
            {
                for (int i = 0; i < 13; i++) cout << "r" << i << ": " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::User, i) << dec << endl;
                for (int i = 13; i < 15; i++) cout << "r" << i << "_<irq>: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::IRQ, i) << dec << endl;
                cout << "r15: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::User, 15) << dec << endl;
                return;
            }
            else if (tokens[1] == "und")
            {
                for (int i = 0; i < 13; i++) cout << "r" << i << ": " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::User, i) << dec << endl;
                for (int i = 13; i < 15; i++) cout << "r" << i << "_<und>: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::Undefined, i) << dec << endl;
                cout << "r15: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::User, 15) << dec << endl;
                return;
            }
            else
            {
                wrong_param(1);
                return;
            }
        }

        // Print all registers
        for (int i = 0; i < 16; i++) cout << "r" << i << ": " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::User, i) << dec << endl;
        for (int i = 8; i < 15; i++) cout << "r" << i << "_<fiq>: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::FIQ, i) << dec << endl;
        for (int i = 13; i < 15; i++) cout << "r" << i << "_<svc>: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::SVC, i) << dec << endl;
        for (int i = 13; i < 15; i++) cout << "r" << i << "_<abt>: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::Abort, i) << dec << endl;
        for (int i = 13; i < 15; i++) cout << "r" << i << "_<irq>: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::IRQ, i) << dec << endl;
        for (int i = 13; i < 15; i++) cout << "r" << i << "_<und>: " << display_word << arm_cpu->GetGeneralRegister(ARM7::ARM7Mode::Undefined, i) << dec << endl;
    )
}

void debugger_shell()
{
    bool debugging = true;
    
    while (debugging)
    {
        string command;
        vector<string> tokens;
        
        // Get next command
        cout << "nsh$ ";
        getline(cin, command);
        
        // Don't process empty input
        if (command.length() == 0)
        {
            continue;
        }

        // Tokenize
        while (command.length() > 0)
        {
            int position = command.find(" ");
            if (position != -1)
            {
                tokens.push_back(command.substr(0, position));
                command = command.substr(position + 1, command.length() - position - 1);
            }
            else
            {
                tokens.push_back(command);
                command = "";
            }
        }
        
        // Execute command
        if (tokens[0] == "help") debugger_help();
        else if (tokens[0] == "bpa") debugger_bp_code(tokens, false);
        else if (tokens[0] == "bpt") debugger_bp_code(tokens, true);
        else if (tokens[0] == "showregs") debugger_showregs(tokens);
        else if (tokens[0] == "c") break;
        else cout << "Invalid command. See \"help\" for help." << endl;
    }
}

static void debugger_cpu_hook(int reason, void* data)
{
    switch (reason)
    {
    case ARM_CALLBACK_EXECUTE:
    {
        ARMCallbackExecute* execute = reinterpret_cast<ARMCallbackExecute*>(data);
        for (vector<GBAExecuteBreakpoint*>::iterator it = code_breakpoints.begin(); it != code_breakpoints.end(); ++it)
        {
            if ((*it)->address == execute->address && (*it)->thumb == execute->thumb)
            {
                cout << "Hit code breakpoint 0x" << display_word << execute->address << (execute->thumb ? " (thumb)" : " (arm)") << endl;
                debugger_shell();
                return;
            }
        }
        break;
    }
    }
}

void debugger_attach(ARM7* arm, GBAMemory* memory)
{
    arm->SetCallback(debugger_cpu_hook);
    arm_cpu = arm;
}
