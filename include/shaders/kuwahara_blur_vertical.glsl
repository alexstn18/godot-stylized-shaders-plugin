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

    if(pixel.x >= size.x || pixel.y >= size.y) return;

    vec2 uv = pixel / size;
    vec2 texel = 1. / vec2(size);

    float sigma = 2.;
    vec4 sum = vec4(0.);
    float ksum = 0.;
    for(int dx = -params.radius; dx <= params.radius; ++dx)
    {
        vec2 sample_uv = clamp(uv + vec2(float(dx), 0.) * texel, vec2(0.), vec2(1.));
        vec4 v = texture(input_image, sample_uv);
        float w = gaussian(float(dx), sigma);
        sum += v * w;
        ksum += w;
    }
    vec3 g = sum / ksum;

    // eigen values
    float trace = g.x + g.y;
    float detpart = sqrt(max(0.0, (g.x - g.y)*(g.x - g.y) + 4.0*g.z*g.z));
    float lambda1 = 0.5 * (trace + detpart);
    float lambda2 = 0.5 * (trace - detpart);

    // eigen vector
    vec2 v = vec2(lambda1 - g.x, -g.z);
    vec2 t = (length(v) > 0.0) ? normalize(v) : vec2(0.0, 1.0);
    float phi = -atan(t.y, t.x);

    float A = (lambda1 + lambda2 > 0.0) ? (lambda1 - lambda2) / (lambda1 + lambda2) : 0.0;

    imageStore(output_image, pixel, vec4(t, phi, A));
}