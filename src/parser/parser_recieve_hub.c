/*
 * parser_recieve_hub.c
 */

#include "parser_json.h"
#include "parser_http.h"
#include "parser_internal.h"

int parser_parse_template(JSONNode *template, IGraphics *hub);
void parser_parse_geometry(JSONNode *geo, IPage *page);
void parser_parse_attribute(JSONNode *attr, IGeometry *geo);
 
void parser_parse_user_frame(JSONNode *frame, IPage *page);
void parser_parse_bind_frame(JSONNode *frame, IPage *page);
void parser_parse_set_frame(JSONNode *frame, IPage *page);

// S -> {'num_temp': num, 'templates': [T]}
int parser_parse_hub(Engine *eng) {
    char addr[PARSE_BUF_SIZE];
    log_file(LogMessage, "Parser", "Requesting Chroma Hub");

    memset(addr, '\0', sizeof addr);
    sprintf(addr, "%s/templates", eng->hub_addr);
    if (parser_http_get(eng->hub_socket, addr) < 0) {
        log_file(LogError, "Parser", "parser_parse_hub: %s %d", __FILE__, __LINE__);
        return -1;
    }

    JSONNode root;
    if (parser_receive_json(eng->hub_socket, &root) < 0 || root.type != JSON_OBJECT) {
        log_file(LogWarn, "Parser", "No hub received");
        parser_clean_json();
        return -1;
    }

    int num_temp = parser_json_get_int(&root, "NumTemplates");
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "Num Templates: %d", num_temp);
    }

    graphics_new_graphics_hub(&eng->hub, num_temp);

    JSONNode *templates = parser_json_attribute(&root, "Templates");
    if (templates == NULL || templates->type != JSON_ARRAY) {
        log_file(LogWarn, "Parser", "No templates received");
        parser_clean_json();
        return 0;
    }

    if (templates->type != JSON_ARRAY) {
        log_file(LogError, "Parser", "Templates is not a JSON_ARRAY");
        return -1;
    }

    for (int i = 0; i < templates->array.num_items; i++) {    
        size_t node_index = json_arena.objects.items[templates->array.start + i];  
        JSONNode *node = &json_arena.items[node_index];                               
        if (parser_parse_template(node, &eng->hub) < 0) {
            log_file(LogError, "Parser", "parser_parse_hub: %s %d", __FILE__, __LINE__);
            log_file(LogError, "Parser", "Array index: %d", i);
            return -1;
        }
    }                                                                                  

    parser_clean_json();
    return 0;
}

int parser_update_template(Engine *eng, int temp_id) {
    char addr[PARSE_BUF_SIZE];
    memset(addr, '\0', sizeof addr);
    sprintf(addr, "%s/template/%d", eng->hub_addr, temp_id);
    if (parser_http_get(eng->hub_socket, addr) < 0) {
        log_file(LogError, "Parser", "parser_update_template: %s %d", __FILE__, __LINE__);
        return -1;
    }

    JSONNode template;
    if (parser_receive_json(eng->hub_socket, &template) < 0 || template.type != JSON_OBJECT) {
        log_file(LogMessage, "Parser", "No template received");
        parser_clean_json();
        return 0;
    }

    if (parser_parse_template(&template, &eng->hub) < 0) {
        return -1;
    } 

    parser_clean_json();
    return 0;
}

static int geo_type;

// T -> {'id': num, 'num_geo': num, 'geometry': [G]} | T, T
int parser_parse_template(JSONNode *template, IGraphics *hub) {
    int max_geo = parser_json_get_int(template, "MaxGeometry");
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tnum geometery: %d", max_geo);
    }

    int max_key = parser_json_get_int(template, "MaxKeyframe");
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tnum keyframe: %d", max_key);
    }

    int temp_id = parser_json_get_int(template, "TempID");
    IPage *page = graphics_hub_new_page(hub, max_geo, max_key, temp_id);
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\ttemplate id: %d", page->temp_id);
    }

    {
        // geometry 
        int num_geo = 7;
        int geo_types[] = {RECT, TEXT, CIRCLE, IMAGE, POLYGON, TEXT, TEXT};
        char *geo_names[] = {"Rectangle", "Text", "Circle", "Image", "Polygon", "Clock", "List"};

        for (int i = 0; i < num_geo; i++) {
            JSONNode *node = parser_json_attribute(template, geo_names[i]);
            geo_type = geo_types[i];
            if (node == NULL || node->type != JSON_ARRAY) {
                log_file(LogError, "Parser", "parser_parse_hub: %s %d", __FILE__, __LINE__);
                return -1;
            }

            for (int i = 0; i < node->array.num_items; i++) {    
                size_t node_index = json_arena.objects.items[node->array.start + i];  
                JSONNode *node = &json_arena.items[node_index];                               
                parser_parse_geometry(node, page);                                                             
            }                                                                                  
        }
    }

    {
        // keyframes
        JSONNode *node = parser_json_attribute(template, "UserFrame");
        if (node == NULL || node->type != JSON_ARRAY) {
            log_file(LogError, "Parser", "parser_parse_hub: %s %d", __FILE__, __LINE__);
            return -1;
        }

        for (int i = 0; i < node->array.num_items; i++) {    
            size_t node_index = json_arena.objects.items[node->array.start + i];  
            JSONNode *node = &json_arena.items[node_index];                               
            parser_parse_user_frame(node, page);                                                             
        }                                                                                  

        node = parser_json_attribute(template, "SetFrame");
        if (node == NULL || node->type != JSON_ARRAY) {
            log_file(LogError, "Parser", "parser_parse_hub: %s %d", __FILE__, __LINE__);
            return -1;
        }

        for (int i = 0; i < node->array.num_items; i++) {    
            size_t node_index = json_arena.objects.items[node->array.start + i];  
            JSONNode *node = &json_arena.items[node_index];                               
            parser_parse_set_frame(node, page);                                                             
        }                                                                                  

        node = parser_json_attribute(template, "BindFrame");
        if (node == NULL || node->type != JSON_ARRAY) {
            log_file(LogError, "Parser", "parser_parse_hub: %s %d", __FILE__, __LINE__);
            return -1;
        }

        for (int i = 0; i < node->array.num_items; i++) {    
            size_t node_index = json_arena.objects.items[node->array.start + i];  
            JSONNode *node = &json_arena.items[node_index];                               
            parser_parse_bind_frame(node, page);                                                             
        }                                                                                  
    }

    graphics_page_default_relations(page);

    if (!graphics_graph_is_dag(&page->keyframe_graph)) {
        log_file(LogError, "Graphics", "Page %d keyframes are not in a dag", page->temp_id);
        return -1;
    }

    return 0;
}

static void parser_parse_keyframe(JSONNode *frame_obj, Keyframe *frame) {
    if (frame_obj == NULL) {
        log_file(LogWarn, "Parser", "Keyframe is null");
        return;
    }

    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "Keyframe");
    }

    frame->frame_num = parser_json_get_int(frame_obj, "FrameNum");
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tFrame Num: %d", frame->frame_num);
    }

    frame->geo_id = parser_json_get_int(frame_obj, "GeoID");
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tGeo ID: %d", frame->geo_id);
    }

    frame->attr = geometry_char_to_attr(parser_json_get_string(frame_obj, "GeoAttr"));
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tGeo Attr: %s", parser_json_get_string(frame_obj, "GeoAttr"));
    }

    frame->expand = parser_json_get_bool(frame_obj, "Expand");
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tExpand: %d", frame->expand);
    }
}

// K -> {'frame_num': num, ...
void parser_parse_set_frame(JSONNode *frame_obj, IPage *page) {
    Keyframe frame;
    parser_parse_keyframe(frame_obj, &frame);
    frame.type = SET_FRAME;

    JSONNode *value = parser_json_attribute(frame_obj, "Value");
    if (value->type == JSON_INT) {
        frame.value = parser_json_get_int(frame_obj, "Value");
    } else if (value->type == JSON_FLOAT) {
        frame.value = parser_json_get_float(frame_obj, "Value");
    } else {
        log_file(LogWarn, "Parser", "Unknown json type for set value: %d", value->type);
        frame.value = 0;
    }

    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tValue: %d", frame.value);
    }

    graphics_page_gen_frame(page, frame);
}

void parser_parse_bind_frame(JSONNode *frame_obj, IPage *page) {
    Keyframe frame;
    parser_parse_keyframe(frame_obj, &frame);
    frame.type = BIND_FRAME;

    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "Bind Frame");
    }

    Keyframe bind_frame;
    JSONNode *bind_obj = parser_json_attribute(frame_obj, "Bind");
    log_assert(bind_obj != NULL, "Parser", "Bind frame is null");
    log_assert(bind_obj->type == JSON_OBJECT, "Parser", "Bind frame node is not a JSON Object");
    parser_parse_keyframe(bind_obj, &bind_frame);

    frame.bind_frame_num = bind_frame.frame_num;
    frame.bind_geo_id = bind_frame.geo_id;
    frame.bind_attr = bind_frame.attr;

    graphics_page_gen_frame(page, frame);
}

void parser_parse_user_frame(JSONNode *frame_obj, IPage *page) {
    Keyframe frame;
    parser_parse_keyframe(frame_obj, &frame);
    frame.type = USER_FRAME;

    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tUser Frame");
    }

    graphics_page_gen_frame(page, frame);
}

// G -> {'id': num, 'type': string, 'attr': [A]} | G, G
void parser_parse_geometry(JSONNode *geo_obj, IPage *page) {
    JSONNode *geo_id = parser_json_attribute(geo_obj, "GeometryID");
    log_assert(geo_id->type == JSON_INT, "Parser", "Geometry ID is not a JSON Int");

    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tGeometry ID: %d", geo_id->integer);
    }

    IGeometry *geo = graphics_page_add_geometry(page, geo_type, geo_id->integer);

    for (int i = 0; i < geo_obj->array.num_items; i++) {
        size_t json_index = json_arena.objects.items[geo_obj->array.start + i];
        JSONNode *attr = &json_arena.items[json_index];

        if (strcmp(attr->name, "Name") == 0) {
            continue;
        } else if (strcmp(attr->name, "GeometryID") == 0) {
            continue;
        } else if (strcmp(attr->name, "GeoType") == 0) {
            continue;
        }

        if (attr->type != JSON_OBJECT) {
            continue;
        }

        parser_parse_attribute(attr, geo);
    }
}

// A -> {'name': string, 'value': string} | A, A
void parser_parse_attribute(JSONNode *attr, IGeometry *geo) {
    char *name = parser_json_get_string(attr, "Name");

    if (LOG_TEMPLATE && name != NULL) {
        log_file(LogMessage, "Parser", "\t\t%s:", name);
    }

    JSONNode *value = parser_json_attribute(attr, "Value");
    if (value != NULL && name != NULL) {
        if (value->type == JSON_STRING) {
            geometry_set_attr(geo, name, value->string);

            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\t\t\tValue: %s", value->string);
            }

        } else if (value->type == JSON_INT) {
            int geo_attr = geometry_char_to_attr(name);
            geometry_set_int_attr(geo, geo_attr, value->integer);

            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\t\t\tValue: %d", value->integer);
            }

        } else if (value->type == JSON_FLOAT) {
            char buf[100];
            memset(buf, '\0', sizeof buf);
            sprintf(buf, "%f", value->f);

            geometry_set_attr(geo, name, buf);

            if (LOG_TEMPLATE) {
                log_file(LogMessage, "Parser", "\t\t\tValue: %f", value->f);
            }
        }
    }

    // parse color
    char *color_names[] = {"Red", "Green", "Blue", "Alpha"};
    int color_attrs[] = {GEO_COLOR_R, GEO_COLOR_G, GEO_COLOR_B, GEO_COLOR_A};
    for (int i = 0; i < 4; i++) {
        JSONNode *node = parser_json_attribute(attr, color_names[i]);
        if (node == NULL) {
            continue;
        }

        if (node->type == JSON_INT) {
            geometry_set_float_attr(geo, color_attrs[i], node->integer);
        } else {
            geometry_set_float_attr(geo, color_attrs[i], node->f);
        }
    }
}
