#include "covid.h"
#include "myhtml_utils.cpp"

// ...
card parse_card(myhtml_tree_node_t *node);
void set_card(app *application, card new_card);

static char *STATE_DESCRIPTIONS[] = {
    "Press U to get data",
    "Connecting...",
    "Receiving response...",
    "Response received.",
    "Reading response body...",
    "Parsing response body...",
    "Network error ocurred.",
};

// 
// App
//
app
init_app() {
    app result = {0};
    result.width = APP_WIDTH;
    result.height = APP_HEIGHT;
    result.current_state = State_NoData;
    result.running = true;
    return result;
}

void
app_set_state(app *application, int new_state) {
    assert(application);
    application->current_state = new_state;
}

char *
app_get_state_description(app *application) {
    assert(application);
    return STATE_DESCRIPTIONS[application->current_state];
}

bool
app_resize(app *application, int width, int height) {
    int old_width = application->width;
    int old_height = application->height;
    application->width = width;
    application->height = height;
    return old_width != width || old_height != height;
}

void
parse_page(app *application, server_response *page) {
    assert(application);
    assert(page);
    
    // basic init
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    // init tree
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    // parse html
    myhtml_parse(tree, MyENCODING_UTF_8, page->body, page->content_length);
    
    // parse html
    char *target_clazz = "card";
    myhtml_collection_t *collection = myhtml_get_nodes_by_attribute_value_whitespace_separated(tree, 0, 0, false, "class", strlen("class"), target_clazz, strlen(target_clazz), 0);
    if (collection && collection->list && collection->length) {
        for(size_t i = 0; i < collection->length; i++) {
            card new_card = parse_card(collection->list[i]);
            set_card(application, new_card);
        }
    }
    
    // release resources
    myhtml_collection_destroy(collection);
    
    /* Destroy  */
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    app_set_state(application, State_Processed);
}

//
// Cards
//
void
set_card(app *application, card new_card) {
    if (new_card.kind == Card_None) {
        return;
    }
    assert(application);
    
    new_card.height = (real32) application->height / (real32) array_count(application->cards);
    new_card.center_y = new_card.height * 0.4f + 6.f;
    
    application->cards[new_card.kind - 1] = new_card;
}

card *
get_card(app *application, int index) {
    assert(application);
    if (index >= 0 && index < array_count(application->cards)) {
        return &application->cards[index]; 
    }
    return 0;
}

card
parse_card(myhtml_tree_node_t *node) {
    card result = card{0};
    
    myhtml_tree_attr_t *attr = get_attribute_by_key(node, "class");
    if (attr) {
        const char *value = myhtml_attribute_value(attr, 0);
        while (*value) {
            // Look for class name preffix "bg-"
            if (value[0] == 'b' &&
                value[1] == 'g' &&
                value[2] == '-') {
                value += 3;
                if (!strncmp(value, "info", strlen("info"))) {
                    result.kind = Card_Total;
                    result.background_color = make_color(0x17a2b8);
                } else if (!strncmp(value, "warning", strlen("warning"))) {
                    result.kind = Card_Investigation;
                    result.background_color = make_color(0xffc107);
                } else if (!strncmp(value, "danger", strlen("danger"))) {
                    result.kind = Card_Confirmed;
                    result.background_color = make_color(0xdc3545);
                } else if (!strncmp(value, "success", strlen("success"))) {
                    result.kind = Card_Discarded;
                    result.background_color = make_color(0x28a745);
                }
            }
            ++value;
        }
        
        if (result.kind != Card_None) {
            // Get value from h2 tag
            myhtml_collection_t *h2_list = myhtml_get_nodes_by_tag_id_in_scope(0, 0, node, MyHTML_TAG_H2, 0);
            if (h2_list && h2_list->list && h2_list->length == 1) {
                attr = get_attribute_by_key(h2_list->list[0], "data-to");
                if (attr) {
                    result.value = atoi(myhtml_attribute_value(attr, 0));
                }
            }
            
            // Get text from p tag
            myhtml_collection_t *p_list= myhtml_get_nodes_by_tag_id_in_scope(0, 0, node, MyHTML_TAG_P, 0);
            if (p_list && p_list->list && p_list->length == 1) {
                myhtml_tree_node_t *text_node = myhtml_node_child(p_list->list[0]);
                if (text_node) {
                    const char *text = myhtml_node_text(text_node, 0);
                    if (text) {
                        result.label = html_utf8_text_to_extended_ascii((char *) text);
                    }
                }
            }
            
            myhtml_collection_destroy(h2_list);
            myhtml_collection_destroy(p_list);
        }
        
    }
    
    return result;
}

//
// Server response
//

static server_response *
create_server_response(char *body, int content_length) {
    server_response *result = new server_response;
    result->body = new char[content_length + 1];
    strcpy(result->body, body);
    result->content_length = content_length;
    return result;
}

static void
destroy_server_response(server_response *response) {
    if (response) {
        delete response;
    }
}
