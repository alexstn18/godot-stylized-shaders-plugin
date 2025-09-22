#[compute]
#version 450

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba16f, binding = 0, set = 0) uniform image2D color_image;
layout(binding = 0, set = 1) uniform sampler2D depth_texture;

layout(push_constant, std430) uniform Params 
{
	vec2 raster_size;
	vec2 reserved;
	float inv_proj_2w;
    float inv_proj_3w;
	float outline_width;
	float outline_mul;
	vec3 outline_color;
	float padding;
} params;

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
    vec4 b3d = vec4(linear_depth(uv + vec2(params.outline_width, 0.)), linear_depth(uv - vec2(params.outline_width, 0.)), linear_depth(uv + vec2(0., params.outline_width)), linear_depth(uv - vec2(0., params.outline_width))); 
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