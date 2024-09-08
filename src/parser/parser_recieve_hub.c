/*
 * parser_recieve_hub.c
 */

#include "chroma-engine.h"
#include "chroma-typedefs.h"
#include "geometry.h"
#include "graphics.h"
#include "graphics/graphics_internal.h"
#include "log.h"
#include "parser/parser_json.h"
#include "parser_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void parser_parse_template(JSONObject *template, IGraphics *hub);
void parser_parse_geometry(JSONObject *geo, IPage *page);
void parser_parse_attribute(JSONObject *attr, IGeometry *geo);

void parser_parse_user_frame(JSONObject *frame, IPage *page);
void parser_parse_bind_frame(JSONObject *frame, IPage *page);
void parser_parse_set_frame(JSONObject *frame, IPage *page);

#define JSON_ARRAY(array_obj, data, f)                                                         \
    do {                                                                                       \
        log_assert((array_obj)->type == JSON_ARRAY, "Parser", "Node is not a JSON_ARRAY");     \
        JSONArray *array = &(array_obj)->array;                                                \
        for (JSONArrayNode *obj = array->head.next; obj != &array->tail; obj = obj->next) {    \
            if ((obj)->node->type != JSON_OBJECT) {                                            \
                continue;                                                                      \
            }                                                                                  \
            f(&(obj)->node->object, (data));                                                   \
        }                                                                                      \
    } while (0);                                                                               \


    
// S -> {'num_temp': num, 'templates': [T]}
void parser_parse_hub(Engine *eng) {
    char *msg = "ver 0 1 full;";
    if (send(engine.hub_socket, msg, strlen(msg), 0) < 0) {
        log_file(LogError, "Parser", "Error requesting graphics hub"); 
    }

    JSONNode *root = parser_receive_json(eng->hub_socket);
    if (root->type != JSON_OBJECT) {
        log_file(LogMessage, "Parser", "No hub received");
        return;
    }

    JSONObject obj = root->object;
    int num_temp = parser_json_get_int(&obj, "NumTemplates");
    eng->hub = graphics_new_graphics_hub(num_temp);
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "Num Templates: %d", num_temp);
    }

    JSONNode *templates = parser_json_attribute(&obj, "Templates");
    JSON_ARRAY(templates, eng->hub, parser_parse_template);

    parser_json_free_node(root);
}

void parser_update_template(Engine *eng, int temp_id) {
    char msg[100];
    memset(msg, '\0', sizeof msg);

    graphics_hub_free_page(eng->hub, temp_id);

    sprintf(msg, "ver 0 1 temp %d;", temp_id);
    if (send(eng->hub_socket, msg, strlen(msg), 0) < 0) {
        log_file(LogError, "Parser", "Error requesting template %d", temp_id); 
    }

    JSONNode *template = parser_receive_json(eng->hub_socket);
    if (template->type != JSON_OBJECT) {
        log_file(LogMessage, "Parser", "Error in chroma hub");
        return;
    }

    parser_parse_template(&template->object, eng->hub); 
}

static int geo_type;

// T -> {'id': num, 'num_geo': num, 'geometry': [G]} | T, T
void parser_parse_template(JSONObject *template, IGraphics *hub) {
    int max_geo = parser_json_get_int(template, "MaxGeometry");
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tnum geometery: %d", max_geo);
    }

    int max_key = parser_json_get_int(template, "MaxKeyframe");
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tnum keyframe: %d", max_key);
    }


    IPage *page = graphics_new_page(max_geo, max_key);
    page->temp_id = parser_json_get_int(template, "TempID");

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
            JSON_ARRAY(node, page, parser_parse_geometry);
        }

    }

    {
        // keyframes
        JSONNode *node = parser_json_attribute(template, "UserFrame");
        JSON_ARRAY(node, page, parser_parse_user_frame);

        node = parser_json_attribute(template, "SetFrame");
        JSON_ARRAY(node, page, parser_parse_set_frame);

        node = parser_json_attribute(template, "BindFrame");
        JSON_ARRAY(node, page, parser_parse_bind_frame);

    }

    graphics_hub_add_page(hub, page);
    graphics_page_default_relations(page);
}

static void parser_parse_keyframe(JSONObject *frame_obj, Keyframe *frame) {
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
void parser_parse_set_frame(JSONObject *frame_obj, IPage *page) {
    Keyframe frame;
    parser_parse_keyframe(frame_obj, &frame);
    frame.type = SET_FRAME;

    frame.value = parser_json_get_int(frame_obj, "Value");
    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tValue: %d", frame.value);
    }

    graphics_page_gen_frame(page, frame);
}

void parser_parse_bind_frame(JSONObject *frame_obj, IPage *page) {
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
    parser_parse_keyframe(&bind_obj->object, &bind_frame);

    frame.bind_frame_num = bind_frame.frame_num;
    frame.bind_geo_id = bind_frame.geo_id;
    frame.bind_attr = bind_frame.attr;

    graphics_page_gen_frame(page, frame);
}

void parser_parse_user_frame(JSONObject *frame_obj, IPage *page) {
    Keyframe frame;
    parser_parse_keyframe(frame_obj, &frame);
    frame.type = USER_FRAME;

    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tUser Frame");
    }

    graphics_page_gen_frame(page, frame);
}

// G -> {'id': num, 'type': string, 'attr': [A]} | G, G
void parser_parse_geometry(JSONObject *geo_obj, IPage *page) {
    JSONNode *geo_id = parser_json_attribute(geo_obj, "GeometryID");
    log_assert(geo_id->type == JSON_INT, "Parser", "Geometry ID is not a JSON Int");

    if (LOG_TEMPLATE) {
        log_file(LogMessage, "Parser", "\tGeometry ID: %d", geo_id->integer);
    }

    IGeometry *geo = graphics_page_add_geometry(page, geo_type, geo_id->integer);

    for (JSONAttributeNode *attr = geo_obj->head.next; attr != &geo_obj->tail; attr = attr->next) {
        if (strcmp(attr->name, "Name") == 0) {
            continue;
        } else if (strcmp(attr->name, "GeometryID") == 0) {
            continue;
        } else if (strcmp(attr->name, "GeoType") == 0) {
            continue;
        }

        if (attr->node->type != JSON_OBJECT) {
            continue;
        }

        parser_parse_attribute(&attr->node->object, geo);
    }
}

// A -> {'name': string, 'value': string} | A, A
void parser_parse_attribute(JSONObject *attr, IGeometry *geo) {
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

    char *color_attrs[] = {"Red", "Green", "Blue", "Alpha"};
    for (int i = 0; i < 4; i++) {
        JSONNode *node = parser_json_attribute(attr, color_attrs[i]);
        if (node == NULL) {
            continue;
        }

        if (node->type == JSON_FLOAT) {
            geometry_set_color(geo, node->f, i);
        } else if (node->type == JSON_INT) {
            geometry_set_color(geo, node->integer, i);
        }
    }
}
