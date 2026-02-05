#ifndef ANYTIMER_H__
#define ANYTIMER_H__

# define MAX_LIST 1024

typedef void at_jobfunc_t(void *);

void anytimer_init();

int at_addjob(int sec, at_jobfunc_t *jonp, void * arg);
/**
 * return >= 0 成功，返回任务ID
 *        == -EINVAL 失败，参数非法
 *        == -ENOSPC 失败，数组满
 *        == -ENOMEM 失败，内存空间不足
*/

//保证异步操作同步化
int at_canceljob(int id);
/** 
 * return == 0 成功，指定任务已经取消
 *        == -EINVAL 失败，参数非法
 *        == -EBUSY 失败，指定任务已经完成
 *        == -ECANCELED 失败，指定任务重复取消
*/

int at_resumejob(int id);
/** 
 * return == 0 成功，指定任务成功恢复
 *        == -EINVAL 失败，参数非法
 *        == -EBUSY 失败，指定任务不再取消态
*/

int at_waitjob(int id);
/**
 * return == 0 成功，指定任务成功释放
 *        == -EINVAL 失败，参数非法
*/

#endif