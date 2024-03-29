#include "covid.h"
#include "covid_html.cpp"
#include "covid_gui.cpp"
#include "covid_font.cpp"
#include "covid_opengl.cpp"
#include "covid_shader.cpp"
#include "covid_draw.cpp"

#include "imgui/imgui.h"

global_variable char *STATE_DESCRIPTIONS[] = {
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
internal app *
app_init() {
    app *result = (app *) malloc(sizeof(app));
    result->width = APP_WIDTH;
    result->height = APP_HEIGHT;
    result->current_state = State_NoData;
    result->running = true;
    return result;
}

internal void
app_init_fonts(app *application) {
    assert(application);
    
    application->font_small = load_font("./data/fonts/Inconsolata-Regular.ttf", 16.f);
    application->font_label = load_font("./data/fonts/Inconsolata-Regular.ttf", 28.f);
    application->font_value = load_font("./data/fonts/Inconsolata-Bold.ttf", 24.f);
}

internal void
app_init_gui(app *application) {
    gui_init(application);
}

internal void
app_free(app *application) {
    assert(application);
    
    if (open_gl) {
        VirtualFree(open_gl, 0, MEM_RELEASE);
    }
    
    free(application);
}

internal void
app_set_clear_color(v4 color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

internal void
app_update(app *application) {
    timer_interval *change_clear_color_interval = &application->change_clear_color_interval;
    timer_interval *frame_time_render_rate = &application->frame_time_render_rate;
    
    gui_state gui = application->gui;
    if (gui.show_debug_info && timer_increment(frame_time_render_rate, application->dt)) {
        application->fps_to_draw = application->fps;
        application->ms_per_frame_to_draw = application->dt;
    }
}

internal void
app_gui_tick(app *application) {
    gui_tick(application);
}

internal void
app_draw_processed(app *application) {
    
    real32 ypos = 0.f;
    real32 card_height = (real32) application->height / (real32) array_count(application->cards);
    
    real32 margin = application->width * 0.05f;
    
    loaded_font label_font = application->font_label;
    loaded_font value_font = application->font_value;
    loaded_font small_font = application->font_small;
    
    for (int i = 0; i < array_count(application->cards); i++) {
        card *card = card_get(application, i);
        if (!card || card->kind == Card_None) {
            continue;
        }
        
        draw_quad(0.f, ypos, (real32) application->width, ypos + card_height, card->background_color);
        
        
        real32 card_margin = card_height * 0.1f;
        real32 card_center_y = card_height * 0.4f + 6.f;
        draw_text(margin*2.f, ypos + card_center_y, card->label, &label_font, make_color(1.f, 1.f, 1.f));
        
        char value[256];
        wsprintf(value, "%d", card->value);
        draw_text(margin*2.f, ypos + card_center_y + 24.f, (u8*) value, &value_font, make_color(1.f, 1.f, 1.f));
        
        real32 width = get_text_width(&small_font, (char *) card->last_updated_at);
        real32 x = application->width - width - margin * .5f;
        draw_text(x, ypos + card_height - 16.f, card->last_updated_at, &small_font, make_color(1.f, 1.f, 1.f, .6f));
        ypos += card_height;
    }
}

internal void
app_draw(app *application) {
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, application->width, application->height);
    
    render_right_handed(application->width, application->height);
    
    // Draw current app state
    int state = application->current_state;
    switch (state) {
        case State_Processed: {
            app_draw_processed(application);
        } break;
        
        default: {
            
            if (state >= 0 && state < State_Count) {
                real32 margin = application->width * 0.05f;
                
                loaded_font value_font = application->font_value;
                loaded_font label_font = application->font_label;
                
                char *desc = app_get_state_description(application);
                draw_text(margin, application->height / 2.0f, (u8*) desc, &value_font, make_color(1.f, 1.f, 1.f));
                
                switch (state) {
                    case State_NetworkError: {
                        draw_text(margin, (application->height / 2.0f) + 24.f, application->loaded_page, &label_font, make_color(1.f, 0.f, 0.f));
                    } break;
                    
                    default: {
                    } break;
                }
            }
            
        } break;
    }
}

internal dimension
app_get_dimensions(app *application) {
    dimension result = {};
    
    if (!application) {
        result.width = APP_WIDTH;
        result.height = APP_HEIGHT;
    } else {
        result.width = application->width;
        result.height = application->height;
    }
    
    return result;
}

internal void
app_set_state(app *application, u32 new_state) {
    assert(application);
    assert(new_state < State_Count);
    
    application->current_state = new_state;
}

internal char *
app_get_state_description(app *application) {
    assert(application);
    
    return STATE_DESCRIPTIONS[application->current_state];
}

internal void
app_parse_page(app *application, server_response *page, u8 *page_url) {
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
    char *target_class = "card";
    myhtml_collection_t *collection = myhtml_get_nodes_by_attribute_value_whitespace_separated(tree, 0, 0, false, "class", strlen("class"), target_class, strlen(target_class), 0);
    if (collection && collection->list && collection->length) {
        for(size_t i = 0; i < collection->length; i++) {
            card new_card = card_parse(collection->list[i]);
            card_set(application, new_card);
        }
    }
    
    // release resources
    myhtml_collection_destroy(collection);
    
    /* Destroy  */
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    app_set_state(application, State_Processed);
    application->loaded_page = page_url;
}

//
// Cards
//

internal void
card_set(app *application, card new_card) {
    if (new_card.kind == Card_None) {
        return;
    }
    
    assert(application);
    
    new_card.height = (real32) application->height / (real32) array_count(application->cards);
    new_card.center_y = new_card.height * 0.4f + 6.f;
    
    application->cards[new_card.kind - 1] = new_card;
}

internal card *
card_get(app *application, u32 card_index) {
    assert(application);
    assert(card_index < array_count(application->cards));
    
    if (card_index >= 0 &&  card_index < array_count(application->cards)) {
        return &application->cards[card_index]; 
    }
    return 0;
}

internal card
card_parse(myhtml_tree_node_t *node) {
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
                if (!strncmp(value, "secondary", strlen("secondary"))) {
                    result.kind = Card_Notifications;
                    result.background_color = make_color(0x6c757d);
                } else if (!strncmp(value, "primary", strlen("primary"))) {
                    result.kind = Card_Negatives;
                    result.background_color = make_color(0x007bff);
                } else if (!strncmp(value, "warning", strlen("warning"))) {
                    result.kind = Card_Confirmed;
                    result.background_color = make_color(0xffc107);
                } else if (!strncmp(value, "success", strlen("success"))) {
                    result.kind = Card_Cured;
                    result.background_color = make_color(0x28a745);
                } else if (!strncmp(value, "danger", strlen("danger"))) {
                    result.kind = Card_Deaths;
                    result.background_color = make_color(0xdc3545);
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
            myhtml_collection_t *p_list = myhtml_get_nodes_by_tag_id_in_scope(0, 0, node, MyHTML_TAG_P, 0);
            if (p_list && p_list->list && p_list->length == 1) {
                myhtml_tree_node_t *text_node = myhtml_node_child(p_list->list[0]);
                if (text_node) {
                    const char *text = myhtml_node_text(text_node, 0);
                    if (text) {
                        char *label = (char *) html_utf8_text_to_extended_ascii((char *) text);
                        strcpy((char *) result.label, label);
                        free(label);
                    }
                }
            }
            
            // Get last updated at
            char *last_updated_at = get_text_from_tag_in_node(node, MyHTML_TAG_SPAN);
            strcpy((char *) result.last_updated_at, last_updated_at);
            free(last_updated_at);
        }
        
    }
    
    return result;
}
