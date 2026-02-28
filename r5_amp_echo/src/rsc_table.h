/* rsc_table.h */
#ifndef RSC_TABLE_H_
#define RSC_TABLE_H_

#include <stddef.h>
#include <openamp/open_amp.h>

#if defined __cplusplus
extern "C" {
#endif

/* 只声明函数，防止重复定义 */
void *get_resource_table (int rsc_id, int *len);

#if defined __cplusplus
}
#endif
#endif /* RSC_TABLE_H_ */
