#pragma once

#include <string>
#include <vector>

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

namespace Convert
{
    auto get_key_values(const Array& values, const String& key) -> Array
    {
        std::vector<std::string> internal_values;

        for(auto i = 0; i < values.size(); i++)
        {
            if(values[i].get_type() == Variant::DICTIONARY)
            {
                Dictionary variant = values[i];
                Array keys = variant.keys();

                if(keys.has(key))
                {
                    String value = variant[key];
                    internal_values.push_back(value.utf8().get_data());
                }
            }
        }

        Array converted_values;

        for(const auto& value : internal_values)
        {
            converted_values.append(value.c_str());
        }

        return converted_values;
    }
}