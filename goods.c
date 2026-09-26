/* 商品目录实现*/
#include "pos.h"

/* 首次运行时写入的默认商品 */
void goods_seed(GoodsCatalog *cat)
{
    cat->count = 0;
    goods_add(cat, "001", "Cola",     3.50, 70);
    goods_add(cat, "002", "Lollipop", 0.50, 80);
    goods_add(cat, "003", "Noodles",  6.00, 20);
}

/* 按条码查找  */
Goods *goods_find(GoodsCatalog *cat, const char *code)
{
    for (int i = 0; i < cat->count; i++)
        if (strcmp(cat->items[i].code, code) == 0)
            return &cat->items[i];
    return NULL;
}

/* 新增商品 */
int goods_add(GoodsCatalog *cat, const char *code,
              const char *name, double price, int stock)
{
    if (cat->count >= MAX_GOODS)   return -1;   /* 数组满 */
    if (goods_find(cat, code))     return -2;   /* 条码重复 */

    Goods *g = &cat->items[cat->count++];
    snprintf(g->code, CODE_LEN, "%s", code);
    snprintf(g->name, NAME_LEN, "%s", name);
    g->price = price;
    g->stock = stock;
    return 0;
}

/* 删除商品（后面的元素前移）*/
int goods_del(GoodsCatalog *cat, const char *code)
{
    for (int i = 0; i < cat->count; i++) {
        if (strcmp(cat->items[i].code, code) == 0) {
            for (int j = i; j < cat->count - 1; j++)
                cat->items[j] = cat->items[j + 1];
            cat->count--;
            return 0;
        }
    }
    return -1;   /* 未找到 */
}

/* 从 CSV 载入 */
int goods_load(GoodsCatalog *cat, const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;   /* 文件不存在 */

    char line[MAX_LINE];
    cat->count = 0;

    while (fgets(line, sizeof line, fp)) {
        trim_newline(line);
        if (line[0] == '\0' || line[0] == '#') continue;   /* 空行/注释 */
        if (strncmp(line, "code", 4) == 0)     continue;   /* 表头 */

        char *f[4];
        int   n = split_csv(line, f, 4);
        if (n < 3) continue;                                /* 字段不够 */
        if (cat->count >= MAX_GOODS) break;

        Goods *g = &cat->items[cat->count];
        snprintf(g->code, CODE_LEN, "%s", f[0]);
        snprintf(g->name, NAME_LEN, "%s", f[1]);
        g->price = atof(f[2]);
        g->stock = (n >= 4) ? atoi(f[3]) : 0;
        cat->count++;
    }
    fclose(fp);
    return cat->count;
}

/* 保存到 CSV */
int goods_save(const GoodsCatalog *cat, const char *path)
{
    FILE *fp = fopen(path, "w");
    if (!fp) return -1;

    fprintf(fp, "code,name,price,stock\n");
    for (int i = 0; i < cat->count; i++) {
        fprintf(fp, "%s,%s,%.2f,%d\n",
                cat->items[i].code, cat->items[i].name,
                cat->items[i].price, cat->items[i].stock);
    }
    fclose(fp);
    return 0;
}

/*打印商品列表*/
void goods_print(const GoodsCatalog *cat, int show_stock)
{
    printf("%-12s %-6s %8s", "Item", "No.", "Pri.");
    if (show_stock) printf(" %8s", "Stock");
    printf("\n");
    printf("----------------------------------------\n");

    for (int i = 0; i < cat->count; i++) {
        printf("%-12s %-6s %8.2f",
               cat->items[i].name, cat->items[i].code, cat->items[i].price);
        if (show_stock) printf(" %8d", cat->items[i].stock);
        printf("\n");
    }

    if (cat->count == 0) printf("(no items)\n");
}