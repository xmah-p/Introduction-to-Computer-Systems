#include <stdio.h>
#include <cstring>

// a: 

int main() {
    char name[25] = "hello";
    printf("name: %s\nlen: %zu\n", name, strlen(name));
    // 输出为
    // name: hello
    // len: 5

    // 显然 问题b的答案是：strlen输出name的实际长度。
    // 更准确地说，是name第一个'\0'之前的长度

    // 问题a：不一定，在这个例子里不是。
    // 在这个例子里，name由 'h' 'e' 'l' 'l' 'o' '\0' 组成
    // “有效”的是前五个字符，第六个字符是 '\0'，它标志字符串的结束
    // strlen就是通过这第六个 '\0' 来判断字符串的长度为5的
    
    // 也就是说，char name[N] 里面的 N 是多少，和 '\0' 在哪没有必然的关系
    // 这个 N 只是标记这个name字符数组拥有多少个元素，
    // 而字符串的长度是由第一个 '\0' 的位置来决定的


    // 问题c：
    name[strlen(name) - 1] = '\0';
    printf("name: %s\nlen: %zu\n", name, strlen(name));
    // 输出为
    // name: hell
    // len: 4

    // 只需要像这样写四行代码 观察一下输出 你的问题就能立刻得到答案
    // 下次请先尝试独立解决问题
}