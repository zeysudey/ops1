#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_fifo> <output_fifo>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *input_fifo = argv[1];
    char *output_fifo = argv[2];
    char buffer[100];

    // Giriş FIFO'sunu aç
    int fd_in = open(input_fifo, O_RDONLY);
    if (fd_in == -1) {
        perror("Failed to open input FIFO");
        return EXIT_FAILURE;
    }

    // Çıkış FIFO'sunu aç
    int fd_out = open(output_fifo, O_WRONLY);
    if (fd_out == -1) {
        perror("Failed to open output FIFO");
        close(fd_in);
        return EXIT_FAILURE;
    }

    while (1) {
        // Giriş FIFO'sından oku
        ssize_t bytes_read = read(fd_in, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break; // Okuma hatası veya EOF
        }

        int a, b;
        if (sscanf(buffer, "%d %d", &a, &b) != 2) {
            snprintf(buffer, sizeof(buffer), "Error: Invalid input");
        } else {
            int result = a + b;
            snprintf(buffer, sizeof(buffer), "%d", result);
        }

        // Çıkış FIFO'sına yaz
        if (write(fd_out, buffer, strlen(buffer) + 1) == -1) {
            perror("Failed to write to output FIFO");
            break;
        }
    }

    close(fd_in);
    close(fd_out);
    return 0;
}
