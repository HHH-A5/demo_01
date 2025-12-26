# 1构建第一个keil程序

## 第一步，前往官网下载资料（gd32mcu.com）

点击一下资料下载，需要下载datasheet看引脚、user manual看外设、firware library库文件、add on芯片包

![image-20251008210119923](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008210119923.png)

![image-20251008210234569](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008210234569.png)

![image-20251008212015688](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008212015688.png)



## 第二步 创建并配置工程文件结构

### 文件结构

**USER:**  keil工程的存放位置, main.c。

**HW：**hardware硬件相关，比如串口，spi,  iic（同时新建Iclude和Source文件夹）。

**LIB:**  GD32的开发库（同时新建Iclude和Source文件夹）。

**CMSIS：**CM系列内核标准化接口（同时新建Iclude和Source文件夹）。

**OBJ：**编译工程的中间文件，生成可执行文件。

![image-20251008212856507](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008212856507.png)

打开add on文件夹，双击芯片安装包进行安装

![image-20251008213927353](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008213927353.png)

### 配置LIB

将**GD32F10x_Firmware_Library_V2.6.0** > Firmware > GD32F10x_standard_peripheral中的Include 和 Source复制到LIB中

![image-20251008214540552](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008214540552.png)

### 配置CMSIS

将**GD32F10x_Firmware_Library_V2.6.0** > Firmware > CMSIS中的这三个与厂商无关的头文件复制到CMSIS的Include文件夹里，以及与厂商相关的两个头文件**GD32F10x_Firmware_Library_V2.6.0** > Firmware > CMSIS\GD\GD32F10x\Include

- **core_cm3.h**：Cortex-M3 核心外设访问层头文件，定义了寄存器结构体、中断控制器（NVIC）、系统控制块（SCB）、SysTick、ITM、DWT、MPU、调试寄存器等的访问接口。
- **core_cmFunc.h**：核心寄存器访问函数头文件，提供了如 `__get_MSP()`、`__set_PSP()`、`__get_CONTROL()` 等内联函数，用于读写特殊功能寄存器（如 MSP、PSP、PRIMASK、BASEPRI 等）。
- **core_cmInstr.h**：核心指令访问头文件，封装了 Cortex-M 指令，如 `__NOP()`、`__WFI()`、`__DSB()`、`__REV()`、`__LDREXW()`、`__STREXW()` 等，用于执行底层汇编指令。

![image-20251008214915219](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008214915219.png)

#### 📁 `gd32f10x.h`

- **作用**：GD32F10x 系列芯片的 **顶层头文件**，相当于芯片的“身份证”。
- **内容**：
  - 定义了芯片型号（如 GD32F10X_MD、HD、XD、CL）；
  - 定义了外部晶振频率（HXTAL、IRC8M、LXTAL 等）；
  - 定义了 **中断号枚举 `IRQn_Type`**，包括 Cortex-M3 内核异常和外设中断；
  - 映射了 **内存地址空间**（Flash、SRAM、外设基地址等）；
  - 引入了 `core_cm3.h` 和 `system_gd32f10x.h`，并启用了标准外设驱动（`USE_STDPERIPH_DRIVER`）。

#### 📁 `system_gd32f10x.h`

- **作用**：系统初始化头文件，**CMSIS 标准接口**的一部分。
- **内容**：
  - 声明了 `SystemInit()`：初始化时钟、Flash 等待周期等；
  - 声明了 `SystemCoreClockUpdate()`：更新当前主频；
  - 可选地提供了 `gd32f10x_firmware_version_get()`：获取固件库版本；
  - 定义了全局变量 `SystemCoreClock`，保存当前系统主频。

![image-20251008215507525](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008215507525.png)



再打开**GD32F10x_Firmware_Library_V2.6.0** > Firmware > CMSIS\GD\GD32F10x\source文件夹，添加图片所示路径的文件，**GD32F10x_Firmware_Library_V2.6.0** > Firmware > CMSIS\GD\GD32F10x\source\RAM文件夹,其中.s文件是启动文件

📁 system_gd32f10x.c
作用：GD32F10x 系列芯片的“时钟 bootloader”，上电后第一个被调用的系统时钟配置引擎。
内容：

- 提供 `SystemInit()`，**上电后第一个被调用的函数**（由启动文件调用），复位后初始化 RCU（时钟控制器）、PLL、AHB/APB 分频、Flash 等待周期；
- 提供 `SystemCoreClockUpdate()`，根据寄存器实时计算并更新全局变量 `SystemCoreClock`；
- 内置多套时钟配置函数（24 M ~ 108 M），支持 HXTAL 或 IRC8M 作为 PLL 源；
- 包含电压波动防护延时宏 `RCU_MODIFY_DE_2()`，避免频率切换时 Vcore 抖动；
- 可选 `gd32f10x_firmware_version_get()` 返回固件库版本号；
- 通过条件编译宏（如 `__SYSTEM_CLOCK_108M_PLL_HXTAL`）决定最终系统频率，默认 108 M HXTAL。

![image-20251008215308636](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008215308636.png)

![image-20251008215403637](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008215403637.png)



### 配置HW

将**GD32F10x_Firmware_Library_V2.6.0** > Template中的与**中断相关**的文件移动到HW的include和source中

![image-20251008215723034](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008215723034.png)



## 第三步，用keil创建新工程

![image-20251008220756252](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008220756252.png)

### 路径选择

选择刚才创建的USER文件夹，再给工程命名（似乎中文也可以）

![image-20251008221019968](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008221019968.png)

### 选择芯片

选择gd32f10x芯片（只有前面安装了芯片包，这里才可以选择），点击ok，接着回跳出一个配置界面，可以直接关闭

![image-20251008221223712](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008221223712.png)

### 添加分组

打开品字型工具， 给工程命名，并添加我们之前创建的组

![image-20251008221553398](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008221553398.png)

![image-20251008221643465](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008221643465.png)



### 添加头文件路径

![image-20251008222106097](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008222106097.png)

### 添加文件

开始往组里面添加文件（右击文件夹，选择以及存在的文件）

![image-20251008222644744](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008222644744.png)



misu.c: 与中断，系统嘀嗒定时器相关

rcu.c: 与时钟配置相关

gpio.c: 更不用说了

这是添加后的文件概览，后面有需要再及时添加

![image-20251008222747336](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008222747336.png)



### 创建一个main.c并调试

创建一个main.c文件，并添加到USER中，其中的总的这个头文件在system_gd32f10x.c的第39行

![image-20251008223138018](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008223138018.png)



删除一些还没使用的

![image-20251008223359318](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008223359318.png)



编译工程发现错误，因为这个头文件我们还没有定义，我们可以自己新建一个

![image-20251008223614286](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008223614286.png)



新建一个名为RTE_components.h的文件，注意不要写错字，保存到USRE下，并加入这三个宏定义（只有这样，LIB里的文件才能include）,此时再编译，发现只有一个错误而已了。![image-20251008224525742](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008224525742.png)



打开错误的位置，先把这两个头文件删掉即可，这样一个0Error,0Warning的基础工程就创建好了

![image-20251008224800377](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251008224800377.png)