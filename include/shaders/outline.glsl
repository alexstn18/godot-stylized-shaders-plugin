#[compute]
#version 450

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba16f, binding = 0, set = 0) uniform image2D color_image;
layout(binding = 0, set = 1) uniform sampler2D depth_texture;

layout(push_constant, std430) uniform Params 
{
	vec2 raster_size;
	float inv_proj_2w;
    float inv_proj_3w;
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

void main() 
{
	ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
	vec2 size = params.raster_size;
	vec2 uv = pixel / size;
	
	if (pixel.x >= size.x || pixel.y >= size.y) return;
	
	vec4 color = imageLoad(color_image, pixel);
    float d = linear_depth(uv);
    vec4 b3d = vec4(linear_depth(uv + vec2(.002, 0.)), linear_depth(uv - vec2(.002, 0.)), linear_depth(uv + vec2(0., .002)), linear_depth(uv - vec2(0., .002))); 
    float outline = absdiff(b3d.x, d)
                    + absdiff(b3d.y, d)
                    + absdiff(b3d.z, d)
                    + absdiff(b3d.w, d);
    outline = step(.01, fract(outline));

    color.rgb *= vec3(1. - outline);
    color.rgb += outline * vec3(0.);

	imageStore(color_image, pixel, color);
}