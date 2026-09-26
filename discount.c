#include "pos.h"
#include <math.h>

/* 全局折扣百分比（0-100） */
int g_discount = 0;

void discount_set(int p)
{
	if (p < 0) p = 0;
	if (p > MAX_DISCOUNT) p = MAX_DISCOUNT;
	g_discount = p;
}

void discount_notify(void)
{
	/* 简单通知：在控制台打印当前折扣（可在未来扩展为日志或事件） */
	if (g_discount == 0) {
		printf("Discount is now OFF.\n");
	} else {
		printf("Discount is now %d%%.\n", g_discount);
	}
}

double discount_apply(double amount)
{
	if (g_discount <= 0) return amount;
	double factor = (100.0 - (double)g_discount) / 100.0;
	/* 按分为单位四舍五入到两位小数 */
	double v = amount * factor;
	v = round(v * 100.0) / 100.0;
	return v;
}
