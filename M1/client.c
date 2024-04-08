//para o exercício, não precisa de mutex ou semaforo
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCK_PATH "/tmp/pipeso"

int contagem = 0, itens = 0; //Contagem ate 1500 para a soma do peso, e valor total de itens
float peso_esteira_1 = 5, peso_esteira_2 = 2, peso_esteira_3 = 0.5; //Valores dos pesos de cada item nas esteiras
float peso[1500] = {0}; //Vetor peso para realizar a contagem depois
float total = 0; //Peso total
int stop = 0; //Botao para parada das esteiras

pthread_mutex_t exclusao_mutua = PTHREAD_MUTEX_INITIALIZER;

void *thread_sensor_1(void *param){    //Esteira 1
   while(1){
        if(stop!= 9){   //Se o botao de parada nao estiver sido adicionado
                pthread_mutex_lock(&exclusao_mutua);
                peso[contagem] = peso_esteira_1;
                itens++;      
                contagem++;
                pthread_mutex_unlock(&exclusao_mutua);

                usleep(1000000); //sleep for 1 s
        }
        else{}   //Se o programa estiver parado
    }
}

void *thread_sensor_2(void *param){    //Esteira 2
    while(1){
        if(stop!= 9){
            pthread_mutex_lock(&exclusao_mutua);
            peso[contagem] = peso_esteira_2;
            itens++;
            contagem++;     
            pthread_mutex_unlock(&exclusao_mutua);

            usleep(500000); //sleep for 500 ms
        }
        else{}
    }
}

void *thread_sensor_3(void *param){   //Esteira 3
    while(1){
        if(stop!=9){
            pthread_mutex_lock(&exclusao_mutua);
            peso[contagem] = peso_esteira_3;
            itens++;
            contagem++;      
            pthread_mutex_unlock(&exclusao_mutua);

            usleep(100000); //sleep for 100 ms
        }
        else{}
    }
}

void *contar_peso(void *param){
    while(1){
        if(contagem>=1500){
            pthread_mutex_lock(&exclusao_mutua);
            for(int i=0; i<contagem;i++){
                total += peso[i];
            }
            contagem = 0;
            pthread_mutex_unlock(&exclusao_mutua);
        }
    }
}

void *ler_teclado(void *param){
    while(1){
        scanf("%d", &stop); //Verifica se o botao de parada foi acionado
    }
}

void *display(void *param){  //Funcao de display usando pipe
    int sockfd, len;
    struct sockaddr_un remote;
    char item[1024];

    while(1){
        // Create socket
        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            perror("Falha em criar o socket");
            break;
        }

        // Connect to server
        memset(&remote, 0, sizeof(remote));
        remote.sun_family = AF_UNIX;
        strncpy(remote.sun_path, SOCK_PATH, sizeof(remote.sun_path) - 1);
        len = strlen(remote.sun_path) + sizeof(remote.sun_family);
        if (connect(sockfd, (struct sockaddr *)&remote, len) < 0)
        {
            perror("Falha em conectar no servidor");
            close(sockfd);
            break;
        }

        // Send data to server
        pthread_mutex_lock(&exclusao_mutua);
        sprintf(item, "Total: %d e Peso: %f", itens, total); //contagem de itens e peso total
        pthread_mutex_unlock(&exclusao_mutua);
        if (write(sockfd, item, strlen(item) + 1) < 0)
        {
            perror("Falha em escrever no socket");
            close(sockfd);
            break;
        }

        // Read data from server
        if (read(sockfd, item, sizeof(item)) < 0)
        {
            perror("Falha em ler do socket");
            close(sockfd);
            break;
        }

        // Close socket and exit
        close(sockfd);
        usleep(2000000); //sleep for 2 s
    }
}

void main(){
    //declarar variáveis e tipos para threads
    pthread_t tid_1, tid_2, tid_3, tid_display, tid_total_peso, tid_teclado;
    pthread_attr_t attr;
        
    pthread_attr_init(&attr);

    //criar thread de display com pipe
    pthread_create(&tid_display,&attr,display,NULL);

    //criar thread 1
    pthread_create(&tid_1,&attr,thread_sensor_1,NULL);
    
    //criar thread 2
    pthread_create(&tid_2,&attr,thread_sensor_2,NULL);

    //criar thread 3
    pthread_create(&tid_3,&attr,thread_sensor_3,NULL);

    //criar thread de contagem de peso
    pthread_create(&tid_total_peso,&attr,contar_peso,NULL);

    //criar thread leitura de teclado
    pthread_create(&tid_teclado,&attr,ler_teclado,NULL);

    //join thread
    pthread_join(tid_display, NULL);
    pthread_join(tid_1, NULL);
    pthread_join(tid_2, NULL);
    pthread_join(tid_3, NULL);
    pthread_join(tid_total_peso, NULL);
    pthread_join(tid_teclado, NULL);
}