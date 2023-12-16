#include <stdio.h>
#include <stdlib.h>

typedef struct users{
    int userId; // benzersiz id
    char name[20];
    char surname[20];
    char password[20];
    char phoneNumber[13];// +901231231212 (13 karakter)
} users;
typedef struct messages{
    int messageId;
    char senderId[15];
    char receiverId[15];
    char message[140];
    char date[20];
    char status[2]; // + : okundu, - : okunmadı
} messages;

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct Message {
    char date[20];  // Örnek bir uzunluk, ihtiyaca göre ayarlanabilir
};


int getLastMessageId(FILE *fp) {
    messages message;
    int lastMessageId = -1;
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);

    if (fileSize > 0) {
        fseek(fp, -1, SEEK_END); // Move one character back from the end

        int newlineCount = 0;
        while (ftell(fp) > 0) {
            char c = fgetc(fp);
            if (c == '\n') {
                newlineCount++;
                if (newlineCount == 2) {
                    break; // Found the second newline character, exit the loop
                }
            }
            fseek(fp, -2, SEEK_CUR); // Move two characters back
        }

        fscanf(fp, "%d,%[^,],%[^,],%[^,],%[^,],%[^\n]", &message.messageId, message.senderId, message.receiverId, message.date, message.status, message.message);
        lastMessageId = message.messageId;
    }
    return lastMessageId;
}





int main() {
    /*
    struct Message message;
    time_t currentTime;
    time(&currentTime);
    struct tm* localTime = localtime(&currentTime);
    strftime(message.date, sizeof(message.date), "%H-%M-%S:%d-%m-%Y", localTime);
    printf("Elde Edilen Tarih: %s\n", message.date);
    */
    FILE *fp = fopen("mesajlar/+905515968786,+901112223344.csv", "r");
    if (fp == NULL) {
        printf("Dosya acilamadi!\n");
        return 1;
    }
    int lastMessageId = getLastMessageId(fp);
    printf("Son mesaj id: %d\n", lastMessageId);



    return 0;
}
