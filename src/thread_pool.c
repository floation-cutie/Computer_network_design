// #include "thread_pool.h"
// // extern CRITICAL_SECTION threadPoolCS; // 线程池临界区
// // extern HANDLE semaphore;              // 信号量，用于线程池和等待队列之间的同步

// // 初始化线程池和等待队列
// void init_thread_pool(struct ThreadPool *pool) {
//     // 初始化线程池
//     pool->count = THREAD_POOL_SIZE; //
//     for (int i = 0; i < THREAD_POOL_SIZE; i++) {
//         pool->params[i] = (struct ThreadParam *)malloc(sizeof(struct ThreadParam));
//         pool->params[i]->clientAddrLen = 0;
//         pool->params[i]->msg = NULL;
//         pool->params[i]->len = 0;
//     }
//     InitializeCriticalSection(&threadPoolCS);                                    // 初始化临界区
//     semaphore = CreateSemaphore(NULL, THREAD_POOL_SIZE, THREAD_POOL_SIZE, NULL); // 初始化信号量
// }

// // 销毁线程池和等待队列
// void destroy_thread_pool(struct ThreadPool *pool) {
//     for (int i = 0; i < pool->count; i++) // 释放线程池中的参数
//     {
//         free(pool->params[i]);
//     }
//     DeleteCriticalSection(&threadPoolCS); // 删除临界区
//     CloseHandle(semaphore);               // 关闭信号量
// }

// // 添加DNS请求到线程池或等待队列中
// void add_to_pool(struct ThreadPool *pool, struct ThreadParam *param) {
//     EnterCriticalSection(&threadPoolCS); // 进入临界区
//     if (pool->count < THREAD_POOL_SIZE)  // 如果线程池中有空闲线程
//     {
//         pool->params[pool->count++] = param;  // 将参数添加到线程池中
//         ReleaseSemaphore(semaphore, 1, NULL); // 释放信号量
//     }
//     LeaveCriticalSection(&threadPoolCS); // 离开临界区
// }

// // 线程入口函数，用于处理DNS请求
// unsigned __stdcall threadProc(void *pParam) {
//     struct ThreadParam *param = (struct ThreadParam *)pParam;                           // 获取参数
//     handle_client_request_loop(server_sock, param->clientAddr, param->msg, param->len); // 处理DNS请求
//     return 0;
// }