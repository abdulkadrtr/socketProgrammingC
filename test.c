#include <stdio.h>
#include <stdlib.h>

typedef struct users{
    int userId; // benzersiz id
    char name[20];
    char surname[20];
    char password[20];
    char phoneNumber[13];// +901231231212 (13 karakter)
} users;


int main(){
    FILE *fp;
    users user;
    fp = fopen("rehber/+905515968786.csv","a+");
    if(fp == NULL){
        printf("Dosya acilamadi!\n");
        exit(1);
    }
    char line[100];
    while(fgets(line, 100, fp) != NULL){
        printf("%s",line);
        //sscanf(line, "%d,%[^,],%[^,],%[^,],%s", &user.userId, user.phoneNumber, user.password, user.name, user.surname);
        //printf("%d %s %s %s %s\n",user.userId,user.phoneNumber,user.password,user.name,user.surname);
    }
    fclose(fp);
    return 0;
}