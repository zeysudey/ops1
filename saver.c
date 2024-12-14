#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


#define BUFFER_SIZE 100

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <result_fifo> <result_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *result_fifo = argv[1];
    const char *result_file = argv[2];
    char buffer[BUFFER_SIZE];

    // FIFO'yu oluştur
    if (mkfifo(result_fifo, 0666) == -1 && errno != EEXIST) {
        perror("Failed to create result FIFO");
        return EXIT_FAILURE;
    }

    // FIFO'yu oku
    int fd = open(result_fifo, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open result FIFO");
        return EXIT_FAILURE;
    }

    // Sonuçları dosyaya kaydet
    FILE *file = fopen(result_file, "a");
    if (file == NULL) {
        perror("Failed to open result file");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Saver is running. Waiting for results...\n");
    while (1) {
        ssize_t bytesRead = read(fd, buffer, BUFFER_SIZE - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the string
            fprintf(file, "%s\n", buffer);
            fflush(file); // Hemen dosyaya yaz
        } else if (bytesRead == 0) {
            // FIFO kapatılmışsa, döngüyü durdur
            printf("FIFO closed by writer. Exiting saver.\n");
            break;
        } else {
            perror("Failed to read from result FIFO");
        }
    }

    fclose(file);
    close(fd);
    return EXIT_SUCCESS;
}
