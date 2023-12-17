#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <dirent.h>

#define PORT 8080
#define MAX_CLIENTS 10

typedef struct users{
    int userId; // benzersiz id
    char name[20];
    char surname[20];
    char password[20];
    char phoneNumber[14];// +901231231212 (13 karakter)
} users;

typedef struct messages{
    int messageId;
    char senderId[15];
    char receiverId[15];
    char message[140];
    char date[20];
    char status[2]; // + : okundu, - : okunmadı
} messages;

typedef struct onlineUsers{
    char phoneNumber[15];
    int clientSocket;
} onlineUsers;

void initializeServer(int* server_fd, struct sockaddr_in* address, int* opt); // Server baslatma fonksiyonu
void handleLogin(char* buffer, int clientSocket); // Giris yapma fonksiyonu
void handleRegister(char* buffer,int clientSocket); // Kayit olma fonksiyonu
void handleListFriends(char* buffer, int clientSocket); // Rehber listeleme fonksiyonu
void handleAddToList(char* buffer,int clientSocket); // Rehbere ekleme fonksiyonu
void handleDeleteFromList(char* buffer,int clientSocket); // Rehberden silme fonksiyonu
void handleSendMessage(char* buffer,int clientSocket); // Mesaj gonderme fonksiyonu
void handleCheckMessage(char* buffer,int clientSocket); // Mesaj kontrol etme fonksiyonu
void handleGetMessages(char* buffer,int clientSocket); // Mesajlari getirme fonksiyonu
void handleDeleteMessage(char* buffer, int clientSocket); // Mesaj silme fonksiyonu
void sendNotification(char* phone,char* senderPhone,char* message); // Bildirim gonderme fonksiyonu
void deleteFromNotificationList(int clientSocket); // Bildirim listesinden silme fonksiyonu
void addToNotificationList(char* phone,int clientSocket); // Bildirim listesine ekleme fonksiyonu
void* handleClient(void* arg); // Her bir istemci icin bir thread olusturur. İstekleri ayirmak icin kullanilir.
int getLastMessageId(FILE *fp); // Bir sohbetteki son mesajın id degerini dondurur.
int getLastUserId(FILE *fp); // users dosyasindaki son kullanici id'sini dondurur.
bool userCheck(char* phoneNumber); // Kullanici var mi yok mu kontrol eder.
bool fileCheck(char* fileName); // Dosya var mi yok mu kontrol eder.
bool checkContactList(FILE *fp,char* phoneNumber); // Rehberde kullanici var mi yok mu kontrol eder.

onlineUsers onlineUsersList[MAX_CLIENTS]; // Global degisken, online kullanicilar listesi.
//Tum threadlarda bu fonksiyonu paylasmak icin gereken struct yapıları parametre aktarımları vs. ile okunabilirligi dusuk kod yerine
//global degisken kullanilmistir.

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    pthread_t thread_id[MAX_CLIENTS];
    initializeServer(&server_fd, &address, &opt);
    // Server baslatiliyor.
    int clientCount = 0;
    printf("SERVER BASLATILDI: %d\n", PORT);
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        if (pthread_create(&thread_id[clientCount], NULL, handleClient, &new_socket) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        clientCount++;
    }
    close(server_fd);
    return 0;
}

// Her bir istemci icin bir thread olusturulur. Bu fonksiyon isteklere cevap verir.
void* handleClient(void* arg) {
    int* clientSocketPtr = (int*)arg;
    int clientSocket = *clientSocketPtr;
    char buffer[1024] = {0};
    char response[1024] = {0};
    int flag = 1;
    printf("%d numarali istemci baglandi.\n", clientSocket);
    while (flag) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(clientSocket, buffer, 1024);
        if (valread == 0) {
            flag = 0;
        }
        //printf("Client %d: %.*s\n", clientSocket, valread, buffer);
        if(strncmp(buffer,"/login",6)==0){
            printf("%d -> Giris yapma istegi\n",clientSocket);
            handleLogin(buffer+7,clientSocket);
        }else if(strncmp(buffer,"/register",9)==0){
            printf("%d -> Kayit olma istegi\n",clientSocket);
            handleRegister(buffer+10,clientSocket);
        }else if(strncmp(buffer,"/listFriends",12)==0){
            printf("%d -> Rehber listeleme istegi\n",clientSocket);
            handleListFriends(buffer+13,clientSocket);
        }else if(strncmp(buffer,"/addToList",10)==0){
            printf("%d -> Rehbere ekleme istegi\n",clientSocket);
            handleAddToList(buffer+11,clientSocket);
        }else if(strncmp(buffer,"/deleteFromList",15)==0){
            printf("%d -> Rehberden silme istegi\n",clientSocket);
            handleDeleteFromList(buffer+16,clientSocket);
        }else if(strncmp(buffer,"/sendMessage",12)==0){
            printf("%d -> Mesaj gonderme istegi\n",clientSocket);
            handleSendMessage(buffer+13,clientSocket);
        }else if(strncmp(buffer,"/checkMessage",13)==0){
            printf("%d -> Mesaj kontrol etme istegi\n",clientSocket);
            handleCheckMessage(buffer+14,clientSocket);
        }else if(strncmp(buffer,"/getMessages",12)==0){
            printf("%d -> Mesajlari getirme istegi\n",clientSocket);
            handleGetMessages(buffer+13,clientSocket);
        }else if(strncmp(buffer,"/deleteMessage",14)==0){
            printf("%d -> Mesaj silme istegi\n",clientSocket);
            handleDeleteMessage(buffer+15,clientSocket);
        }else if(strncmp(buffer,"/getNotification",16)==0){
            printf("%d -> Bildirimleri getirme istegi\n",clientSocket);
            addToNotificationList(buffer+17,clientSocket);
        }
    }
    printf("%d numarali istemcinin baglantisi kesildi.\n", clientSocket);
    close(clientSocket);
    deleteFromNotificationList(clientSocket);
    pthread_exit(NULL);
}

//Kullanıcı offline olunca bildirim listesinden siler.
void deleteFromNotificationList(int clientSocket){
    int i;
    int flag=0;
    for(i=0;i<MAX_CLIENTS;i++){
        if(onlineUsersList[i].clientSocket == clientSocket){
            onlineUsersList[i].phoneNumber[0] = '\0';
            onlineUsersList[i].clientSocket = 0;
            flag = 1;
        }
    }
    if(flag == 1)
        printf("%d numarali istemci bildirim listesinden silindi.\n", clientSocket);
}
//Kullanici online olunca bildirim listesine ekler.
void addToNotificationList(char* buffer,int clientSocket){
    char phone[15];
    int i,flag=0;
    sscanf(buffer, "%s",phone);
    while(flag == 0 && i < MAX_CLIENTS){
        if(onlineUsersList[i].phoneNumber[0] == '\0'){
            strcpy(onlineUsersList[i].phoneNumber,phone);
            onlineUsersList[i].clientSocket = clientSocket;
            flag = 1;
        }
        i++;
    }
    printf("%d numarali istemci bildirim listesine eklendi.\n", clientSocket);
}
//Mesaj yollandigindan kullanici online ise bildirim gonderir.
void sendNotification(char* phone,char* senderPhone,char* message){
    int i;
    for(i=0;i<MAX_CLIENTS;i++){
        if(strcmp(onlineUsersList[i].phoneNumber,phone) == 0){
            char* result = malloc(strlen(senderPhone) + strlen(message) + strlen("kullanicisindan mesaj var") + 20);
            sprintf(result,"%s kullanicisindan mesaj var:\n%s",senderPhone,message);
            send(onlineUsersList[i].clientSocket, result, strlen(result), 0);
            free(result);
        }
    }
}

bool fileCheck(char* fileName){
    FILE *fp;
    fp = fopen(fileName,"r");
    if(fp == NULL){
        return false;
    }
    fclose(fp);
    return true;
}
//Bir sohbetteki mesaj id değerini döndürür.
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
//Burada alıcı,verici.csv veya verici,alıcı.csv dosyasında her sohbet kaydı tutulur.
void handleSendMessage(char* buffer,int clientSocket){
    char fileName[50];
    int lastMessageId;
    messages message;
    ///senderId , receiverId , message , date , status
    sscanf(buffer, "%[^,],%[^,],%[^,],%[^,],%[^\n]",message.senderId,message.receiverId,message.date,message.status,message.message);
    sprintf(fileName, "mesajlar/%s,%s.csv",message.senderId,message.receiverId);
    if(fileCheck(fileName)==false){
        fileName[0] = '\0';
        sprintf(fileName, "mesajlar/%s,%s.csv",message.receiverId,message.senderId);
    }
    FILE *fp;
    fp = fopen(fileName,"a+");
    if(fp == NULL){
        printf("Dosya acilamadi!\n");
        exit(1);
    }
    lastMessageId = getLastMessageId(fp);
    message.messageId = (lastMessageId == -1) ? 1 : lastMessageId + 1;
    fprintf(fp, "%d,%s,%s,%s,%s,%s\n", message.messageId,message.senderId,message.receiverId,message.date, message.status,message.message);
    fclose(fp);
    char* result = malloc(strlen("valid") + 1);
    strcpy(result, "valid");
    send(clientSocket, result, strlen(result), 0);
    free(result);
    //bildirim icin
    sendNotification(message.receiverId,message.senderId,message.message);
}
//Sohbet kayıtlarını kontrol eder. Listelenmesini saglar.
void handleCheckMessage(char* buffer,int clientSocket){
    char phone[15];
    char known[15];
    char unknown[15];
    char result[15];
    sscanf(buffer, "%s",phone);
    DIR *d;
    struct dirent *dir;
    int count=0;
    d = opendir("mesajlar");
    if (d){
        while ((dir = readdir(d)) != NULL){
            if(strstr(dir->d_name,phone) != NULL){
                count++;
                known[0] = '\0';
                unknown[0] = '\0';
                result[0] = '\0';
                sscanf(dir->d_name, "%[^,],%[^.]",known,unknown);
                if(strcmp(phone,known) == 0){
                    strcpy(result, unknown);
                    send(clientSocket, result, strlen(result), 0);
                }else{
                    strcpy(result, known);
                    send(clientSocket, result, strlen(result), 0);
                }
            }
            sleep(0.4);
        }
        closedir(d);
    }
    sleep(0.6);
    sprintf(result, "stop");
    send(clientSocket, result, strlen(result), 0);
}
//Bir sohbetteki mesajları gönderir.
void handleGetMessages(char* buffer,int clientSocket){
    char fileName[50];
    char phone[15];
    char phone2[15];
    sscanf(buffer, "%[^,],%s",phone,phone2);
    sprintf(fileName, "mesajlar/%s,%s.csv",phone,phone2);
    if(fileCheck(fileName)==false){
        fileName[0] = '\0';
        sprintf(fileName, "mesajlar/%s,%s.csv",phone2,phone);
    }
    FILE *fp;
    FILE *fpTemp;
    char tempFileName[50] ="temp.csv";
    fpTemp = fopen(tempFileName,"w");
    fp = fopen(fileName,"a+");
    if(fp == NULL || fpTemp == NULL){
        printf("Dosya acilamadi!\n");
        exit(1);
    }
    char line[300];
    char* result = NULL;
    messages message;
    while(fgets(line, 300, fp) != NULL){
        memset(&message,0,sizeof(message));
        sscanf(line, "%d,%[^,],%[^,],%[^,],%[^,],%[^\n]", &message.messageId,message.senderId,message.receiverId,message.date, message.status,message.message);
        if(strcmp(message.status,"-") == 0 && strcmp(message.receiverId,phone) == 0){
            strcpy(message.status,"+");
            sprintf(line, "%d,%s,%s,%s,%s,%s\n", message.messageId,message.senderId,message.receiverId,message.date, message.status,message.message);
            //dosyada da bu değişikliği yap
        }
        fputs(line,fpTemp);
        result = malloc(strlen(line) + 1);
        strcpy(result, line);
        send(clientSocket, result, strlen(result), 0);
        free(result);
        sleep(0.3);
    }
    fclose(fp);
    fclose(fpTemp);
    remove(fileName);
    rename(tempFileName,fileName);
    result = malloc(strlen("stop") + 1);
    strcpy(result, "stop");
    send(clientSocket, result, strlen(result), 0);
    free(result);
}
//mesaj silme işlemini gerçekleştirir
void handleDeleteMessage(char* buffer, int clientSocket){
    char fileName[50];
    char phone[15];
    char phone2[15];
    char senderPhone[15];
    int flag = 0,messageId;
    sscanf(buffer, "%[^,],%[^,],%d",phone,phone2,&messageId);
    strcpy(senderPhone,phone);
    sprintf(fileName, "mesajlar/%s,%s.csv",phone,phone2);
    if(fileCheck(fileName)==false){
        memset(fileName,0,sizeof(fileName));
        sprintf(fileName, "mesajlar/%s,%s.csv",phone2,phone);
    }
    FILE *fp;
    FILE *fpTemp;
    char tempFileName[50] ="temp.csv";
    fpTemp = fopen(tempFileName,"w");
    fp = fopen(fileName,"a+");
    if(fp == NULL || fpTemp == NULL){
        printf("Dosya acilamadi!\n");
        exit(1);
    }
    char line[300];
    char* result = NULL;
    messages message;
    while(fgets(line, 300, fp) != NULL){
        memset(&message,0,sizeof(message));
        sscanf(line, "%d,%[^,],%[^,],%[^,],%[^,],%[^\n]", &message.messageId,message.senderId,message.receiverId,message.date, message.status,message.message);
        if(message.messageId != messageId || strcmp(senderPhone,message.senderId) != 0){
            fputs(line,fpTemp);
        }else{
            flag = 1;
        }
    }
    fclose(fp);
    fclose(fpTemp);
    remove(fileName);
    rename(tempFileName,fileName);
    if(flag == 0){
        result = malloc(strlen("invalid") + 1);
        strcpy(result, "invalid");
        send(clientSocket, result, strlen(result), 0);
        free(result);
    }else{
        result = malloc(strlen("deleted") + 1);
        strcpy(result, "deleted");
        send(clientSocket, result, strlen(result), 0);
        free(result);
    }
}

void initializeServer(int* server_fd, struct sockaddr_in* address, int* opt) {
    if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, opt, sizeof(*opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);
    if (bind(*server_fd, (struct sockaddr*)address, sizeof(*address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(*server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}
// Giris yapma islemini gerceklestirir.
void handleLogin(char* buffer, int clientSocket){
    users user,userIn;
    int flag = 0;
    FILE *fp;
    char* result;
    fp = fopen("users.csv","r");
    if(fp == NULL){
        printf("Dosya acilamadi!\n");
        exit(1);
    }
    char line[100];
    while(fgets(line, 100, fp) != NULL && flag == 0){
        sscanf(line, "%d,%[^,],%[^,],%[^,],%s", &user.userId, user.phoneNumber, user.password, user.name, user.surname);
        sscanf(buffer, "%[^,],%s", userIn.phoneNumber, userIn.password);
        if(strcmp(user.phoneNumber, userIn.phoneNumber) == 0 && strcmp(user.password, userIn.password) == 0){
            result = malloc(strlen(user.name)+strlen(user.surname)+8);
            sprintf(result,"%s %s %d",user.name,user.surname,user.userId);
            send(clientSocket, result, strlen(result), 0);
            free(result);
            flag = 1;
        }
    }
    fclose(fp);
    if(flag == 0){
        result = malloc(strlen("invalid") + 1);
        strcpy(result, "invalid");
        send(clientSocket, result, strlen(result), 0);
        free(result);
    }
}
// kullanıcı var mı yok mu kontrol eder
bool userCheck(char* phoneNumber){
    users user;
    FILE *fp;
    fp = fopen("users.csv","r");
    if(fp == NULL){
        printf("Dosya acilamadi!\n");
        exit(1);
    }
    char line[100];
    while(fgets(line, 100, fp) != NULL){
        sscanf(line, "%d,%[^,],%[^,],%[^,],%s", &user.userId, user.phoneNumber, user.password, user.name, user.surname);
        if(strcmp(user.phoneNumber, phoneNumber) == 0){
            return true;
        }
    }
    fclose(fp);
    return false;
}
//dosyanın sonundaki son kullanıcı id'sini döndürür
int getLastUserId(FILE *fp){
    users user;
    int lastUserId = -1;
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
        fscanf(fp, "%d,%[^,],%[^,],%[^,],%s", &user.userId, user.phoneNumber, user.password, user.name, user.surname);
        lastUserId = user.userId;
    }
    return lastUserId;
}
//kullanıcı kaydı yapar
void handleRegister(char* buffer,int clientSocket){
    users user;
    FILE *fp;
    sscanf(buffer, "%[^,],%[^,],%[^,],%s", user.phoneNumber, user.password, user.name, user.surname);
    if(userCheck(user.phoneNumber)){
        char* result = malloc(strlen("valid") + 1);
        strcpy(result, "valid");
        send(clientSocket, result, strlen(result), 0);
        free(result);
        return;
    }
    fp = fopen("users.csv","a+");
    if (fp == NULL){
        printf("Dosya acilamadi!\n");
        exit(1);
    }
    int lastUserId = getLastUserId(fp);
    user.userId = (lastUserId == -1) ? 100 : lastUserId + 1;
    fprintf(fp, "%d,%s,%s,%s,%s\n", user.userId, user.phoneNumber, user.password, user.name, user.surname);
    fclose(fp);
    char* result = malloc(strlen("append") + 1);
    strcpy(result, "append");
    send(clientSocket, result, strlen(result), 0);
    free(result);
}
//kullanıcının rehberini listeler
void handleListFriends(char* buffer, int clientSocket){
    char fileName[30];
    users user;
    sscanf(buffer, "%d,%s", &user.userId, user.phoneNumber);
    FILE *fp;
    // filename = rehber + user.phoneNumber + .csv
    sprintf(fileName, "rehber/%s.csv", user.phoneNumber);
    fp = fopen(fileName,"a+");
    if(fp == NULL){
        printf("Dosya acilamadi!\n");
        exit(1);
    }
    char line[100];
    char* result = NULL;
    while(fgets(line, 100, fp) != NULL){
        if(result == NULL){
            result = malloc(strlen(line) + 1);
            strcpy(result, line);
        }else{
            result = realloc(result, strlen(result) + strlen(line) + 1);
            strcat(result, line);
        }
    }
    if(result == NULL){
        result = malloc(strlen("empty") + 1);
        strcpy(result, "empty");
    }
    send(clientSocket, result, strlen(result), 0);
    free(result);
    fclose(fp);
}
//adres listesinde kullanıcı var mı yok mu kontrol eder
bool checkContactList(FILE *fp,char* phoneNumber){
    users user;
    char line[100];
    while(fgets(line, 100, fp) != NULL){
        // phone , name , surname
        sscanf(line, "%[^,],%[^,],%[^,]", user.phoneNumber, user.name, user.surname);
        if(strcmp(user.phoneNumber, phoneNumber) == 0){
            return true;
        }
    }
    return false;
}
//rehbere kullanıcı ekler
void handleAddToList(char* buffer,int clientSocket){
    char fileName[30];
    users user1,user2;
    sscanf(buffer, "%[^,],%[^,],%[^,],%s", user1.phoneNumber,user2.phoneNumber, user2.name,user2.surname);
    FILE *fp;
    // filename = rehber + user.phoneNumber + .csv
    sprintf(fileName, "rehber/%s.csv", user1.phoneNumber);
    fp = fopen(fileName,"a+");
    if(fp == NULL){
        printf("Dosya acilamadi!\n");
        exit(1);
    }
    if(checkContactList(fp,user2.phoneNumber)){
        char* result = malloc(strlen("valid") + 1);
        strcpy(result, "valid");
        send(clientSocket, result, strlen(result), 0);
        free(result);
        return;
    }else{
        fprintf(fp, "%s,%s,%s\n", user2.phoneNumber, user2.name, user2.surname);
        fclose(fp);
        char* result = malloc(strlen("append") + 1);
        strcpy(result, "append");
        send(clientSocket, result, strlen(result), 0);
        free(result);
    }
}
//rehberden kullanıcı siler
void handleDeleteFromList(char* buffer,int clientSocket){
    char fileName[30];
    char phoneNumber[14];
    char phoneNumberD[14];
    char* result;
    char line[100];
    int found;
    sscanf(buffer, "%[^,],%s", phoneNumber, phoneNumberD);
    // filename = rehber + user.phoneNumber + .csv
    sprintf(fileName, "rehber/%s.csv", phoneNumber);
    FILE* inputFp = fopen(fileName, "r");
    FILE* tempFp = fopen("temp.csv", "w");
    if(inputFp == NULL || tempFp == NULL){
        printf("Dosya acilamadi!\n");
        exit(1);
    }
    while(fgets(line,100,inputFp) != NULL){
        if(strstr(line,phoneNumberD) != NULL){
            found = 1;
        
        }else{
            fputs(line,tempFp);
        }
    }
    fclose(inputFp);
    fclose(tempFp);
    if(found){
        remove(fileName);
        rename("temp.csv",fileName);
        char* result = malloc(strlen("deleted") + 1);
        strcpy(result, "deleted");
        send(clientSocket, result, strlen(result), 0);
        free(result);
    }else{
        remove("temp.csv");
        char* result = malloc(strlen("invalid") + 1);
        strcpy(result, "invalid");
        send(clientSocket, result, strlen(result), 0);
        free(result);
    }
}
