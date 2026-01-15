## 文件系统

类ls的实现，如myls

### 一、目录和文件
* 1、获取文件属性
    1.stat: 通过文件路径获取文件的属性，面对文件描述符时获取的是所指向目标文件的属性
    2.fstat：通过文件描述符获取文件的属性
    3.lstat：面对文件描述符时获取的是链接文件的属性
     man 2 系统调用，man 3 查询stat结构体, 
      * 参数st_mode（16位的位图） 可以查询文件的类型、用户权限等参数）使用宏S_ISREG等（man 6 inode）
        位置    含义                    位数
        ━━━━━━━━━━━━━━━━━━━━━
        0-2      其他用户权限 (o)        3 位  (x w r)
        3-5      所属组权限 (g)          3 位  (x w r)
        6-8      所有者权限 (u)          3 位  (x w r)
        9-11     特殊权限位              3 位  (sticky bit, setgid, setuid)
        12-15    文件类型                4 位  (regular, directory, symlink, etc.)
        ━━━━━━━━━━━━━━━━━━━━━

* 2、获取文件访问权限

* 3、umask
    防止产生权限过松的文件
    man 2 umask:在进程中设置umask的值

* 4、文件权限的更改/管理
    chmod,fchmod

* 5、粘住位
    t位
    ✅ 目录所有者可以删除自己的文件
    ✅ 目录所有者可以删除目录中的任何文件
    ❌ 普通用户只能删除自己的文件，不能删除其他用户的文件（即使该文件对用户可写）

* 6、文件系统：FAT、UFS
* 7、硬链接，符号链接
    硬链接 ln source distination,两个文件指向同一个inode
    软链接 ln -s source distination,软链接有自己的inode,其inode会存储被链接文件的路径

    系统调用:  
    link (man 2)
    unlink 从磁盘中删除（硬链接计数减1->类似于删除目录项）
    remove: 删除文件或目录 man 3 
    rename：重命名或者改变位置 = mv指令 man 2

* 8、utime
    man 2 更改最后一次读或写的时间
    
* 9、目录的创建和销毁
    mkdir man 2  创建一个目录 man 2
    rmdir man 2  删除一个目录(空) man 2

* 10、更改当前工作路径
    chdir 
    fchdir
    == cd: 修改当前进程的工作目录
    这两个函数可以突破假根技术(chroot)：让当前进程以为自己在根目录
    getcwd == pwd: 获得当前的工作目录

* 11、分析目录/读取目录内容
    glob可以通过设置flags设置自己想要的功能，从而实现对目录的操作
       int glob(const char *restrict pattern, int flags,
                int (*errfunc)(const char *epath, int eerrno),
                glob_t *restrict pglob);:解析模式/通配符 man 3 （作用：目录分析）
    typedef struct {
    size_t   gl_pathc;    /* 匹配的字符数*/
    char   **gl_pathv;    /* 路径集合 */
    size_t   gl_offs;     /* Slots to reserve in gl_pathv.  */
} glob_t;

void globfree(glob_t *pglob);
 用完记得globfree

  (创建文件目录 mkdir rmdir)
  opendir() man 3
  closedir man 3 
  readdir() man 3 
  rewenddir() man 3
  seekdir() man 3
  telldir() man 3 

### 二、系统数据文件和信息
    见DAY4
### 三、进程环境
    见DAY5