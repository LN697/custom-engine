#version 120
uniform sampler2D uTexture;
uniform int uEffect;

void main() {
    vec2 uv = gl_TexCoord[0].st;
    vec4 c = texture2D(uTexture, uv);
    if (uEffect == 1) {
        float gray = dot(c.rgb, vec3(0.299,0.587,0.114));
        c = vec4(vec3(gray), c.a);
    } else if (uEffect == 2) {
        c = vec4(vec3(1.0) - c.rgb, c.a);
    } else if (uEffect == 3) {
        vec3 s;
        s.r = dot(c.rgb, vec3(0.393,0.769,0.189));
        s.g = dot(c.rgb, vec3(0.349,0.686,0.168));
        s.b = dot(c.rgb, vec3(0.272,0.534,0.131));
        c = vec4(s, c.a);
    }
    gl_FragColor = c;
}
