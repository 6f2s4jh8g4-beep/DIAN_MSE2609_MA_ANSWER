//#include "pos.h"
//typedef struct {
//    char   code[16];
//    char   name[32];
//    double price;               /*加入购物车时的价格*/
//    int    qty;
//} CartItem;
//
//typedef struct {
//    CartItem items[64];
//    int      count;
//} Cart;
//
//void            cart_clear(Cart* c);
//int             cart_add(Cart* c, const Goods* g, int delta);
//int             cart_qty(const Cart* c, const char* code);
//const CartItem* cart_get(const Cart* c, const char* code);
//double          cart_total(const Cart* c);
//void            cart_print(const Cart* c, const char* title);
//
