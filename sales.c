/* =========================================================
 *  src/sales.c  —— 销售记录实现
 * ---------------------------------------------------------
 *  CSV 格式（文件：data/sales.csv）：
 *      day,seq,time,code,name,qty,price,amount
 *      1,1,10:15:32,001,Cola,1,3.50,3.50
 *      1,1,10:15:32,002,Lollipop,1,0.50,0.50
 *  同一张小票由 (day, seq) 唯一标识，可能占多行。
 * ========================================================= */
#include "pos.h"

/* ---------- 载入全部销售记录 ---------- */
int sales_load(SaleLog *log, const char *path)
{
    log->count = 0;

    FILE *fp = fopen(path, "r");
    if (!fp) return 0;   /* 还没销售文件，返回 0 */

    char line[MAX_LINE];
    while (fgets(line, sizeof line, fp)) {
        trim_newline(line);
        if (line[0] == '\0') continue;
        if (strncmp(line, "day", 3) == 0) continue;   /* 表头 */

        char *f[8];
        int   n = split_csv(line, f, 8);
        if (n < 8) continue;
        if (log->count >= MAX_SALES) break;

        SaleRecord *r = &log->rows[log->count];
        r->day    = atoi(f[0]);
        r->seq    = atoi(f[1]);
        snprintf(r->time, sizeof r->time, "%s", f[2]);
        snprintf(r->code, CODE_LEN, "%s", f[3]);
        snprintf(r->name, NAME_LEN, "%s", f[4]);
        r->qty    = atoi(f[5]);
        r->price  = atof(f[6]);
        r->amount = atof(f[7]);
        log->count++;
    }
    fclose(fp);
    return log->count;
}

/* ---------- 追加写入 ---------- */
int sales_append(const SaleRecord *rows, int n, const char *path)
{
    FILE *fp = fopen(path, "a");   /* 追加模式 */
    if (!fp) return -1;

    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d,%d,%s,%s,%s,%d,%.2f,%.2f\n",
                rows[i].day, rows[i].seq, rows[i].time,
                rows[i].code, rows[i].name,
                rows[i].qty, rows[i].price, rows[i].amount);
    }
    fclose(fp);
    return 0;
}

/* ---------- 某天最大流水号 ---------- */
int sales_max_seq(const SaleLog *log, int day)
{
    int m = 0;
    for (int i = 0; i < log->count; i++)
        if (log->rows[i].day == day && log->rows[i].seq > m)
            m = log->rows[i].seq;
    return m;
}

/* ---------- 打印某天的销售报表 ---------- */
void sales_report(const SaleLog *log, int day)
{
    printf("Date: %d\n", day);

    int    i          = 0;
    double daily      = 0;
    int    header_out = 0;

    while (i < log->count) {
        if (log->rows[i].day != day) { i++; continue; }

        if (!header_out) {
            printf("%-4s %-9s %-26s %9s\n", "No.", "Time", "Items", "Amount");
            header_out = 1;
        }

        int  seq   = log->rows[i].seq;
        char items[256];
        items[0] = '\0';

        double total = 0;
        int    j     = i;

        /* 同一个 seq 的行合并成一张小票 */
        while (j < log->count &&
               log->rows[j].day == day &&
               log->rows[j].seq == seq) {
            char buf[64];
            snprintf(buf, sizeof buf, "%s x%d ",
                     log->rows[j].name, log->rows[j].qty);
            if (strlen(items) + strlen(buf) + 1 < sizeof items)
                strcat(items, buf);
            total += log->rows[j].amount;
            j++;
        }

        printf("%-4d %-9s %-26s %9.2f\n", seq, log->rows[i].time, items, total);
        daily += total;
        i = j;
    }

    printf("Daily: %.2f\n", daily);
}   