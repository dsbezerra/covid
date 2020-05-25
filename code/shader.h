/* date = May 21st 2020 7:39 pm */

struct shader {
    GLuint program;
    
    GLint projection_loc;
	GLint view_loc;
    GLint texture_loc;
    
    GLint position_loc;
    GLint color_loc;
    GLint uv_loc;
    GLint z_index_loc;
};
