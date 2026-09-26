
/*全局数据结构
头文件：结构化定义
*/
#ifndef POS_H
#define POS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

 /* ---------- 全局常量 ---------- */
#define CODE_LEN       16      /* 条码最大长度 */
#define NAME_LEN       32      /* 商品名最大长度 */
#define MAX_GOODS      128     /* 商品目录最多商品数 */
#define MAX_CART       64      /* 购物车最多条目数 */
#define MAX_SALES      4096    /* 销售记录最大行数 */
#define MAX_LINE       256     /* 命令行/文件行最大长度 */
#define MAX_STOCK      999999   /* 单商品库存上限，防止误输入 */

#define ADMIN_PASSWORD "admin123"   /* 管理员初始密码，可自行修改 */

/*商品目录*/
    typedef struct {
    char   code[CODE_LEN];      /* 条码，如 "001" */
    char   name[NAME_LEN];      /* 名称，如 "Cola" */
    double price;               /* 单价 */
    int    stock;               /* 库存数量 */
} Goods;

typedef struct {
    Goods items[MAX_GOODS];     /* 商品数组 */
    int   count;                /* 当前商品数量 */
} GoodsCatalog;

/* 首次运行时写入三件默认商品 */
void   goods_seed(GoodsCatalog* cat);

/* 从 CSV 读取商品目录，返回读到的商品数；文件不存在返回 -1 */
int    goods_load(GoodsCatalog* cat, const char* path);

/* 保存商品目录到 CSV，成功返回 0，失败返回 -1 */
int    goods_save(const GoodsCatalog* cat, const char* path);

/* 按条码查找商品，找不到返回 NULL */
Goods* goods_find(GoodsCatalog* cat, const char* code);

/* 添加商品：0 成功，-1 数组满，-2 条码重复 */
int    goods_add(GoodsCatalog* cat, const char* code,
    const char* name, double price, int stock);

/* 删除商品：0 成功，-1 未找到 */
int    goods_del(GoodsCatalog* cat, const char* code);

/* 打印商品列表，show_stock 为 1 时附带库存列 */
void   goods_print(const GoodsCatalog* cat, int show_stock);

/* 购物车*/
typedef struct {
    char   code[CODE_LEN];      /* 条码 */
    char   name[NAME_LEN];      /* 名称（快照） */
    double price;               /* 加入购物车时的价格快照 */
    int    qty;                 /* 数量 */
} CartItem;

typedef struct {
    CartItem items[MAX_CART];   /* 购物车条目 */
    int      count;             /* 条目数 */
} Cart;

void            cart_clear(Cart* c);                        /* 清空购物车 */
int             cart_add(Cart* c, const Goods* g, int delta);/* +1 或 -1 */
int             cart_qty(const Cart* c, const char* code);   /* 查某商品数量 */
const CartItem* cart_get(const Cart* c, const char* code);   /* 查某条目 */
double          cart_total(const Cart* c);                   /* 小计 */
void            cart_print(const Cart* c, const char* title);/* 打印购物车 */

/*销售记录*/
typedef struct {
    int    day;                 /* 逻辑日 */
    int    seq;                 /* 当日流水号 */
    char   time[16];            /* HH:MM:SS */
    char   code[CODE_LEN];      /* 条码 */
    char   name[NAME_LEN];      /* 名称 */
    int    qty;                 /* 数量 */
    double price;               /* 单价 */
    double amount;              /* qty * price */
} SaleRecord;

typedef struct {
    SaleRecord rows[MAX_SALES];
    int        count;
} SaleLog;

/* 从 CSV 载入全部销售记录，返回读到的行数；文件不存在返回 0 */
int  sales_load(SaleLog* log, const char* path);

/* 追加若干行到 CSV，成功返回 0 */
int  sales_append(const SaleRecord* rows, int n, const char* path);

/* 求某天的最大流水号，用于生成下一张小票的 seq */
int  sales_max_seq(const SaleLog* log, int day);

/* 打印某天的销售报表 + 当日总额 */
void sales_report(const SaleLog* log, int day);

/* =========================================================
 *  工具函数
 * ========================================================= */
void trim_newline(char* s);                     /* 去掉行尾 \n\r */
int  split_args(char* line, char* argv[], int max);   /* 按空白拆命令 */
int  split_csv(char* line, char* out[], int max);   /* 按逗号拆 CSV */

/* =========================================================
 *  6. 管理员认证
 * ========================================================= */
int admin_auth(void);   /* 通过返回 1，失败返回 0 */

/* =========================================================
 *  折扣功能
 * =========================================================
 */
extern int g_discount; /* 全局折扣百分比，0 表示无折扣 */

/* 设置折扣（0- MAX_DISCOUNT） */
void discount_set(int p);

/* 当折扣变更时的通知（例如向终端输出或刷新状态） */
void discount_notify(void);

/* 将折扣应用到某一项金额，返回折后金额（保留到分） */
double discount_apply(double amount);

#endif /* POS_H */