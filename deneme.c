#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 100

int main() {
    const char input_fifo= "/tmp/addition_in";
    const char output_fifo= "/tmp/addition_out";

    int i = 0;
    mkfifo(input_fifo, 0666);
    mkfifo(output_fifo, 0666);
int choice;
        scanf("%d", &choice);
    char buffer[BUFFER_SIZE];
    int a, b;
        printf("Enter two numbers: ");
        scanf("%d %d", &a, &b);
     snprintf(buffer, BUFFER_SIZE, "%d %d", a, b);
     int fd_in = open(input_fifo, O_WRONLY);
        if (fd_in == -1) {
            perror("Failed to open input FIFO for writing");
          
        }
        write(fd_in, buffer, strlen(buffer) + 1);
        close(fd_in);
        pid_t pid = fork();  // Alt programı çalıştırmak için süreç oluştur
        if (pid == -1) {
            perror("Fork failed");
            
        }

        if (pid == 0) {  // Çocuk süreç
            char *args[] = {
                NULL,  // Çalıştırılacak dosya
                input_fifo,  // Giriş FIFO
                output_fifo, // Çıkış FIFO
                NULL
            };
         switch (1) {
                case 1: args[0] = "./addition"; break;
}
         execv(args[0], args);  // Alt programı çalıştır
            perror("Exec failed");
            exit(1);
            //Alt programın sonucunu FIFO'dan oku
        int fd_out = open(output_fifo, O_RDONLY);
        if (fd_out == -1) {
            perror("Failed to open output FIFO for reading");
        }
        read(fd_out, buffer, BUFFER_SIZE);
        printf("Result: %s\n", buffer);
        close(fd_out);

        wait(NULL);  // Çocuk sürecin tamamlanmasını bekle
    }
}