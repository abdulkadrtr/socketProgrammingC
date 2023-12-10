#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<stdbool.h>

#define PORT 8080
#define MAX_MESSAGE_SIZE 1024

int createSocket();
void setServerAddress(struct sockaddr_in *serv_addr);
int establishConnection(int client_fd, struct sockaddr_in *serv_addr);
void sendMessage(int client_fd, const char *message);
void receiveMessage(int client_fd, char *buffer);


typedef struct users{
    int userId; // benzersiz id
    char name[20];
    char surname[20];
    char password[20];
    char phoneNumber[13];// +901231231212 (13 karakter)
} users;

users userlogin(int client_fd){
    users user;
    char data[100];
    char buffer[10];
    printf("Telefon numaranizi giriniz (+90.. seklinde):  ");
    //scanf("%s",user.phoneNumber);
    strcpy(user.phoneNumber,"+905515968786");
    printf("Sifrenizi giriniz: ");
    //scanf("%s",user.password);
    strcpy(user.password,"1234");
    sprintf(data,"/login,%s,%s",user.phoneNumber,user.password);
    sendMessage(client_fd,data);
    receiveMessage(client_fd,buffer);
    if(strcmp(buffer,"invalid")==0){
        //printf("Kullanici adi veya sifre yanlis!\n");
        user.userId = -1;
    }else{
        sscanf(buffer,"%s %s %d",user.name,user.surname,&user.userId);
    }
    return user;
}

void userRegister(int client_fd){
    users user;
    char data[100];
    char buffer[10];
    printf("Adiniz: ");
    scanf("%s",user.name);
    printf("Soyadiniz: ");
    scanf("%s",user.surname);
    printf("Telefon numaranizi giriniz (+90.. seklinde):  ");
    scanf("%s",user.phoneNumber);
    printf("Sifrenizi giriniz: ");
    scanf("%s",user.password);
    sprintf(data,"/register,%s,%s,%s,%s",user.phoneNumber,user.password,user.name,user.surname);
    sendMessage(client_fd,data);
    receiveMessage(client_fd,buffer);
    if(strcmp(buffer,"valid")==0){
        printf("Bu numara zaten kayitli!\n");
        return;
    }else if(strcmp(buffer,"append")==0){
        printf("Kayit basariyla gerceklestirildi!\n");
        return;
    }else{
        printf("HATA!\n");
        return;
    }
}

void userListFriends(users user,int client_fd){
    char data[100];
    char buffer[10];
    sprintf(data,"/listFriends,%d,%s",user.userId,user.phoneNumber);
    sendMessage(client_fd,data);
    receiveMessage(client_fd,buffer);
    if(strcmp(buffer,"empty")==0){
        printf("Rehberinde kimse yok. Yapayalnizsin!\n");
        return;
    }else{
        printf("Telefon Numarasi\tAd\tSoyad\n");
        printf("%s",buffer);
        return;
    }
}

void userAddToList(users user,int client_fd){
    char data[100];
    char buffer[10];
    users userIn;
    printf("Eklemek istediginiz kisinin telefon numarasini giriniz (+90..) : ");
    scanf("%s",userIn.phoneNumber);
    printf("Eklemek istediginiz kisinin adini giriniz: ");
    scanf("%s",userIn.name);
    printf("Eklemek istediginiz kisinin soyadini giriniz: ");
    scanf("%s",userIn.surname);
    sprintf(data,"/addToList,%s,%s,%s,%s",user.phoneNumber,userIn.phoneNumber,userIn.name,userIn.surname);
    sendMessage(client_fd,data);
    receiveMessage(client_fd,buffer);
    if(strcmp(buffer,"valid")==0){
        printf("Bu numara zaten kayitli!\n");
        return;
    }else if(strcmp(buffer,"append")==0){
        printf("Kayit basariyla gerceklestirildi!\n");
        return;
    }else{
        printf("HATA!\n");
        return;
    }
}

void userDeleteFromList(users user,int client_fd){
    char data[100];
    char buffer[10];
    char phoneNumberD[15];
    printf("Silmek istediginiz kisinin telefon numarasini giriniz (+90..) : ");
    scanf("%s",phoneNumberD);
    sprintf(data,"/deleteFromList,%s,%s",user.phoneNumber,phoneNumberD);
    sendMessage(client_fd,data);
    receiveMessage(client_fd,buffer);
    if(strcmp(buffer,"invalid")==0){
        printf("Bu numara zaten kayitli degil!\n");
        return;
    }else if(strcmp(buffer,"deleted")==0){
        printf("Kayit basariyla silindi!\n");
        return;
    }else{
        printf("HATA!\n");
        return;
    }
}

int main(int argc, char const* argv[]) {
    int client_fd, status,choice,flagMenu;
    struct sockaddr_in serv_addr;
    char buffer[MAX_MESSAGE_SIZE] = { 0 }, response[20];
	int flag = 1;
    client_fd = createSocket();
    setServerAddress(&serv_addr);
    status = establishConnection(client_fd, &serv_addr);
    printf("Giris Yapiniz\n");
    while (flag) {
        printf("Yapmak istediginiz islemi seciniz:\n1) Giris Yap\n2) Kayit Ol\n3) Cikis\n");
        scanf("%d",&choice);
        switch(choice){
            case 1:
                users user = login(client_fd);
                if(user.userId == -1){
                    printf("Kullanici adi veya sifre hatali!\n");
                }
                else{
                    system("clear");
                    printf("Hosgeldiniz %s %s\n",user.name,user.surname);
                    flagMenu = 1;
                    while(flagMenu){
                        printf("\n1) Arkadaslarim\n2) Arkadas Ekle\n3) Arkadas Sil\n4) Mesaj Gonder\n5) Mesajlari Al\n6) Cikis\n");
                        scanf("%d",&choice);
                        switch(choice){
                            case 1:
                                userListFriends(user,client_fd);
                                break;
                            case 2:
                                userAddToList(user,client_fd);
                                break;
                            case 3:
                                userDeleteFromList(user,client_fd);
                                break;
                            case 4:
                                break;
                            case 5:
                                break;
                            case 6: 
                                flagMenu = 0;
                                break;
                            default:
                                printf("Gecersiz islem!\n");
                                break;
                        }
                    }

                }
                break;
            case 2:
                userRegister(client_fd);
                break;
            case 3:
                flag = 0;
                break;
            default:
                printf("Gecersiz islem!\n");
                break;
        }
        /*
        printf("Gondermek istedigin mesaj: (cikis icin 'exit'): ");
        fgets(buffer, MAX_MESSAGE_SIZE, stdin);
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        if (strcmp(buffer, "exit") == 0) {
            flag = 0;
        }
        sendMessage(client_fd, buffer);
        */
        //receiveMessage(client_fd, buffer);
    }
    close(client_fd);
    return 0;
}

int createSocket() {
    int client_fd;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    return client_fd;
}

void setServerAddress(struct sockaddr_in *serv_addr) {
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr->sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(-1);
    }
}

int establishConnection(int client_fd, struct sockaddr_in *serv_addr) {
    int status;
    if ((status = connect(client_fd, (struct sockaddr*)serv_addr, sizeof(*serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        exit(-1);
    }
    return status;
}

void sendMessage(int client_fd, const char *message) {
    send(client_fd, message, strlen(message), 0);
}

void receiveMessage(int client_fd, char *buffer) {
    int valread = read(client_fd, buffer, MAX_MESSAGE_SIZE - 1);
    buffer[valread] = '\0';
    //printf("Gelen Mesaj: %s\n", buffer);
}