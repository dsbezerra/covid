#include <myhtml/api.h>

struct server_response {
    u32 content_length;
    char body[2048 * 2048];
};

internal server_response *create_server_response(char *body, u32 content_length);
internal void destroy_server_response(server_response *response);

internal myhtml_tree_attr_t *get_attribute_by_key(myhtml_tree_node_t *node, char *key);
internal size_t utf8_to_codepoint(u8 *data, size_t *codepoint);
internal u8 *html_utf8_text_to_extended_ascii(char *src);
internal char *get_text_from_tag_in_node(myhtml_tree_node_t *node, int tag);