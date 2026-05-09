#include <toml.hpp>

#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    std::string s(reinterpret_cast<const char *>(data), size);
    const auto ref = toml::try_parse_str(s);
    return 0;
}
