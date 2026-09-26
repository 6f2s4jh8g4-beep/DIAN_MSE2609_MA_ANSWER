/* 
主循环、命令解析
数据文件放在可执行文件同目录的 data/ 下：
      data/goods.csv   商品目录
      data/sales.csv   销售记录
      data/day.txt     当前逻辑日
*/
#include "pos.h"

/* Windows 用 _mkdir，Linux/Mac 用 mkdir */
#ifdef _WIN32
#  include <direct.h>
#  define MKDIR(p) _mkdir(p)
#else
#  include <sys/stat.h>
#  include <sys/types.h>
#  define MKDIR(p) mkdir((p), 0755)
#endif

#define GOODS_FILE "data/goods.csv"
#define SALES_FILE "data/sales.csv"
#define DAY_FILE   "data/day.txt"

/*全局状态*/
static GoodsCatalog g_cat;    /* 商品目录 */
static Cart         g_cart;   /* 当前购物车 */
static SaleLog      g_log;    /* 销售记录缓存 */
static int          g_day  = 1;   /* 当前逻辑日 */
static int          g_quit = 0;   /* 退出标志 */

/*保存当前逻辑日*/
static void save_day(void)
{
    FILE *fp = fopen(DAY_FILE, "w");
    if (fp) { fprintf(fp, "%d\n", g_day); fclose(fp); }
}

/*帮助*/
static void print_cashier_help(void)
{
    printf("--- cashier commands ---\n");
    printf("  <code>          scan an item (+1)\n");
    printf("  -<code>         remove one from cart\n");
    printf("  print           show current cart\n");
    printf("  drop            clear cart\n");
    printf("  checkout        pay and print receipt\n");
    printf("  prices          list all items\n");
    printf("  sales [day]     show sales of a day\n");
    printf("  newday          start a new business day\n");
    printf("  admin           enter admin mode\n");
    printf("  quit | exit     exit program\n");
}

static void print_admin_help(void)
{
    printf("--- admin commands ---\n");
    printf("  setprice <code> <price>\n");
    printf("  itemadd  <code> <name> <price> [stock]\n");
    printf("  itemdel  <code>\n");
    printf("  restock  <code> <qty>\n");
    printf("  setstock <code> <qty>\n");
    printf("  prices\n");
    printf("  back\n");
}

/* 扫码处理 */
static void handle_scan_line(char *argv[], int argc)
{
    char touched[MAX_CART][CODE_LEN];
    int  tn = 0;

    /* --- 第一步：逐个应用到购物车 --- */
    for (int i = 0; i < argc; i++) {
        char *tok   = argv[i];
        int   delta = 1;
        const char *code = tok;

        if (tok[0] == '-') { delta = -1; code = tok + 1; }

        Goods *g = goods_find(&g_cat, code);
        if (!g) {
            printf("ERROR: code not found\n");
            continue;
        }

        if (delta < 0 && cart_qty(&g_cart, code) <= 0) {
            printf("ERROR: %s is not in the cart\n", g->name);
            continue;
        }

        cart_add(&g_cart, g, delta);

        /* 记录本行接触过的商品（去重） */
        int found = 0;
        for (int k = 0; k < tn; k++)
            if (strcmp(touched[k], code) == 0) { found = 1; break; }
        if (!found && tn < MAX_CART) {
            snprintf(touched[tn], CODE_LEN, "%s", code);
            tn++;
        }
    }

    /* --- 第二步：按首次出现顺序输出每个商品的最终状态 --- */
    for (int k = 0; k < tn; k++) {
        const CartItem *it = cart_get(&g_cart, touched[k]);
        if (it && it->qty > 0) {
            printf("%s %.2f x%d =%.2f\n",
                   it->name, it->price, it->qty, it->price * it->qty);
        } else {
            Goods *g = goods_find(&g_cat, touched[k]);
            printf("%s removed from cart\n", g ? g->name : touched[k]);
        }
    }
}

/*结账*/
static void do_checkout(void)
{
    if (g_cart.count == 0) {
        printf("Cart is empty, nothing to checkout.\n");
        return;
    }

    /* --- 1) 库存校验 --- */
    for (int i = 0; i < g_cart.count; i++) {
        Goods *g = goods_find(&g_cat, g_cart.items[i].code);
        if (!g) {
            printf("ERROR: item %s no longer exists in catalog.\n",
                   g_cart.items[i].name);
            return;
        }
        if (g->stock < g_cart.items[i].qty) {
            printf("ERROR: not enough stock for %s "
                   "(need %d, only %d left).\n",
                   g->name, g_cart.items[i].qty, g->stock);
            return;
        }
    }

    /* --- 2) 时间 & 流水号 --- */
    time_t     now = time(NULL);
    struct tm *lt  = localtime(&now);
    char       timestr[16];
    strftime(timestr, sizeof timestr, "%H:%M:%S", lt);

    sales_load(&g_log, SALES_FILE);
    int seq = sales_max_seq(&g_log, g_day) + 1;

    /* --- 3) 组织销售记录（每件商品一行） --- */
    SaleRecord rows[MAX_CART];
    int        n = 0;

    for (int i = 0; i < g_cart.count; i++) {
        SaleRecord *r = &rows[n++];
        r->day    = g_day;
        r->seq    = seq;
        snprintf(r->time, sizeof r->time, "%s", timestr);
        snprintf(r->code, CODE_LEN, "%s", g_cart.items[i].code);
        snprintf(r->name, NAME_LEN, "%s", g_cart.items[i].name);
        r->qty    = g_cart.items[i].qty;
        r->price  = g_cart.items[i].price;
        /* 按比例分摊折扣，保证 sum(amount) == 折后总额 */
        r->amount = discount_apply(
                        g_cart.items[i].price * g_cart.items[i].qty);
    }

    /* --- 4) 打印小票 --- */
    cart_print(&g_cart, "Receipt");

    /* --- 5) 写入销售文件 --- */
    sales_append(rows, n, SALES_FILE);

    /* --- 6) 扣库存并保存商品文件 --- */
    for (int i = 0; i < g_cart.count; i++) {
        Goods *g = goods_find(&g_cat, g_cart.items[i].code);
        if (g) g->stock -= g_cart.items[i].qty;
    }
    goods_save(&g_cat, GOODS_FILE);

    /* --- 7) 清空购物车 --- */
    cart_clear(&g_cart);
}

/* =========================================================
 *  管理员模式子循环
 * ========================================================= */
static void admin_loop(void)
{
    char  line[MAX_LINE];
    char *argv[8];

    printf("Admin mode. Type 'back' to return.\n");

    while (1) {
        printf("admin> ");
        fflush(stdout);

        if (!fgets(line, sizeof line, stdin)) return;
        trim_newline(line);

        int argc = split_args(line, argv, 8);
        if (argc == 0) continue;
        char *cmd = argv[0];

        /* ---------- back ---------- */
        if (strcmp(cmd, "back") == 0) {
            printf("Bye.\n");
            return;
        }
        /* ---------- help ---------- */
        else if (strcmp(cmd, "help") == 0) {
            print_admin_help();
        }
        /* ---------- prices ---------- */
        else if (strcmp(cmd, "prices") == 0) {
            goods_print(&g_cat, 1);
        }
        /* ---------- setprice <code> <price> ---------- */
        else if (strcmp(cmd, "setprice") == 0) {
            if (argc < 3) { printf("Usage: setprice <code> <price>\n"); continue; }
            Goods *g = goods_find(&g_cat, argv[1]);
            if (!g) { printf("ERROR: code not found\n"); continue; }
            g->price = atof(argv[2]);
            goods_save(&g_cat, GOODS_FILE);
            printf("Price updated.\n");
        }
        /* ---------- itemadd <code> <name> <price> ---------- */
       /* ---------- itemadd <code> <name> <price> [stock] ---------- */
        else if (strcmp(cmd, "itemadd") == 0) {
            if (argc < 4) {
                printf("Usage: itemadd <code> <name> <price> [stock]\n");
                continue;
            }

            if (goods_find(&g_cat, argv[1])) {
                printf("ERROR: code already exists\n");
                continue;
            }

            /* 校验价格 */
            char* endp = NULL;
            double price = strtod(argv[3], &endp);
            if (endp == argv[3] || *endp != '\0' || price < 0) {
                printf("ERROR: price must be a non-negative number\n");
                continue;
            }

            /* 默认库存为 0；如果输入了第 5 个参数，则校验库存 */
            int stock = 0;
            if (argc >= 5) {
                endp = NULL;
                long tmp = strtol(argv[4], &endp, 10);

                if (endp == argv[4] || *endp != '\0') {
                    printf("ERROR: stock must be an integer\n");
                    continue;
                }

                if (tmp < 0) {
                    printf("ERROR: stock is not enough (must be >= 0)\n");
                    continue;
                }

                if (tmp > MAX_STOCK) {
                    printf("ERROR: stock exceeds the limit (max %d)\n", MAX_STOCK);
                    continue;
                }

                stock = (int)tmp;
            }

            if (goods_add(&g_cat, argv[1], argv[2], price, stock) != 0) {
                printf("ERROR: cannot add item\n");
                continue;
            }

            goods_save(&g_cat, GOODS_FILE);

            printf("%s(%s) added. price=%.2f, stock=%d\n",
                argv[2], argv[1], price, stock);

            /* 库存为 0 时给个提醒 */   
            if (stock == 0) {
                printf("Warning: current stock is 0, please restock before sale.\n");
            }
        }
        /* ---------- itemdel <code> ---------- */
        else if (strcmp(cmd, "itemdel") == 0) {
            if (argc < 2) { printf("Usage: itemdel <code>\n"); continue; }
            Goods *g = goods_find(&g_cat, argv[1]);
            if (!g) { printf("ERROR: code not found\n"); continue; }

            /* 先保存名字，删除后再打印 */
            char nm[NAME_LEN], cd[CODE_LEN];
            snprintf(nm, sizeof nm, "%s", g->name);
            snprintf(cd, sizeof cd, "%s", g->code);

            goods_del(&g_cat, argv[1]);
            goods_save(&g_cat, GOODS_FILE);
            printf("%s(%s) removed.\n", nm, cd);
        }
        /* ---------- restock <code> <qty> ---------- */
        else if (strcmp(cmd, "restock") == 0) {
            if (argc < 3) { printf("Usage: restock <code> <qty>\n"); continue; }
            Goods *g = goods_find(&g_cat, argv[1]);
            if (!g) { printf("ERROR: code not found\n"); continue; }

            int q = atoi(argv[2]);
            if (q < 0) { printf("ERROR: qty must be >= 0\n"); continue; }

            g->stock += q;
            goods_save(&g_cat, GOODS_FILE);
            printf("%s stock is now %d.\n", g->name, g->stock);
        }
        /* ---------- setstock <code> <qty> ---------- */
        else if (strcmp(cmd, "setstock") == 0) {
            if (argc < 3) { printf("Usage: setstock <code> <qty>\n"); continue; }
            Goods *g = goods_find(&g_cat, argv[1]);
            if (!g) { printf("ERROR: code not found\n"); continue; }

            int q = atoi(argv[2]);
            if (q < 0) { printf("ERROR: qty must be >= 0\n"); continue; }

            g->stock = q;
            goods_save(&g_cat, GOODS_FILE);
            printf("%s stock is now %d.\n", g->name, g->stock);
        }
        /* ---------- discount <percent> | off ---------- */
        else if (strcmp(cmd, "discount") == 0) {
            if (argc < 2) {
                printf("Usage: discount <0-%d>  |  discount off\n", MAX_DISCOUNT);
                printf("Current discount: %d%%\n", g_discount);
                continue;
            }

            if (strcmp(argv[1], "off") == 0) {
                discount_set(0);
                printf("Discount is now OFF.\n");
                continue;
            }

            char *endp = NULL;
            long  p    = strtol(argv[1], &endp, 10);

            if (endp == argv[1] || *endp != '\0'
                || p < 0 || p > MAX_DISCOUNT) {
                printf("ERROR: discount must be an integer between 0 and %d\n",
                       MAX_DISCOUNT);
                continue;
            }

            discount_set((int)p);

            if (p == 0) {
                printf("Discount is now OFF.\n");
            } else {
                printf("Discount is now %ld%%.\n", p);
                discount_notify();
            }
        }
        /* ---------- 未知命令 ---------- */
        else {
            printf("Unknown admin command: %s  (type 'help')\n", cmd);
        }
    }
}

/*收银员主循环*/
static void cashier_loop(void)
{
    char  line[MAX_LINE];
    char *argv[8];

    while (!g_quit) {
        if (g_discount > 0) printf("[Discount %d%%] ", g_discount);
        printf("> ");
        fflush(stdout);

        if (!fgets(line, sizeof line, stdin)) break;
        trim_newline(line);

        int argc = split_args(line, argv, 8);
        if (argc == 0) continue;
        char *cmd = argv[0];

        /* ---------- 退出 ---------- */
        if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
            g_quit = 1;
        }
        /* ---------- 帮助 ---------- */
        else if (strcmp(cmd, "help") == 0) {
            print_cashier_help();
        }
        /* ---------- 商品列表 ---------- */
        else if (strcmp(cmd, "prices") == 0) {
            goods_print(&g_cat, 0);
        }
        /* ---------- 购物车操作 ---------- */
        else if (strcmp(cmd, "print") == 0) {
            cart_print(&g_cart, "Cart");
        }
        else if (strcmp(cmd, "drop") == 0) {
            cart_clear(&g_cart);
            printf("Cart cleared.\n");
        }
        else if (strcmp(cmd, "checkout") == 0) {
            do_checkout();
        }
        /* ---------- 销售统计 ---------- */
        else if (strcmp(cmd, "sales") == 0) {
            int day = g_day;
            if (argc >= 2) day = atoi(argv[1]);
            sales_load(&g_log, SALES_FILE);
            sales_report(&g_log, day);
        }
        else if (strcmp(cmd, "newday") == 0) {
            g_day++;
            save_day();
            printf("New day started. Today's sales records cleared.\n");
        }
        /* ---------- 管理员 ---------- */
        else if (strcmp(cmd, "admin") == 0) {
            if (admin_auth()) admin_loop();
        }
        /* ---------- 扫码：'<' 数字 或 '-数字' ---------- */
        else if (cmd[0] == '-' || isdigit((unsigned char)cmd[0])) {
            handle_scan_line(argv, argc);
        }
        /* ---------- 未知命令 ---------- */
        else {
            printf("Unknown command: %s  (type 'help')\n", cmd);
        }
    }
}

/* main */
int main(void)
{
    /* 0) 准备数据目录（不存在就创建） */
    MKDIR("data");

    /* 1) 读取逻辑日 */
    FILE *fp = fopen(DAY_FILE, "r");
    if (fp) {
        if (fscanf(fp, "%d", &g_day) != 1) g_day = 1;
        fclose(fp);
    }
    if (g_day < 1) g_day = 1;

    /* 2) 读取商品目录；文件不存在则写入默认商品 */
    if (goods_load(&g_cat, GOODS_FILE) <= 0) {
        goods_seed(&g_cat);
        goods_save(&g_cat, GOODS_FILE);
    }

    /* 3) 清空购物车 */
    cart_clear(&g_cart);

    /* 4) 欢迎语 */
    printf("=====================================\n");
    printf("     7-11 Convenience Store POS\n");
    printf("     (type 'help' for commands)\n");
    printf("=====================================\n");

    /* 5) 进入主循环 */
    cashier_loop();

    printf("Bye.\n");
    return 0;
}