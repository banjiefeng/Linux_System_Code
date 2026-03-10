/*
    CPS：每秒的传输速率
    BURST：桶的最大TOKEN
    TOKEN：
*/
#ifndef MYTBF_H__
#define MYTBF_H__  

# define MAX_TBF 1024

typedef void tbf_st;

tbf_st *tbf_init(int, int);

int tbf_fetchtoken(tbf_st*, int);

int tbf_returntoken(tbf_st*, int);

int tbf_destory(tbf_st*);

#endif