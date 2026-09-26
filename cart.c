/* =========================================================
 *  src/cart.c  —— 购物车实现
 * ---------------------------------------------------------
 *  说明：
 *  - 加入购物车时记录"价格快照"，管理员事后改价不影响已加入项。
 *  - qty 减到 0 会自动从购物车移除。
 * ========================================================= */
#include "pos.h"

 /* 清空 */
void cart_clear(Cart* c)
{
    c->count = 0;
}

/* 按条码查条目 */
const CartItem* cart_get(const Cart* c, const char* code)
{
    for (int i = 0; i < c->count; i++)
        if (strcmp(c->items[i].code, code) == 0)
            return &c->items[i];
    return NULL;
}

/* 查某商品数量，不在购物车里返回 0 */
int cart_qty(const Cart* c, const char* code)
{
    const CartItem* it = cart_get(c, code);
    return it ? it->qty : 0;
}

/* delta = +1 加一件，-1 减一件
 * 返回 0 成功，-1 失败（如购物车满 / 减一个不存在的商品）
 */
int cart_add(Cart* c, const Goods* g, int delta)
{
    /* 1) 已在购物车中：直接改数量 */
    for (int i = 0; i < c->count; i++) {
        if (strcmp(c->items[i].code, g->code) == 0) {
            c->items[i].qty += delta;
            if (c->items[i].qty <= 0) {
                /* 减到 0 就删掉 */
                for (int j = i; j < c->count - 1; j++)
                    c->items[j] = c->items[j + 1];
                c->count--;
            }
            return 0;
        }
    }

    /* 2) 不在购物车中：只有加号才能新建 */
    if (delta <= 0)               return -1;
    if (c->count >= MAX_CART)     return -1;

    CartItem* it = &c->items[c->count++];
    snprintf(it->code, CODE_LEN, "%s", g->code);
    snprintf(it->name, NAME_LEN, "%s", g->name);
    it->price = g->price;    /* 价格快照 */
    it->qty = delta;
    return 0;
}

/* 购物车小计 */
double cart_total(const Cart* c)
{
    double t = 0;
    for (int i = 0; i < c->count; i++)
        t += c->items[i].price * c->items[i].qty;
    return t;
}

/* 打印购物车（草稿或小票共用） */
void cart_print(const Cart* c, const char* title)
{
    if (c->count == 0) {
        printf("(cart is empty)\n");
        return;
    }

    printf("%s\n", title);
    printf("%-12s %8s %4s %8s\n", "Item", "Pri.", "Qty", "Amount");

    for (int i = 0; i < c->count; i++) {
        printf("%-12s %8.2f  x%-3d %8.2f\n",
            c->items[i].name,
            c->items[i].price,
            c->items[i].qty,
            c->items[i].price * c->items[i].qty);
    }
    printf("%-12s %8s %4s %8.2f\n", "Total", "", "", cart_total(c));
}