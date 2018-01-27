
#include <string.h>
#include <stdio.h>
#include "image_argb_ayuv.h"
#include "xw_logsrv.h"




static uint16_t *  ayuv_buf = NULL;


int  image_ayuv_talbe_init(int  format){

    if(format > RGB565_YUV565 || format < ARGB4444_AYUV4444)
        return -1;

    if(ayuv_buf != NULL)
        return -2;

    ayuv_buf  = (uint16_t *)malloc((0x1 << 16) *sizeof(uint16_t));
    if(ayuv_buf == NULL)
        return -3;

    int index = 0;
    uint8_t     r,b,g,y,u,v ,a;

    if(format == RGB565_YUV565){

        for (index = 0; index < (0x1 << 16); index++) {
            r = (index & 0xF800) >> 8;
            g = (index & 0x07E0) >> 3;
            b = (index & 0x001F) << 3;

            y = (( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
            u = ((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
            v = ((112 * r -  94 * g -  18 * b + 128) >> 8) + 128;

            y >>= 2;
            u >>= 3;
            v >>= 3;

            ayuv_buf[index] = ((u << 11) | (y << 5) | (v));
        }


    }else if(format == ARGB4444_AYUV4444){
        for (index = 0; index < (0x1 << 16); index++) {
            a = (index & 0xF000) >> 12;
            r = (index & 0x0F00) >> 4;
            g = (index & 0x00F0) >> 0;
            b = (index & 0x000F) << 4;

            y = (( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
            u = ((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
            v = ((112 * r -  94 * g -  18 * b + 128) >> 8) + 128;

            y >>= 4;
            u >>= 4;
            v >>= 4;

            ayuv_buf[index] = ((a << 12) | (y << 8) | (u << 4) | (v << 0));
        }

          xw_logsrv_debug("ARGB4444 0xffff ayuv:%x index:%x\n",ayuv_buf[0xffff],index);

    }else if(format == RGBA4444_AYUV4444){

        for (index = 0; index < (0x1 << 16); index++) {
            r = (index & 0xF000) >> 8;
            g = (index & 0x0F00) >> 4;
            b = (index & 0x00F0) >> 0;
            a = (index & 0x000F) >> 0;

            y = (( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
            u = ((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
            v = ((112 * r -  94 * g -  18 * b + 128) >> 8) + 128;

            y >>= 4;
            u >>= 4;
            v >>= 4;

            ayuv_buf[index] = ((a << 12) | (y << 8) | (u << 4) | (v << 0));
        }
        xw_logsrv_debug(" RGBA4444 0xffff ayuv:%x index:%x\n",ayuv_buf[0xffff],index);

    }else if(format == RGBA8888_AYUV4444){
        for (index = 0; index < (0x1 << 16); index++) {
          
                       
#if 1
            a = (index & 0xF000)  >> 12;
            b = (index & 0x0F00)  >> 4;
            g = (index & 0x00F0)  >> 0;
            r = (index & 0x000F)  << 4;
#else
            a = (index & 0xF000)  >> 12;
            b = (index & 0x0F00)  >> 4;
            g = (index & 0x00F0)  >> 0;
            r = (index & 0x000F)  << 4;

#endif

            y = (( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
            u = ((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
            v = ((112 * r -  94 * g -  18 * b + 128) >> 8) + 128;

            y >>= 4;
            u >>= 4;
            v >>= 4;

            ayuv_buf[index] = ((a << 12) | (y << 8) | (u << 4) | (v << 0));
        }

        xw_logsrv_debug(" RGBA8888  0xffff ayuv 4444:%x index:%x\n",ayuv_buf[0xffff],index);
 

    }else if(format == AGBR8888_AYUV4444){
        for (index = 0; index < (0x1 << 16); index++) {
            a = (index & 0xF000) >> 12;
            b = (index & 0x0F00) >> 4;
            g = (index & 0x00F0) >> 0;
            r = (index & 0x000F) << 4;

            y = (( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
            u = ((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
            v = ((112 * r -  94 * g -  18 * b + 128) >> 8) + 128;

            y >>= 4;
            u >>= 4;
            v >>= 4;

            ayuv_buf[index] = ((a << 12) | (y << 8) | (u << 4) | (v << 0));
        }

    }else if(format == RGBA8888_AYUV1555)
    {
        #if 1
            a = (index & 0xF000)  >> 15;
            b = (index & 0xEF00)  >> 3;
            g = (index & 0x0CF0)  >> 0;
            r = (index & 0x00CF)  << 3;
#else
            a = (index & 0xF000) >> 12;
            b = (index & 0x0F00)  >> 4;
            g = (index & 0x00F0)  >> 0;
            r = (index & 0x000F) << 4;

#endif

            y = (( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
            u = ((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
            v = ((112 * r -  94 * g -  18 * b + 128) >> 8) + 128;

            y >>= 3;
            u >>= 3;
            v >>= 3;
            if(a != 0)
                a = 1;
            ayuv_buf[index] = ((a << 15) | (y << 10) | (u << 5) | (v << 0));

        xw_logsrv_debug(" RGBA8888 0xffff ayuv1555:%x index:%x\n",ayuv_buf[0xffff],index);
 

 
    }else{

            xw_logsrv_err("init ayuv tabel fail \n");

    }


    return  0;


}

void  inline image_argb4444_to_ayuv(uint16_t argb, uint16_t *ayuv){
    *ayuv  =  ayuv_buf[argb];
     return;
}
void  inline  image_rgba8888_to_ayuv(uint32_t rgba,uint16_t *ayuv){
     
    uint16_t rgba16;
#if 1
    rgba16 = ((rgba & 0xf0000000) >> 16) |
            ((rgba & 0x00f00000) >> 12) |
            ((rgba & 0x0000f000) >> 8) |
            ((rgba & 0x000000f0) >> 4);
#endif
#if 0
    rgba16 = ((rgba & 0xf0000000) >> 16) |
            ((rgba & 0x00f00000) >> 9) |
            ((rgba & 0x0000f000) >> 6) |
            ((rgba & 0x000000f0) >> 3);
    
#endif
    *ayuv = ayuv_buf[rgba16];
    return ;
}
void inline image_rgb565_to_ayuv(uint16_t rgb,uint16_t *ayuv){
    
    *ayuv = ayuv_buf[rgb];
    return ;
    
}

void image_ayuv_talbe_deinit(void)
{
    if(ayuv_buf)
        free(ayuv_buf);
    ayuv_buf = NULL;
    return ;

}


#if 0

static inline void _convert_rgb565_2_uyv565(VO_OSD_InfoT *posd_info,
    u16 *pSrc, u16 *pDst,
    u32 pitch, u32 xres, u32 yres)
{
    u32                 i, size;
    u32                 *prgb, *puyv;
    u32                 rgb32, uyv32;
    u16                 *pLUT;

    prgb = (u32 *)pSrc;
    puyv = (u32 *)pDst;
    pLUT = posd_info->rgb565_2_uyv565_info.pLUT;
    size = (pitch * yres) >> 4;
G
    for (i = 0; i < size; i++) {
        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;
    }

}



static void _convert_argb4444_2_ayuv4444(VO_OSD_InfoT *posd_info,
    u16 *pSrc, u16 *pDst,
    u32 pitch, u32 xres, u32 yres)
{
    u32                i, size;
    u32                *prgb, *puyv;
    u32                rgb32, uyv32;
    u16                *pLUT;

    prgb = (u32 *)pSrc;
    puyv = (u32 *)pDst;
    pLUT = posd_info->argb4444_2_ayuv4444_info.pLUT;
    size = (pitch * yres) >> 4;

    for (i = 0; i < size; i++) {
        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;
    }

}



/*===========================rgba4444_2_ayuv4444==============================*/
static int _rgba4444_2_ayuv4444_init(VO_OSD_InfoT *posd_info,
    u32 pitch, u32 yres)
{
    VO_OSD_Rgb2YuvT    *prgb2yuv;
    u16                 **ppLUT;
    u8                  **ppOSDBuffer;
    u32                 index;
    u8                  a, r, g, b;
    u8                  y, u, v;
    int                 errorCode = 0;

    prgb2yuv    = &posd_info->rgba4444_2_ayuv4444_info;
    ppLUT       = &prgb2yuv->pLUT;
    ppOSDBuffer = (u8 **)&prgb2yuv->pOSDBuffer;

    if (*ppLUT) {
        kfree(*ppLUT);
        *ppLUT = NULL;
    }

    *ppLUT = kmalloc((0x1 << 16) * sizeof(u16), GFP_KERNEL);
    if (!(*ppLUT)) {
        vo_err("%s: pLUT out of memory!\n", __func__);
        errorCode = -ENOMEM;
        return errorCode;
    }

    if (*ppOSDBuffer) {
        kfree(*ppOSDBuffer);
        *ppOSDBuffer = NULL;
    }

    if (!posd_info->conversion_buf.available) {
        *ppOSDBuffer = kzalloc(pitch * yres * 2, GFP_KERNEL);
        if (!(*ppOSDBuffer)) {
            vo_err("%s: pOSDBuffer out of memory(pitch = %d, "
                "yres = %d)!\n", __func__, pitch, yres);
            errorCode = -ENOMEM;
            return errorCode;
        }
    }

    for (index = 0; index < (0x1 << 16); index++) {
        r = (index & 0xF000) >> 8;
        g = (index & 0x0F00) >> 4;
        b = (index & 0x00F0) >> 0;
        a = (index & 0x000F) >> 0;

        y = (( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
        u = ((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
        v = ((112 * r -  94 * g -  18 * b + 128) >> 8) + 128;

        y >>= 4;
        u >>= 4;
        v >>= 4;

        (*ppLUT)[index] = ((a << 12) | (y << 8) | (u << 4) | (v << 0));
    }

    return errorCode;
}

static void _convert_rgba4444_2_ayuv4444(VO_OSD_InfoT *posd_info,
    u16 *pSrc, u16 *pDst,
    u32 pitch, u32 xres, u32 yres)
{
    u32                 i, size;
    u32                 *prgb, *puyv;
    u32                 rgb32, uyv32;
    u16                 *pLUT;

    prgb = (u32 *)pSrc;
    puyv = (u32 *)pDst;
    pLUT = posd_info->rgba4444_2_ayuv4444_info.pLUT;
    size = (pitch * yres) >> 4;

    for (i = 0; i < size; i++) {
        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        uyv32 = pLUT[rgb32 >> 16] << 16;
        uyv32 |= pLUT[(rgb32 << 16) >> 16];
        *puyv++ = uyv32;
    }
}


/*===========================rgba8888_2_ayuv4444==============================*/
static int _rgba8888_2_ayuv4444_init(VO_OSD_InfoT *posd_info,
    u32 pitch, u32 yres)
{
    VO_OSD_Rgb2YuvT    *prgb2yuv;
    u16                 **ppLUT;
    u8                  **ppOSDBuffer;
    u32                 index;
    u8                  a, r, g, b;
    u8                  y, u, v;
    int                 errorCode = 0;

    prgb2yuv    = &posd_info->rgba8888_2_ayuv4444_info;
    ppLUT       = &prgb2yuv->pLUT;
    ppOSDBuffer = (u8 **)&prgb2yuv->pOSDBuffer;

    if (*ppLUT) {
        kfree(*ppLUT);
        *ppLUT = NULL;
    }

    *ppLUT = kmalloc((0x1 << 16) * sizeof(u16), GFP_KERNEL);
    if (!(*ppLUT)) {
        vo_err("%s: pLUT out of memory!\n", __func__);
        errorCode = -ENOMEM;
        return errorCode;
    }

    if (*ppOSDBuffer) {
        kfree(*ppOSDBuffer);
        *ppOSDBuffer = NULL;
    }

    if (!posd_info->conversion_buf.available) {
        *ppOSDBuffer = kzalloc(pitch * yres * 2, GFP_KERNEL);
        if (!(*ppOSDBuffer)) {
            vo_err("%s: pOSDBuffer out of memory(pitch = %d, "
                "yres = %d)!\n", __func__, pitch, yres);
            errorCode = -ENOMEM;
            return errorCode;
        }
    }

    for (index = 0; index < (0x1 << 16); index++) {
        r = (index & 0xF000) >> 8;
        g = (index & 0x0F00) >> 4;
        b = (index & 0x00F0) >> 0;
        a = (index & 0x000F) >> 0;

        y = (( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
        u = ((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
        v = ((112 * r -  94 * g -  18 * b + 128) >> 8) + 128;

        y >>= 4;
        u >>= 4;
        v >>= 4;

        (*ppLUT)[index] = ((a << 12) | (y << 8) | (u << 4) | (v << 0));
    }

    return errorCode;
}

static void _convert_rgba8888_2_ayuv4444(VO_OSD_InfoT *posd_info,
    u16 *pSrc, u16 *pDst,
    u32 pitch, u32 xres, u32 yres)
{
    u32                 i, size;
    u32                 *prgb, *puyv;
    u32                 rgb32, uyv32;
    u16                 rgb16, *pLUT;

    prgb = (u32 *)pSrc;
    puyv = (u32 *)pDst;
    pLUT = posd_info->rgba8888_2_ayuv4444_info.pLUT;
    size = (pitch * yres) >> 5;

    for (i = 0; i < size; i++) {
        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 = pLUT[rgb16] << 16;

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 |= pLUT[rgb16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 = pLUT[rgb16] << 16;

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 |= pLUT[rgb16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 = pLUT[rgb16] << 16;

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 |= pLUT[rgb16];
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 = pLUT[rgb16] << 16;

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 |= pLUT[rgb16];
        *puyv++ = uyv32;
    }

}


/*===========================abgr8888_2_ayuv4444==============================*/
static int _abgr8888_2_ayuv4444_init(VO_OSD_InfoT *posd_info,
    u32 pitch, u32 yres)
{
    VO_OSD_Rgb2YuvT    *prgb2yuv;
    u16                 **ppLUT;
    u8                  **ppOSDBuffer;
    u32                 index;
    u8                  a, r, g, b;
    u8                  y, u, v;
    int                 errorCode = 0;

    prgb2yuv    = &posd_info->abgr8888_2_ayuv4444_info;
    ppLUT       = &prgb2yuv->pLUT;
    ppOSDBuffer = (u8 **)&prgb2yuv->pOSDBuffer;

    if (*ppLUT) {
        kfree(*ppLUT);
        *ppLUT = NULL;
    }

    *ppLUT = kmalloc((0x1 << 16) * sizeof(u16), GFP_KERNEL);
    if (!(*ppLUT)) {
        vo_err("%s: pLUT out of memory!\n", __func__);
        errorCode = -ENOMEM;
        return errorCode;
    }

    if (*ppOSDBuffer) {
        kfree(*ppOSDBuffer);
        *ppOSDBuffer = NULL;
    }

    if (!posd_info->conversion_buf.available) {
        *ppOSDBuffer = kzalloc(pitch * yres * 2, GFP_KERNEL);
        if (!(*ppOSDBuffer)) {
            vo_err("%s: pOSDBuffer out of memory(pitch = %d, "
                "yres = %d)!\n", __func__, pitch, yres);
            errorCode = -ENOMEM;
            return errorCode;
        }
    }

    for (index = 0; index < (0x1 << 16); index++) {
        a = (index & 0xF000) >> 12;
        b = (index & 0x0F00) >> 4;
        g = (index & 0x00F0) >> 0;
        r = (index & 0x000F) << 4;

        y = (( 66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
        u = ((-38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
        v = ((112 * r -  94 * g -  18 * b + 128) >> 8) + 128;

        y >>= 4;
        u >>= 4;
        v >>= 4;

        (*ppLUT)[index] = ((a << 12) | (y << 8) | (u << 4) | (v << 0));
    }

    return errorCode;
}

static void _convert_abgr8888_2_ayuv4444(VO_OSD_InfoT *posd_info,
    u16 *pSrc, u16 *pDst,
    u32 pitch, u32 xres, u32 yres)
{
    u32                 i, size;
    u32                 *prgb, *puyv;
    u32                 rgb32, uyv32;
    u16                 rgb16, *pLUT;

    prgb = (u32 *)pSrc;
    puyv = (u32 *)pDst;
    pLUT = posd_info->abgr8888_2_ayuv4444_info.pLUT;
    size = (pitch * yres) >> 5;

    for (i = 0; i < size; i++) {
        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 = pLUT[rgb16];

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 |= (pLUT[rgb16] << 16);
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 = pLUT[rgb16];

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 |= (pLUT[rgb16] << 16);
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 = pLUT[rgb16];

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 |= (pLUT[rgb16] << 16);
        *puyv++ = uyv32;

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 = pLUT[rgb16];

        rgb32 = *prgb++;
        rgb16 = ((rgb32 & 0xf0000000) >> 16) |
            ((rgb32 & 0x00f00000) >> 12) |
            ((rgb32 & 0x0000f000) >> 8) |
            ((rgb32 & 0x000000f0) >> 4);
        uyv32 |= (pLUT[rgb16] << 16);
        *puyv++ = uyv32;
    }
}

#endif

