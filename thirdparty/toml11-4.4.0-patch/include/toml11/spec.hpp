#ifndef TOML11_SPEC_HPP
#define TOML11_SPEC_HPP

#include <array>
#include <functional>
#include <ostream>
#include <sstream>
#include <utility>

#include <cstdint>

namespace toml
{

struct semantic_version
{
    constexpr semantic_version(std::uint32_t mjr, std::uint32_t mnr, std::uint32_t p) noexcept
        : major{mjr}, minor{mnr}, patch{p}
    {}

    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;
};

constexpr inline semantic_version
make_semver(std::uint32_t mjr, std::uint32_t mnr, std::uint32_t p) noexcept
{
    return semantic_version(mjr, mnr, p);
}

constexpr inline bool
operator==(const semantic_version& lhs, const semantic_version& rhs) noexcept
{
    return lhs.major == rhs.major &&
           lhs.minor == rhs.minor &&
           lhs.patch == rhs.patch;
}
constexpr inline bool
operator!=(const semantic_version& lhs, const semantic_version& rhs) noexcept
{
    return !(lhs == rhs);
}
constexpr inline bool
operator<(const semantic_version& lhs, const semantic_version& rhs) noexcept
{
    return lhs.major < rhs.major ||
           (lhs.major == rhs.major && lhs.minor < rhs.minor) ||
           (lhs.major == rhs.major && lhs.minor == rhs.minor && lhs.patch < rhs.patch);
}
constexpr inline bool
operator>(const semantic_version& lhs, const semantic_version& rhs) noexcept
{
    return rhs < lhs;
}
constexpr inline bool
operator<=(const semantic_version& lhs, const semantic_version& rhs) noexcept
{
    return !(lhs > rhs);
}
constexpr inline bool
operator>=(const semantic_version& lhs, const semantic_version& rhs) noexcept
{
    return !(lhs < rhs);
}

inline std::ostream& operator<<(std::ostream& os, const semantic_version& v)
{
    os << v.major << '.' << v.minor << '.' << v.patch;
    return os;
}

inline std::string to_string(const semantic_version& v)
{
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

struct spec
{
    constexpr static spec default_version() noexcept
    {
        return spec::v(1, 0, 0);
    }

    constexpr static spec v(std::uint32_t mjr, std::uint32_t mnr, std::uint32_t p) noexcept
    {
        return spec(make_semver(mjr, mnr, p));
    }

    constexpr explicit spec(const semantic_version& semver) noexcept
        : version{semver},
          v1_1_0_allow_control_characters_in_comments {semantic_version{1, 1, 0} <= semver},
          v1_1_0_allow_newlines_in_inline_tables      {semantic_version{1, 1, 0} <= semver},
          v1_1_0_allow_trailing_comma_in_inline_tables{semantic_version{1, 1, 0} <= semver},
          v1_1_0_allow_non_english_in_bare_keys       {semantic_version{1, 1, 0} <= semver},
          v1_1_0_add_escape_sequence_e                {semantic_version{1, 1, 0} <= semver},
          v1_1_0_add_escape_sequence_x                {semantic_version{1, 1, 0} <= semver},
          v1_1_0_make_seconds_optional                {semantic_version{1, 1, 0} <= semver},
          ext_hex_float {false},
          ext_num_suffix{false},
          ext_null_value{false}
    {}

    semantic_version version; // toml version

    // diff from v1.0.0 -> v1.1.0
    bool v1_1_0_allow_control_characters_in_comments;
    bool v1_1_0_allow_newlines_in_inline_tables;
    bool v1_1_0_allow_trailing_comma_in_inline_tables;
    bool v1_1_0_allow_non_english_in_bare_keys;
    bool v1_1_0_add_escape_sequence_e;
    bool v1_1_0_add_escape_sequence_x;
    bool v1_1_0_make_seconds_optional;

    // library extensions
    bool ext_hex_float;  // allow hex float (in C++ style)
    bool ext_num_suffix; // allow number suffix (in C++ style)
    bool ext_null_value; // allow `null` as a value
};

namespace detail
{
inline std::pair<const semantic_version&, std::array<bool, 10>>
to_tuple(const spec& s) noexcept
{
    return std::make_pair(std::cref(s.version), std::array<bool, 10>{{
            s.v1_1_0_allow_control_characters_in_comments,
            s.v1_1_0_allow_newlines_in_inline_tables,
            s.v1_1_0_allow_trailing_comma_in_inline_tables,
            s.v1_1_0_allow_non_english_in_bare_keys,
            s.v1_1_0_add_escape_sequence_e,
            s.v1_1_0_add_escape_sequence_x,
            s.v1_1_0_make_seconds_optional,
            s.ext_hex_float,
            s.ext_num_suffix,
            s.ext_null_value
        }});
}
} // detail

inline bool operator==(const spec& lhs, const spec& rhs) noexcept
{
    return detail::to_tuple(lhs) == detail::to_tuple(rhs);
}
inline bool operator!=(const spec& lhs, const spec& rhs) noexcept
{
    return detail::to_tuple(lhs) != detail::to_tuple(rhs);
}
inline bool operator< (const spec& lhs, const spec& rhs) noexcept
{
    return detail::to_tuple(lhs) <  detail::to_tuple(rhs);
}
inline bool operator<=(const spec& lhs, const spec& rhs) noexcept
{
    return detail::to_tuple(lhs) <= detail::to_tuple(rhs);
}
inline bool operator> (const spec& lhs, const spec& rhs) noexcept
{
    return detail::to_tuple(lhs) >  detail::to_tuple(rhs);
}
inline bool operator>=(const spec& lhs, const spec& rhs) noexcept
{
    return detail::to_tuple(lhs) >= detail::to_tuple(rhs);
}

} // namespace toml
#endif // TOML11_SPEC_HPP
