#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define MAX_QUESTIONS 3

// Global değişkenler: Yanıt istatistiklerini tutmak için
int answers[MAX_QUESTIONS][4] = {0}; // [Soru][Seçenek Sayacı]

void handle_client(void* client_socket) {
    SOCKET client_sock = *(SOCKET*)client_socket;
    char buffer[BUFFER_SIZE] = {0};
    char* questions[MAX_QUESTIONS] = {
        "Soru 1: Hangi programlama dilini tercih edersiniz? (1) C (2) Python (3) Java (4) Diğer",
        "Soru 2: Gunluk kac saat kod yaziyorsunuz? (1) 0-2 (2) 3-5 (3) 6-8 (4) 9+",
        "Soru 3: Backend mi yoksa frontend mi sizin icin daha ilgi cekici? (1) Backend (2) Frontend (3) İkisi de (4) Hicbiri"
    };

    printf("Yeni bir istemci baglandi.\n");

    // Soruları istemciye gönder
    for (int i = 0; i < MAX_QUESTIONS; i++) {
        // Soru gönder
        send(client_sock, questions[i], strlen(questions[i]), 0);
        printf("Gonderilen soru: %s\n", questions[i]);

        // Cevap al
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            int answer = atoi(buffer); // Yanıtı tamsayıya çevir
            if (answer >= 1 && answer <= 4) {
                answers[i][answer - 1]++; // Yanıtı kaydet
                printf("Soru %d icin yanit %d olarak kaydedildi.\n", i + 1, answer);
            } else {
                printf("Gecersiz yanit alindi: %s\n", buffer);
            }
        } else {
            printf("İstemci baglantisi kesildi.\n");
            break;
        }
    }

    printf("İstemci baglantisi kapatiliyor...\n");
    closesocket(client_sock);
    _endthread();
}

void print_statistics() {
    printf("\nAnket Sonuclari:\n");
    for (int i = 0; i < MAX_QUESTIONS; i++) {
        printf("Soru %d Sonuclari:\n", i + 1);
        printf("(1) %d yanit\n", answers[i][0]);
        printf("(2) %d yanit\n", answers[i][1]);
        printf("(3) %d yanit\n", answers[i][2]);
        printf("(4) %d yanit\n", answers[i][3]);
    }
    printf("\n");
}

int main() {
    WSADATA wsaData;
    SOCKET server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    // Winsock başlat
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock baslatilamadi. Hata kodu: %d\n", WSAGetLastError());
        return -1;
    }

    // Soket oluştur
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        printf("Soket olusturulamadi. Hata kodu: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    // Sunucu adresini ayarla
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Soketi bağla
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Baglama basarisiz. Hata kodu: %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return -1;
    }

    // Dinlemeye başla
    if (listen(server_sock, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("Dinleme basarisiz. Hata kodu: %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return -1;
    }

    printf("Sunucu dinliyor...\n");

    // İstemcileri kabul et
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock == INVALID_SOCKET) {
            printf("Baglanti kabul edilemedi. Hata kodu: %d\n", WSAGetLastError());
            continue;
        }

        // Yeni bir thread başlat
        _beginthread(handle_client, 0, (void*)&client_sock);

        // Sonuçları her yeni istemciden sonra yazdır
        print_statistics();
    }

    // Sunucuyu kapat
    closesocket(server_sock);
    WSACleanup();

    return 0;
}
