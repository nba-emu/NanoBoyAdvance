+++
title = "toml spec"
type  = "docs"
weight = 70
+++

# TOML Language Version

You can specify the version of the TOML language and individual feature flags to use with `toml::parse` or `toml::format` through [`toml::spec`]({{< ref "docs/reference/spec#tomlspec" >}}).

## Specifying TOML Version

You can construct a [`toml::spec`]({{< ref "docs/reference/spec#tomlspec" >}}) from [`toml::semantic_version`]({{< ref "docs/reference/spec#tomlsemantic_version" >}}).

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec(toml::semantic_version(1, 1, 0));
    return 0;
}
```

However, to make this shorter, the `toml::spec::v()` function is provided.

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec = toml::spec::v(1, 1, 0);
    return 0;
}
```

If not specified explicitly, `toml::spec::default_version()` is used to construct with default values.

The default value depends on the version of toml11 and follows the latest version of the TOML language released at that time.

As of v4.4.0, TOML v1.1.0 has not been released yet, so the default TOML version is v1.0.0.

{{<hint warning>}}

Some features of TOML v1.1.0 are still under fairly lengthy discussion and may still be reverted.

If they are indeed reverted, toml11 will remove those features in a minor version upgrade or move them to a corresponding later version.

As such, any features related to future versions should be considered unstable.

{{</hint>}}

### Parsing with Version Specification

The overload of [`toml::parse`]({{< ref "docs/reference/parser#parse" >}}) takes a `toml::spec` following the file name.

This allows you to change the TOML version being used.

```cpp
#include <toml.hpp>

int main()
{
    toml::value input = toml::parse("input.toml", toml::spec::v(1, 1, 0));
    return 0;
}
```

### Serializing with Version Specification

The overload of [`toml::format`]({{< ref "docs/reference/serializer" >}}) takes a `toml::spec` following the `toml::value`.

This allows you to change the TOML version being used.

```cpp
#include <toml.hpp>

int main()
{
    toml::value v = toml::parse("input.toml", toml::spec::v(1, 1, 0));
    std::cout << toml::format(v, toml::spec::v(1, 1, 0)) << std::endl;
    return 0;
}
```

If a format is passed that is not permitted by the provided `toml::spec`, it will be ignored, and another format will be used as a fallback.

## Specifying Newly Added Features in TOML

With version upgrades in TOML, multiple new features are introduced, and it's possible to enable only some of them.

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec = toml::spec::v(1, 0, 0);

    // Allowing newlines in inline tables
    spec.v1_1_0_allow_newlines_in_inline_tables = true;

    toml::value input = toml::parse("input.toml", spec);
    return 0;
}
```

For a full list of all flags, refer to [`toml::spec`]({{< ref "docs/reference/spec#tomlspec" >}}).
