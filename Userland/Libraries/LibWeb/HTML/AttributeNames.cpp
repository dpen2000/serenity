/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/HTML/AttributeNames.h>

namespace Web {
namespace HTML {
namespace AttributeNames {

#define __ENUMERATE_HTML_ATTRIBUTE(name) FlyString name;
ENUMERATE_HTML_ATTRIBUTES
#undef __ENUMERATE_HTML_ATTRIBUTE

[[gnu::constructor]] static void initialize()
{
    static bool s_initialized = false;
    if (s_initialized)
        return;

#define __ENUMERATE_HTML_ATTRIBUTE(name) \
    name = #name;
    ENUMERATE_HTML_ATTRIBUTES
#undef __ENUMERATE_HTML_ATTRIBUTE

    // NOTE: Special cases for C++ keywords.
    class_ = "class";
    for_ = "for";
    default_ = "default";
    char_ = "char";

    // NOTE: Special cases for attributes with dashes in them.
    accept_charset = "accept-charset";
    http_equiv = "http-equiv";

    s_initialized = true;
}

}

// https://html.spec.whatwg.org/#boolean-attribute
bool is_boolean_attribute(FlyString const& attribute)
{
    // NOTE: This is the list of attributes from https://html.spec.whatwg.org/#attributes-3
    //       with a Value column value of "Boolean attribute".
    return attribute.is_one_of(
        AttributeNames::allowfullscreen,
        AttributeNames::async,
        AttributeNames::autofocus,
        AttributeNames::autoplay,
        AttributeNames::checked,
        AttributeNames::controls,
        AttributeNames::default_,
        AttributeNames::defer,
        AttributeNames::disabled,
        AttributeNames::formnovalidate,
        AttributeNames::inert,
        AttributeNames::ismap,
        AttributeNames::itemscope,
        AttributeNames::loop,
        AttributeNames::multiple,
        AttributeNames::muted,
        AttributeNames::nomodule,
        AttributeNames::novalidate,
        AttributeNames::open,
        AttributeNames::playsinline,
        AttributeNames::readonly,
        AttributeNames::required,
        AttributeNames::reversed,
        AttributeNames::selected);
}

}
}
