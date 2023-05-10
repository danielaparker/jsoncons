// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_JSONSCHEMA_HPP
#define JSONCONS_JSONSCHEMA_JSONSCHEMA_HPP

#include <jsoncons_ext/jsonschema/keywords.hpp>
#include <jsoncons_ext/jsonschema/json_validator.hpp>
#include <jsoncons_ext/jsonschema/draft7/keyword_factory.hpp>


namespace jsoncons {
namespace jsonschema {

    template <class Json>
    using schema_draft7 = jsoncons::jsonschema::draft7::schema_draft7<Json>;

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& schema)
    {
        jsoncons::jsonschema::draft7::keyword_factory<Json> kwFactory{ jsoncons::jsonschema::draft7::default_uri_resolver<Json>()};
        kwFactory.load_root(schema);

        return kwFactory.get_schema();
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& schema, const URIResolver& resolver)
    {
        jsoncons::jsonschema::draft7::keyword_factory<Json> kwFactory(resolver);
        kwFactory.load_root(schema);

        return kwFactory.get_schema();
    }

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_JSONSCHEMA_HPP
