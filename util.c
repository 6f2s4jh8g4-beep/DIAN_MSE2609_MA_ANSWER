/* 字符串与命令行工具
 这里放的是与业务无关的小工具：
 去掉行尾换行
 按空白拆分命令行
 按逗号拆分 CSV
*/
#include "pos.h"

 /* 去掉行尾的 \n / \r */
void trim_newline(char* s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r'))
        s[--n] = '\0';
}

/* 按空白拆分命令行，返回参数个数
 * 注意：会修改 line，把分隔符替换成 '\0'
 */
int split_args(char* line, char* argv[], int max)
{
    int   n = 0;
    char* p = line;

    while (*p) {
        /* 跳过多余空白 */
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            p++;
        if (!*p) break;
        if (n >= max) break;

        /* 记录当前参数起点 */
        argv[n++] = p;

        /* 走到参数末尾 */
        while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
            p++;
        if (*p) *p++ = '\0';   /* 截断 */
    }
    return n;
}

/* 按逗号拆分 CSV 行，返回字段个数
 * 注意：会修改 line，把逗号替换成 '\0'
 */
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