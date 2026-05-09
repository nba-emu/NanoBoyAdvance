#ifndef TOML11_LOCATION_IMPL_HPP
#define TOML11_LOCATION_IMPL_HPP

#include "../fwd/location_fwd.hpp"
#include "../utility.hpp"
#include "../version.hpp"

namespace toml
{
namespace detail
{

TOML11_INLINE void location::advance(std::size_t n) noexcept
{
    assert(this->is_ok());
    if(this->location_ + n < this->source_->size())
    {
        this->advance_impl(n);
    }
    else
    {
        this->advance_impl(this->source_->size() - this->location_);

        assert(this->location_ == this->source_->size());
    }
}
TOML11_INLINE void location::retrace(/*restricted to n=1*/) noexcept
{
    assert(this->is_ok());
    if(this->location_ == 0)
    {
        this->location_ = 0;
        this->line_number_ = 1;
        this->column_number_ = 1;
    }
    else
    {
        this->retrace_impl();
    }
}

TOML11_INLINE bool location::eof() const noexcept
{
    assert(this->is_ok());
    return this->location_ >= this->source_->size();
}
TOML11_INLINE location::char_type location::current() const
{
    assert(this->is_ok());
    if(this->eof()) {return '\0';}

    assert(this->location_ < this->source_->size());
    return this->source_->at(this->location_);
}

TOML11_INLINE location::char_type location::peek()
{
    assert(this->is_ok());
    if(this->location_ >= this->source_->size())
    {
        return '\0';
    }
    else
    {
        return this->source_->at(this->location_ + 1);
    }
}

TOML11_INLINE std::string location::get_line() const
{
    assert(this->is_ok());
    const auto iter = std::next(this->source_->cbegin(), static_cast<difference_type>(this->location_));
    const auto riter = cxx::make_reverse_iterator(iter);

    const auto prev = std::find(riter, this->source_->crend(), char_type('\n'));
    const auto next = std::find(iter,  this->source_->cend(),  char_type('\n'));

    return make_string(std::next(prev.base()), next);
}

TOML11_INLINE std::size_t location::calc_column_number() const noexcept
{
    assert(this->is_ok());
    const auto iter  = std::next(this->source_->cbegin(), static_cast<difference_type>(this->location_));
    const auto riter = cxx::make_reverse_iterator(iter);
    const auto prev  = std::find(riter, this->source_->crend(), char_type('\n'));

    assert(prev.base() <= iter);
    return static_cast<std::size_t>(std::distance(prev.base(), iter) + 1); // 1-origin
}

TOML11_INLINE void location::advance_impl(const std::size_t n)
{
    assert(this->is_ok());
    assert(this->location_ + n <= this->source_->size());

    auto iter = this->source_->cbegin();
    std::advance(iter, static_cast<difference_type>(this->location_));

    for(std::size_t i=0; i<n; ++i)
    {
        const auto c = *iter;
        if(c == char_type('\n'))
        {
            this->line_number_  += 1;
            this->column_number_ = 1;
        }
        else
        {
            this->column_number_ += 1;
        }
        iter++;
    }
    this->location_ += n;
    return;
}
TOML11_INLINE void location::retrace_impl(/*n == 1*/)
{
    assert(this->is_ok());
    assert(this->location_ != 0);

    this->location_ -= 1;

    auto iter = this->source_->cbegin();
    std::advance(iter, static_cast<difference_type>(this->location_));
    if(*iter == '\n')
    {
        this->line_number_ -= 1;
        this->column_number_ = this->calc_column_number();
    }
    return;
}

TOML11_INLINE bool operator==(const location& lhs, const location& rhs) noexcept
{
    if( ! lhs.is_ok() || ! rhs.is_ok())
    {
        return (!lhs.is_ok()) && (!rhs.is_ok());
    }
    return lhs.source()       == rhs.source()      &&
           lhs.source_name()  == rhs.source_name() &&
           lhs.get_location() == rhs.get_location();
}
TOML11_INLINE bool operator!=(const location& lhs, const location& rhs)
{
    return !(lhs == rhs);
}

TOML11_INLINE location prev(const location& loc)
{
    location p(loc);
    p.retrace();
    return p;
}
TOML11_INLINE location next(const location& loc)
{
    location p(loc);
    p.advance(1);
    return p;
}

TOML11_INLINE location make_temporary_location(const std::string& str) noexcept
{
    location::container_type cont(str.size());
    std::transform(str.begin(), str.end(), cont.begin(),
        [](const std::string::value_type& c) {
            return cxx::bit_cast<location::char_type>(c);
        });
    return location(std::make_shared<const location::container_type>(
            std::move(cont)), "internal temporary");
}

TOML11_INLINE result<location, none_t>
find(const location& first, const location& last, const location::char_type val)
{
    return find_if(first, last, [val](const location::char_type c) {
            return c == val;
        });
}
TOML11_INLINE result<location, none_t>
rfind(const location& first, const location& last, const location::char_type val)
{
    return rfind_if(first, last, [val](const location::char_type c) {
            return c == val;
        });
}

TOML11_INLINE std::size_t
count(const location& first, const location& last, const location::char_type& c)
{
    if(first.source() != last.source())             { return 0; }
    if(first.get_location() >= last.get_location()) { return 0; }

    auto loc = first;
    std::size_t num = 0;
    while(loc.get_location() != last.get_location())
    {
        if(loc.current() == c)
        {
            num += 1;
        }
        loc.advance();
    }
    return num;
}

} // detail
} // toml
#endif // TOML11_LOCATION_HPP
