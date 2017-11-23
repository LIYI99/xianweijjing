#ifndef __GK_IMAGE_SDK_H__
#define __GK_IMAGE_SDK_H__

#include <stdint.h>
#include <ptread.h>
#include "gk_mouse_event.h"
#include "object_pool.h"

#ifdef __cplusplus
    extern "C"{
#endif

typedef  enum{
    OBJECT_NOR,
    OBJECT_BUTTION,
    OBJECT_MENU,
    OBJECT_LINE,
    OBJECT_TXT_WIN,
    OBJECT_BAR,
    OBJECT_NUMS,

}WIN_TYPE_S;



typedef struct window_node      window_node_t;

typedef enum {
    NOT_MOVE,
    MOUSE_OFFET_MOVE,
    MOUSE_LDOWN_MOVE,
    MOUSE_LUP_MOVE,
    MOUSE_RDOWN_MOVE,
    MOUSE_RUP_MOVE,
    MOVE_NUMS,
}NODE_MOVE_ARRT;

typedef enum{
    NORTHING,
    NEED_FRESHEN,
    NEED_CLEAR,
    FRESHEN_NUMS,
}NODE_FRESHEN_ARRT;

struct window_node{

#define     MENU_LEVEL              5
    char            node_id[MENU_LEVEL]; // ever level A -z sum 52 level 
    window_node_t   *prev;
    window_node_t   *next;
    window_node_t   *s_head;
    window_node_t   *s_end;
    window_node_t   *f_node;
    uint8_t         en_node;
    uint8_t         check_node;
    uint8_t         en_submenu;
    NODE_FRESHEN_ARRT   freshen_arrt;

#define         CLOSE_REFLECT       0
#define         OFFSET_REFLECT      1
#define         LDOWN_REFLECT       2
#define         LUP_REFLECT         4
#define         RDOWN_REFLECT       8
#define         RUP_REFLECT         16
    uint8_t         mouse_garp_reflect;
    NODE_MOVE_ARRT  move_arrt;
    WIN_TYPE_S      win_type;
    void            *window;
};


typedef void  (*win_func)(void *data);



typedef struct window_func{
    
    win_func        mouse_offset;
    win_func        mouse_left_down;
    win_func        mouse_left_up;
    win_func        mouse_right_down;
    win_func        mouse_right_up;
    win_func        mouse_leave;
    void            *data;

}window_func_t;

typedef struct window_node_button{
   
    window_node_t   *this_node; 
    uint16_t        x;
    uint16_t        y;
    uint16_t        w;
    uint16_t        h;
    uint16_t        size;
    uint32_t        color; 
    char            *image_cache;
    
    window_func_t   func_set;
    void            *user_data;

}window_node_button_t;


typedef struct window_node_menu{
    window_node_t   *this_node;
    uint16_t        x;
    uint16_t        y;
    uint16_t        w;
    uint16_t        h;
    uint16_t        size;
    uint32_t        color; 
    char            *image_cache;
    window_func_t   func_set;
}window_node_menu_t;

typedef struct window_node_line{
       
    window_node_t   *this_node;
    uint16_t        start_x;
    uint16_t        start_y;
    uint16_t        end_x;
    uint16_t        end_y;
    uint16_t        size;
    uint32_t        color; 
    char            *image_cache;
    
    window_func_t   func_set;
}window_node_line_t;

typedef struct window_node_text{
       
    window_node_t   *this_node;
    uint16_t        x;
    uint16_t        y;
    uint16_t        size;
    uint16_t        lens;
    uint32_t        win_color;
    uint32_t        text_color;
    //char            *text;
    char            *text_cache;
    
    window_func_t   func_set;

}window_node_text_t;

typedef struct window_node_bar{
       
    window_node_t   *this_node;
    uint16_t        x;
    uint16_t        y;
    uint16_t        size;
    uint16_t        lens;
    uint16_t        max_value;
    uint16_t        now_value;
    uint32_t        bar_color;
    window_func_t   func_set;

}window_node_bar_t;

typedef struct window_node_mouse{
    
    uint16_t        x_last;        //old
    uint16_t        y_last;        //old
    uint16_t        x;
    uint16_t        y;
    uint16_t        size;
    uint32_t        color;
    char            *image_cache0;
    char            *image_cache1;
    char            *image_cache2;
    char            *save_cache;

}window_node_mouse_t;


typedef struct  image_sdk_s{
         
    uint16_t        en;
    //video params
    uint16_t        scree_w;
    uint16_t        scree_h;
    uint16_t        color_fmt;
    uint16_t        disp_fps;
    int             video_fd;
    void            *mmap_p;
    
    //mouse
    int             mouse_fd;
    GK_MOUSE_DATA   mouse_new_data; 
    uint8_t         need_restart_mouse;
    uint8_t         mouse_data_updated;
    
    //window root node
    window_node     *root;
    //pool
    object_pool_t   *window_node_pool;
    object_pool_t   *object_pool;
    
    //mouse check node record
    window_node_t   *last_check_node[MENU_LEVEL];
    window_node_t   *check_node[MENU_LEVEL];
    uint16_t        check_level_cnt;             ; 
    //mouse image node
    window_node_mouse_t *mouse;
}image_sdk_t;




void    Image_SDK_Init(void);
void    Image_SDK_deInit(void);
void    Image_SDK_Run(void);


struct user_set_node_atrr{
    char            node_id[MENU_LEVEL]; 
    uint8_t         en_node;
    NODE_FRESHEN_ARRT   en_freshen;
    NODE_MOVE_ARRT  move_arrt;
};



int    Image_SDK_Create_Button( struct user_set_node_atrr attr,
        window_node_button_t _button);
int    Image_SDK_Create_Menu(struct user_set_node_atrr attr,
        window_node_menu_t _menu);
int    Image_SDK_Create_Line(struct user_set_node_atrr attr,
        window_node_line_t _line);
int    Image_SDK_Create_Text(struct user_set_node_atrr arrt,
        window_node_text_t _text);

int     Image_SDK_Set_Node_Move_Atrr(char *node_id,NODE_MOVE_ARRT _arrt);
int     Image_SDK_Set_Node_En(char *node_id,uint8_t en);
int     Image_SDK_Set_Node_En_Freshen(char *node_id,uint8_t en_freshen);






#ifdef __cplusplus
}
#endif



#endif