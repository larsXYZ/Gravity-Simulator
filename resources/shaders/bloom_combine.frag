uniform sampler2D scene;
uniform sampler2D bloom;
uniform float bloomIntensity;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 sceneColor = texture2D(scene, uv);
    vec4 bloomColor = texture2D(bloom, uv);
    gl_FragColor = sceneColor + bloomColor * bloomIntensity;
}
