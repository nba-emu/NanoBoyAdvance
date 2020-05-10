// Credits to Talarubi and Near for the color correction algorithm.
// https://byuu.net/video/color-emulation
uniform sampler2D tex;
varying vec2 uv;

void main(void) {
  vec4 color = texture2D(tex, uv);
  color.rgb = pow(color.rgb, vec3(4.0));
  gl_FragColor.rgb = pow(vec3(color.r + 0.196 * color.g,
                              0.039 * color.r + 0.901 * color.g + 0.117 * color.b,
                              0.196 * color.r + 0.039 * color.g + 0.862 * color.b), vec3(1.0/2.2));
  gl_FragColor.a = 1.0;
}