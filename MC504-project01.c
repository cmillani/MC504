#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

/*
	Contador de thread atual e cada thread tem seu numero -- como thread vai saber seu numero? -- CONTADOR DE RODAAANDO, RODANDO, piao do baú
	A fila basicamente chama numero da thread++
	Cada fila tem que ser adicionada de elementos, e o carrinho tem que saber se esta travado ou nao
	O batman controla a sequencia e sinaliza corrinhos de impasses e os carrinhos sinalizam o outro para passar caso a fila fique muuito grande
*/

//Tentativa de criar queues linked lists;
typedef struct bat_queue
{
	pthread_t node;
	struct bat_queue *next;
} bat_queue;

uint8_t n_size = 0, w_size = 0, s_size = 0, e_size = 0; //Tudo inicia igual a zero, espera condition 

bat_queue north, south, west, east;

//pthread_t n_queue[40], w_queue[40], s_queue4[40], e_queue[40]; //One set of threads for each of the sides of the cross -- talvez melhor implementar uma lista ligada? just call (saul) next
//lista ligada elimina necessidade do side_now :v e o codigo nao tem limite de carrinhows \o/ codigo monxtro
pthread_t batman_thread;
pthread_mutex_t n_insert_mutex = PTHREAD_MUTEX_INITIALIZER, s_insert_mutex = PTHREAD_MUTEX_INITIALIZER, w_insert_mutex = PTHREAD_MUTEX_INITIALIZER, e_insert_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t n_remove_mutex = PTHREAD_MUTEX_INITIALIZER, s_remove_mutex = PTHREAD_MUTEX_INITIALIZER, w_remove_mutex = PTHREAD_MUTEX_INITIALIZER, e_remove_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t n_wait_mutex = PTHREAD_MUTEX_INITIALIZER, s_wait_mutex = PTHREAD_MUTEX_INITIALIZER, w_wait_mutex = PTHREAD_MUTEX_INITIALIZER, e_wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t batman_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t n_wait_first = PTHREAD_COND_INITIALIZER, s_wait_first = PTHREAD_COND_INITIALIZER, w_wait_first = PTHREAD_COND_INITIALIZER, e_wait_first = PTHREAD_COND_INITIALIZER;
pthread_cond_t batman_help_me = PTHREAD_COND_INITIALIZER;

void *north_arrive(void *tid)
{
	int* my_number = (int*)tid;
	printf("BAT %d N chegou no cruzamento\n", *my_number);
	pthread_mutex_lock(&n_insert_mutex);
	n_size++;//Se deixar vetor tem que checar o maximo!
	//insere na fila
	pthread_mutex_unlock(&n_insert_mutex);
	
	pthread_mutex_lock(&n_remove_mutex);
	if (pthread_equal(pthread_self(), north.next->node))//Primeiro dessa fila? se sim ve se pode ir e notifica proximo
	{
		if (west.next == NULL)//verifica direita
		{
			printf("First at north\n");
		}
		else//Tem na direita, espera sinal e avisa batman que ta esperando;
		{
			pthread_mutex_lock(&batman_lock);
			pthread_cond_signal(&batman_help_me);
			pthread_mutex_lock(&batman_lock);
			printf("Better call BATMAN!\n");
		}//lembrar caso que se a fila ficar vazia nunca mais volta.....
		//chama o prox da fila
		pthread_mutex_lock(&n_wait_mutex);
		pthread_cond_signal(&n_wait_first);
		pthread_mutex_unlock(&n_wait_mutex);
	}
	else//Aguarda ser chamado pelo da frente
	{
		pthread_mutex_lock(&n_wait_mutex);
		pthread_cond_wait(&n_wait_first, &n_wait_mutex);
		printf("Called next north\n");
	}
	pthread_mutex_unlock(&n_remove_mutex);
}
void *south_arrive(void *tid)
{
	int* my_number = (int*)tid;
	printf("BAT %d S chegou no cruzamento\n", *my_number);
	pthread_mutex_lock(&s_insert_mutex);
	s_size++;
	//insere na fila
	pthread_mutex_unlock(&s_insert_mutex);

	pthread_mutex_lock(&s_remove_mutex);
	if (pthread_equal(pthread_self(), south.next->node))//Primeiro dessa fila? se sim ve se pode ir e notifica proximo
	{
		if (east.next == NULL)//verifica direita
		{
			printf("First at south\n");
		}
		else//Tem na direita, espera sinal e avisa bat que ta esperando;
		{
			
		}
		//chama o prox da fila
	}
	else//Aguarda ser chamado pelo da frente
	{
		printf("sec at south\n");
	}
	pthread_mutex_unlock(&s_remove_mutex);
}
void *east_arrive(void *tid)
{
	int* my_number = (int*)tid;
	printf("BAT %d E chegou no cruzamento\n", *my_number);
	pthread_mutex_lock(&e_insert_mutex);
	e_size++;
	//insere na fila
	pthread_mutex_unlock(&e_insert_mutex);
	
	pthread_mutex_lock(&e_remove_mutex);
	if (pthread_equal(pthread_self(), east.next->node))//Primeiro dessa fila? se sim ve se pode ir e notifica proximo
	{
		if (north.next == NULL)//verifica direita
		{
			printf("First at East\n");
		}
		else//Tem na direita, espera sinal e avisa bat que ta esperando;
		{
			
		}
		//chama o prox da fila
	}
	else//Aguarda ser chamado pelo da frente
	{
		printf("sec at east\n");
	}
	pthread_mutex_unlock(&e_remove_mutex);
}
void *west_arrive(void *tid)
{
	int* my_number = (int*)tid;
	printf("BAT %d W chegou no cruzamento\n", *my_number);
	pthread_mutex_lock(&w_insert_mutex);
	w_size++;
	//insere na fila
	pthread_mutex_unlock(&w_insert_mutex);
	
	pthread_mutex_lock(&w_remove_mutex);
	if (pthread_equal(pthread_self(), west.next->node))//Primeiro dessa fila? se sim ve se pode ir e notifica proximo
	{
		if (south.next == NULL)//verifica direita
		{
			printf("First at west\n");
		}
		else//Tem na direita, espera sinal e avisa bat que ta esperando;
		{
			
		}
		//chama o prox da fila
	}
	else//Aguarda ser chamado pelo da frente
	{
		printf("sec at west\n");
	}
	pthread_mutex_unlock(&w_remove_mutex);
}

void *batman(void* tid)
{
	int block_count = 0;
	while (block_count < 4)
	{
		pthread_mutex_lock(&batman_lock);
		printf("Wait\n");
		pthread_cond_wait(&batman_help_me, &batman_lock);
		block_count++;
	}
}
//contador de giro--ruim, quando batman deve agir, deve ser fila?
//Posso usar um semaforo ou tem que ser cruzamento mesmo? o batman deve gerenciar tudo ou os proprios bats devem ir andando ate dar condicao de travamento?

int main(int argc, char* argv[])
{
	bat_queue *n_end = &north, *s_end = &south, *w_end = &west, *e_end = &east;
	north.next = NULL;
	if (argc > 1)
	{
		pthread_create(&batman_thread, NULL, batman, 0);
		int i = 0;
		while (argv[1][i] != '\0')
		{
			int *i_copy = malloc(sizeof(int));
			*i_copy = i + 1;
			switch (argv[1][i]) //newsensnennse
			{
				case 'e': {
					e_end->next = malloc(sizeof(bat_queue));
					e_end = e_end->next;
					pthread_create(&(e_end->node), NULL, east_arrive,(void*)(i_copy));
					break;
				}
				case 'w': {
					w_end->next = malloc(sizeof(bat_queue));
					w_end = w_end->next;
					pthread_create(&(w_end->node), NULL, west_arrive,(void*)(i_copy));
					break;
				}
				case 'n': {
					n_end->next = malloc(sizeof(bat_queue));
					n_end = n_end->next;
					pthread_create(&(n_end->node), NULL, north_arrive,(void*)(i_copy));
					break;
				}
				case 's': {
					s_end->next = malloc(sizeof(bat_queue));
					s_end = s_end->next;
					pthread_create(&(s_end->node), NULL, south_arrive,(void*)(i_copy));
					break;
				}
				default : {
					exit(EXIT_FAILURE);
				}
			}
			if (i == 0) //Se foi a primeira BAT a chegar nosso sistema já notifica o BATMAN para comecar a trabalhar.
			{
			}
			i++;
		}
	}
	setbuf(stdin, 0);
	getchar();
	printf("w%d e%d s%d n%d\n",w_size, e_size, s_size, n_size);
	setbuf(stdin, 0);
	getchar();
	return 0;
}
