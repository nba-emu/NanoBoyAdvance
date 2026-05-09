#ifndef TOML11_SYNTAX_FWD_HPP
#define TOML11_SYNTAX_FWD_HPP

#include "../scanner.hpp"
#include "../spec.hpp"

namespace toml
{
namespace detail
{
namespace syntax
{

using char_type = location::char_type;

// ===========================================================================
// UTF-8

// avoid redundant representation and out-of-unicode sequence

character_in_range const& utf8_1byte (const spec&);
sequence           const& utf8_2bytes(const spec&);
sequence           const& utf8_3bytes(const spec&);
sequence           const& utf8_4bytes(const spec&);

class non_ascii final : public scanner_base
{
  public:

    using char_type = location::char_type;

  public:

    explicit non_ascii(const spec& s) noexcept
        : utf8_2B_(utf8_2bytes(s)),
          utf8_3B_(utf8_3bytes(s)),
          utf8_4B_(utf8_4bytes(s))
    {}
    ~non_ascii() override = default;

    region scan(location& loc) const override
    {
        {
            const auto reg = utf8_2B_.scan(loc);
            if(reg.is_ok()) {return reg;}
        }
        {
            const auto reg = utf8_3B_.scan(loc);
            if(reg.is_ok()) {return reg;}
        }
        {
            const auto reg = utf8_4B_.scan(loc);
            if(reg.is_ok()) {return reg;}
        }
        return region{};
    }

    std::string expected_chars(location&) const override
    {
        return "non-ascii utf-8 bytes";
    }

    scanner_base* clone() const override
    {
        return new non_ascii(*this);
    }

    std::string name() const override
    {
        return "non_ascii";
    }

  private:
    sequence utf8_2B_;
    sequence utf8_3B_;
    sequence utf8_4B_;
};

// ===========================================================================
// Whitespace

character_either const& wschar(const spec&);

repeat_at_least const& ws(const spec& s);

// ===========================================================================
// Newline

either const& newline(const spec&);

// ===========================================================================
// Comments

either const& allowed_comment_char(const spec& s);

// XXX Note that it does not take newline
sequence const& comment(const spec& s);

// ===========================================================================
// Boolean

either const& boolean(const spec&);

// ===========================================================================
// Integer

class digit final : public scanner_base
{
  public:

    using char_type = location::char_type;

  public:

    explicit digit(const spec&) noexcept
      : scanner_(char_type('0'), char_type('9'))
    {}


    ~digit() override = default;

    region scan(location& loc) const override
    {
        return scanner_.scan(loc);
    }

    std::string expected_chars(location&) const override
    {
        return "digit [0-9]";
    }

    scanner_base* clone() const override
    {
        return new digit(*this);
    }

    std::string name() const override
    {
        return "digit";
    }

  private:

    character_in_range scanner_;
};

class alpha final : public scanner_base
{
  public:

    using char_type = location::char_type;

  public:

    explicit alpha(const spec&) noexcept
      : lowercase_(char_type('a'), char_type('z')),
        uppercase_(char_type('A'), char_type('Z'))
    {}
    ~alpha() override = default;

    region scan(location& loc) const override
    {
        {
            const auto reg = lowercase_.scan(loc);
            if(reg.is_ok()) {return reg;}
        }
        {
            const auto reg = uppercase_.scan(loc);
            if(reg.is_ok()) {return reg;}
        }
        return region{};
    }

    std::string expected_chars(location&) const override
    {
        return "alpha [a-zA-Z]";
    }

    scanner_base* clone() const override
    {
        return new alpha(*this);
    }

    std::string name() const override
    {
        return "alpha";
    }

  private:

    character_in_range lowercase_;
    character_in_range uppercase_;
};

class hexdig final : public scanner_base
{
  public:

    using char_type = location::char_type;

  public:

    explicit hexdig(const spec& s) noexcept
      : digit_(s),
        lowercase_(char_type('a'), char_type('f')),
        uppercase_(char_type('A'), char_type('F'))
    {}
    ~hexdig() override = default;

    region scan(location& loc) const override
    {
        {
            const auto reg = digit_.scan(loc);
            if(reg.is_ok()) {return reg;}
        }
        {
            const auto reg = lowercase_.scan(loc);
            if(reg.is_ok()) {return reg;}
        }
        {
            const auto reg = uppercase_.scan(loc);
            if(reg.is_ok()) {return reg;}
        }
        return region{};
    }

    std::string expected_chars(location&) const override
    {
        return "hex [0-9a-fA-F]";
    }

    scanner_base* clone() const override
    {
        return new hexdig(*this);
    }

    std::string name() const override
    {
        return "hexdig";
    }

  private:

    digit              digit_;
    character_in_range lowercase_;
    character_in_range uppercase_;
};

sequence const& num_suffix(const spec& s);

sequence const& dec_int(const spec& s);
sequence const& hex_int(const spec& s);
sequence const& oct_int(const spec&);
sequence const& bin_int(const spec&);
either   const& integer(const spec& s);

// ===========================================================================
// Floating

sequence const& zero_prefixable_int(const spec& s);
sequence const& fractional_part(const spec& s);
sequence const& exponent_part(const spec& s);
sequence const& hex_floating(const spec& s);
either   const& floating(const spec& s);

// ===========================================================================
// Datetime

sequence const& local_date(const spec& s);
sequence const& local_time(const spec& s);
either   const& time_offset(const spec& s);
sequence const& full_time(const spec& s);
character_either const& time_delim(const spec&);
sequence const& local_datetime(const spec& s);
sequence const& offset_datetime(const spec& s);

// ===========================================================================
// String

sequence const& escaped_x2(const spec& s);
sequence const& escaped_u4(const spec& s);
sequence const& escaped_U8(const spec& s);

sequence const& escaped     (const spec& s);
either   const& basic_char  (const spec& s);
sequence const& basic_string(const spec& s);

// ---------------------------------------------------------------------------
// multiline string

sequence const& escaped_newline(const spec& s);
sequence const& ml_basic_string(const spec& s);

// ---------------------------------------------------------------------------
// literal string

either   const& literal_char(const spec& s);
sequence const& literal_string(const spec& s);
sequence const& ml_literal_string(const spec& s);
either   const& string(const spec& s);

// ===========================================================================
// Keys

// to keep `expected_chars` simple
class non_ascii_key_char final : public scanner_base
{
  public:

    using char_type = location::char_type;

  private:

    using in_range = character_in_range; // make definition short

  public:

    explicit non_ascii_key_char(const spec& s) noexcept;
    ~non_ascii_key_char() override = default;

    region scan(location& loc) const override;

    std::string expected_chars(location&) const override
    {
        return "bare key non-ASCII script";
    }

    scanner_base* clone() const override
    {
        return new non_ascii_key_char(*this);
    }

    std::string name() const override
    {
        return "non-ASCII bare key";
    }

  private:

    std::uint32_t read_utf8(location& loc) const;
};


repeat_at_least const& unquoted_key(const spec& s);
either   const& quoted_key(const spec& s);
either   const& simple_key(const spec& s);
sequence const& dot_sep(const spec& s);
sequence const& dotted_key(const spec& s);

class key final : public scanner_base
{
  public:

    using char_type = location::char_type;

  public:

    explicit key(const spec& s) noexcept
        : dotted_(dotted_key(s)),
          simple_(simple_key(s))
    {}
    ~key() override = default;

    region scan(location& loc) const override
    {
        {
            const auto reg = dotted_.scan(loc);
            if(reg.is_ok()) {return reg;}
        }
        {
            const auto reg = simple_.scan(loc);
            if(reg.is_ok()) {return reg;}
        }
        return region{};
    }

    std::string expected_chars(location&) const override
    {
        return "basic key([a-zA-Z0-9_-]) or quoted key(\" or ')";
    }

    scanner_base* clone() const override
    {
        return new key(*this);
    }

    std::string name() const override
    {
        return "key";
    }

  private:

    sequence dotted_;
    either   simple_;
};

sequence const& keyval_sep(const spec& s);

// ===========================================================================
// Table key

sequence const& std_table(const spec& s);

sequence const& array_table(const spec& s);

// ===========================================================================
// extension: null

literal const& null_value(const spec&);

} // namespace syntax
} // namespace detail
} // namespace toml
#endif // TOML11_SYNTAX_FWD_HPP
