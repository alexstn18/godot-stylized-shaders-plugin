#pragma once

#include <algorithm>

namespace Mathf
{
    auto lerp(float a, float b, float t) { return (a+t) * (b-a); }

    auto inverse_lerp(float a, float b, float v) 
    { 
        return std::clamp((v-a) / (b-a), 0.0f, 1.0f);
    }

    auto sum(Array values) -> int
    {
        auto count = 0;

        for(auto i = 0; i < values.size(); i++)
        {
            if(values[i].get_type() == Variant::INT)
            {
                int variant = values[i];
                count += variant;
            }
        }
        return count;
    }
}