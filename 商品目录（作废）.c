//#ifndef POS_H   
//#define POS_H 
///*看B站学的这个，因为是一个单项目多程序的系统*/
//
///*设置该程序的全局变量*/
//
//typedef struct {
//    char   code[16];            /* 编码 */    // 商品条码，必须用字符串！如 "001"、"002"，不能用 int，否则前导 0 会丢失
//    char   name[32];            /* 名称（可能包含中英文） */
//    double price;               /* 单价 */
//    int    stock;               /* 库存 */
//} Goods;
//
//typedef struct {
//    Goods items[128];           /* 定义最大商品种类防止溢出 */
//    int   count;                /*定义计数*/
//} GoodsCatalog;                 /* 完成商品目录 */
//
//#endif /* POS_H */
//
//
///*首次运行时写入的默认商品*/
//void goods_seed(GoodsCatalog* cat)
//{
//    cat->count = 0;
//    goods_add(cat, "001", "Cola", 3.50, 70);
//    goods_add(cat, "002", "Lollipop", 0.50, 80);
//    goods_add(cat, "003", "Noodles", 6.00, 20);
//}
//       