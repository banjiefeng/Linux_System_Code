#ifndef MYSEM_H__
#define MYSEM_H__

typedef void mysem_st;

mysem_st * mysem_init(int inival);

int mysem_add(mysem_st *, int);

int mysem_sub(mysem_st *, int);

int mysem_destory(mysem_st *);

#endif