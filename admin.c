/* =========================================================
 *  src/admin.c  —— 管理员认证
 * ---------------------------------------------------------
 *  初始密码见 pos.h 中的 ADMIN_PASSWORD 宏。
 * ========================================================= */
#include "pos.h"

int admin_auth(void)
{
    char pw[64];

    printf("Password: ");
    fflush(stdout);   /* 确保 Password: 先输出再读输入 */

    if (!fgets(pw, sizeof pw, stdin)) return 0;
    trim_newline(pw);

    if (strcmp(pw, ADMIN_PASSWORD) == 0) return 1;

    printf("Wrong password.\n");
    return 0;
}