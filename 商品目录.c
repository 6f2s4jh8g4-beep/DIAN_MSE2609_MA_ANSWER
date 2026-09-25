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

#endif /* POS_H */
       