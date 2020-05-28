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

u8 *
html_utf8_text_to_extended_ascii(char *src) {
    size_t size = strlen(src);
    u8 *result = (u8 *) malloc(size + 1);
    u8 *at = result;
    while (*src) {
        u8 c = (u8) *src;
        if ((c & 0x80) == 0x00) {
            *at++ = *src++;
            continue;
        }
        
        // Handle only necessary codepoints
        if ((c & 0xE0) == 0xC0 && c == 0xC3) {
            *src++;
            switch ((u8) *src) {
                case 0xA3:   // ç
                case 0xA7:   // ã
                case 0xB5: { // õ
                    *at++ = (u32) *src + 0x40;
                } break;
                
                default: {
                    
                } break;
            }
            *src++;
        }
    }
    
    *at = '\0';
    
    return result;
}
