shader_type canvas_item;

uniform float outline_size = 1.0;
uniform vec4 outline_color : source_color = vec4(0.0, 0.0, 0.0, 1.0);

void fragment() {
    vec2 size = vec2(textureSize(TEXTURE, 0));
    vec2 uv = UV;
    vec4 base = texture(TEXTURE, uv);

    float diff = 0.0;
    diff += distance(base.rgb, texture(TEXTURE, uv + vec2(outline_size/size.x, 0.0)).rgb);
    diff += distance(base.rgb, texture(TEXTURE, uv - vec2(outline_size/size.x, 0.0)).rgb);
    diff += distance(base.rgb, texture(TEXTURE, uv + vec2(0.0, outline_size/size.y)).rgb);
    diff += distance(base.rgb, texture(TEXTURE, uv - vec2(0.0, outline_size/size.y)).rgb);

    if (diff > 0.2)
    {
        COLOR = outline_color;
    } 
    else 
    {
        COLOR = base;
    }
}
