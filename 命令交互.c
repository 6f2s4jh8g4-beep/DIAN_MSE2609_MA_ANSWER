#include "pos.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void   goods_seed(GoodsCatalog* cat);
int    goods_load(GoodsCatalog* cat, const char* path);
int    goods_save(const GoodsCatalog* cat, const char* path);
Goods* goods_find(GoodsCatalog* cat, const char* code);
int    goods_add(GoodsCatalog* cat, const char* code,
    const char* name, double price, int stock);
int    goods_del(GoodsCatalog* cat, const char* code);
void   goods_print(const GoodsCatalog* cat, int show_stock);


/* 去掉行尾的 \n / \r */
void trim_newline(char* s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r'))
        s[--n] = '\0';
}

/* 按空白拆分命令行，返回参数个数 */
int split_args(char* line, char* argv[], int max)
{
    int   n = 0;
    char* p = line;

    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            p++;
        if (!*p) break;
        if (n >= max) break;

        argv[n++] = p;
        while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
            p++;
        if (*p) *p++ = '\0';
    }
    return n;
}

/* 按逗号拆分 CSV 行，返回字段个数 */
int split_csv(char* line, char* out[], int max)
{
    int n = 0;
    if (max <= 0) return 0;

    out[n++] = line;
    for (char* p = line; *p; p++) {
        if (*p == ',') {
            *p = '\0';
            if (n < max) out[n++] = p + 1;
        }
    }
    return n;
}

/* 查找 */
Goods* goods_find(GoodsCatalog* cat, const char* code)
{
    for (int i = 0; i < cat->count; i++)
        if (strcmp(cat->items[i].code, code) == 0)
            return &cat->items[i];
    return NULL;
}

/* 新增 */
int goods_add(GoodsCatalog* cat, const char* code,
    const char* name, double price, int stock)
{
    if (cat->count >= MAX_GOODS)   return -1;
    if (goods_find(cat, code))     return -2;   /* 条码重复 */

    Goods* g = &cat->items[cat->count++];
    snprintf(g->code, CODE_LEN, "%s", code);
    snprintf(g->name, NAME_LEN, "%s", name);
    g->price = price;
    g->stock = stock;
    return 0;
}

/*删除*/
int goods_del(GoodsCatalog* cat, const char* code)
{
    for (int i = 0; i < cat->count; i++) {
        if (strcmp(cat->items[i].code, code) == 0) {
            for (int j = i; j < cat->count - 1; j++)
                cat->items[j] = cat->items[j + 1];
            cat->count--;
            return 0;
        }
    }
    return -1;
}