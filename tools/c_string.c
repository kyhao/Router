#include "c_string.h"

// 字符串转数字函数 整数
// atoi为非标准C语言
// @param
// @str 输入字符串
// @return 转化成后输出数字
int stoint(const char *str)
{
    int i, len;
    int num = 0;
    len = strlen(str);
    for (i = 0; i < len; i++)
    {
        if (str[i] > '9' || str[i] < '0')
        {
            return -1;
        }
        num = num * 10 + (int)(str[i] - '0');
    }
    return num;
}

// 数字转字符串函数 整数
// itoa为非标准C语言
// @param
// @str 输入字符串
// @return 转化成后输出数字
int stoint(const char *str)
{
    int i, len;
    int num = 0;
    len = strlen(str);
    for (i = 0; i < len; i++)
    {
        if (str[i] > '9' || str[i] < '0')
        {
            return -1;
        }
        num = num * 10 + (int)(str[i] - '0');
    }
    return num;
}
