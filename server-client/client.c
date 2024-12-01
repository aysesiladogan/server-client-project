#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Winsock başlat
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock baslatilamadi. Hata kodu: %d\n", WSAGetLastError());
        return -1;
    }

    // Soket oluştur
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Soket olusturulamadi. Hata kodu: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    // Sunucu adresini ayarla
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bağlan
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Sunucuya baglanilamadi.\n");
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    printf("Sunucuya baglanildi.\n");

    // Soruları al ve cevapla
    for (int i = 0; i < 3; i++) {
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("Sunucudan gelen soru: %s\n", buffer);

        // Kullanıcıdan cevap al
        printf("Cevabinizi (1-4 arasi bir sayi) girin: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Cevabı sunucuya gönder
        send(sock, buffer, strlen(buffer), 0);
    }

    printf("Baglanti kapatiliyor...\n");
    closesocket(sock);
    WSACleanup();

    return 0;
}
