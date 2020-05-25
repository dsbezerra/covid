#ifdef VERTEX
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
out vec4 real_color;
out vec3 out_pos;
out vec3 out_normal;
void main() {
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
	real_color = color;
}
#endif
#ifdef FRAGMENT
out vec4 frag_color;
in vec4 real_color;
void main() {
    frag_color = real_color;
}
#endif