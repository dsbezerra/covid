#include <myhtml/api.h>

myhtml_tree_attr_t * 
get_attribute_by_key(myhtml_tree_node_t *node, char *key) {
    myhtml_tree_attr_t *attr = myhtml_node_attribute_first(node);
    while (attr) {
        const char *name = myhtml_attribute_key(attr, 0);
        if (name && strcmp(key, name) == 0) {
            return attr;
        }
        attr = myhtml_attribute_next(attr);
    }
    return 0;
}

size_t
utf8_to_codepoint(u8 *data, size_t *codepoint) {
    size_t result = 0;
    
    
    if(!(data[0] & 0x80)) {      // 0xxxxxxx
        *codepoint = data[0];
        result = 1;
    } 
    else if((data[0] & 0xe0) == 0xc0) {  // 110xxxxx
        *codepoint  = (data[0] & 0x1f) << 6;
        *codepoint |= (data[1] & 0x3f); 
        
        result = 2;
    }
    else if((data[0] & 0xf0) == 0xe0) {  // 1110xxxx
        *codepoint  = ((data[0] & 0x0f) << 12);
        *codepoint |= ((data[1] & 0x3f) << 6);
        *codepoint |= (data[2] & 0x3f);
        
        result = 3;
    }
    
    return result;
}

u8 *
html_utf8_text_to_extended_ascii(char *src) {
    size_t size = strlen(src);
    u8 *result = (u8 *) malloc(size + 1);
    u8 *at = result;
    while (*src) {
        size_t codepoint;
        size_t adv = utf8_to_codepoint((u8 *)src, &codepoint);
        *at++ = (u8) codepoint;
        src += adv;
    }
    
    *at = '\0';
    
    return result;
}

char*
get_text_from_tag_in_node(myhtml_tree_node_t *node, int tag) {
    
    char *result = 0;
    
    myhtml_collection_t *list = myhtml_get_nodes_by_tag_id_in_scope(0, 0, node, tag, 0);
    if (list && list->list && list->length == 1) {
        myhtml_tree_node_t *text_node = myhtml_node_child(list->list[0]);
        if (text_node) {
            const char *text = myhtml_node_text(text_node, 0);
            if (text) {
                // Copy string and don't forget to free!!!
                result = (char *) malloc(strlen(text) + 1);
                strcpy(result, text);
            }
        }
    }
    
    myhtml_collection_destroy(list);
    
    return result;
}