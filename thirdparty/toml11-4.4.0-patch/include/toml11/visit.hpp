#ifndef TOML11_VISIT_HPP
#define TOML11_VISIT_HPP

#include "exception.hpp"
#include "traits.hpp"
#include "value.hpp"

namespace toml
{

namespace detail
{

template<typename F, typename ... Ts>
using visit_result_t = decltype(std::declval<F>()(std::declval<Ts>().as_boolean() ...));

template<typename F, typename T>
struct front_binder
{
    template<typename ... Args>
    auto operator()(Args&& ... args) -> decltype(std::declval<F>()(std::declval<T>(), std::forward<Args>(args)...))
    {
        return func(std::move(front), std::forward<Args>(args)...);
    }
    F func;
    T front;
};

template<typename F, typename T>
front_binder<cxx::remove_cvref_t<F>, cxx::remove_cvref_t<T>>
bind_front(F&& f, T&& t)
{
    return front_binder<cxx::remove_cvref_t<F>, cxx::remove_cvref_t<T>>{
        std::forward<F>(f), std::forward<T>(t)
    };
}

template<typename Visitor, typename TC, typename ... Args>
visit_result_t<Visitor, const basic_value<TC>&, Args...>
visit_impl(Visitor&& visitor, const basic_value<TC>& v, Args&& ... args);

template<typename Visitor, typename TC, typename ... Args>
visit_result_t<Visitor, basic_value<TC>&, Args...>
visit_impl(Visitor&& visitor, basic_value<TC>& v, Args&& ... args);

template<typename Visitor, typename TC, typename ... Args>
visit_result_t<Visitor, basic_value<TC>, Args...>
visit_impl(Visitor&& visitor, basic_value<TC>&& v, Args&& ... args);


template<typename Visitor>
visit_result_t<Visitor> visit_impl(Visitor&& visitor)
{
    return visitor();
}

template<typename Visitor, typename TC, typename ... Args>
visit_result_t<Visitor, basic_value<TC>&, Args...>
visit_impl(Visitor&& visitor, basic_value<TC>& v, Args&& ... args)
{
    switch(v.type())
    {
        case value_t::boolean        : {return visit_impl(bind_front(visitor, std::ref(v.as_boolean        ())), std::forward<Args>(args)...);}
        case value_t::integer        : {return visit_impl(bind_front(visitor, std::ref(v.as_integer        ())), std::forward<Args>(args)...);}
        case value_t::floating       : {return visit_impl(bind_front(visitor, std::ref(v.as_floating       ())), std::forward<Args>(args)...);}
        case value_t::string         : {return visit_impl(bind_front(visitor, std::ref(v.as_string         ())), std::forward<Args>(args)...);}
        case value_t::offset_datetime: {return visit_impl(bind_front(visitor, std::ref(v.as_offset_datetime())), std::forward<Args>(args)...);}
        case value_t::local_datetime : {return visit_impl(bind_front(visitor, std::ref(v.as_local_datetime ())), std::forward<Args>(args)...);}
        case value_t::local_date     : {return visit_impl(bind_front(visitor, std::ref(v.as_local_date     ())), std::forward<Args>(args)...);}
        case value_t::local_time     : {return visit_impl(bind_front(visitor, std::ref(v.as_local_time     ())), std::forward<Args>(args)...);}
        case value_t::array          : {return visit_impl(bind_front(visitor, std::ref(v.as_array          ())), std::forward<Args>(args)...);}
        case value_t::table          : {return visit_impl(bind_front(visitor, std::ref(v.as_table          ())), std::forward<Args>(args)...);}
        case value_t::empty          : break;
        default: break;
    }
    throw type_error(format_error("[error] toml::visit: toml::basic_value "
            "does not have any valid type.", v.location(), "here"), v.location());
}

template<typename Visitor, typename TC, typename ... Args>
visit_result_t<Visitor, const basic_value<TC>&, Args...>
visit_impl(Visitor&& visitor, const basic_value<TC>& v, Args&& ... args)
{
    switch(v.type())
    {
        case value_t::boolean        : {return visit_impl(bind_front(visitor, std::cref(v.as_boolean        ())), std::forward<Args>(args)...);}
        case value_t::integer        : {return visit_impl(bind_front(visitor, std::cref(v.as_integer        ())), std::forward<Args>(args)...);}
        case value_t::floating       : {return visit_impl(bind_front(visitor, std::cref(v.as_floating       ())), std::forward<Args>(args)...);}
        case value_t::string         : {return visit_impl(bind_front(visitor, std::cref(v.as_string         ())), std::forward<Args>(args)...);}
        case value_t::offset_datetime: {return visit_impl(bind_front(visitor, std::cref(v.as_offset_datetime())), std::forward<Args>(args)...);}
        case value_t::local_datetime : {return visit_impl(bind_front(visitor, std::cref(v.as_local_datetime ())), std::forward<Args>(args)...);}
        case value_t::local_date     : {return visit_impl(bind_front(visitor, std::cref(v.as_local_date     ())), std::forward<Args>(args)...);}
        case value_t::local_time     : {return visit_impl(bind_front(visitor, std::cref(v.as_local_time     ())), std::forward<Args>(args)...);}
        case value_t::array          : {return visit_impl(bind_front(visitor, std::cref(v.as_array          ())), std::forward<Args>(args)...);}
        case value_t::table          : {return visit_impl(bind_front(visitor, std::cref(v.as_table          ())), std::forward<Args>(args)...);}
        case value_t::empty          : break;
        default: break;
    }
    throw type_error(format_error("[error] toml::visit: toml::basic_value "
            "does not have any valid type.", v.location(), "here"), v.location());
}

template<typename Visitor, typename TC, typename ... Args>
visit_result_t<Visitor, basic_value<TC>, Args...>
visit_impl(Visitor&& visitor, basic_value<TC>&& v, Args&& ... args)
{
    switch(v.type())
    {
        case value_t::boolean        : {return visit_impl(bind_front(visitor, std::move(v.as_boolean        ())), std::forward<Args>(args)...);}
        case value_t::integer        : {return visit_impl(bind_front(visitor, std::move(v.as_integer        ())), std::forward<Args>(args)...);}
        case value_t::floating       : {return visit_impl(bind_front(visitor, std::move(v.as_floating       ())), std::forward<Args>(args)...);}
        case value_t::string         : {return visit_impl(bind_front(visitor, std::move(v.as_string         ())), std::forward<Args>(args)...);}
        case value_t::offset_datetime: {return visit_impl(bind_front(visitor, std::move(v.as_offset_datetime())), std::forward<Args>(args)...);}
        case value_t::local_datetime : {return visit_impl(bind_front(visitor, std::move(v.as_local_datetime ())), std::forward<Args>(args)...);}
        case value_t::local_date     : {return visit_impl(bind_front(visitor, std::move(v.as_local_date     ())), std::forward<Args>(args)...);}
        case value_t::local_time     : {return visit_impl(bind_front(visitor, std::move(v.as_local_time     ())), std::forward<Args>(args)...);}
        case value_t::array          : {return visit_impl(bind_front(visitor, std::move(v.as_array          ())), std::forward<Args>(args)...);}
        case value_t::table          : {return visit_impl(bind_front(visitor, std::move(v.as_table          ())), std::forward<Args>(args)...);}
        case value_t::empty          : break;
        default: break;
    }
    throw type_error(format_error("[error] toml::visit: toml::basic_value "
            "does not have any valid type.", v.location(), "here"), v.location());
}

} // detail

template<typename Visitor, typename ... Args>
detail::visit_result_t<Visitor, Args...>
visit(Visitor&& visitor, Args&& ... args)
{
    return detail::visit_impl(std::forward<Visitor>(visitor), std::forward<Args>(args)...);
}

} // toml
#endif // TOML11_VISIT_HPP
