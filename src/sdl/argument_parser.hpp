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
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <iostream>

const int NO_LIMIT = -1;

struct Option {
    std::string name;
    std::string value_name;
    //std::string description;
    
    bool has_value = false;
    bool optional  = true;
    
    Option() {
    }
    
    Option(std::string name, bool optional) {
        this->name = name;
        this->optional = optional;
    }
    
    Option(std::string name, std::string value_name, bool optional) : Option(name, optional) {
        this->has_value  = true;
        this->value_name = value_name;
    }
};

struct ParsedArguments {
    bool error;
    std::string app_name;
    
    std::vector<std::string> files;
    std::map<std::string, std::string> options;
    
    auto option_exists(std::string name) -> bool {
        return options.count(name) == 1;
    }
    
    auto get_option_int(std::string name) -> int {
        if (!option_exists(name)) {
            throw std::runtime_error("option '" + name + "' not found.");
        }
        return std::stoi(options[name]);
    }
    
    auto get_option_bool(std::string name) -> bool {
        if (option_exists(name)) {
            std::string value = options[name];
            
            if (value.empty()) {
                return true;
            } else {
                if (value == "true" || value == "1") {
                    return true;
                }
                if (value == "false" || value == "0") {
                    return false;
                }
                throw std::runtime_error("value of option '" + name + "' is not a valid boolean.");
            }
        }
        return false;
    }
    
    auto get_option_string(std::string name) -> std::string {
        if (!option_exists(name)) {
            throw std::runtime_error("option '" + name + "' not found.");
        }
        return options[name];
    }
    
    void get_option_int(std::string name, int& target) {
        target = get_option_int(name);
    }
    
    void get_option_bool(std::string name, bool& target) {
        target = get_option_bool(name);
    }
    
    void get_option_string(std::string name, std::string& target) {
        target = get_option_string(name);
    }
};

class ArgumentParser {
protected:
    std::string m_prefix = "--";
    std::vector<Option> m_options;
    
    struct FileLimit {
        int min = NO_LIMIT;
        int max = NO_LIMIT;
    } m_limit;
    
    auto is_option(std::string str) -> bool {
        return str.substr(0, m_prefix.size()) == m_prefix;
    }
    
    auto is_valid(ParsedArguments& parsed_args) -> bool {
        auto file_count = parsed_args.files.size();
        
        int& min = m_limit.min;
        int& max = m_limit.max;
        
        if ((min != NO_LIMIT && file_count < min) ||
                (max != NO_LIMIT && file_count > max)) {
            return false;
        }
        
        auto first = std::begin(m_options);
        auto last  = std::end(m_options);
        
        for (auto it = first; it != last; ++it) {
            if (!it->optional && !parsed_args.option_exists(it->name)) {
                return false;
            }
        }
        
        return true;
    }
    
    void parse_internal(int argc, char** argv, ParsedArguments* parsed_args) {
        int i = 1;
        
        auto first = std::begin(m_options);
        auto last  = std::end(m_options);
            
        while (i < argc) {
            std::string arg_str(argv[i++]);
            
            if (is_option(arg_str)) {
                    
                auto iter = std::find_if(first, last, [this, arg_str](const Option& arg) -> bool {
                    return arg_str == m_prefix + arg.name;
                });

                if (iter != last) {
                    if (iter->has_value) {   
                        if (i >= argc || is_option(argv[i])) {
                            parsed_args->error = true;
                            return;
                        }
                        parsed_args->options[iter->name] = argv[i++];
                    } else {
                        parsed_args->options[iter->name] = "";
                    }
                } else {
                    parsed_args->error = true;
                    return;
                }
            } else {
                parsed_args->files.push_back(arg_str);
            }
        }
    }
    
public:
    ArgumentParser() {
    }
    
    ArgumentParser(std::vector<Option>& options) {
        m_options = options;
    }
    
    void add_option(Option& opt) {
        m_options.push_back(opt);
    }
    
    void set_prefix(std::string prefix) {
        m_prefix = prefix;
    }
    
    void set_file_limit(int min, int max) {
        m_limit.min = min;
        m_limit.max = max;
    }
    
    void print_usage(std::string app_name) {
        // TODO: print and somehow name file parameters...
        std::cout << "usage: " << app_name;
        
        auto opts = m_options;
        
        for (auto it = std::begin(opts); it != std::end(opts); ++it) {
            std::string text = m_prefix + it->name;
            
            if (it->has_value) {
                std::string value_name = it->value_name;
                
                if (value_name.empty()) {
                    value_name = "value";
                }
                
                text += " " + value_name;
            }
            if (it->optional) {
                text = "[" + text + "]";
            }
            
            std::cout << " " + text;
        }
        
        std::cout << std::endl << std::endl;
    }
    
    auto parse(int argc, char** argv) -> std::shared_ptr<ParsedArguments> {
        
        ParsedArguments* parsed_args = new ParsedArguments();
        
        parsed_args->error = false;
        parsed_args->app_name = argv[0];
        
        if (argc > 1) {
            parse_internal(argc, argv, parsed_args);
        }
        
        if (parsed_args->error || !is_valid(*parsed_args)) {
            parsed_args->error = true; // force error flag
            
            print_usage(argv[0]);
        }
        
        return std::shared_ptr<ParsedArguments>(parsed_args);
    }
};