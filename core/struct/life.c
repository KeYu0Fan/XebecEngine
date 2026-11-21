#include "life.h"
#if COM_USE(CONFIG_STRUCT_LIFE)


/*
 * 初始化ptr为生命周期life
 * off life在宿主结构体中的偏移量
 * dest 销毁回调 可为NULL
 */
PORT_API struct life* life_init(void* ptr, qty_t off, void (*des)(void*))
{
	COM_RETURN(ptr == 0, 0);
	struct life* life = (struct life*)ptr;
	life->is_undel = 0;
	life->is_des = 0;
	life->count = 1;
	life->off = off;
	life->des = des;
	return life;
}

/* 增加引用 可能溢出时静默成功 */
PORT_API bool_t life_add(struct life* life)
{
	COM_RETURN(life == 0 || life->is_des, 0);
	if (life->count + 1) life->count++;
	return 1;
}

/* 减少引用 归零时触发des回调 */
PORT_API bool_t life_sub(struct life* life)
{
	COM_RETURN(life == 0 || life->is_des, 0);
	if (life->count) life->count--;
	if (life->is_undel) return 1;
	if (life->count == 0) life_des(life);
	return 1;
}

/* 强制销毁 无视计数 */
PORT_API bool_t life_des(struct life* life)
{
	COM_RETURN(life == 0 || life->is_des, 0);
	life->is_des = 1;
	if (life->des)
		life->des((void*)((char*)life - life->off));
	return 1;
}

/* 开启/关闭暂停销毁 无法阻止is_des */
PORT_API bool_t life_undel(struct life* life, qty_t undel)
{
	COM_RETURN(life == 0 || life->is_des, 0);
	life->is_undel = undel ? 1 : 0;
	if (undel == 0 && life->count == 0)
		life_des(life);
	return 1;
}

/* 获取当前引用计数 */
PORT_API qty_t life_count(struct life* life)
{
	COM_RETURN(life == 0 || life->is_des, 0);
	return life->count;
}
/* 获取是否存活 */
PORT_API bool_t life_exit(struct life* life)
{
	COM_RETURN(life == 0 || life->is_des, 0);
	return life->count || life->is_undel;
}

#else


PORT_API struct life* life_init(void* ptr, qty_t off, void (*destroy)(void*)) { ; }
PORT_API bool_t life_add(struct life* life) { ; }
PORT_API bool_t life_sub(struct life* life) { ; }
PORT_API bool_t life_des(struct life* life) { ; }
PORT_API bool_t life_undel(struct life* life, qty_t undel) { ; }
PORT_API qty_t life_count(struct life* life) { ; }
PORT_API bool_t life_exit(struct life* life) { ; }


#endif