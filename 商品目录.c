#ifndef POS_H   
#define POS_H 
/*看B站学的这个，因为是一个单项目多程序的系统*/

/*设置该程序的全局变量*/

typedef struct {
    char   code[16];            /* 编码 */
    char   name[32];            /* 名称（可能包含中英文） */
    double price;               /* 单价 */
    int    stock;               /* 库存 */
} Goods;

typedef struct {
    Goods items[128];           /* 定义最大商品种类防止溢出 */
    int   count;                /*定义计数*/
} GoodsCatalog;                 /* 完成商品目录 */

void   goods_seed(GoodsCatalog* cat);
int    goods_load(GoodsCatalog* cat, const char* path);
int    goods_save(const GoodsCatalog* cat, const char* path);
Goods* goods_find(GoodsCatalog* cat, const char* code);
int    goods_add(GoodsCatalog* cat, const char* code,
    const char* name, double price, int stock);
int    goods_del(GoodsCatalog* cat, const char* code);
void   goods_print(const GoodsCatalog* cat, int show_stock);

#endif /* POS_H */
       