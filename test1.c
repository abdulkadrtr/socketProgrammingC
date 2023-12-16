#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Struct tanımlaması
typedef struct {
    int id;
    char name[30];
} MyStruct;

// Thread fonksiyonu
void* print_struct(void* arg) {
    MyStruct* myStruct = (MyStruct*) arg;
    printf("ID: %d\n", myStruct->id);
    printf("Name: %s\n", myStruct->name);
    return NULL;
}

int main() {
    // Struct oluşturma
    MyStruct myStruct;
    myStruct.id = 1;
    sprintf(myStruct.name, "MyStruct");

    // Thread oluşturma
    pthread_t thread;
    if(pthread_create(&thread, NULL, print_struct, &myStruct)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    // Thread'in bitmesini bekleme
    if(pthread_join(thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }

    return 0;
}
