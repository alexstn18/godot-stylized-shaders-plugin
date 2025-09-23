#[compute]
#version 450

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba16f, binding = 0, set = 0) uniform image2D color_image;
layout(binding = 0, set = 1) uniform sampler2D depth_texture;

layout(push_constant, std430) uniform Params 
{
	vec3 outline_color;
	float _pad0;
	vec2 raster_size;
	float inv_proj_2w;
    float inv_proj_3w;
	float outline_width;
	float outline_mul;
	float delta_time;
	float seed;
	float jitter_enabled;
	float _pad1;
} params;

const float TAU = 6.2831;

float hash(vec2 pixel)
{
	return fract(sin(dot(pixel, vec2(12.9898,78.233)) + params.seed) * 43758.5453);
}

vec2 jitter(vec2 uv, int sample_idx)
{
	float h = hash(uv + float(sample_idx)) * TAU;
	float t = params.delta_time * 2.;
	float r = .5 + .5 * sin(t + h);
	return (vec2(cos(h), sin(h)) * r * 0.5);
}

float linear_depth(vec2 uv)
{
	float depth = texture(depth_texture, uv).r;
	depth = 1. / (depth * params.inv_proj_2w + params.inv_proj_3w);
	depth = fract(depth / 50.);
	return -depth;
}

float absdiff(float a, float b)
{
    return abs(abs(a) - abs(b));
}

float sample_edge(vec2 uv)
{
	float d = linear_depth(uv);
	vec2 w0 = uv + vec2(params.outline_width, 0.);
    vec2 w1 = uv - vec2(params.outline_width, 0.);
	vec2 w2 = uv + vec2(0., params.outline_width);
	vec2 w3 = uv + vec2(0., params.outline_width);
	if(params.jitter_enabled == 1.)
	{
		vec2 jitter0 = jitter(uv, 0) * .1;
		vec2 jitter1 = jitter(uv, 1) * .1;
		vec2 jitter2 = jitter(uv, 2) * .1;
		vec2 jitter3 = jitter(uv, 3) * .1;

		w0 += jitter0; w1 += jitter1; w2 += jitter2; w3 += jitter3;
	}
    vec4 b3d = vec4(linear_depth(w0), linear_depth(w1), linear_depth(w2), linear_depth(w3)); 
	float outline = absdiff(b3d.x, d)
                    + absdiff(b3d.y, d)
                    + absdiff(b3d.z, d)
                    + absdiff(b3d.w, d);
	return fract(outline);
}

vec2 offsets[4] = vec2[](
    vec2(0.25, 0.25),
    vec2(-0.25, 0.25),
    vec2(0.25, -0.25),
    vec2(-0.25, -0.25)
);

void main() 
{
	ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
	vec2 size = params.raster_size;
	
	if (pixel.x >= size.x || pixel.y >= size.y) return;
	
	vec2 uv = pixel / size;
	vec2 texel = 1. / size;

	vec4 color = imageLoad(color_image, pixel);
	float edge_sum = 0.;
	for(int i = 0; i < 4; i++)
	{
		edge_sum += sample_edge(uv + offsets[i] * texel);
	}
	float edge_strength = edge_sum / 4.;
	float outline = smoothstep(params.outline_mul - 0.05,
                           params.outline_mul + 0.05,
                           edge_strength);

    color.rgb *= vec3(1. - outline);
    color.rgb += outline * params.outline_color;

	imageStore(color_image, pixel, color);
}