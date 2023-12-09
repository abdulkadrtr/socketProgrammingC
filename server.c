#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define PORT 8080
#define MAX_CLIENTS 10

typedef struct users{
    int userId; // benzersiz id
    char name[20];
    char surname[20];
    char password[20];
    char phoneNumber[13];// +901231231212 (13 karakter)
} users;

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
    int lastUserId = -1,flag = 0;
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    do {
        fseek(fp, -2, SEEK_CUR); // İki karakter geri gidin (bir karakter \n, diğeri rakam)
        if (ftell(fp) <= 0) {
            fseek(fp, 0, SEEK_SET); // Dosyanın başına gelin, eğer dosyanın başına gelinmişse
            flag = 1;
        }
    } while (fgetc(fp) != '\n' && flag == 0);
    if (ftell(fp) > 0) {
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
    printf("%d\n", user.userId);
    fprintf(fp, "\n%d,%s,%s,%s,%s", user.userId, user.phoneNumber, user.password, user.name, user.surname);
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
        printf("Client %d: %.*s\n", clientSocket, valread, buffer);
        if(strncmp(buffer,"/login",6)==0){
            printf("login\n");
            handleLogin(buffer+7,clientSocket);
        }else if(strncmp(buffer,"/register",9)==0){
            printf("register\n");
            handleRegister(buffer+10,clientSocket);
        }else if(strncmp(buffer,"/listFriends",12)==0){
            printf("listFriends\n");
            handleListFriends(buffer+13,clientSocket);
        }else if(strncmp(buffer,"/addToList",10)==0){
            printf("addToList\n");
            handleAddToList(buffer+11,clientSocket);
        }
    }
    printf("%d numarali istemcinin baglantisi kesildi.\n", clientSocket);
    close(clientSocket);
    pthread_exit(NULL);
}

void sendMessageToClient(int clientSocket) {
    char message[1024];
    printf("Enter message to send to client: ");
    fgets(message, sizeof(message), stdin);
    send(clientSocket, message, strlen(message), 0);
}

void initializeServer(int* server_fd, struct sockaddr_in* address, int* opt);

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