#[compute]
#version 450

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, set = 0) uniform sampler2D input_image;
layout(rgba16f, binding = 1, set = 0) uniform image2D output_image;

layout(push_constant, std430) uniform Params
{
    vec2 raster_size;
    float radius;
} params;

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    vec2 size = params.raster_size;

    if (pixel.x >= size.x || pixel.y >= size.y)
        return;

    vec2 uv = pixel / size;
    float x = params.radius;
    float y = params.radius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(input_image, vec2(uv.x - x, uv.y + y)).rgb;
    vec3 b = texture(input_image, vec2(uv.x, uv.y + y)).rgb;
    vec3 c = texture(input_image, vec2(uv.x + x, uv.y + y)).rgb;
    vec3 d = texture(input_image, vec2(uv.x - x, uv.y)).rgb;
    vec3 e = texture(input_image, vec2(uv.x, uv.y)).rgb;
    vec3 f = texture(input_image, vec2(uv.x + x, uv.y)).rgb;
    vec3 g = texture(input_image, vec2(uv.x - x, uv.y - y)).rgb;
    vec3 h = texture(input_image, vec2(uv.x, uv.y - y)).rgb;
    vec3 i = texture(input_image, vec2(uv.x + x, uv.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    vec3 upsampled_color = e * 4.0;
    upsampled_color += (b + d + f + h) * 2.0;
    upsampled_color += (a + c + g + i);
    upsampled_color *= 1.0 / 16.0;

    imageStore(output_image, pixel, vec4(upsampled_color, 1.0));
}