# 文件系统
## 一、目录和文件

👉 见 **DAY3**

## 二、系统数据文件和信息

👉 见 **DAY4**

---
## 三、进程环境
1. `main函数`
    int main(int argc, char * argv[])

2. `进程的终止情况（牢记5 + 3）`
    - 正常终止：
        1.从main函数返回 （echo $? 打印返回上一个进程的状态）
        2.调用exit(): 执行钩子函数、IO处理后调用_exit或_Exit
        调用_exit (严重出错时)
        ```c
        int f()
        return 0/1/2

        main()
        swithc()
        case 0:
        case 1:
        case 2:
        default: 
        _exit()
        ```
        3.调用_exit或_Exit: 不会执行钩子函数、IO处理
        4.最后一个线程从其启动例程返回
        5.最后一个线程调用pthread_exit

    - 异常终止：
        1.调用abort
        2.接到一个信号并终止
        3.最后一个线程对其取消请求被迫作出响应
    
    - `atexit()、on_exit() 钩子函数`
        在进程`正常终止`前调用所有注册的函数，调用顺序先进后出
        例子：多文件打开关闭时调用
        ```c
        if(fd1 < 0)
        报错、退出
        else 
        atexit(f1) -> 关闭f1 //打开成功注册钩子函数，作用关闭文件

        if(fd2 < 0)
        报错、关闭fd1、退出//注册钩子函数后不需要在fd2中手动关闭，钩子函数自动关闭
        else
        atexit(f2)
        ```

3. `命令令行参数分析`
   
    实现带选项时则需要在相应字符后面添加`:`用
    全局变量`optarg`获取相应参数的10进制形式
    `-`参数位于开头用于指明后面匹配的都是选项参数
    若遇到非选项参数则返回1 
    例 `"-abcd"` 说明 ls -a -b -c -d  test 其中 -a -b -c -d 为选项参数
    test为非选项参数
    `optind`返回参数位于argv中下标的后一位置

    ```c
    // 短参数 -l
       int getopt(int argc, char *const argv[], const char *optstring);   
    ```
    ```c
    // 长参数 -long
       int getopt_long(int argc, char *const argv[], const char *optstring);
    ```

4. `环境变量 每个C的虚拟空间都有`
    KEY = VALUE
    全局变量 extern char ** environ
    getenv()//获取环境变量
    setenv()//改变或者添加环境变量
    unsetenv()//删除一个环境变量
    putenv()//改变或者添加一个环境变量 缺陷：没有const 可能会被系统改变

5. `C程序的存储空间布局`
    pmap pid  man 1 查看进程的存储空间 

6. `库`
    动态库
    静态库
    手工装载库 //类似于插件
    dlopen();
    dlerror()
    dlsym 查找手工装载库中的关键字 

7. `函数跳转`
    goto 不安全
    安全的跨函数跳转 安全的goto
    setjmp();//设置跳转点 man 2
    longjmp();//跳转到指定位置 man 2

8. `资源的获取以及控制`
    getrlimit(); //获取资源总量 man 3
    setrlimit(); // 设置资源总理 man 3
    这两个函数可以封装为limit 
        struct rlimit {
        rlim_t rlim_cur;  /* Soft limit */
        rlim_t rlim_max;  /* Hard limit (ceiling for rlim_cur) */
    };

    软限制小于硬限制

    普通用户：可以升高或降低软限制，可以`降低`硬限制
    root：可以升高或者降低硬限制
