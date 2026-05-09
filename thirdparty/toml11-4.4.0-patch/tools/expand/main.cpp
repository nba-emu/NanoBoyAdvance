#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <set>

#include <cassert>

std::optional<std::filesystem::path>
is_include(const std::string& line, const std::filesystem::path& filepath)
{
    // [ws] # [ws] include [ws] \".+\"
    auto iter = line.begin();
    while(iter < line.end())
    {
        if(*iter != ' ' && *iter != '\t')
        {
            break;
        }
        ++iter;
    }
    if(iter == line.end() || *iter != '#') {return std::nullopt;}

    assert(*iter == '#');
    ++iter;

    while(iter < line.end())
    {
        if(*iter != ' ' && *iter != '\t')
        {
            break;
        }
        ++iter;
    }
    if(iter == line.end() || *iter != 'i') {return std::nullopt;} else {++iter;}
    if(iter == line.end() || *iter != 'n') {return std::nullopt;} else {++iter;}
    if(iter == line.end() || *iter != 'c') {return std::nullopt;} else {++iter;}
    if(iter == line.end() || *iter != 'l') {return std::nullopt;} else {++iter;}
    if(iter == line.end() || *iter != 'u') {return std::nullopt;} else {++iter;}
    if(iter == line.end() || *iter != 'd') {return std::nullopt;} else {++iter;}
    if(iter == line.end() || *iter != 'e') {return std::nullopt;} else {++iter;}

    while(iter < line.end())
    {
        if(*iter != ' ' && *iter != '\t')
        {
            break;
        }
        ++iter;
    }
    if(iter == line.end() || *iter != '"') {return std::nullopt;} else {++iter;}

    std::string filename;
    while(iter < line.end())
    {
        if(*iter == '"') {break;}
        filename += *iter;
        ++iter;
    }
    if(iter == line.end() || *iter != '"') {return std::nullopt;} else {++iter;}

    return std::filesystem::canonical(filepath.parent_path() / std::filesystem::path(filename));
}

struct File
{
    File() = default;

    explicit File(std::filesystem::path f)
        : filename(std::move(f))
    {
        std::ifstream ifs(filename);
        if( ! ifs.good())
        {
            throw std::runtime_error("file open error: " + filename.string());
        }

        std::string line;
        while(std::getline(ifs, line))
        {
            if(const auto incl = is_include(line, filename))
            {
                includes.push_back(incl.value());
            }
            else
            {
                content.push_back(line);
            }
        }
    }

    File(std::filesystem::path f, std::vector<std::string> c,
         std::vector<std::filesystem::path> i)
        : filename(std::move(f)), content(std::move(c)), includes(std::move(i))
    {}

    std::filesystem::path filename;
    std::vector<std::string> content; // w/o include
    std::vector<std::filesystem::path> includes;
};

struct Graph
{
    struct Node
    {
        std::vector<std::filesystem::path> included;
        std::vector<std::filesystem::path> includes;
    };

    std::map<std::filesystem::path, Node> nodes;
};

int main(int argc, char** argv)
{
    using namespace std::literals::string_literals;
    if(argc != 2)
    {
        std::cerr << "Usage: ./a.out path/to/toml.hpp > single_include/toml.hpp" << std::endl;
        return 1;
    }

    const auto input_file = std::filesystem::path(std::string(argv[1]));
    assert(input_file.filename() == "toml.hpp");

    const auto include_path = input_file.parent_path();

    // -------------------------------------------------------------------------
    // load files and detect `include "xxx.hpp"`.
    // If the file has `_fwd` and `_impl`, expand those files first.

    std::set<std::string> fwd_impl_files;
    for(const auto& entities : std::filesystem::directory_iterator(include_path/"toml11"/"fwd"))
    {
        if( ! entities.is_regular_file()) {continue;}
        std::string fname = entities.path().filename().string();
        if(fname.ends_with("_fwd.hpp"))
        {
            for(const auto c : "_fwd.hpp"s) {fname.pop_back(); (void)c;}
            fwd_impl_files.insert(std::move(fname));
        }
    }
    for(const auto& entities : std::filesystem::directory_iterator(include_path/"toml11"/"impl"))
    {
        if( ! entities.is_regular_file()) {continue;}
        std::string fname = entities.path().filename().string();
        if(fname.ends_with("_impl.hpp"))
        {
            for(const auto c : "_impl.hpp"s) {fname.pop_back(); (void)c;}
            // all impl files has fwd file
            assert(fwd_impl_files.contains(fname));
        }
    }

    const auto input = File(input_file);

    std::map<std::filesystem::path, File> files;
    files[input_file] = input;

    for(const auto& fname : input.includes)
    {
        if(fwd_impl_files.contains(fname.stem().string()))
        {
            std::cerr << "expanding fwd/impl file of " << fname.string() << std::endl;

            // expand the first include
            std::ifstream ifs(fname);

            std::vector<std::string> content;
            std::vector<std::filesystem::path> includes;

            std::string line;
            while(std::getline(ifs, line))
            {
                // expand _fwd and _impl files first.
                const auto incl = is_include(line, fname);
                if(incl.has_value())
                {
                    // if a file has _fwd/_impl files, it only includes fwd/impl files.
                    assert(incl.value().string().ends_with("_impl.hpp") ||
                           incl.value().string().ends_with("_fwd.hpp")  );

                    const File included(incl.value());
                    for(const auto& l : included.content)
                    {
                        content.push_back(l);
                    }
                    for(const auto& i : included.includes)
                    {
                        includes.push_back(i);
                    }
                }
                else
                {
                    content.push_back(line);
                }
            }
            files[fname] = File(fname, std::move(content), std::move(includes));
        }
        else
        {
            files[fname] = File(fname);
        }
        std::cerr << "file " << fname << " has " << files.at(fname).content.size() << " lines." << std::endl;
    }

    std::cerr << "-------------------------------------------------------------\n";
    std::cerr << "files have been read. next: constructing dependency graph...\n";
    std::cerr << "-------------------------------------------------------------\n";

    // -------------------------------------------------------------------------
    // construct dependency graph

    Graph g;
    for(const auto& [k, v] : files)
    {
        g.nodes[k] = Graph::Node{};
    }

    for(const auto& [fname, file] : files)
    {
        for(auto incl : file.includes)
        {
            auto incl_stem = incl.stem().string();
            if(incl_stem.ends_with("_fwd"))
            {
                for(const char c : "_fwd"s) {incl_stem.pop_back(); (void)c;}

                // include original file instaed
                incl = incl.parent_path() / ".." / std::filesystem::path(incl_stem + ".hpp");
            }
            else if(incl_stem.ends_with("_impl"))
            {
                for(const char c : "_impl"s) {incl_stem.pop_back(); (void)c;}

                // include original file instaed
                incl = incl.parent_path() / ".." / std::filesystem::path(incl_stem + ".hpp");
            }
            incl = std::filesystem::canonical(incl);

            // avoid self include loop
            if(fname != incl)
            {
                std::cerr << fname << " includes " << incl << std::endl;

                g.nodes.at(fname).includes.push_back(incl);
                g.nodes.at(incl) .included.push_back(fname);
            }
        }
    }

    std::cerr << "-------------------------------------------------------------\n";
    std::cerr << "graph has been constructed. flattening...\n";
    std::cerr << "-------------------------------------------------------------\n";

    // -------------------------------------------------------------------------
    // flatten graph by topological sort

    // collect files that does not include
    std::vector<std::filesystem::path> sources;
    for(const auto& [fname, node] : g.nodes)
    {
        if(node.includes.empty())
        {
            sources.push_back(fname);
        }
    }
    assert( ! sources.empty());

    std::vector<std::filesystem::path> sorted;
    while( ! sources.empty())
    {
        const auto file = sources.back();
        sorted.push_back(sources.back());
        sources.pop_back();

        for(const auto& included : g.nodes.at(file).included)
        {
            auto found = std::find(g.nodes.at(included).includes.begin(),
                                   g.nodes.at(included).includes.end(), file);
            g.nodes.at(included).includes.erase(found);

            if(g.nodes.at(included).includes.empty())
            {
                sources.push_back(included);
            }
        }
    }

    std::cerr << "-------------------------------------------------------------\n";
    std::cerr << "flattened. outputting...\n";
    std::cerr << "-------------------------------------------------------------\n";

    // -------------------------------------------------------------------------
    // output all the file in the sorted order

    for(const auto& fname : sorted)
    {
        std::cerr << "expanding: " << fname << std::endl;
        for(const auto& line : files.at(fname).content)
        {
            std::cout << line << '\n';
        }
    }

    return 0;
}
