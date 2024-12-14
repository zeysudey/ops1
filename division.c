#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 100

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s input_fifo output_fifo\n", argv[0]);
        return 1;
    }

    // Giriş FIFO'sını aç
    int fd_in = open(argv[1], O_RDONLY);
    if (fd_in == -1) {
        printf("Failed to open input FIFO\n");
        return 1;
    }

    // Veriyi oku
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(fd_in, buffer, BUFFER_SIZE - 1);
    close(fd_in);

    if (bytes_read <= 0) {
        printf("Failed to read from input FIFO\n");
        return 1;
    }

    buffer[bytes_read] = '\0'; // Verinin sonunu belirle

    // İki sayıyı al
    int a, b;
    if (sscanf(buffer, "%d %d", &a, &b) != 2) {
        printf("Error: Invalid input format\n");
        return 1;
    }

    // Bölme sıfıra yapılabilir mi kontrolü
    if (b == 0) {
        printf("Error: Division by zero is not allowed\n");
        return 1;
    }

    // Bölme işlemi
    int result = a / b;

    // Çıkış FIFO'sına yaz
    int fd_out = open(argv[2], O_WRONLY);
    if (fd_out == -1) {
        printf("Failed to open output FIFO\n");
        return 1;
    }

    dprintf(fd_out, "%d", result); // Sonucu çıkış FIFO'sına yaz
    close(fd_out); // FIFO'yu kapat

    return 0;
}
