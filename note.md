[TOC]

# 信息的表示和处理

$$(a\mid b)\mid c=a\mid (b\mid c)$$

$$(a\ \&\ b)\ \&\ c=a\ \&\ (b\ \&\ c)$$

$\&$ 和 $\mid $ 有结合律

$$a\ \&\ (b\mid c) = a\ \&\ b\mid a\ \&\ c$$

$$a\mid (b\ \&\ c) = a\mid b\ \&\ a\mid c$$

$\&$ 和 $\mid $ 对彼此都有分配律

$$a\mid (a \ \&\ b)=a$$

$$a\ \& \ (a \mid b)=a$$

吸收律

$$\~{}\  (a\ \& \ b)=\~{}\  a\mid \~{}\  b$$

$$\~{}\  (a \mid b)=\~{}\  a\ \& \  \~{}\  b$$

德摩根律

$$a\ \^{}\ a=0$$

$$b\ \^{}\ 0=b$$

$$a\ \^{}\ b=b\ \^{}\ a$$

$$a\ \^{}\ (b\ \^{}\ c)=(a\ \^{}\ b)\ \^{}\ c$$


$$(a\ \^{} \ b)\ \^{} \ a=b$$  

异或有交换律和结合律

```c++
// 利用异或的性质
void swap(int* x, int* y) {
    *y = *x ^ *y;
    *x = *x ^ *y;
    *y = *x ^ *y;
}
```

位运算的性质：

- 与 $1$ 异或：取补
- 与 $0$ 异或：保留

- 与 $1$ 与：保留
- 与 $0$ 与：复位

- 与 $1$ 或：置位
- 与 $0$ 或：保留

C 中的**移位**运算：
- 左移：丢弃最高 k 位，低位补 0
- 逻辑右移（无符号数）：丢弃最低 k 位，高位补 0
- 算术右移（一般有符号数）：丢弃最低 k 位，高位补最高有效位

> 反码（Ones' Complement）：除了最高有效位的权是 $-(2^{\omega-1}-1)$ 而不是 $-2^{\omega-1}$，和补码相同。
>
> 原码（Sign-Magnitude）：最高有效位是符号位，用来确定剩下的位的权。
>
> 这两种编码都存在正负 0 的问题。

有符号数和无符号数的转换：数值可能改变，但位模式不变。

```c++
unsigned x = 0x1;
int y = -1;
x < y;    // 比较时 y 被转换成无符号数
```

**扩展位表示**：
- 无符号：高位补 0
- 补码：高位补最高有效位（$[1011]$ 和 $[111011]$ 都是 $-5$ 的补码表示）

```c++
// 既有扩展 又有有无符号的转换
short sx = -12345;
unsigned uy = sx;
printf("uy = %u:\t", uy);
show_bytes((BytePtr)&uy, sizeof(unsigned));
// 输出：uy = 4294954951:    ff ff cf c7
// C 标准规定：先扩展，再转换符号
```

**截断**：直接丢弃高位
- 无符号数：模运算
- 补码：先模运算，然后再转补码

**无符号整数加法**：加，然后截断，即模数加法。溢出当且仅当 $s=(x+y) \mod 2^\omega<x$。如果溢出，则实际结果比真实值小 $2^\omega$。

**无符号数的求反**：$x=0$ 时，$-x=0$，$x>0$ 时，$-x=2^\omega-x$.

补码加法：加，然后截断。负溢出：$x<0,\ y<0,\ z=x+y+2^\omega >0$；正溢出：$x>0,\ y>0,\ z=x+y-2^\omega <0$

补码加形成一个 Abel 群。

```c++
int buggy_tadd_ok(int x, int y) {
    int s = x + y;
    return (s - x == y) && (s - y == x);    // 永远返回 true
    // 正确做法：分别判断有无正溢出和负溢出
}
```

补码的求反：$x=\operatorname{TMin}_\omega$ 时，$-x=\operatorname{TMin}_\omega$，$x>\operatorname{TMin}_\omega$ 时，$-x=-x$.

计算补码非的技巧：
- $-x=$~$x+1$：每位求补再加 1
- 找到最低的置 1 位，对其左侧所有为求补


无符号乘法：乘，然后截断。

补码乘法：乘，然后截断。

补码乘法和无符号乘法同样具有位级等价性：完整的乘积的位级表示可能不同，但**截断后的乘积的位级表示是相同的**。

```c++
// 检验补码乘法是否溢出
int tmult_ok(int x, int y) {
    int p = x * y;
    return !x || p/x == y;
}

int tmult_ok2(int x, int y) {
    int64_t pll = static_cast<int64_t>(x * y);  // 这里也要转换！
    return pll == static_cast<int>(pll);
}
```

计算 $x * 14$：$14$ 的二进制表示为 $[1110]$，因此编译器可以将常数乘法优化为 `(x << 3) + (x << 2) + (x << 1)` 或 `(x << 4) - (x << 1)`（也可能不优化，直接用乘法指令计算）。

整数除法总是向 0 舍入。

对于无符号数和补码，**向下舍入的** $x/2^k$ 总是等价于 $x>>k$。右移导致向下舍入，是因为低位消失了。

通过**偏置（biasing）**，可以令负数除法向上舍入：$\lceil x/2^k \rceil=$ `(x + (1 << k) - 1) >> k`. 也就是说，二进制加上一个 $k$ 位全 $1$ 数再右移 $k$ 位，等价于做负整数除法。这是因为 $\lceil x/y \rceil=\lfloor(x+y-1)/y\rfloor$. 再换句话说，加上偏置就等价于“如果低 $k$ 位不是全 $0$，那么结果向上进 $1$”



对于使用算术右移的补码机器，`(x < 0 ? x + (1 << k) - 1 : x) >> k` 将会计算 $x/2^k$

```c++
// 不使用除法/模运算/乘法/条件语句的除以16
int div16(int x) {
    // 诀窍：如果 x < 0，那么 x << 31 全 1
    // 否则 x << 31 全 0
    int bias = (x << 31) & 0xf;  // 掩码
    return (x + bias) >> 4;
}
```

无法用除以 $2$ 的幂的除法的组合表示除以任意常数的除法。

**当 $x=\operatorname{TMin}_{32}$ 时，$-x=x$**.

由于无符号数和补码在加法和乘法（**截断后**）具有**相同的位级表示**，所以对任何 `int x` 和 `int y`，`x+y == ux+uy` 和 `x*y == ux*uy` 恒成立（`unsigned ux = x; unsigned uy = y`）。

补码取反和非：`-y = ~y + 1`


## 浮点

IEEE 浮点表示

$V=(-1)^s \times M \times 2^E$
- 符号（sign）：$s$
- 尾数（significand）：$M$ 是一个二进制小数，范围是 $1 \sim 2-\varepsilon$（规格化）或 $0 \sim 1-\varepsilon$（非规格化）
- 阶码（exponent）：$E$ 对浮点数加权

浮点数的格式：
- `float`：$1$ 位的 `s`，$8$ 位的 `exp`，$23$ 位的 `frac`
- `double`：$1$ 位的 `s`，$11$ 位的 `exp`，$52$ 位的 `frac`

浮点数的值：

1. 规格化的值：如果 `exp` 既不是全 $0$，也不是全 $1$，那么阶码的值 $E=e-Bias$，其中 $Bias=2^{k-1}-1$. 小数字段 `frac` 被解释为二进制小数 $f$ 的值，$0 \leq f < 1$，尾数定义为 $M=f+1$.
2. 非规格化的值：当阶码域 `exp` 全 $0$ 时。此时阶码值 $E=1-Bias$，尾数 $M=f$.
3. 特殊值：当阶码全为 $1$ 时。若小数域全 $0$，则得到无穷，否则得到 NaN.

$12345$ 具有二进制表示 $11000000111001$，小数点左移 $13$ 位得 $1.1000000111001_2 \times 2^{13}$，丢弃开头的 $1$ ，并在末尾添加 $10$ 个 $0$，得小数字段 $[10000001110010000000000]$。阶码为 $13+127=140=[10001100]$，加上符号位的 $0$，就得到浮点表示 $[0\ 10001100\ 10000001110010000000000]$.

浮点数的舍入：向偶数舍入。“正中间”的值（也就是形如 $Y100\ldots$ 的值，$Y$ 为要舍入到的位）向偶数舍入，不是“正中间”的值向最接近的数舍入。例如将一下数字舍入到最接近的二分之一：
- $10.010_2 : 10.0$
- $10.011_2 : 10.1$
- $10.110_2 : 11.0$
- $11.001_2 : 11.0$

浮点加法和乘法：可交换，但**不可结合**，乘法对加法**没有分配律**。

浮点加法**有单调性**：若 $a \geq b$ 且 $a, b, x \neq NaN$，则 $x+a\geq x+b$. 乘法同理.

## Datalab

```c++
int isTmax(int x) {
    // Tmax = 0x7fffffff
    int k = x + x + 2;    // 利用溢出
    int s1 = !k;
    int s2 = !!(x + 1);
    return s1 & s2;    // x = 0x7fffffff 时，-O0 返回1 -O1 返回1
}
```

若 `y = 0x7fffffff`，则 `~y == y + 1`。又因为补码有：`~y = -y - 1`，联立可知只需要排除 `-1` 即可。通过溢出凑 `0` 的判断方法和编译器行为有关，使用**异或判等**：`x^x == 0`。

```c
int isTmax(int x) {
    return !(~x ^ (x + 1)) & !!(x + 1);
}
```

```c
int howManyBits(int x) {
    // 其实就是返回 最高有效位+1 因此负数和正数是一样的
    // 用二分法找
    int t = x ^ (x >> 31);    // 负数取反 正数不变
    int z = ~!t + 1;    // z = t == 0 ? 0xffffffff : 0x0
    int b16, b8, b4, b2, b1;
    b16 = (!(!(t >> 16))) << 4;    // 高16位非空 ? 16 : 0
    t = t >> b16;
    b8 = !(!(t >> 8)) << 3;
    t = t >> b8;
    b4 = !(!(t >> 4)) << 2;
    t = t >> b4;
    b2 = !(!(t >> 2)) << 1;
    t = t >> b2;
    b1 = !(!(t >> 1));
    // t = 0 需要特判 
    return ((b1 + b2 + b4 + b8 + b16 + 2) & ~z) | (1 & z);
}
```

# 程序的机器级表示

## 数据格式

| C 声明 | Intel 数据类型 | 汇编代码后缀 | 大小（字节）|
| :----: | :----: | :----: | :----: |
| `char` | 字节 | `b` | 1 |
| `short` | 字 | `w` | 2 |
| `int` | 双字 | `l` | 4 |
| `long` | 四字 | `q` | 8 |
| `char *`（指针） | 四字 | `q` | 8 |
| `float` | 单精度 | `s` | 4 |
| `double` | 双精度 | `l` | 8 |

## 访问信息

### 寄存器

x86-64 的 CPU 包含一组 16 个存储 64 位值的**通用目的寄存器**。

| 四字（64位） | 双字（32位） | 字（16位） | 字节（8位）| 用途 |
| :----: | :----: | :----: | :----: | :----: |
| `%rax` | `%eax` | `%ax` | `%al` | 返回值 |
| `%rbx` | `%ebx` | `%bx` | `%bl` | 被调用者保存 |
| `%rcx` | `%ecx` | `%cx` | `%cl` | 第 4 个参数 |
| `%rdx` | `%edx` | `%dx` | `%dl` | 第 3 个参数 |
| `%rsi` | `%esi` | `%si` | `%sil` | 第 2 个参数 |
| `%rdi` | `%edi` | `%di` | `%dil` | 第 1 个参数 |
| `%rbp` | `%ebp` | `%bp` | `%bpl` | 被调用者保存 |
| `%rsp` | `%esp` | `%sp` | `%spl` | 栈指针 |
| `%r8` | `%r8d` | `%r8w` | `%r8b` | 第 5 个参数 |
| `%r9` | `%r9d` | `%r9w` | `%r9b` | 第 6 个参数 |
| `%r10` | `%r10d` | `%r10w` | `%r10b` | 调用者保存 |
| `%r11` | `%r11d` | `%r11w` | `%r11b` | 调用者保存 |
| `%r12` | `%r12d` | `%r12w` | `%r12b` | 被调用者保存 |
| `%r13` | `%r13d` | `%r13w` | `%r13b` | 被调用者保存 |
| `%r14` | `%r14d` | `%r14w` | `%r14b` | 被调用者保存 |
| `%r15` | `%r15d` | `%r15w` | `%r15b` | 被调用者保存 |

当指令**以 64 位寄存器为目标时**：生成 1 字节和 2 字节数字的指令会保持高位的字节不变；生成 4 字节数字的指令会把高位 4 个字节置 0.

### 操作数格式

| 类型 | 格式 | 操作数值 | 名称 |
| :----: | :----: | :----: | :----: |
| 立即数 | $$ \$\operatorname{Imm}$$ | $\operatorname{Imm}$ | 立即数寻址 |
| 寄存器 | $r_a$ | $R[r_a]$ | 寄存器寻址 |
| 存储器 | $\operatorname{Imm}(r_b,r_i,s)$ | $M[\operatorname{Imm}+R[r_b]+R[r_i]\times s]$ | 比例变址寻址 |

比例因子 $s$ 必须是 $1, 2, 4, 8$ 中的一个。

在 64 位机器上，地址总是 64 位的，因此像 `(%eax)` 这样的操作数很有可能是错误的。

### 数据传送指令

MOV 类指令把数据从源位置复制到目的位置。MOV 类指令包括：
- `movb`：字节
- `movw`：字
- `movl`：双字
- `movq`：四字

常规的 `movq` 指令只能以**表示 32 位补码的立即数作为源操作数**，把这个值**符号扩展**得到 64 位的值，放到目的位置。
`movabsq` 指令能以**任意 64 位立即数值作为源操作数**，且**只能以寄存器作为目的位置**。

x86-64 限制传送指令的**两个操作数不能都指向内存位置**（只能为 内存 -> 寄存器 -> 内存 的形式）。

```x86asm
movl $0x4050,%eax       ; Imm -> Reg
movw %bp,%sp            ; Reg -> Reg
movb (%rdi,%rcx),%al    ; Mem -> Reg
movb $-17,(%rsp)        ; Imm -> Mem
movq %rax,-12(%rbp)     ; Reg -> Mem
```

MOVZ 类和 MOVS 类将较小的源值复制到较大的目的位置。MOVZ 类指令执行**零扩展**，MOVS 类指令执行**符号扩展**。这些指令的目的位置**必须是寄存器**。

- `movzbq`：字节 -> 四字（零扩展）
- `movzbl`：字节 -> 双字（零扩展）
- `movzwl`：字 -> 双字（零扩展）
- `movzbq`：字节 -> 四字（零扩展）
- `movzwq`：字 -> 四字（零扩展）

- `movsbw`：字节 -> 字（符号扩展）
- `movsbl`：字节 -> 双字（符号扩展）
- `movswl`：字 -> 双字（符号扩展）
- `movsbq`：字节 -> 四字（符号扩展）
- `movswq`：字 -> 四字（符号扩展）
- `movslq`：双字 -> 四字（符号扩展）

为什么没有 `movzlq`？因为不同于 `movb` 和 `movw`，`movl` 会把高位的字节置 0，因此不需要双字到四字的零扩展。

`movzbq` 经常被 `movzbl` 替代，因为 `movzbl` 的编码更短。`movzwl` 和 `movzwq` 也是如此。

- `cltq`：`%eax` -> `%rax`（符号扩展）

`cltq` 无操作数，完全等价于 `movslq %eax,%rax`。

示例：

```c
src_t* sp;
dest_t* dp;

*dp = (dest_t) *sp;
```

假设 `sp` 和 `dp` 分别存储在寄存器 `%rdi` 和 `%rsi` 中，对于 `src_t` 和 `dest_t` 的各种情况，给出数据传送的两条指令。第一条从内存中读数，做适当转换，把结果写到寄存器 `%rax` 的适当部分；第二条把 `%rax` 的适当部分写到内存。

| `src_t` | `dest_t` | 第一条指令 | 第二条指令 | 注释 |
| :----: | :----: | :----: | :----: | :----: |
| `long` | `long` | `movq (%rdi),%rax` | `movq %rax,(%rsi)` |
| `char` | `int` | `movsbl (%rdi),%eax` | `movl %eax,(%rsi)` | 符号扩展 |
| `char` | `unsigned` | `movsbl (%rdi),%eax` | `movl %eax,(%rsi)` | 符号扩展 |
| `char` | `short` | `movsbw (%rdi),%ax` | `movw %ax,(%rsi)` | 符号扩展 |
| `unsigned char` | `long` | `movzbq (%rdi),%rax` | `movq %rax,(%rsi)` | 零扩展 |
| `int` | `char` | `movl (%rdi),%eax` | `movb %al,(%rsi)` | 截断 |

扩展时采用符号扩展还是零扩展，**取决于 `src_t` 是否为有符号表示**。这是因为 C 语言在类型转换时**先扩展，再转换符号**，即若 `ch` 类型为 `char`，则`(unsigned) ch` 等价于 `(unsigned) (int) ch`。


### 压入和弹出栈数据

- `pushq Src`：栈指针 `%rsp` 减 8，然后把 `Src` 的值复制到新 `%rsp` 指向的内存位置，等价于 `subq $8,%rsp`  `movq Src,(%rsp)`
- `popq Dest`：把 `%rsp` 指向的内存位置的值复制到 `Dest`，然后 `%rsp` 加 8，等价于 `movq (%rsp),Dest`  `addq $8,%rsp`

栈指针 `%rsp` 保存着**栈顶元素的地址**，x86-64 中，栈向低地址方向增长，故**压栈时栈指针减小**。可以用标准的内存寻址方法访问栈内任意位置，比如 `movq 8(%rsp),%rdx` 会把栈顶起第二个元素的值复制到 `%rdx`。


## 算术和逻辑操作


`leaq` （load effective address）指令是 `movq` 的变形。`leaq S,D` 将把 `S` 的有效地址（而不是它的内容）复制到 `D`。它可以灵活地用于算术操作，如 `leaq 7(%rdx, %rdx, 4),%rax` 设置 `%rax` 的值为 `5 * %rdx + 7`。**`leaq` 的目的操作数必须是寄存器**。

```c
long scale(long x, long y, long z) {
    long t = x + 4 * y + 12 * z;
    return t;
}
```

```x86asm
leaq    (%rdi,%rsi,4), %rax    ; t = x + 4 * y
leaq    (%rdx,%rdx,2), %rdx    ; z = z + 2 * z
leaq    (%rax,%rdx,4), %rax    ; t = t + 4 * z
ret
```

- `INC Dest`：加 1
- `DEC Dest`：减 1
- `NEG Dest`：取负
- `NOT Dest`：取补

`Dest` 可以是寄存器或内存位置

- `ADD Src,Dest`：加
- `SUB Src,Dest`：减
- `IMUL Src,Dest`：乘
- `XOR Src,Dest`：异或
- `OR Src,Dest`：或
- `AND Src,Dest`：与

以上二元操作中，第一个操作数可以是立即数、寄存器或内存位置，第二个操作数可以是寄存器或内存位置。第二个操作数既是源又是目的位置，例如 `subq %rax,%rdx` 就相当于 `%rdx -= %rax`。

- `SAL k,Dest`：左移
- `SHL k,Dest`：左移（等同于 `SAL`）
- `SAR k,Dest`：算术右移
- `SHR k,Dest`：逻辑右移

移位量 `k` 可以是**立即数**或者**单字节寄存器 `%cl`**。x86-64 中，移位量是**由 `%cl` 寄存器的低位给出**的。即，若 `%cl` 的值位 `0xFF`，则 `salb %cl,%rax` 会将 `%rax` 的值左移 7 位，`salw` 会移 15 位，`sall` 会移 31 位，而 `salq` 会移 63 位。


以上指令中，除了右移指令要求区分补码和无符号数以外，其他指令都不区分补码和无符号数。**这是补码的优点：兼容无符号数**。


x86-64 有限地支持 128 位数操作，它们被称为**八字（oct word）**

`imulq Src` `mulq Src` 分别是补码八字乘法和无符号数八字乘法指令。它们都要求**一个乘数必须在 `%rax` 中**，而**另一个乘数作为操作数给出**。乘积存放在 `%rdx`（高 64 位）和 `%rax`（低 64 位）两个寄存器中。

> 指令的双操作数形式是 `imulq Src，Dest`。注意到**乘积被截断为 64 位后，无符号乘法和补码乘法的结果是一样的**，因此 64 位乘法指令只有一条。

`clto` 指令把 `%rax` 的值符号扩展，结果存放在 `%rdx`（高 64 位）和 `%rax`（低 64 位）两个寄存器中

`idivq Src` **将 `%rdx`（高 64 位）和 `%rax`（低 64 位）存放的 128 位数作为被除数**，`Src` 作为除数，执行**补码除法**，商存放在 `%rax` 中，余数存放在 `%rdx` 中。`divq Src` 执行**无符号数除法**，其余与 `idivq` 相同。

对大多数 64 位除法应用来说，除数也常常是 64 位数。64 位的被除数存放在 `%rax` 中，而 `rdx` 应当置为全 0 或 `%rax` 的符号位（由 `clto` 指令实现）。以下是 64 位除法的例子：

```c
void remdiv(long x, long y, long* qp, long* rp) {
    long q = x / y;
    long r = x % y;
    *qp = q;
    *rp = r;
}
```

```x86asm
; void remdiv(long x, long y, long* qp, long* rp)
; x in %rdi, y in %rsi, qp in %rdx, rp in %rcx
remdiv:
    movq    %rdx, %r8       ; r8 = qp, rdx clear for use
    movq    %rdi, %rax      ; rax = x
    cqto
    idivq   %rsi            ; rax = x/y, rdx = x%y
    movq    %rax, (%r8)    ; *qp = rax
    movq    %rdx, (%rcx)   ; *rp = rdx
    ret
```


## 控制

常用的条件码（condition code）：
- `CF`：进位标志。最近的操作使最高位产生了进位。可用于检查无符号数的溢出。
- `ZF`：零标志。最近的操作结果为 0.
- `SF`：符号标志。最近的操作结果为负数。
- `OF`：溢出标志。最近的操作导致补码溢出。

它们被保存在 CPU 的一组单个位的条件码寄存器里。

`leaq` 是用于地址计算的指令，它不会改变任何条件码。除此之外，所有的算术和逻辑指令（`INC` `DEC` `NEG` `NOT` `ADD` `SUB` `IMUL` `XOR` `OR` `AND` `SAL` `SHL` `SAR` `SHR`）都会更新条件码。

逻辑操作的**进位标志**和**溢出标志**会被置 0.

移位操作的**进位标志**会被设置为**最后一个被移出的位**，**溢出标志**会被设置为 0。

`INC` `DEC` 指令会设置**溢出标志**和**零标志**，但不会改变**进位标志**。

`CMP` 指令和 `TEST` 指令只设置条件码，不改变其他寄存器。

`CMP S1, S2` 等价于 `SUB S1, S2`，而 `TEST S1, S2` 等价于 `AND S1, S2`。区别在于它们不会改变 `S2` 的值。


### 访问条件码

`SET` 指令根据条件码的某种组合设置一个字节的值。

| 指令 | 同义名 | 条件码组合 | 设置条件 |
| :----: | :----: | :----: | :----: |
| `sete D` | `setz` | `ZF` | 相等 / 零 |
| `setne D` | `setnz` | `~ZF` | 不等 / 非零 |
| `sets D` |  | `SF` | 负 |
| `setns D` |  | `~SF` | 非负 |
| `setg D` | `setnle` | `~(SF^OF)&~ZF` | 大于（有符号 >） |
| `setge D` | `setnl` | `~(SF^OF)` | 大于等于（有符号 >=） |
| `setl D` | `setnge` | `SF^OF` | 小于（有符号 <） |
| `setle D` | `setng` | `(SF^OF)|ZF` | 小于等于（有符号 <=） |
| `seta D` | `setnbe` | `~CF&~ZF` | 超过（无符号 >） |
| `setae D` | `setnb` | `~CF` | 超过或相等（无符号 >=） |
| `setb D` | `setnae` | `CF` | 低于（无符号 <） |
| `setbe D` | `setna` | `CF|ZF` | 低于或相等（无符号 <=） |

`set D` 中的目标位置 `D` 必须是**单字节寄存器**。

```x86asm
int comp(data_t a, data_t b)
a in %rdi, b in %rsi

comp:
    cmpq    %rsi, %rdi    ; %rdi - %rsi
    setl    %al
    movzbl  %al, %eax     ; 和 movzbq %al, %rax 相同
    ret
```


### 跳转指令


| 指令 | 同义名 | 跳转条件 | 描述 |
| :----: | :----: | :----: | :----: |
| `jmp Label` |  | 1 | 直接跳转 |
| `jmp *Operand` |  | 1 | 间接跳转 |
| `je Label` | `jz` | `ZF` | 相等 / 零 |
| `jne Label` | `jnz` | `~ZF` | 不等 / 非零 |
| `js Label` |  | `SF` | 负 |
| `jns Label` |  | `~SF` | 非负 |
| `jg Label` | `jnle` | `~(SF^OF)&~ZF` | 大于（有符号 >） |
| `jge Label` | `jnl` | `~(SF^OF)` | 大于等于（有符号 >=） |
| `jl Label` | `jnge` | `SF^OF` | 小于（有符号 <） |
| `jle Label` | `jng` | `(SF^OF)|ZF` | 小于等于（有符号 <=） |
| `ja Label` | `jnbe` | `~CF&~ZF` | 超过（无符号 >） |
| `jae Label` | `jnb` | `~CF` | 超过或相等（无符号 >=） |
| `jb Label` | `jnae` | `CF` | 低于（无符号 <） |
| `jbe Label` | `jna` | `CF|ZF` | 低于或相等（无符号 <=） |

无条件跳转可以是直接跳转，也可以是间接跳转。条件跳转只能是直接跳转。

跳转指令有几种不同的编码，最常用的是 PC 相对（PC-relative）的，其次是给出“绝对”地址。PC 相对的机器代码会将**目标指令的地址**与**紧跟在跳转指令之后的指令的地址**之间的差作为目标的编码，这些地址偏移量可以编码为 1, 2 或 4 字节。“绝对”地址的编码直接用 4 个字节指定目标。

当执行 PC 相对寻址时，程序计数器的值时跳转指令的后一条指令的地址，这与 CPU 的早期实现有关。当时的 CPU 将更新 PC 的值作为执行指令的第一步。通过使用 PC 相对寻址，指令的编码可以很简洁，且目标代码可以在链接过程中不做改变。

```c
// C 中的 if-else
if (test-expr) 
    then-statement
else 
    else-statement

// 通常的汇编实现
    t = test-expr
    if (!t)
        goto false;
    then-statement
    goto done;
false:
    else-statement
done:

// 另一种汇编实现
    t = test-expr
    if (t)
        goto true;
    else-statement
    goto done;
true:
    then-statement
done:

// 通常的汇编实现对于无 else 的 if 语句更简练：
    t = test-expr
    if (!t)
        goto done;
    then-statement
done:
```

### 条件传送



