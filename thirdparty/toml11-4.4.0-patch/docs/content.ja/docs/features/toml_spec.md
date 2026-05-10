+++
title = "toml spec"
type  = "docs"
weight = 70
+++

# TOML言語バージョン

[`toml::spec`]({{< ref "docs/reference/spec#tomlspec" >}})
によって、 `toml::parse` や `toml::format` で使用するTOML言語のバージョンや、個別の機能フラグを指定することができます。

## TOMLのバージョンを指定する

[`toml::spec`]({{< ref "docs/reference/spec#tomlspec" >}})
は
[`toml::semantic_version`]({{< ref "docs/reference/spec#tomlsemantic_version" >}})
から構築できます。

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec(toml::semantic_version(1, 1, 0));
    return 0;
}
```

ですがこれは長いので、`toml::spec::v()`関数が用意されています。

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec = toml::spec::v(1, 1, 0);
    return 0;
}
```

特に指定しない場合、デフォルトの値で構築する `toml::spec::default_version()` が使用されます。

デフォルトの値はtoml11のバージョンによって変わりますが、その時点でリリースされているTOML言語の最新バージョンに追従します。

v4.4.0現在、TOML v1.1.0はまだリリースされていないため、デフォルトのTOMLバージョンはv1.0.0です。

{{<hint warning>}}

TOML v1.1.0の一部の機能にはかなり長い議論が続いており、まだ差し戻される可能性があります。

実際に差し戻された場合、toml11はマイナーバージョンアップでそれらの機能を削除、もしくは対応するそれ以降のバージョンに移動します。

そのような意味で、将来のバージョンに関する機能は全て不安定なものと考えてください。

{{</hint>}}

### バージョン指定でパースする

[`toml::parse`]({{< ref "docs/reference/parser" >}})
のオーバーロードは、ファイル名に続いて`toml::spec`を受け取ります。

これによって、使用するTOMLバージョンを変更できます。

```cpp
#include <toml.hpp>

int main()
{
    toml::value input = toml::parse("input.toml", toml::spec::v(1, 1, 0));
    return 0;
}
```

### バージョン指定でシリアライズする

[`toml::format`]({{< ref "docs/reference/serializer" >}})
のオーバーロードは、 `toml::value` に続いて `toml::spec` を受け取ります。

これによって、使用するTOMLバージョンを変更できます。

```cpp
#include <toml.hpp>

int main()
{
    toml::value v = toml::parse("input.toml", toml::spec::v(1, 1, 0));
    std::cout << toml::format(v, toml::spec::v(1, 1, 0)) << std::endl;
    return 0;
}
```

もしフォーマット変数などによって指定されているフォーマットが渡された `toml::spec`
では許可されていないものだった場合、指定は無視されて他のフォーマットにフォールバックされます。

## TOMLに追加された新機能を個別に指定する

TOMLのバージョンアップで追加された機能は複数あり、そのうちの一部だけを有効にすることが可能です。

```cpp
#include <toml.hpp>

int main()
{
    toml::spec spec = toml::spec::v(1, 0, 0);

    // インラインテーブル内での改行を許可
    spec.v1_1_0_allow_newlines_in_inline_tables = true;

    toml::value input = toml::parse("input.toml", spec);
    return 0;
}
```

全てのフラグのリストは、
[`toml::spec`]({{< ref "docs/reference/spec#tomlspec" >}})
を参照してください。
