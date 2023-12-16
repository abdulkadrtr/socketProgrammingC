#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#define PORT 8080
#define MAX_MESSAGE_SIZE 1024

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

int createSocket();
int establishConnection(int client_fd, struct sockaddr_in *serv_addr);
void setServerAddress(struct sockaddr_in *serv_addr);
void sendMessage(int client_fd, const char *message);
void receiveMessage(int client_fd, char *buffer);
void userRegister(int client_fd);
void userListFriends(users user,int client_fd);
void userAddToList(users user,int client_fd);
void userDeleteFromList(users user,int client_fd);
void userSendMessage(users user,int client_fd, char *phoneNumberD);
void userCheckMessage(users user,int client_fd);
void* notificationThreadFunc(void* arg);
users userLogin(int client_fd);

int main(int argc, char const* argv[]) {
    int client_fd, status,choice,flagMenu,ret,flag=1;
    struct sockaddr_in serv_addr;
    pthread_t notificationThread;
    client_fd = createSocket();
    setServerAddress(&serv_addr);
    status = establishConnection(client_fd, &serv_addr);
    if (status < 0) {
        printf("Baglanti hatasi!\n");
        exit(-1);
    }
    while (flag) {
        printf("Yapmak istediginiz islemi seciniz:\n1) Giris Yap\n2) Kayit Ol\n3) Cikis\n");
        scanf("%d",&choice);
        switch(choice){
            case 1:
                users user = userLogin(client_fd);
                if(user.userId == -1){
                    printf("Kullanici adi veya sifre hatali!\n");
                }
                else{
                    system("clear");
                    printf("Hosgeldiniz %s %s\n",user.name,user.surname);
                    users* userPtr = malloc(sizeof(users));
                    *userPtr = user;
                    if(pthread_create(&notificationThread, NULL, notificationThreadFunc, userPtr)) {
                        fprintf(stderr, "Error creating thread\n");
                        exit(-1);
                    }
                    flagMenu = 1;
                    while(flagMenu){
                        printf("\n1) Arkadaslarim\n2) Arkadas Ekle\n3) Arkadas Sil\n4) Mesaj Gonder\n5) Sohbetlerim\n6) Cikis\n");
                        scanf("%d",&choice);
                        switch(choice){
                            case 1:
                                userListFriends(user,client_fd);
                                break;
                            case 2:
                                userAddToList(user,client_fd);
                                break;
                            case 3:
                                userListFriends(user,client_fd);
                                userDeleteFromList(user,client_fd);
                                break;
                            case 4:
                                char *phoneNumberD = NULL;
                                userSendMessage(user,client_fd,phoneNumberD);
                                break;
                            case 5:
                                userCheckMessage(user,client_fd);
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
    }
    close(client_fd);
    return 0;
}


void* notificationThreadFunc(void* arg){
    users* user = (users*)arg;
    int client_fd, status;
    struct sockaddr_in serv_addr;
    client_fd = createSocket();
    setServerAddress(&serv_addr);
    status = establishConnection(client_fd, &serv_addr);
    if (status < 0) {
        printf("Baglanti hatasi!\n");
        exit(-1);
    }
    char data[100];
    char buffer[10];
    sprintf(data,"/getNotification,%s",user->phoneNumber);
    sendMessage(client_fd,data);
    while(1){
        receiveMessage(client_fd,buffer);
        printf("Bildirim: %s\n",buffer);
    }
}
// mesaj gonderme islemini gerceklestirir.
void userSendMessage(users user,int client_fd, char *phoneNumberD){
    int flag = 0;
    messages message;
    char data[300];
    char buffer[10];
    time_t currentTime;
    if(phoneNumberD == NULL){
        phoneNumberD = (char*)malloc(sizeof(char)*15);
        userListFriends(user,client_fd);
        printf("Mesaj gondermek istediginiz kisinin telefon numarasini giriniz (+90..) : ");
        scanf("%s",phoneNumberD);
        flag = 1;
    }
    while(getchar() != '\n');
    printf("Mesajinizi giriniz: ");
    scanf("%[^\n]", message.message);
    time(&currentTime);
    struct tm* localTime = localtime(&currentTime);
    strftime(message.date, sizeof(message.date), "%H-%M-%S:%d-%m-%Y", localTime);
    ///sendMessage , senderId , receiverId , message , date , status
    strcpy(message.status,"-");
    sprintf(data,"/sendMessage,%s,%s,%s,%s,%s",user.phoneNumber,phoneNumberD,message.date,message.status,message.message);
    sendMessage(client_fd,data);
    receiveMessage(client_fd,buffer);
    if(strcmp(buffer,"valid")==0){
        printf("Mesaj basariyla gonderildi!\n");
        if(flag == 1){
            free(phoneNumberD);
        }
        return;
    }else{
        printf("HATA!\n");
        return;
    }
}
// Sohbetlerim istegi yapar, gelen mesajlari listeler, mesaj silme islemi yapar
void userCheckMessage(users user,int client_fd){
    char data[60];
    char buffer[300];
    char phone[15];
    int flag = 0,messageId;
    sprintf(data,"/checkMessage,%s",user.phoneNumber);
    sendMessage(client_fd,data);
    receiveMessage(client_fd,buffer);
    printf("Sohbetleriniz:\n");
    while(strcmp(buffer,"stop")!=0){
        flag = 1;
        sscanf(buffer,"%s",phone);
        printf("%s ile sohbet kutunuz var. \n",phone);
        phone[0] = '\0';
        receiveMessage(client_fd,buffer);        
    }
    if(flag == 0){
        printf("Sohbet kutunuz bos!\n");
        return;
    }
    phone[0] = '\0';
    printf("Mesajlarinizi gormek istediginiz kisinin telefon numarasi(+90..): ");
    scanf("%s",phone);
    memset(data,0,sizeof(data));
    sprintf(data,"/getMessages,%s,%s",user.phoneNumber,phone);
    sendMessage(client_fd,data);
    receiveMessage(client_fd,buffer);
    messages message;
    flag = 0;
    while(strcmp(buffer,"stop")!=0){
        flag = 1;
        memset(&message,0,sizeof(message));
        sscanf(buffer, "%d,%[^,],%[^,],%[^,],%[^,],%[^\n]",&message.messageId,message.senderId,message.receiverId,message.date,message.status,message.message);
        if(strcmp(message.senderId,user.phoneNumber)==0){
            printf("%d Ben: %s %s %s\n",message.messageId,message.date,message.status,message.message);
        }else{
            printf("%d O  : %s   %s\n",message.messageId,message.date,message.message);
        }
        receiveMessage(client_fd,buffer);
    }
    if(flag == 0){
        printf("Bu kisiyle henuz hic konusmadiniz!\n");
    }
    printf("Yeni mesaj icin 1'e, mesaj silmek icin 2'ye, cikmak icin 3'e basiniz:");
    int choice;
    scanf("%d",&choice);
    if(choice == 1){
        userSendMessage(user,client_fd,phone);
    }else if(choice == 2){
        printf("Silmek istediginiz mesajin id'sini giriniz: ");
        int messageId;
        scanf("%d",&messageId);
        sprintf(data,"/deleteMessage,%s,%s,%d",user.phoneNumber,phone,messageId);
        sendMessage(client_fd,data);
        receiveMessage(client_fd,buffer);
        if(strcmp(buffer,"invalid")==0){
            printf("%d id degerine sahip mesaj yok veya mesajın gondericisi siz degilsiniz.!\n",messageId);
        }else if(strcmp(buffer,"deleted")==0){
            printf("Mesaj basariyla silindi!\n");
        }else{
            printf("HATA!\n");
        }
    }
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

users userLogin(int client_fd){
    users user;
    char data[100];
    char buffer[10];
    pthread_t notificationThread;
    printf("Telefon numaranizi giriniz (+90.. seklinde):  ");
    scanf("%s",user.phoneNumber);
    //strcpy(user.phoneNumber,"+905515968786");
    printf("Sifrenizi giriniz: ");
    scanf("%s",user.password);
    //strcpy(user.password,"1234");
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
