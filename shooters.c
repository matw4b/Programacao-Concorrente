/*
* Aluno: Matheus Guaraci Lima Bouças Alves
* Matricula: 180046951
* 
* compilacao: gcc -pthreads -ansi -Wall -o shooters shooters.c
* execucao: valgrind --leak-check=full ./shooters
*
* obs: sempre uso o valgrind para verificar erros, como ifs e loops com variaveis nao inicializadas,
* alem de checar vazamento de memoria
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define TRUE 1
#define FALSE 0
#define NUMBER_OF_SHOOTERS 2
#define SUPPLY 60

/* 	supply: indica a quantidade de municoes disponiveis
	score: indica o total de acerto dos atiradores em seus alvos
	
	ambos sao variaveis compartilhadas com seus respectivos locks inicializados abaixo.
	o supply é decrementado de forma randomica, simulando que o atirador esta competindo
	pela municao e carregando na mao o que conseguiu agarrar na hora.

	o score soma os acertos de ambos os atiradores, para criar um "suspense" sobre quem
	esta ganhando e mostrar se eles estao indo bem ou mal (acertando muito, ou errando muito).
*/
int global_supply, global_score;
pthread_mutex_t supply_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t score_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct{ /*struct contendo os pontos e disparos realizados por cada atirador*/
	int fired_shots;
	int score;
}individual_data;

individual_data final_report[NUMBER_OF_SHOOTERS]; /*vetor para exibir o resultado da operação das threads as variaveis globais*/

void* pthread_shooter(void* id_pointer){

	int id = *((int *) id_pointer);
	int i, reload, score, luck, hits; /*score eh a pontuação individual do competidor, enquanto hits eh o numero de acertos naquela rodada de arremecos*/
	int stop_flag = FALSE; /* flag para parar de atirar, se a municao acabar, setamos ela para true */
	score = 0;

	while(!stop_flag){
		
		hits = 0;
		reload =  3 + (rand() % 8); /*determina quanto o atirador pegara dessa vez. Podendo pegar de 3 a 10 municoes com a mao*/
		printf("Atirador %d recarregando.\n", id);
		sleep(3);
		pthread_mutex_lock(&supply_lock);
		if(global_supply>=reload)
			global_supply = global_supply - reload; /*pega as municoes para si.*/
		else if(global_supply < reload && global_supply > 0){ /*poderia carregar mais, porem tinha menos municao disponivel do que a sua capacidade*/
			reload = global_supply;	/*entao ele pega tudo e faz os ultimos arremecos da partida */
			global_supply = 0;
			stop_flag = TRUE;
		}
		else if(global_supply == 0){
			stop_flag = TRUE; /*para o atirador, pois nao tem mais municao disponivel*/
			reload = 0;
		}
		printf("Municao restante: %d\n", global_supply);
		printf("Municao capturada (id: %d): %d\n", id, reload);
		pthread_mutex_unlock(&supply_lock);

		printf("Atirador %d arremecando\n", id);
		sleep(2);
		for(i=reload; i>0; i--){ /*simulacao dos arremecos*/
			luck = 1 + (rand()%10); /*sorte eh uma variavel que determina a chance de acertar o alvo*/
			if(luck>=9){ /*se a sorte do atirador for 9 ou maior, ele acerta o alvo, ou seja, tem 20% de chance de acerto*/
				score++;
				hits++;
			}
		}
		printf("Disparos realizados pelo atirador %d: %d\n", id, reload);
		final_report[id-1].fired_shots += reload; /*guarda quantos disparos foram feitos*/
		pthread_mutex_lock(&score_lock); /*tranca o score global para atualiza-lo apos zerar a propria municao*/
		global_score += hits;
		printf("Score global: %d\n", global_score);
		pthread_mutex_unlock(&score_lock);
	}
	final_report[id-1].score = score;
	free(id_pointer);
	pthread_exit(0);
}

void create_shooters(){
	int i;
	int* id;

	pthread_t shooters[NUMBER_OF_SHOOTERS]; /* vetor de atiradores */

	for(i = 1; i<=NUMBER_OF_SHOOTERS; i++){
		id = (int*) malloc(sizeof(int));
		*id = i;
		pthread_create(&shooters[i], NULL, pthread_shooter, (void*) (id));
	}

	for(i = 1; i<= NUMBER_OF_SHOOTERS; i++){
		pthread_join(shooters[i], NULL);
	}
}

int main(){

	srand(time(NULL)); /*inicializa a semente*/
	int again;
	int i;

	do{
		system("clear");
		global_score = 0;
		global_supply = SUPPLY;
		for(i=0; i<NUMBER_OF_SHOOTERS; i++){ /*inicializa o vetor que sera utilizado para mostrar os resultados finais de cada atirador*/
			final_report[i].fired_shots = 0;
			final_report[i].score = 0;
		}
		printf("Inicio\n");
		create_shooters();

		for(i=0; i<NUMBER_OF_SHOOTERS;i++){
			printf("Pontos do atirador %d: %d\n", i+1, final_report[i].score);
			printf("Disparos realizados pelo atirador %d: %d\n", i+1, final_report[i].fired_shots);
		}

		printf("Deseja executar novamente?\n[0 - nao]\n[1 - sim]\n");
		scanf("%d", &again);
	}while(again == TRUE);

	return 0;
}
