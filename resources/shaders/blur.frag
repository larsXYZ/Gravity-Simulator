uniform sampler2D texture;
uniform vec2 direction;  // (1/width, 0) for horizontal, (0, 1/height) for vertical

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 color = vec4(0.0);

    // 9-tap gaussian kernel
    color += texture2D(texture, uv - 4.0 * direction) * 0.0162;
    color += texture2D(texture, uv - 3.0 * direction) * 0.0540;
    color += texture2D(texture, uv - 2.0 * direction) * 0.1218;
    color += texture2D(texture, uv - 1.0 * direction) * 0.1903;
    color += texture2D(texture, uv)                    * 0.2354;
    color += texture2D(texture, uv + 1.0 * direction) * 0.1903;
    color += texture2D(texture, uv + 2.0 * direction) * 0.1218;
    color += texture2D(texture, uv + 3.0 * direction) * 0.0540;
    color += texture2D(texture, uv + 4.0 * direction) * 0.0162;

    gl_FragColor = color;
}
