#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define SHM_KEY 12345
#define SEM_KEY 54321
#define SHM_SIZE 1024

// 信号量操作结构
struct sembuf sem_op;

void sem_lock(int sem_id) {
    sem_op.sem_num = 0;
    sem_op.sem_op = -1; // P操作，锁定信号量
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

void sem_unlock(int sem_id) {
    sem_op.sem_num = 0;
    sem_op.sem_op = 1; // V操作，解锁信号量
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

int main() {
    // 创建共享内存
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget");
        exit(1);
    }

    // 附加共享内存
    char *shared_memory = (char *)shmat(shm_id, NULL, 0);
    if (shared_memory == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    // 创建信号量
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id < 0) {
        perror("semget");
        exit(1);
    }

    // 初始化信号量
    semctl(sem_id, 0, SETVAL, 1);

    while (1) {
        // 等待客户端写入数据
        sem_lock(sem_id);

        // 从共享内存读取数据
        printf("Server received: %s\n", shared_memory);

        // 清空共享内存（简单处理，防止残留数据）
        memset(shared_memory, 0, SHM_SIZE);

        // 通知客户端数据已读取
        sem_unlock(sem_id);

        sleep(1);
    }

    // 分离共享内存
    shmdt(shared_memory);

    // 删除共享内存和信号量
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);

    return 0;
}