#include <stdio.h>
#include <sys/stat.h>

int main() {
    char filename[] = "example.txt"; // 文件名
    struct stat file_stat;

    // 获取文件信息
    if (stat(filename, &file_stat) == 0) {
        // 如果成功获取到文件信息，则输出文件大小
        printf("File Size: %ld bytes\n", file_stat.st_size);
    } else {
        // 如果获取文件信息失败，则输出错误信息
        perror("Error getting file information");
        return 1;
    }

    return 0;
}