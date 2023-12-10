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
    int senderId;
    int receiverId;
    char message[140];
    char date[16];
} messages;

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct Message {
    char date[20];  // Örnek bir uzunluk, ihtiyaca göre ayarlanabilir
};

int main() {
    struct Message message;
    time_t currentTime;
    time(&currentTime);
    struct tm* localTime = localtime(&currentTime);
    strftime(message.date, sizeof(message.date), "%H-%M-%S:%d-%m-%Y", localTime);
    printf("Elde Edilen Tarih: %s\n", message.date);

    return 0;
}
