#ifndef TOML11_REGION_IMPL_HPP
#define TOML11_REGION_IMPL_HPP

#include "../fwd/region_fwd.hpp"
#include "../utility.hpp"

#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>

namespace toml
{
namespace detail
{

// a value defined in [first, last).
// Those source must be the same. Instread, `region` does not make sense.
TOML11_INLINE region::region(const location& first, const location& last)
    : source_(first.source()), source_name_(first.source_name()),
      length_(last.get_location() - first.get_location()),
      first_(first.get_location()),
      first_line_(first.line_number()),
      first_column_(first.column_number()),
      last_(last.get_location()),
      last_line_(last.line_number()),
      last_column_(last.column_number())
{
    assert(first.source()      == last.source());
    assert(first.source_name() == last.source_name());
}

    // shorthand of [loc, loc+1)
TOML11_INLINE region::region(const location& loc)
    : source_(loc.source()), source_name_(loc.source_name()), length_(0),
      first_line_(0), first_column_(0), last_line_(0), last_column_(0)
{
    // if the file ends with LF, the resulting region points no char.
    if(loc.eof())
    {
        if(loc.get_location() == 0)
        {
            this->length_       = 0;
            this->first_        = 0;
            this->first_line_   = 0;
            this->first_column_ = 0;
            this->last_         = 0;
            this->last_line_    = 0;
            this->last_column_  = 0;
        }
        else
        {
            const auto first = prev(loc);
            this->first_        = first.get_location();
            this->first_line_   = first.line_number();
            this->first_column_ = first.column_number();
            this->last_         = loc.get_location();
            this->last_line_    = loc.line_number();
            this->last_column_  = loc.column_number();
            this->length_       = 1;
        }
    }
    else
    {
        this->first_        = loc.get_location();
        this->first_line_   = loc.line_number();
        this->first_column_ = loc.column_number();
        this->last_         = loc.get_location() + 1;
        this->last_line_    = loc.line_number();
        this->last_column_  = loc.column_number() + 1;
        this->length_       = 1;
    }
}

TOML11_INLINE region::char_type region::at(std::size_t i) const
{
    if(this->last_ <= this->first_ + i)
    {
        throw std::out_of_range("range::at: index " + std::to_string(i) +
                " exceeds length " + std::to_string(this->length_));
    }
    const auto iter = std::next(this->source_->cbegin(),
            static_cast<difference_type>(this->first_ + i));
    return *iter;
}

TOML11_INLINE region::const_iterator region::begin() const noexcept
{
    return std::next(this->source_->cbegin(),
            static_cast<difference_type>(this->first_));
}
TOML11_INLINE region::const_iterator region::end() const noexcept
{
    return std::next(this->source_->cbegin(),
            static_cast<difference_type>(this->last_));
}
TOML11_INLINE region::const_iterator region::cbegin() const noexcept
{
    return std::next(this->source_->cbegin(),
            static_cast<difference_type>(this->first_));
}
TOML11_INLINE region::const_iterator region::cend() const noexcept
{
    return std::next(this->source_->cbegin(),
            static_cast<difference_type>(this->last_));
}

TOML11_INLINE std::string region::as_string() const
{
    if(this->is_ok())
    {
        const auto begin = std::next(this->source_->cbegin(), static_cast<difference_type>(this->first_));
        const auto end   = std::next(this->source_->cbegin(), static_cast<difference_type>(this->last_ ));
        return ::toml::detail::make_string(begin, end);
    }
    else
    {
        return std::string("");
    }
}

TOML11_INLINE std::pair<std::string, std::size_t>
region::take_line(const_iterator begin, const_iterator end) const
{
    // To omit long line, we cap region by before/after 30 chars
    const auto dist_before = std::distance(source_->cbegin(), begin);
    const auto dist_after  = std::distance(end, source_->cend());

    const const_iterator capped_begin = (dist_before <= 30) ? source_->cbegin() : std::prev(begin, 30);
    const const_iterator capped_end   = (dist_after  <= 30) ? source_->cend()   : std::next(end,   30);

    const auto lf = char_type('\n');
    const auto lf_before = std::find(cxx::make_reverse_iterator(begin),
                                     cxx::make_reverse_iterator(capped_begin), lf);
    const auto lf_after  = std::find(end, capped_end, lf);

    auto offset = static_cast<std::size_t>(std::distance(lf_before.base(), begin));

    std::string retval = make_string(lf_before.base(), lf_after);

    if(lf_before.base() != source_->cbegin() && *lf_before != lf)
    {
        retval = "... " + retval;
        offset += 4;
    }

    if(lf_after != source_->cend() && *lf_after != lf)
    {
        retval = retval + " ...";
    }

    return std::make_pair(retval, offset);
}

TOML11_INLINE std::vector<std::pair<std::string, std::size_t>> region::as_lines() const
{
    assert(this->is_ok());
    if(this->length_ == 0)
    {
        return std::vector<std::pair<std::string, std::size_t>>{
            std::make_pair("", std::size_t(0))
        };
    }

    // Consider the following toml file
    // ```
    // array = [
    //   1, 2, 3,
    // ] # comment
    // ```
    // and the region represnets
    // ```
    //         [
    //   1, 2, 3,
    // ]
    // ```
    // but we want to show the following.
    // ```
    // array = [
    //   1, 2, 3,
    // ] # comment
    // ```
    // So we need to find LFs before `begin` and after `end`.
    //
    // But, if region ends with LF, it should not include the next line.
    // ```
    // a = 42
    //     ^^^- with the last LF
    // ```
    // So we start from `end-1` when looking for LF.

    const auto begin_idx = static_cast<difference_type>(this->first_);
    const auto end_idx   = static_cast<difference_type>(this->last_) - 1;

    // length_ != 0, so begin < end. then begin <= end-1
    assert(begin_idx <= end_idx);

    const auto begin = std::next(this->source_->cbegin(), begin_idx);
    const auto end   = std::next(this->source_->cbegin(), end_idx);

    assert(this->first_line_number() <= this->last_line_number());

    if(this->first_line_number() == this->last_line_number())
    {
        return std::vector<std::pair<std::string, std::size_t>>{
            this->take_line(begin, end)
        };
    }

    // we have multiple lines. `begin` and `end` points different lines.
    // that means that there is at least one `LF` between `begin` and `end`.

    const auto after_begin = std::distance(begin, this->source_->cend());
    const auto before_end  = std::distance(this->source_->cbegin(), end);

    const_iterator capped_file_end   = this->source_->cend();
    const_iterator capped_file_begin = this->source_->cbegin();
    if(60 < after_begin) {capped_file_end   = std::next(begin, 50);}
    if(60 < before_end)  {capped_file_begin = std::prev(end,   50);}

    const auto lf = char_type('\n');
    const auto first_line_end  = std::find(begin, capped_file_end, lf);
    const auto last_line_begin = std::find(capped_file_begin, end, lf);

    const auto first_line = this->take_line(begin, first_line_end);
    const auto last_line  = this->take_line(last_line_begin, end);

    if(this->first_line_number() + 1 == this->last_line_number())
    {
        return std::vector<std::pair<std::string, std::size_t>>{
            first_line, last_line
        };
    }
    else
    {
        return std::vector<std::pair<std::string, std::size_t>>{
            first_line, std::make_pair("...", 0), last_line
        };
    }
}

} // namespace detail
} // namespace toml
#endif // TOML11_REGION_IMPL_HPP
