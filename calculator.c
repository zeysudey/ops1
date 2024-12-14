#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 100
#define FIFO_COUNT 4
#define FIFO_NAME_SIZE 20

char input_fifo[FIFO_COUNT][FIFO_NAME_SIZE];
char output_fifo[FIFO_COUNT][FIFO_NAME_SIZE];

void cleanup_fifos() {
    for (int i = 0; i < FIFO_COUNT; i++) {
        unlink(input_fifo[i]);
        unlink(output_fifo[i]);
    }
}

int main() {
    char buffer[BUFFER_SIZE];
    pid_t pids[FIFO_COUNT];

    // FIFO'ları oluştur
    for (int i = 0; i < FIFO_COUNT; i++) {
        snprintf(input_fifo[i], FIFO_NAME_SIZE, "input_fifo_%d", i);
        snprintf(output_fifo[i], FIFO_NAME_SIZE, "output_fifo_%d", i);

        unlink(input_fifo[i]);
        unlink(output_fifo[i]);

        if (mkfifo(input_fifo[i], 0666) == -1) {
            perror("Failed to create input FIFO");
            cleanup_fifos();
            return EXIT_FAILURE;
        }

        if (mkfifo(output_fifo[i], 0666) == -1) {
            perror("Failed to create output FIFO");
            cleanup_fifos();
            return EXIT_FAILURE;
        }
    }

    // Çocuk süreçleri oluştur
    for (int i = 0; i < FIFO_COUNT; i++) {
        pids[i] = fork();

        if (pids[i] == -1) {
            perror("Fork failed");
            cleanup_fifos();
            return EXIT_FAILURE;
        }

        if (pids[i] == 0) { // Çocuk süreç
            execlp(i == 0 ? "./addition" : i == 1 ? "./subtraction" : i == 2 ? "./multiplication" : "./division", 
                   i == 0 ? "addition" : i == 1 ? "subtraction" : i == 2 ? "multiplication" : "division",
                   input_fifo[i], output_fifo[i], NULL);

            perror("execlp failed");
            exit(EXIT_FAILURE);
        }
    }

    // Kullanıcı işlemleri
    while (1) {
        printf("Calculator\n");
        printf("1- Addition\n2- Subtraction\n3- Multiplication\n4- Division\n5- Exit\n");
        printf("Select an operation: ");

        int choice;
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input, please enter a valid choice.\n");
            continue;
        }

        if (choice == 5) {
            printf("Exiting...\n");
            for (int i = 0; i < FIFO_COUNT; i++) {
                kill(pids[i], SIGTERM);
                waitpid(pids[i], NULL, 0);
            }
            cleanup_fifos();
            break;
        }

        if (choice < 1 || choice > 4) {
            printf("Invalid selection. Try again.\n");
            continue;
        }

        int a, b;
        printf("Enter two numbers: ");
        if (scanf("%d %d", &a, &b) != 2) {
            while (getchar() != '\n');
            printf("Invalid numbers, please try again.\n");
            continue;
        }

        snprintf(buffer, BUFFER_SIZE, "%d %d", a, b);

        // Giriş FIFO'sına yazma
        int fd_in;
        while ((fd_in = open(input_fifo[choice - 1], O_WRONLY)) == -1) {
            if (errno != ENXIO) {
                perror("Failed to open input FIFO for writing");
                break;
            }
            usleep(100000);  // FIFO'nun başka bir süreç tarafından açılmasını beklemek için
        }

        if (fd_in == -1) {
            continue; // FIFO'ya yazılamazsa, işlemi atla
        }

        if (write(fd_in, buffer, strlen(buffer) + 1) == -1) {
            perror("Failed to write to input FIFO");
            close(fd_in);
            continue;
        }

        close(fd_in);  // Giriş FIFO'sını kapat

        // Çıkış FIFO'sından okuma
        int fd_out;
        while ((fd_out = open(output_fifo[choice - 1], O_RDONLY)) == -1) {
            if (errno != ENXIO) {
                perror("Failed to open output FIFO for reading");
                break;
            }
            usleep(100000);  // FIFO'nun başka bir süreç tarafından açılmasını beklemek için
        }

        if (fd_out == -1) {
            continue; // Çıkış FIFO'sına da okunamazsa, işlemi atla
        }

        ssize_t bytesRead = read(fd_out, buffer, BUFFER_SIZE);  // Çıkış FIFO'sından veriyi oku
        if (bytesRead <= 0) {
            perror("Failed to read from output FIFO");
            close(fd_out);
            continue;
        }

        // Sonucun yalnızca bir kez yazılmasını sağla
        buffer[bytesRead] = '\0';  // Buffer'ı temizle (okuma sonrasını garanti et)
        printf("Result: %s\n", buffer);
        close(fd_out);  // Çıkış FIFO'sını kapat
    }

    return 0;
}
