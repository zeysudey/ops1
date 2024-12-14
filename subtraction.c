#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 100

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_fifo output_fifo\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Giriş ve çıkış FIFO dosyalarını aç
    char buffer[BUFFER_SIZE];
    int fd_in = open(argv[1], O_RDONLY);
    if (fd_in == -1) {
        perror("Failed to open input FIFO");
        return EXIT_FAILURE;
    }

    int fd_out = open(argv[2], O_WRONLY);
    if (fd_out == -1) {
        perror("Failed to open output FIFO");
        close(fd_in); // Giriş FIFO'sunu kapat
        return EXIT_FAILURE;
    }

    // Giriş verisini oku
    ssize_t bytes_read = read(fd_in, buffer, BUFFER_SIZE);
    if (bytes_read <= 0) {
        perror("Failed to read from input FIFO");
        close(fd_in);
        close(fd_out);
        return EXIT_FAILURE;
    }

    // Giriş verisini ayıkla
    int a, b;
    if (sscanf(buffer, "%d %d", &a, &b) != 2) {
        snprintf(buffer, BUFFER_SIZE, "Error: Invalid input");
    } else {
        // Sonucu hesapla
        int result = a - b;
        snprintf(buffer, BUFFER_SIZE, "%d", result);
    }

    // Sonucu çıkış FIFO'suna yaz
    if (write(fd_out, buffer, strlen(buffer) + 1) == -1) {
        perror("Failed to write to output FIFO");
        close(fd_in);
        close(fd_out);
        return EXIT_FAILURE;
    }

    // FIFO dosyalarını kapat
    close(fd_in);
    close(fd_out);

    return 0;
}