
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <pthread.h>

#include"gk_image_sdk_new.h"
#include"image_text_put.h"
//device info set 
#define     VO_SCREE_W          1920
#define     VO_SCREE_H          1080
#define     VO_DISP_FPS         25
#define     VO_COLOR_FMT        16
#define     GK_DEVICE_MOUSE     "/dev/input/mouse0"
#define     GK_DEVICE_FB        "/dev/fb0"
#define     GK_MOUSE_SPEED      1

//image disp info set
#define     DISP_OBJ_MAX        100
#define     MOUSE_SIZE          30

//font set

#define     FONT_SIZE_T         16
#define     FONT_PATH_T         "/usr/local/bin/font/asc16"




static      image_sdk_t         *sdk_handle = NULL;

//video fb
static void*  _image_fb_init(char *dev_path,int *fd);
static void*  _image_fb_deinit(void *mmap_p,int fd);
static void   _image_fb_push(int xoffset,int yoffset);

// mouse image load
static void     _image_mouse_image_load(window_node_mouse_t *mouse);
static void     _image_mouse_image_deload(window_node_mouse_t *mouse);

static void*    _image_mouse_event_read_thread(void *data);

//handle data proess
static void*    _image_sdk_handle_data_process(void *data);


static void     _image_mouse_image_load(window_node_mouse_t *mouse)
{

#define     MOUSE_IMAGE_PATH_1  NULL
#define     MOUSE_IMAGE_PATH_2  NULL
    
    //16bit color
    mouse->image_cache = (char *)malloc(mouse->size *mouse->size * sdk_handle->color_fmt / 8 *2);
    if(mouse->image_cache == NULL)
    {
        printf("mouse load fail\n");
        return ;
    }
    
    mouse->save_cache = (char *)malloc(mouse->size *mouse->size * sdk_handle->color_fmt/8);


    int i,k,j,r;
    uint16_t    *buf = NULL;
    r = mouse->size / 2;

    mouse->color = 0xbbca;
    if(MOUSE_IMAGE_PATH_1 == NULL || MOUSE_IMAGE_PATH_2 == NULL){
        //build color mouse
        buf = (uint16_t *)mouse->image_cache;
        for(k = 0 ,j = 0; k < mouse->size; k ++ ){
            for(i = 0; i < mouse->size; i++,j++)
                if(((k-r)*(k-r) + (i-r)*(i-r)) <= r *r)
                {
                    buf[j] = mouse->color;

                }else{
                    buf[j] = 0;
                }
        }    
        buf = (uint16_t *)(mouse->image_cache + mouse->size* mouse->size*2);

        for(k = 0 ,j = 0; k < mouse->size; k ++ ){
            for(i = 0; i < mouse->size; i++,j++)
                if(((k-r)*(k-r) + (i-r)*(i-r)) <= r *r)
                {
                    buf[j] = 0xFF80;

                }else{
                    buf[j] = 0;
                }
        }

        
    
    }else{
        //load image to buf
    }
    mouse->image_p = mouse->image_cache;    
    return ;


}

static void     _image_mouse_image_deload(window_node_mouse_t *mouse)
{

    if(mouse == NULL)
        return;

    
    free(mouse->image_cache);
    free(mouse->save_cache);

    return ;

}





//ok
void    Image_SDK_Init(void){
    
    if(sdk_handle != NULL)
        return ;
    sdk_handle =  (image_sdk_t *)malloc(sizeof(image_sdk_t));
    if(sdk_handle == NULL)
        return;
    memset(sdk_handle ,0x0,sizeof(image_sdk_t));
    
    sdk_handle->en = 1;
    sdk_handle->scree_w = VO_SCREE_W;
    sdk_handle->scree_h = VO_SCREE_H;
    sdk_handle->color_fmt = VO_COLOR_FMT;
    sdk_handle->disp_fps = VO_DISP_FPS;    
    
    //open fb fd
    sdk_handle->mmap_p = _image_fb_init(GK_DEVICE_FB,&sdk_handle->video_fd);
    if(sdk_handle->video_fd < 0)
        goto ERR1;
    //open mouse fd 
    sdk_handle->mouse_fd =  gk_mouse_open(GK_DEVICE_MOUSE,VO_SCREE_W,
            VO_SCREE_H,GK_MOUSE_SPEED,MOUSE_SIZE);
   
    if(sdk_handle->mouse_fd < 0){
        printf("not use mouse\n");
    }

    //create window node pool
    sdk_handle->window_node_pool =  
        object_pool_create(sizeof(window_node_t),DISP_OBJ_MAX+1,NULL);
    if(sdk_handle->window_node_pool == NULL)
        goto ERR2;
    
    //create image meadie obj pool
    unsigned int obj_size = 0;
    obj_size = sizeof(window_node_button_t) > sizeof(window_node_menu_t)? 
        sizeof(window_node_button_t) : sizeof(window_node_menu_t);
    obj_size = obj_size > sizeof(window_node_line_t)? obj_size : sizeof(window_node_line_t);
    
    obj_size = obj_size > sizeof(window_node_text_t)? obj_size : sizeof(window_node_text_t);
    
    sdk_handle->object_pool = object_pool_create(obj_size,DISP_OBJ_MAX+1,NULL);

    if(sdk_handle->object_pool == NULL)
        goto ERR3;
    
    //init root node
    window_node_t *root = (window_node_t *)object_pool_get(sdk_handle->window_node_pool);
    memset(root, 0x0 ,sizeof(window_node_t));
    memcpy(root->node_id,"A",1);
    root->en_node = 1;
    sdk_handle->root = root;
    printf("window root:%p\n",root);
    //add mouse window node
    window_node_mouse_t *mouse = NULL;
    mouse = (window_node_mouse_t *)malloc(sizeof(window_node_mouse_t));
    if(mouse == NULL){
        printf("fisrt get obj fail???\n");
    }
    
    memset(mouse,0x0,sizeof(window_node_mouse_t));
    mouse->size = MOUSE_SIZE;
     _image_mouse_image_load(mouse);
    sdk_handle->mouse = mouse;
    
    //en sdk handle
    sdk_handle->en = 1;
    
    //load font lib
    int ret = 0; 
    ret = image_text_lib_init(FONT_SIZE_T,FONT_PATH_T);
    if(ret < 0)
        printf("load font lib fail ret:%d\n",ret);

    return;

ERR3:
    object_pool_destory(sdk_handle->window_node_pool);

ERR2:
   if(sdk_handle->mouse_fd > 0)
        gk_mouse_close(sdk_handle->mouse_fd); 
    
   _image_fb_deinit(sdk_handle->mmap_p,sdk_handle->video_fd);

ERR1:
    free(sdk_handle);
    sdk_handle = NULL;
    return ;
}


//ok
void    Image_SDK_deInit(void)
{
    if(sdk_handle == NULL)
        return;
     _image_fb_deinit(sdk_handle->mmap_p,sdk_handle->video_fd);
    
     if(sdk_handle->mouse_fd > 0)
            gk_mouse_close(sdk_handle->mouse_fd); 

    object_pool_destory(sdk_handle->window_node_pool);
    object_pool_destory(sdk_handle->object_pool);
    _image_mouse_image_deload(sdk_handle->mouse);
    free(sdk_handle->mouse);
    free(sdk_handle);
    sdk_handle = NULL;
    return;
}


//ok
static int inline  node_id_level_re(char *node_id){
    
    int i;
    for(i = 0 ;i  < MENU_LEVEL ; i++){
        if(node_id[i] < 'A'  || node_id[i] > 'z' )
            break;
    }
    return i;
}
//ok
static inline window_node_t  *key_find_node(window_node_t *father,char key,int level)
{
   window_node_t *temp = father->s_head;
  // printf("father->shead:%p,father:%p,root:%p\n",temp,father,sdk_handle->root);
    for (;temp != NULL; temp = temp->next)
   {
    //    printf("temp->node_id[2]:%c\n",temp->node_id[1]); 
 
       if(temp->node_id[level] ==  key)
            break;
   }
   return temp;
}
//ok
static window_node_t  *find_father_node(char *node_id,int level){
    
    if(level == 2)
        return sdk_handle->root;
    
    window_node_t *temp  = sdk_handle->root;
    
    int k = 1;
    for (k = 1 ; k < level -1 && temp != NULL; k ++ ){
        temp = key_find_node(temp,node_id[k],k);
    }
    return temp;
}
//ok
static int  window_node_inster(window_node_t *node){

    int level ;
    //get window level
    level = node_id_level_re(node->node_id);
    if(level < 2){
        printf(" node_id wirte erro \n");
        return -1;
    }

    //get window father window
    printf("get level:%d\n",level);
    window_node_t *ftemp = NULL,*same = NULL;
    ftemp = find_father_node(node->node_id,level);
    if(ftemp == NULL){
        printf("not find father node \n");
        return -2;
    }
    //find same window
    same = key_find_node(ftemp,node->node_id[level -1],level -1);
    if(same != NULL){
        printf("a same node ,you set name error \n");
        return -3;
    }
    //inster node 
    node->f_node = ftemp;

    if(ftemp->s_head == NULL){
        ftemp->s_head = node;
        ftemp->s_end = node;
        return 0;
    }

    node->next = ftemp->s_head;
    node->prev = NULL;
    ftemp->s_head->prev = node;
    ftemp->s_head = node;
    
    return 0;
}

//ok
int    Image_SDK_Create_Button( struct user_set_node_atrr attr,
        window_node_button_t _button){

    if(sdk_handle == NULL)
        return -2;
    
    window_node_t  *lq = NULL;
    lq = (window_node_t *)object_pool_get(sdk_handle->window_node_pool);
    if(lq == NULL){
        
        return -3;
    }
    printf("linenode:%p \n",lq);
 
    window_node_button_t * button = NULL;
    button = (window_node_button_t *)object_pool_get(sdk_handle->object_pool);
    if(button == NULL){
        object_pool_free(sdk_handle->window_node_pool,lq);
        return -5;
    }
    
    memcpy(lq->node_id ,attr.node_id,MENU_LEVEL);
    int ret = 0;
    ret = window_node_inster(lq); 
    if(ret < 0 ){
        object_pool_free(sdk_handle->window_node_pool,lq);
        object_pool_free(sdk_handle->object_pool,button);
        return -6;
    }

    *button =  _button;
    button->this_node = lq;
    if(_button.video_set.data == NULL)
        button->video_set.data  = (void *)button;

    if(button->x > sdk_handle->scree_w){
        button->x = sdk_handle->scree_w - button->w;
        printf("you set x > srcee_w\n");
    }
    if(button->y > sdk_handle->scree_h){
        button->y = sdk_handle->scree_h -button->h;
        printf("you set y > srcee_h\n");
    }
    if(button->w > sdk_handle->scree_w){
        button->w = 60;
        printf("you set w > srcee_w,now set 60 pixl\n");

    }
    if(button->h > sdk_handle->scree_h){
        button->h = 30;
        printf("you set h > srcee_h,now set 30pixl\n");

    }

    lq->en_node = attr.en_node;
    lq->freshen_arrt = attr.en_freshen;
    lq->win_type = OBJECT_BUTTION;
    lq->window = (void *)button;
    lq->mouse_garp_reflect = attr.mouse_garp_reflect;
    return 0;



}
int    Image_SDK_Create_Menu(struct user_set_node_atrr attr,
        window_node_menu_t _menu){
    
    if(sdk_handle == NULL)
        return -2;
    
    window_node_t  *lq = NULL;
    lq = (window_node_t *)object_pool_get(sdk_handle->window_node_pool);
    if(lq == NULL){
        
        return -3;
    }
   
    window_node_menu_t * menu = NULL;
    menu = (window_node_menu_t *)object_pool_get(sdk_handle->object_pool);
    if(menu == NULL){
        object_pool_free(sdk_handle->window_node_pool,lq);
        return -5;
    }
    
    memcpy(lq->node_id ,attr.node_id,MENU_LEVEL);
    int ret = 0;
    ret = window_node_inster(lq); 
    if(ret < 0 ){
        object_pool_free(sdk_handle->window_node_pool,lq);
        object_pool_free(sdk_handle->object_pool,menu);
        return -6;
    }

    *menu =  _menu;
    menu->this_node = lq; 
    if(_menu.video_set.data == NULL)
        menu->video_set.data  = (void *)menu;

    if(menu->x > sdk_handle->scree_w){
        menu->x = sdk_handle->scree_w - menu->w;
        printf("you set x > srcee_w\n");
    }
    if(menu->y > sdk_handle->scree_h){
        menu->y = sdk_handle->scree_h -menu->h;
        printf("you set y > srcee_h\n");
    }
    if(menu->w > sdk_handle->scree_w){
        menu->w = 60;
        printf("you set w > srcee_w,now set 60 pixl\n");

    }
    if(menu->h > sdk_handle->scree_h){
        menu->h = 30;
        printf("you set h > srcee_h,now set 30pixl\n");

    }

    lq->en_node = attr.en_node;
    lq->freshen_arrt = attr.en_freshen;
    lq->win_type = OBJECT_MENU;
    lq->window = (void *)menu;
    lq->mouse_garp_reflect = attr.mouse_garp_reflect;

    return 0;

}
int    Image_SDK_Create_Line(struct user_set_node_atrr attr,
        window_node_line_t _line)
{
    
    if(sdk_handle == NULL)
        return -2;
    
    window_node_t  *lq = NULL;
    lq = (window_node_t *)object_pool_get(sdk_handle->window_node_pool);
    if(lq == NULL){
        
        return -3;
    }
   // printf("linenode:%p \n",lq);

    window_node_line_t * line = NULL;
    line = (window_node_line_t *)object_pool_get(sdk_handle->object_pool);
    if(line == NULL){
        object_pool_free(sdk_handle->window_node_pool,lq);
        return -5;
    }
    
    memcpy(lq->node_id ,attr.node_id,MENU_LEVEL);
    int ret = 0;
    ret = window_node_inster(lq); 
    if(ret < 0 ){
        object_pool_free(sdk_handle->window_node_pool,lq);
        object_pool_free(sdk_handle->object_pool,line);
        return -6;
    }

    *line =  _line;
    line->this_node = lq; 
    if(_line.video_set.data == NULL)
        line->video_set.data  = (void *)line;

    if(line->start_x > sdk_handle->scree_w){
        line->start_x = sdk_handle->scree_w -line->size;
        printf("you set x > srcee_w\n");
    }
    if(line->start_y > sdk_handle->scree_h){
        line->start_y = sdk_handle->scree_h -line->size;
        printf("you set y > srcee_h\n");
    }
    if(line->end_x > sdk_handle->scree_w){
        line->end_x = sdk_handle->scree_w - line->size;
        printf("you set w > srcee_w,now set 60 pixl\n");

    }
    if(line->end_y > sdk_handle->scree_h){
        line->end_y = sdk_handle->scree_h - line->size;
        printf("you set h > srcee_h,now set 30pixl\n");

    }

    lq->en_node = attr.en_node;
    lq->freshen_arrt = attr.en_freshen;
    lq->win_type = OBJECT_LINE;
    lq->window = (void *)line;
    lq->mouse_garp_reflect = attr.mouse_garp_reflect;

    return 0;


}
int    Image_SDK_Create_Text(struct user_set_node_atrr attr,
        window_node_text_t _text)
{
    
    if(sdk_handle == NULL)
        return -2;

    if(image_text_get_font_width() <= 0){
        
        printf("text lib not load \n");
        return -9;
    }
    uint8_t asc_width = image_text_get_font_width();
    uint8_t font_size = image_text_get_font_size();

          

    window_node_t  *lq = NULL;
    lq = (window_node_t *)object_pool_get(sdk_handle->window_node_pool);
    if(lq == NULL){
        
        return -3;
    }
   
    window_node_text_t * text = NULL;
    text = (window_node_text_t *)object_pool_get(sdk_handle->object_pool);
    if(text == NULL){
        object_pool_free(sdk_handle->window_node_pool,lq);
        return -5;
    }
    
    memcpy(lq->node_id ,attr.node_id,MENU_LEVEL);
    int ret = 0;
    ret = window_node_inster(lq); 
    if(ret < 0 ){
        object_pool_free(sdk_handle->window_node_pool,lq);
        object_pool_free(sdk_handle->object_pool,text);
        return -6;
    }

    *text =  _text;
    text->this_node = lq;
       text->asc_width = asc_width;
    text->font_size = font_size;

    //need change mem pool;
#if 1
    text->text_cache = (char *)malloc(text->lens +1); //save text 
    memset(text->text_cache,0x0,text->lens + 1);
#endif

    if(_text.video_set.data == NULL)
        text->video_set.data  = (void *)text;

    if(text->x > sdk_handle->scree_w){
        text->x = sdk_handle->scree_w - text->lens *text->asc_width;
        printf("you set x > srcee_w\n");
    }
    if(text->y > sdk_handle->scree_h){
        text->y = sdk_handle->scree_h -text->asc_width;
        printf("you set y > srcee_h\n");
    }
   
    lq->en_node = attr.en_node;
    lq->freshen_arrt = attr.en_freshen;
    lq->win_type = OBJECT_TEXT_WIN;
    lq->window = (void *)text;
    lq->mouse_garp_reflect = attr.mouse_garp_reflect;

    return 0;


}

int    Image_SDK_Create_Bar(struct user_set_node_atrr attr,
        window_node_bar_t _bar)
{
    
    printf("create bar w:%d max_value:%d now_value:%d \n",_bar.w,
            _bar.max_value,_bar.now_value);
    if(sdk_handle == NULL)
        return -2;
    
    window_node_t  *lq = NULL;
    lq = (window_node_t *)object_pool_get(sdk_handle->window_node_pool);
    if(lq == NULL){
        
        return -3;
    }
   
    window_node_bar_t *     bar = NULL;
    bar = (window_node_bar_t *)object_pool_get(sdk_handle->object_pool);
    if(bar == NULL){
        object_pool_free(sdk_handle->window_node_pool,lq);
        return -5;
    }
    
    memcpy(lq->node_id ,attr.node_id,MENU_LEVEL);
    int ret = 0;
    ret = window_node_inster(lq); 
    if(ret < 0 ){
        object_pool_free(sdk_handle->window_node_pool,lq);
        object_pool_free(sdk_handle->object_pool,bar);
        return -6;
    }

    *bar =  _bar;
     bar->this_node = lq;

    if(_bar.video_set.data == NULL)
        bar->video_set.data  = (void *)bar;

    if(bar->x > sdk_handle->scree_w){
        bar->x = sdk_handle->scree_w - bar->w;
        printf("you set x > srcee_w\n");
    }
    if(bar->y > sdk_handle->scree_h){
        bar->y = sdk_handle->scree_h -bar->h;
        printf("you set y > srcee_h\n");
    }
   
    lq->en_node = attr.en_node;
    lq->freshen_arrt = attr.en_freshen;
    lq->win_type = OBJECT_BAR;
    lq->window = (void *)bar;
    lq->mouse_garp_reflect = attr.mouse_garp_reflect;
    printf("bar->now_value:%d _bar.now_value:%d\n",bar->now_value,_bar.now_value);
    return 0;


}


//ok
static window_node_t  *find_all_key_node(char *node_id,int level){
    
    if(level == 1)
        return sdk_handle->root;
    
    window_node_t *temp  = sdk_handle->root;
    
    int k ;
    for (k = 1 ; k < level ; k ++ ){
        temp = key_find_node(temp,node_id[k],k);
    }
    return temp;
}

//ok
int     Image_SDK_Set_Node_Move_Atrr(char *node_id,NODE_MOVE_ARRT _arrt)
{
    
    int level = 0;
    level = node_id_level_re(node_id);
    window_node_t *temp = NULL;
    temp =  find_all_key_node(node_id,level);
    if(temp == NULL)
        return -2;
    temp->move_arrt = _arrt;
    
    return 0;

}
//ok
int     Image_SDK_Set_Node_En(char *node_id,uint8_t en)
{

    int level = 0;
    level = node_id_level_re(node_id);
    window_node_t *temp = NULL;
    temp =  find_all_key_node(node_id,level);
    if(temp == NULL)
    {
        printf("not find this window :%s \n",node_id);

        return -2;
    }
    printf("find node id:%c%c%c level:%d in node_id:%s\n",temp->node_id[0],temp->node_id[1],temp->node_id[2],level,node_id);
    temp->en_node = en;
    
    return 0;

}

//ok
int     Image_SDK_Set_Node_En_Freshen(char *node_id, NODE_FRESHEN_ARRT en_freshen)
{

    int level = 0;
    level = node_id_level_re(node_id);
    window_node_t *temp = NULL;
    temp =  find_all_key_node(node_id,level);
    if(temp == NULL){
        printf("not find this window :%s \n",node_id);
        return -2;
    }
    temp->freshen_arrt = en_freshen;
    printf("temp set freansh_arrt:%d,node_id:%c%c%c\n",temp->freshen_arrt,temp->node_id[0],
            temp->node_id[1],temp->node_id[2]); 
    return 0;

}

int     Image_SDK_Set_Node_Order(char *node_id,NODE_ORDER_ATTR _attr){

    int level = 0;
    level = node_id_level_re(node_id);
    window_node_t *temp = NULL;
    temp =  find_all_key_node(node_id,level);
    if(temp == NULL)
    {
        printf("not find this window :%s \n",node_id);

        return -2;
    }
   // printf("find node id:%c%c%c level:%d in node_id:%s\n",
     //       temp->node_id[0],temp->node_id[1],temp->node_id[2],level,node_id);
    
    temp->order_attr = _attr;
    
    return 0;


}


int     Image_SDK_Set_Text_Node_Text(char *node_id, char *text,int size)
{

    
    int level = 0;
    level = node_id_level_re(node_id);
    window_node_t *temp = NULL;
    temp =  find_all_key_node(node_id,level);
    if(temp == NULL){
 //       printf("not find node_id\n");
        return -2;
    }
    //printf("win_type:%d\n",temp->win_type);
    if(temp->win_type != OBJECT_TEXT_WIN)
        return -3;

    window_node_text_t *tt = (window_node_text_t *)temp->window;
     
  //  printf("find node:%p tt->lens:%d\n",temp,tt->lens);
    if(tt->lens < size)
        return -5;

    memcpy(tt->text_cache,text,size);
    
    tt->text_cache[size] = '\0';
//    printf("set char tt->text:%s\n",tt->text_cache);
    temp->freshen_arrt = NEED_FRESHEN;
    
    return 0;

}

int     Image_SDK_Set_Line_Node_Param(char *node_id, window_node_line_t  *lt)
{

    if(node_id == NULL || lt == NULL)
        return -1;
    int level = 0;
    level = node_id_level_re(node_id);
    window_node_t *temp = NULL;
    temp =  find_all_key_node(node_id,level);
    if(temp == NULL){
 //       printf("not find node_id\n");
        return -2;
    }
    //printf("win_type:%d\n",temp->win_type);
    if(temp->win_type != OBJECT_LINE)
        return -3;

    window_node_line_t *tt = (window_node_line_t *)temp->window;
    
    temp->freshen_arrt = NEED_FRESHEN;
    
    *tt = *lt;
    tt->this_node = temp;
    
    return 0;

}




static inline int image_buttont_xy_analysis(void *data,window_func_t *set,
        GK_MOUSE_DATA mdata)
{
    
    window_node_button_t *bt = (window_node_button_t *)data;
    //GK_MOUSE_DATA  mdata = sdk_handle->mouse_new_data;
    //mdata.x = x + 

    if( (mdata.x  >= bt->x || mdata.x+ MOUSE_SIZE >= bt->x)  && mdata.x <= bt->x+bt->w 
            && (mdata.y >= bt->y || mdata.y + MOUSE_SIZE >= bt->y)&& mdata.y <= bt->y+bt->h){
         
        bt->this_node->mouse_data = sdk_handle->mouse_new_data; 
        //printf("mouse check mouse.x:%d mouse.y:%d button.x:%d button.y:%d\n",mdata.x,mdata.y,
          //      bt->x,bt->y);
        return 1;
    }
    else 
        return 0;

}

static inline int image_menu_xy_analysis(void *data,
        window_func_t *set,GK_MOUSE_DATA mdata){
    
    window_node_menu_t *bt = (window_node_menu_t *)data;
    //GK_MOUSE_DATA  mdata = sdk_handle->mouse_new_data;
    
    if( mdata.x  >= bt->x  && mdata.x <= bt->x+bt->w 
            && mdata.y >= bt->y && mdata.y <= bt->y+bt->h)
    {
        
        bt->this_node->mouse_data = sdk_handle->mouse_new_data; 

        return 1;
    }
    else 
        return 0;

}

static inline int image_line_xy_analysis(void *data,
        window_func_t *set,GK_MOUSE_DATA mdata){
    
    
    window_node_line_t *bt = (window_node_line_t *)data;
    
 //   GK_MOUSE_DATA  mdata = sdk_handle->mouse_new_data;
    //printf("chcek  line set->data:%p line:%p\n" ,set->data,bt);
    if( mdata.x  >= bt->start_x  && mdata.x <= bt->end_x 
            && mdata.y >= bt->start_y-5 && mdata.y <= bt->start_y+bt->size+5){
         
        bt->this_node->mouse_data = sdk_handle->mouse_new_data; 

        return 1;
    }
    else 
        return 0;

    
    return 0;

}


static inline int image_text_xy_analysis(void *data,
        window_func_t *set,GK_MOUSE_DATA mdata){
    
    
    window_node_text_t *bt = (window_node_text_t *)data;
    
    //GK_MOUSE_DATA  mdata = sdk_handle->mouse_new_data;
    //printf("chcek  line set->data:%p line:%p\n" ,set->data,bt);
    if( mdata.x  >= bt->x  && mdata.x <= bt->x + bt->asc_width * bt->lens 
            && mdata.y >= bt->y  && mdata.y <= bt->y+bt->font_size){
       
         bt->this_node->mouse_data = sdk_handle->mouse_new_data; 

         return 1;
    }
    else 
        return 0;

    
    return 0;

}

static inline int image_bar_xy_analysis(void *data,
        window_func_t *set,GK_MOUSE_DATA mdata){
    
    
    window_node_bar_t *bt = (window_node_bar_t *)data;
    
    //GK_MOUSE_DATA  mdata = sdk_handle->mouse_new_data;
    //printf("chcek  line set->data:%p line:%p\n" ,set->data,bt);
    if( (mdata.x + MOUSE_SIZE /2)  >= bt->x  && (mdata.x + MOUSE_SIZE/2) <= bt->x + bt->w  
            && (mdata.y + MOUSE_SIZE /2) >= bt->y - 2  && (mdata.y+ MOUSE_SIZE/2) <= (bt->y+bt->h+2)){
        //add mouse data
        
        bt->this_node->mouse_data = sdk_handle->mouse_new_data; 
        printf("check bar \n"); 
        return 1;
    }
    else 
        return 0;

    
    return 0;

}




static  inline  window_node_t    *find_frist_free_node(window_node_t *head)
{   
    window_node_t *temp = head;
    
    for(;temp != NULL; temp =  temp->next){
        if(temp->order_attr == FREE_ORDER)
            break;
    }
    
    return temp;

}


static void   _image_analysis_mdata(GK_MOUSE_DATA mdata)

{
    
    if(sdk_handle->root == NULL)
        return ;
   
    int ks;
    
    //clear sdk hanlde save mouse check node buf
    for(ks = 0 ; ks < MENU_LEVEL ; ks++)
        sdk_handle->check_node[ks] = NULL;
    
    //set sdk handle last node is mouse not check mode
    for(ks = 0; ks < MENU_LEVEL ; ks ++){
        if(sdk_handle->last_check_node[ks] == NULL)
            break;
        sdk_handle->last_check_node[ks]->check_node = 0;
    }

    window_node_t  *node = NULL,*save_node[MENU_LEVEL+1],*ft_stack[MENU_LEVEL] ;
    memset(save_node,0x0,sizeof(void *) * MENU_LEVEL+1);
    int check_cnt = 0,ret = 0,stack_cnt = 1;
    
    ft_stack[0] = sdk_handle->root;
       
    node = sdk_handle->root->s_head;
     
    while ( node  ){
        
        if(node->en_node){

            switch(node->win_type)
            {
                case OBJECT_BUTTION:

                    ret = image_buttont_xy_analysis(node->window,NULL,mdata);
                    break;
                case OBJECT_MENU:
                    ret = image_menu_xy_analysis(node->window,NULL,mdata);
                    break;
                case OBJECT_LINE:
                    ret = image_line_xy_analysis(node->window,NULL,mdata);
                    break;
                case OBJECT_TEXT_WIN:
                    ret  = image_text_xy_analysis(node->window,NULL,mdata);
                    break;
                case OBJECT_BAR:
                    ret  = image_bar_xy_analysis(node->window,NULL,mdata);
                    break;
                default:
                    break;
            }
        }

        // 
        if(ret > 0)
        {
            if( check_cnt == 0 || (check_cnt > 0 && node->f_node == save_node[check_cnt - 1])){
                node->check_node = 1;
                save_node[check_cnt] = node;
                check_cnt++;
            
                printf("check node_id:%c%c%c node:%p check_cnt:%d\n\n",node->node_id[0],node->node_id[1],
                    node->node_id[2],node,check_cnt);
            }
        }
        
        if(ret > 0  && node->s_head == NULL ){
            break;
        
        }

        // if check node have childe node ,than tarlve the son node 
        // else tarvle bother node
        if( ret > 0  && node->s_head != NULL && node->en_submenu != 0 ){

            ft_stack[stack_cnt] = node;
            stack_cnt++;
            node = node->s_head;

        }else{
            node = node->next;
        }
        
        if(node == NULL && stack_cnt > 0 ){
            stack_cnt--;
            node = ft_stack[stack_cnt]->next;

        }
    
    }
    
    // there have a bug ,now the mouse data get a pthread ,poces s another thread 
    if( check_cnt != 0 )
    {
        if(save_node[check_cnt -1]->last_event == sdk_handle->mouse_new_data.event
                &&save_node[check_cnt - 1]->move_arrt == NOT_MOVE )
        {
            check_cnt = 0;
        }
    }
     
    //printf("########mdata.event:%d\n",mdata.event);
    //move top
    window_node_t *temp = NULL;
    int  k;

    for (k  =  0 ; k < check_cnt ; k++)
    {
        // ****** #### *****
        // if the mouse is offset event ,is not need move node,but maybe ....
        // some scene have bugs.....
        if(mdata.event == GK_MOUSE_OFFSET)
            break;

        node = save_node[k];
        // the node is head or FIXD node ,not need move 
        if(node == node->f_node->s_head || node->order_attr == FIXD_ORDER )
            continue;
        //if it is perv node is fixd node ,not need move
        if(node->prev->order_attr == FIXD_ORDER)
            continue;

        node->prev->next = node->next;
        if(node->next){
            node->next->prev = node->prev;
        }else{ //is end
            node->f_node->s_end = node->prev;
        }
        //need find free fisrt node 
        temp = find_frist_free_node(node->f_node->s_head);
        if(temp == NULL)
            continue;
        
        node->prev = temp->prev;
        node->next = temp;
        temp->prev = node;
        if(node->prev)
            node->prev->next = node;
        else{ 
            node->f_node->s_head = node;
        }
        //if the video state is put ,and the freshen not is clear than need update
        if(node->video_state == VIDEO_STATE && node->freshen_arrt != NEED_CLEAR){
            node->freshen_arrt = NEED_FRESHEN;

            printf("##2######mdata.event:%d\n",mdata.event);
        }
    }
    
    sdk_handle->check_level_cnt = check_cnt;
    //save now mouse check node
    for (k  =  0 ; k < check_cnt ; k++){
        sdk_handle->check_node[k] = save_node[k];
        sdk_handle->check_node[k]->last_event = sdk_handle->mouse_new_data.event;
    }
    
    return  ;

 }

static window_func_t inline *get_window_func(window_node_t *node)
{


    if(node == NULL)
        return NULL;

    window_func_t *func = NULL;

    switch(node->win_type){

        case OBJECT_BUTTION:
            func = &(((window_node_button_t *)node->window)->video_set);
            break;
        case OBJECT_MENU:
            func = &(((window_node_menu_t *)node->window)->video_set);
            break;
        case OBJECT_LINE:
            func = &(((window_node_line_t *)node->window)->video_set);
            break;
        case OBJECT_TEXT_WIN:
            func = &(((window_node_text_t *)node->window)->video_set);
            break;
        case OBJECT_BAR:
            func = &(((window_node_bar_t *)node->window)->video_set);
            break;
        default:
            func = NULL;
            break;
    }


    return func;;
}

static void  _image_window_func_run(void *data)
{
    
    if(sdk_handle->root->s_head == NULL)
        return;
    window_node_t  *node = NULL ;
    int k = 0;
    //build leave event and run
    window_func_t *funcs = NULL;
    for(k = MENU_LEVEL -1  ; k >= 0; k--){
        
        if(sdk_handle->last_check_node[k] == NULL)
            continue;
            
        node = sdk_handle->last_check_node[k];
       // printf("node->chcek_node:%d\n",node->check_node);
        
        if(node->check_node)
            continue;

        funcs = get_window_func(node);
        if(funcs->mouse_leave){
            printf("func:%s line:%d run leave func %c%c%c \n",
                    __func__,__LINE__,node->node_id[0],node->node_id[1],
                    node->node_id[2]); 
            funcs->mouse_leave(funcs->data);
        }
        node->last_event = 0;
        sdk_handle->last_check_node[k] = NULL; 
    }
    
    if(sdk_handle->check_level_cnt == 0)
        return;
    //clear save buf
    for(k = 0; k < MENU_LEVEL ; k++){
        sdk_handle->last_check_node[k] = NULL;
    }
    //clear flag and save
    for(k = 0 ;k < sdk_handle->check_level_cnt; k++){
        sdk_handle->check_node[k]->check_node = 0;
        sdk_handle->last_check_node[k] = sdk_handle->check_node[k];
    }
    

    //get top event,and only run top window event 
    node = sdk_handle->check_node[sdk_handle->check_level_cnt -1];
    
    //clear levevl save 
    sdk_handle->check_level_cnt = 0; 
    
    if(!node->en_node)
        return;
    
    funcs = get_window_func(node);
    if(funcs == NULL)
        return;
    switch(sdk_handle->mouse_new_data.event)

    {

        case GK_MOUSE_OFFSET:
            if(funcs->mouse_offset)    
                funcs->mouse_offset(funcs->data);
            break;
        case GK_MOUSE_LEFT_DOWN:
            if(funcs->mouse_left_down)    
                funcs->mouse_left_down(funcs->data);
            break;
        case GK_MOUSE_LEFT_UP:
            if(funcs->mouse_left_up)
                funcs->mouse_left_up(funcs->data);
            break;
        case GK_MOUSE_RIGHT_DOWN:
            if(funcs->mouse_right_down)
                funcs->mouse_right_down(funcs->data);
            break;
        case GK_MOUSE_RIGHT_UP:
            if(funcs->mouse_right_up)    
                funcs->mouse_right_up(funcs->data);
            break;
        case GK_MOUSE_LEAVE:
            if(funcs->mouse_leave)
                funcs->mouse_leave(funcs->data);
            break;
        
        case GK_MOUSE_NOTHOR:
            break;
        
        case GK_MOUSE_DEL:
            break;
        
        default:
            break;
    }
#if 0
    //clear buf
    for(k = 0; k < MENU_LEVEL ; k++){
        sdk_handle->last_check_node[k] = NULL;
    }
    //clear flag and save
    for(k = 0 ;k < sdk_handle->check_level_cnt; k++){
        sdk_handle->check_node[k]->check_node = 0;
        sdk_handle->last_check_node[k] = sdk_handle->check_node[k];
    }
    sdk_handle->check_level_cnt = 0; 
#endif
    return ;

}



static void freshen_image_buttion(void *data){

    window_node_button_t *bt =  (window_node_button_t *)data;

    uint16_t *buf = (uint16_t *)sdk_handle->mmap_p; 

    if(bt->user_video_freshen){
      //  printf("__freshen_image_buttion__\n");
        bt->user_video_freshen(data,buf,VO_SCREE_W,VO_SCREE_H);
        bt->this_node->freshen_arrt = NORTHING;
        bt->this_node->video_state = VIDEO_STATE;
        return;
    }


    int k,i;
    //
    if(bt->this_node->freshen_arrt  == NEED_CLEAR){
        printf("button clear\n");
        for(k = bt->last_y ; k < (bt->h + bt->last_y) ;k++){
            for(i = bt->last_x ;  i < (bt->w + bt->last_x) ; i++ )
                *(buf+ sdk_handle->scree_w*k + i) = 0;    
        }
        bt->this_node->video_state = CLEAR_STATE;
    }else if(bt->this_node->freshen_arrt == NEED_FRESHEN){
    
        printf("freshen button\n");
        //if open move attr ,need clear before x,y
        if(bt->this_node->move_arrt != NOT_MOVE){
            for(k = bt->last_y ; k < (bt->h + bt->last_y) ;k++){
                for(i = bt->last_x ;  i < (bt->w + bt->last_x) ; i++ )
                    *(buf+ sdk_handle->scree_w*k + i) = 0;    
            }
        }

        for(k = bt->y ; k < (bt->h + bt->y) ;k++){
            for(i = bt->x ;  i < (bt->w + bt->x) ; i++ )
                *(buf+ sdk_handle->scree_w*k + i) = bt->color;    
        }
        bt->last_x = bt->x;
        bt->last_y = bt->y;
        bt->this_node->video_state = VIDEO_STATE;
    }
   
    bt->this_node->freshen_arrt = NORTHING;

}

static void freshen_image_menu(void *data){
    
    window_node_menu_t *bt =  (window_node_menu_t *)data;
    
    uint16_t *buf = (uint16_t *)sdk_handle->mmap_p; 
    
    if(bt->user_video_freshen){

        bt->user_video_freshen(data,buf,VO_SCREE_W,VO_SCREE_H);
        bt->this_node->video_state = VIDEO_STATE;
        return;
    }

    int k,i ,s;
    //
    if(bt->this_node->freshen_arrt  == NEED_CLEAR){
        printf("clear menu \n");

        for(k = bt->last_y ; k < (bt->last_h + bt->last_y) ;k++){
            for(i = bt->last_x ;  i < (bt->last_w + bt->last_x) ; i++ )
                *(buf+ sdk_handle->scree_w*k + i) = 0;    
        }
        bt->this_node->video_state = CLEAR_STATE;

    }else if(bt->this_node->freshen_arrt == NEED_FRESHEN){
       // printf("push mune video \n");
        //if open move attr ,need clear before x,y
        if(bt->this_node->move_arrt != NOT_MOVE){
            for(k = bt->last_y ; k < (bt->h + bt->last_y) ;k++){
                for(i = bt->last_x ;  i < (bt->w + bt->last_x) ; i++ )
                    *(buf+ sdk_handle->scree_w*k + i) = 0;    
            }
        }
        
        for(s = 0 ,k = bt->y ; k < (bt->h + bt->y) ;k++) {
            for(i = bt->x ;  i < (bt->w + bt->x) ; i++,s++ )
            {
               
                if(bt->image_cache){
                  *(buf+ sdk_handle->scree_w*k + i) = *(((int16_t *)bt->image_cache) +s);    
                }else{
                    *(buf+ sdk_handle->scree_w*k + i) = bt->color;
                }
             
            }    
        }
        bt->last_h = bt->h;
        bt->last_w = bt->w;
        bt->last_x = bt->x;
        bt->last_y = bt->y;
        
        bt->this_node->video_state = VIDEO_STATE;

    }

    bt->this_node->freshen_arrt = NORTHING;
    
    return ;
       
}
static void freshen_image_line(void *data){
    
    window_node_line_t *bt =  (window_node_line_t *)data;
    
   uint16_t *buf = (uint16_t *)sdk_handle->mmap_p; 
    
    if(bt->user_video_freshen){

        bt->user_video_freshen(data,buf,VO_SCREE_W,VO_SCREE_H);

        return;
    }
    int k,i;
    //
    if(bt->this_node->freshen_arrt  == NEED_CLEAR){

        for(k = bt->last_y ; k < (bt->size + bt->last_y) ;k++){
            for(i = bt->last_x ;  i < bt->end_x ; i++ )
                *(buf+ sdk_handle->scree_w*k + i) = 0;    
        }
    }else if(bt->this_node->freshen_arrt == NEED_FRESHEN){

        //if open move attr ,need clear before x,y
        if(bt->this_node->move_arrt != NOT_MOVE){
            for(k = bt->last_y ; k < (bt->size + bt->last_y) ;k++){
                for(i = bt->last_x ;  i < bt->end_x  ; i++ )
                    *(buf+ sdk_handle->scree_w*k + i) = 0;    
            }
        }

        for(k = bt->start_y ; k < (bt->size + bt->start_y) ;k++){
            for(i = bt->start_x ;  i < bt->end_x  ; i++ )
                *(buf+ sdk_handle->scree_w*k + i) = bt->color;    
        }
        bt->last_x = bt->start_x;
        bt->last_y = bt->start_y;
    }
    
    return ;
}


static void freshen_image_bar(void *data){
    
    window_node_bar_t *bt =  (window_node_bar_t *)data;
    
   uint16_t *buf = (uint16_t *)sdk_handle->mmap_p; 
    
    if(bt->user_video_freshen){

        bt->user_video_freshen(data,buf,VO_SCREE_W,VO_SCREE_H);

        return;
    }
    int k,i;
    //
    if(bt->this_node->freshen_arrt  == NEED_CLEAR){

        for(k = bt->last_y ; k < (bt->h + bt->last_y) ;k++){
            for(i = bt->last_x ;  i < (bt->last_x +bt->w); i++ )
                *(buf+ sdk_handle->scree_w*k + i) = 0;    
        }
    }else if(bt->this_node->freshen_arrt == NEED_FRESHEN){

        for(k = bt->y ; k < (bt->h + bt->y) ;k++){
            for(i = bt->x ;  i < (bt->x +bt->w) ; i++ )
                *(buf+ sdk_handle->scree_w*k + i) = 0xFFFF;    
        }
        int vl = bt->w * bt->now_value/bt->max_value;
        printf("xxxx bar bar now need push bar lens:%d  bt->w:%d now_value:%d color:%d\n",vl,bt->w,bt->now_value,
                bt->bar_color);
        for(k = bt->y ; k < (bt->h + bt->y) ;k++){
            for(i = bt->x ;  i < ( bt->x + vl )  ; i++ )
                *(buf+ sdk_handle->scree_w*k + i) = bt->bar_color;    
        }
        bt->last_x = bt->x;
        bt->last_y = bt->y;
                
        bt->this_node->freshen_arrt = NORTHING;

    }
    
    return ;
}
//need wirte text load lib

static void freshen_image_text(void *data){

    window_node_text_t *bt =  (window_node_text_t *)data;

    uint16_t *buf = (uint16_t *)sdk_handle->mmap_p; 
    
    if(bt->user_video_freshen){

        bt->user_video_freshen(data,buf,VO_SCREE_W,VO_SCREE_H);

        return;
    }
    uint8_t  *text_bit = NULL; 

    int k,i;
    //
    if(bt->this_node->freshen_arrt  == NEED_CLEAR){

        for(k = bt->last_y ; k < (bt->font_size + bt->last_y) ;k++){
            for(i = bt->last_x ;  i < (bt->asc_width *bt->lens + bt->last_x) ; i++ )
                *(buf+ sdk_handle->scree_w*k + i) = bt->win_color;    
        }
    }else if(bt->this_node->freshen_arrt == NEED_FRESHEN){

        for(k = bt->last_y ; k < (bt->font_size + bt->last_y) ;k++){
            for(i = bt->last_x ;  i < (bt->asc_width *bt->lens + bt->last_x) ; i++ )
                *(buf+ sdk_handle->scree_w*k + i) = bt->win_color;    
        }

        char  s;
        int   nums = 0 ;

        for(nums = 0; nums < bt->lens; nums++){
            s = bt->text_cache[nums];
            if(s == '\0')
                break;
            //printf("freshen put c:%c \n",s);
            text_bit = image_text_lib_put_pixl(&s);

            if(text_bit == NULL)
                break;
            int j = 0; 
            for(k = bt->y ; k < (bt->font_size + bt->y) ;k++){
                for(i = bt->x + nums*bt->asc_width ;  i < (bt->x + nums*bt->asc_width+bt->asc_width) ; i++,j++ )
                {
                    if(text_bit[j] == 0)
                        *(buf+ sdk_handle->scree_w*k + i) = bt->win_color;    
                    else
                        *(buf+ sdk_handle->scree_w*k + i) = bt->text_color;  
                }
            }
        }

        bt->this_node->freshen_arrt = NORTHING;

        bt->last_x = bt->x;
        bt->last_y = bt->y;
    }

    return ;


}


static void freshen_image_mouse(void)
{
    
    
    
    if(!sdk_handle->mouse_data_updated)
        return  ;


    sdk_handle->mouse->x = sdk_handle->mouse_new_data.x;
    sdk_handle->mouse->y = sdk_handle->mouse_new_data.y;
    window_node_mouse_t *bt = sdk_handle->mouse;
    uint16_t *buf = (uint16_t *)sdk_handle->mmap_p; 
    int k,i;
 
#define  MOUSE_LINE_SIZE    2
    //reset data
    //clear
#if 1
  //  printf("frensh mouse \n");
    //1.reset data  2.save data 3.push data
    
    uint16_t *savebuf ;
    savebuf =  (uint16_t *)bt->save_cache;
 
    //reset data 
    for(k = bt->y_last + bt->size/2; k < (MOUSE_LINE_SIZE + bt->y_last+bt->size/2) ; k++)
    {
        for(i = bt->x_last ;  i < (bt->size + bt->x_last) ; i++){
            if( *(buf+ sdk_handle->scree_w*k + i) == bt->color )  //if mouse use image ,this need change
                    *(buf+ sdk_handle->scree_w*k + i) = *savebuf++;
            else
            {
                    savebuf++; 
                    //printf(" W :srcecc buf x,y peixl:%u,mouse color:%u\n",*(buf + sdk_handle->scree_w*k +i),bt->color);
            }
        }
    }
    
    for(k = bt->x_last + bt->size/2 ; k < (MOUSE_LINE_SIZE + bt->x_last+bt->size/2) ; k++)
    {
        for(i = bt->y_last ;  i < (bt->size + bt->y_last) ; i++){
            if(*(buf+ sdk_handle->scree_w*i + k) == bt->color) //if mouse use image ,this need change        
                *(buf+ sdk_handle->scree_w*i + k) = *savebuf++;
            else{
              //  printf("H:srecc buf,x,y peixl:%u,mouse color:%u\n",
                //               *(buf + sdk_handle->scree_w*i + k),bt->color);
                savebuf++;
            }
        }
    }
    //save data;
    savebuf =  (uint16_t *)bt->save_cache;

    for(k = bt->y + bt->size/2 ; k < (MOUSE_LINE_SIZE + bt->y+bt->size/2) ; k++)
    {
        for(i = bt->x ;  i < (bt->size + bt->x) ; i++ )
           *savebuf++  =  *(buf+ sdk_handle->scree_w*k + i);
    }
    for(k = bt->x + bt->size/2 ; k < (MOUSE_LINE_SIZE + bt->x + bt->size/2) ; k++)
    {
        for(i = bt->y ;  i < (bt->size + bt->y) ; i++ )
            *savebuf++  = *(buf+ sdk_handle->scree_w*i + k);
    } 

    //pushdata
    for(k = bt->y + bt->size/2 ; k < (MOUSE_LINE_SIZE + bt->y+bt->size/2) ; k++)
    {
        for(i = bt->x ;  i < (bt->size + bt->x) ; i++ ){
            *(buf+ sdk_handle->scree_w*k + i) = bt->color;
           // printf("mem :%d \n",*(buf + sdk_handle->scree_w*k +i));
        }
    }
    for(k = bt->x + bt->size/2 ; k < (MOUSE_LINE_SIZE + bt->x + bt->size/2) ; k++)
    {
        for(i = bt->y ;  i < (bt->size + bt->y) ; i++ )
            *(buf+ sdk_handle->scree_w*i + k) = bt->color;
    } 
#endif

#if 0
    for(k = bt->y_last + bt->size/2 ; k < (MOUSE_LINE_SIZE + bt->y_last+bt->size/2) ; k++)
    {
        for(i = bt->x_last ;  i < (bt->size + bt->x_last) ; i++ ,j++)
            *(buf+ sdk_handle->scree_w*k + i) = 0;
    }
    for(k = bt->x_last + bt->size/2 ; k < (MOUSE_LINE_SIZE + bt->x_last+bt->size/2) ; k++)
    {
        for(i = bt->y_last ;  i < (bt->size + bt->y_last) ; i++ )
            *(buf+ sdk_handle->scree_w*i + k) = 0;
    }

    //push
    for(k = bt->y + bt->size/2 ; k < (MOUSE_LINE_SIZE + bt->y+bt->size/2) ; k++)
    {
        for(i = bt->x ;  i < (bt->size + bt->x) ; i++ ,j++)
            *(buf+ sdk_handle->scree_w*k + i) = bt->color;;
    }
    for(k = bt->x + bt->size/2 ; k < (MOUSE_LINE_SIZE + bt->x + bt->size/2) ; k++)
    {
        for(i = bt->y ;  i < (bt->size + bt->y) ; i++ )
            *(buf+ sdk_handle->scree_w*i + k) = bt->color;
    } 
#endif


#if 0
    //push 
    for(k = bt->y,j= 0 ; k < (bt->size + bt->y) ;k++){
        for(i = bt->x ;  i < (bt->size + bt->x) ; i++,j++)

            *(buf+ sdk_handle->scree_w*k + i) = *(((int16_t *)bt->image_p)+j);    
    }
   
#endif
    bt->x_last = bt->x;
    bt->y_last = bt->y;
    
    return ;
}


static inline void _image_freshen(window_node_t *node)
{
   
    switch(node->win_type){
        
        case OBJECT_BUTTION:
            freshen_image_buttion(node->window);
            break;
        case OBJECT_MENU:
            freshen_image_menu(node->window);
 
            break;

        case OBJECT_LINE:
             freshen_image_line(node->window);

            break;
        case OBJECT_TEXT_WIN:
            freshen_image_text(node->window);
            break;
        case OBJECT_BAR:
            freshen_image_bar(node->window);
            break;

        default:
            break;
    }

}




static void image_freshen_set_sub_freshen(window_node_t *node)
{
    window_node_t  *temp = node->s_head;
    for(;temp != NULL;temp = temp->next){
        if(temp->video_state == VIDEO_STATE && temp->freshen_arrt ==  NORTHING)
            temp->freshen_arrt = NEED_FRESHEN; 
    }
    return;

} 

static inline void image_set_bother_clear_freshen(window_node_t *head)
{
   
    window_node_t *f_node = head->f_node;
    // if father menu is need disp 
    if(f_node->video_attr == OPEN_DISP && f_node != sdk_handle->root)
    {
        
        f_node->freshen_arrt = NEED_FRESHEN;
        for(;head != NULL;head = head->next)
        {
            
            if( head->freshen_arrt == NEED_CLEAR || (head->video_state == CLEAR_STATE
                    && head->freshen_arrt != NEED_FRESHEN))
            {
                head->freshen_arrt =  NORTHING;
            }else{  
                head->freshen_arrt = NEED_FRESHEN;
            }
        }
        return;
    }
    // father menu is not disp
    for(;head != NULL;head = head->next)
    {
        if(head->en_intersection){
            if(head->video_state == VIDEO_STATE && head->freshen_arrt != NEED_CLEAR)
                head->freshen_arrt = NEED_FRESHEN;
        }    
    }
    return;

}

static void image_clear_video(void)
{

    //clear biggan the head  
    if(sdk_handle->root->s_head == NULL)
        return ;


    window_node_t *node = NULL,*ft_stack[MENU_LEVEL],*clear_stack[100];
    int stack_cnt = 1,clear_cnt = 0;
    ft_stack[0] = sdk_handle->root;

    node = sdk_handle->root->s_head;
    //set attr
    while ( node  )
    {

        if( node->freshen_arrt == NEED_CLEAR)
        {

            if(node->en_intersection){
                image_set_bother_clear_freshen(node->f_node->s_head); 
            }
            clear_stack[clear_cnt] = node;
            clear_cnt++;
        }

        if(node->en_submenu && node->s_head != NULL){
            ft_stack[stack_cnt] = node;
            node = node->s_head;
            stack_cnt++;
        }else{
            node = node->next;
        }
        if(node == NULL){
            stack_cnt--;
            node = ft_stack[stack_cnt]->next;
        }         

    }
    //clear not user order
    int i = 0 ;

    for(i = 0 ; i < clear_cnt ; i++)
    {
        if(clear_stack[i]->freshen_arrt == NEED_CLEAR)
             _image_freshen(clear_stack[i]);
    }

    
    return;

}

static inline void image_set_bother_put_freshen(window_node_t *node)
{
    node = node->prev; 
    for( ; node != NULL; node = node->prev)
    {
        if(node->en_intersection){
            if(node->video_state == VIDEO_STATE && node->freshen_arrt != NEED_CLEAR)
                node->freshen_arrt = NEED_FRESHEN;
        }    
    }
    return;

}
static inline int  find_set_freshen_fnode(window_node_t **savebuf,window_node_t *fnode,int cnt)
{
    
    int i = 0;
    for(i  = 0 ; i < cnt ;i++)
        if(savebuf[i] == fnode)
            return 1;
    
    return 0;
}

static void  image_put_video(void)
{
    //tarlve list set all  ,very level only set one times
    if(sdk_handle->root->s_end == NULL)
        return ;
    
  
    window_node_t *node = NULL,*ft_stack[MENU_LEVEL],*save_set[100];
    memset(save_set,0x0,sizeof(void *)* 100);

    int stack_cnt = 1,save_cnt = 0;
    ft_stack[0] = sdk_handle->root;
    
    node = sdk_handle->root->s_end;
    //set freshen attr
    while ( node  ){
      
        if(node->en_node && node->freshen_arrt == NEED_FRESHEN )
        {
            
            //the every level only set one times the fshen attr ,becares the image put run before,compler the all
            //window intersection attr,and set flags
            if(node->en_intersection && !find_set_freshen_fnode(save_set,node->f_node,save_cnt))
            {
                image_set_bother_put_freshen(node);
                save_set[save_cnt] = node->f_node;
                save_cnt++;
            }

            //freshen now node
            if(node->video_attr == OPEN_DISP)
                _image_freshen(node);
            // if have sub window set sub window freshen
            if(node->en_submenu && node->s_head != NULL)
                image_freshen_set_sub_freshen(node);
        }
        
        if(/*node->en_submenu &&*/ node->s_end != NULL){
            ft_stack[stack_cnt] = node;
            node = node->s_end;
            stack_cnt++;
        }else{
            node = node->prev;
        }
        if(node == NULL){
            stack_cnt--;
            node = ft_stack[stack_cnt]->prev;
        }         
           
    }

    
    
    return ;

}



static void  _image_freshen_video(void)
{
   
    image_clear_video();
    image_put_video();
    freshen_image_mouse();
    return ;

}

static  void*   _image_mouse_event_read_thread(void *data)
{
    
    GK_MOUSE_DATA       mdata;
    memset(&mdata,0x0,sizeof(GK_MOUSE_DATA));

    fd_set              readfds;
    struct  timeval     tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0 ;
    int                 retval = 0;
    while(1)
    {

        //host instret mouse
        if (sdk_handle->need_restart_mouse)
        {

            sdk_handle->mouse_fd =  
                gk_mouse_open(GK_DEVICE_MOUSE,VO_SCREE_W,VO_SCREE_H,GK_MOUSE_SPEED,MOUSE_SIZE);
            sdk_handle->need_restart_mouse = 0;
        }
        //read mouse data
        if(sdk_handle->mouse_fd > 0)
        {

            FD_ZERO(&readfds);
            FD_SET(sdk_handle->mouse_fd, &readfds);
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            retval = select(sdk_handle->mouse_fd+1,&readfds,NULL,NULL,&tv);
            if(FD_ISSET(sdk_handle->mouse_fd,&readfds))
            {

                if(gk_mouse_read_data(sdk_handle->mouse_fd,&mdata) < 0 )
                {
                    continue;
                }
                sdk_handle->mouse_new_data = mdata;
                sdk_handle->mouse_data_updated = 1;

            }
        }else{
            sleep(1);
        }
    }

    return NULL;
}

static void* _image_sdk_handle_data_process_thread(void *data)
{
    
    
    int wait_times = 1000*1000 / sdk_handle->disp_fps;

    GK_MOUSE_DATA _ldata;
    
    
    while(1)
    {

        if(sdk_handle->mouse_data_updated){

            _ldata = sdk_handle->mouse_new_data;

            _image_analysis_mdata(sdk_handle->mouse_new_data);
            _image_window_func_run(NULL);

            if(_ldata.x == sdk_handle->mouse_new_data.x &&
                    _ldata.y == sdk_handle->mouse_new_data.y &&
                    _ldata.event == sdk_handle->mouse_new_data.event)
                sdk_handle->mouse_data_updated = 0;

        }else{

            usleep(wait_times);
        } 

        _image_freshen_video();
    }

    return NULL;

}

struct  update_param_save{

    uint16_t    x;
    uint16_t    y;
    uint16_t    end_x;
    uint16_t    end_y;
    WIN_TYPE_S  win_type; 
};

static  int update_param_get(window_node_t *node,struct update_param_save *data)
{
    data->win_type = node->win_type;
    switch(node->win_type){
        
        case OBJECT_BUTTION:
            data->x         = ((window_node_button_t *)(node->window))->x;
            data->y         = ((window_node_button_t *)(node->window))->y;
            data->end_x     = ((window_node_button_t *)(node->window))->w + data->x;
            data->end_y     = ((window_node_button_t *)(node->window))->h + data->y;
            
            break;
        case OBJECT_MENU:
            data->x         = ((window_node_menu_t *)(node->window))->x;
            data->y         = ((window_node_menu_t *)(node->window))->y;
            data->end_x     = ((window_node_menu_t *)(node->window))->w + data->x;
            data->end_y     = ((window_node_menu_t *)(node->window))->h + data->y;
            break;

        case OBJECT_LINE:
            
            data->x         = ((window_node_line_t *)(node->window))->start_x;
            data->y         = ((window_node_line_t *)(node->window))->start_y;
            data->end_x     = ((window_node_line_t *)(node->window))->end_x;
            data->end_y     = ((window_node_line_t *)(node->window))->end_y;
            if(data->x == data->end_x){ // line
                data->end_x = ((window_node_line_t *)(node->window))->size;

            }else{  // row line
               data->end_y = ((window_node_line_t *)(node->window))->size;

            }
            break;
        case OBJECT_TEXT_WIN:
            data->x = ((window_node_text_t *)(node->window))->x;
            data->y = ((window_node_text_t *)(node->window))->y;
            data->end_x = ((window_node_text_t *)(node->window))->lens;
            data->end_x = data->end_x * ((window_node_text_t *)(node->window))->asc_width;
            data->end_y = ((window_node_text_t *)(node->window))->font_size;

            break;
        case OBJECT_BAR:
            data->x = ((window_node_bar_t *)(node->window))->x;
            data->y = ((window_node_bar_t *)(node->window))->y;
            data->end_x = ((window_node_bar_t *)(node->window))->w;
            data->end_y = ((window_node_bar_t *)(node->window))->h;
            break;
        default:
            break;
    }
 
    return 0;
}

static inline int ab_abs(int a,int b)
{
    
    return a > b ? a - b : b - a;

}

static int compler_image(window_node_t *node,window_node_t *temp)
{
        
    struct  update_param_save   data_s,data_n;
    update_param_get(temp,&data_n);
    update_param_get(node,&data_s);
    int x1,y1,x2,y2;
    x1 = (data_s.x - data_s.end_x)/2 + data_s.x;
    x2 = (data_n.x - data_n.end_x)/2 + data_n.x;

    y1 = (data_s.y - data_s.end_y)/2 + data_s.y;
    y2 = (data_n.y - data_n.end_y)/2 + data_n.y;
    
    printf("x1:%d y1:%d x2:%d y2:%d \n",x1,y1,x2,y2); 

    if((ab_abs(x1 , x2) < (data_s.end_x - data_s.x + data_n.end_x - data_n.x)/2) &&
            (ab_abs(y1 , y2) < (data_s.end_y - data_s.y + data_n.end_y - data_n.y)/2)){
        
        node->en_intersection = 1;
        temp->en_intersection = 1;
        printf("%s:line:%d node:%p temp:%p\n",__func__,__LINE__,node,temp);
        return 1;
    }

}


static void  compler_intersection_attr(window_node_t *node)
{
    window_node_t *temp ,*savenode = node;

    for(; node!= NULL; node = node->next )
    {
        if(node->en_intersection == 0)
        {
            for(temp =  savenode ; temp  != NULL ; temp =  temp->next)
            {
                if(temp == node)
                    continue;
                compler_image(node,temp); 
            }    
        }    

    }
    
    return ;

}



static void  _image_all_window_comper_intersection_attr(void)
{
    
    
    
    if(sdk_handle->root->s_head == NULL)
        return ;


    window_node_t *node = NULL,*ft_stack[MENU_LEVEL];
    int stack_cnt = 1,clear_cnt = 0;
    ft_stack[0] = sdk_handle->root;

    node = sdk_handle->root->s_head;
    //set attr
    while ( node  )
    {
        if(node  == node->f_node->s_head){
            
            compler_intersection_attr(node); 
        
        }
        if(node->en_submenu && node->s_head != NULL){
            ft_stack[stack_cnt] = node;
            node = node->s_head;
            stack_cnt++;
        }else{
            node = node->next;
        }
        if(node == NULL){
            stack_cnt--;
            node = ft_stack[stack_cnt]->next;
        }         

    }
       
    return;


}


static  pthread_t   mouse_thread_id, process_thread_id;
void    Image_SDK_Run(void)
{
   



   pthread_create(&mouse_thread_id,NULL,
            _image_mouse_event_read_thread,NULL);
    
    //set all window intersction attr
    
   _image_all_window_comper_intersection_attr();

    pthread_create(&process_thread_id,NULL,
            _image_sdk_handle_data_process_thread,NULL);
  
#if 1  //block use test
    for(;;)
        sleep(1);
    pthread_join(mouse_thread_id,NULL);
    pthread_join(process_thread_id,NULL);

#endif

    return ;

}




static void*  _image_fb_init(char *dev_path,int *fd)
{
    
    int     fbHandle = -1;
    void    *fbBuffer = NULL;

    fbHandle = open (dev_path, O_RDWR);
    if (fbHandle < 0)
    {
        *fd = -1;
        perror("open /dev/fb0 error:\n");
        return NULL;
    }

    struct fb_fix_screeninfo fixInfo;
    struct fb_var_screeninfo varInfo;
    if(ioctl(fbHandle, FBIOGET_FSCREENINFO, &fixInfo) < 0)
    {
        printf("Cannot get fixed screen info\n.");
        close (fbHandle);
        *fd = -1;

        return NULL;
    }

    varInfo.xres = VO_SCREE_W;
    varInfo.yres = VO_SCREE_H;
    varInfo.bits_per_pixel = VO_COLOR_FMT;
    
    if(ioctl(fbHandle, FBIOGET_VSCREENINFO, &varInfo) < 0)
    {
        printf("Cannot get var screen info\n.");
        close (fbHandle);
        *fd = -1;
        return NULL;
    }
    printf("framebuffer: %d x %d @ %d -- %dbpp\n", varInfo.xres, varInfo.yres,
                fixInfo.line_length, varInfo.bits_per_pixel);

    fbBuffer = (unsigned short *)mmap(NULL, fixInfo.smem_len,
                                     PROT_WRITE,  MAP_SHARED, fbHandle, 0);
    printf("framebuffer addr:0x%08x, len=%d\n", (int)fbBuffer, fixInfo.smem_len);
    if (fbBuffer == MAP_FAILED)
    {
        printf("Cannot mmap framebuffer memory.");
        close (fbHandle);
        *fd = -1;
        return NULL;
    }

    memset(fbBuffer,0x0,fixInfo.smem_len);
    *fd = fbHandle;
    
    return fbBuffer;
}

static void*  _image_fb_deinit(void *mmap_p,int fd)

{
    if(mmap_p == NULL || fd < 0)
        return NULL;


    munmap(mmap_p, VO_SCREE_H * VO_SCREE_W *(VO_COLOR_FMT/8));
    close(fd);
    return NULL;
}
static void _image_fb_push(int xoffset,int yoffset){

    
    struct fb_var_screeninfo varInfo;
    varInfo.xres_virtual = VO_SCREE_W;
    varInfo.yres_virtual = VO_SCREE_H;
    varInfo.yoffset      = yoffset;
    varInfo.xoffset      = xoffset;
    if(ioctl(sdk_handle->mmap_p, FBIOPAN_DISPLAY, &varInfo) < 0)
    {
        printf("Cannot display Pan\n.");
        return ;
    }
    return ;
}





