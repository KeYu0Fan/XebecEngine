#ifndef STRUCT_LIFE_H
/* 生命周期 */
#define STRUCT_LIFE_H
#include "core/platform.h"
#include "core/eximport.h"
#include "core/typedef.h"
#include "core/comdef.h"
EXTERN_C_BEGIN


/* 生命周期控制块 */
struct life
{
    /* 暂停销毁标志 */
    unsigned char is_undel;
    /* 销毁中标志 */
    unsigned char is_des;
    /* 引用计数 初始为1 */
    qty_t count;
    /* 节点在父体中的偏移量 */
    qty_t off;
    /* 归零时的销毁回调 传入父体指针 */
    void (*des)(void* host);
};

/*
 * 初始化ptr为生命周期life
 * off life在宿主结构体中的偏移量
 * dest 销毁回调 可为NULL
 */
PORT_API struct life* life_init(void* ptr, qty_t off, void (*des)(void*));

/* 增加引用 可能溢出时静默成功 */
PORT_API bool_t life_add(struct life* life);

/* 减少引用 归零时触发des回调 */
PORT_API bool_t life_sub(struct life* life);

/* 强制销毁 无视计数 无视其他标志 */
PORT_API bool_t life_des(struct life* life);

/* 开启/关闭暂停销毁 无法阻止is_des */
PORT_API bool_t life_undel(struct life* life, qty_t undel);

/* 获取当前引用计数 */
PORT_API qty_t life_count(struct life* life);
/* 获取是否存活 */
PORT_API bool_t life_exit(struct life* life);


EXTERN_C_END
#endif