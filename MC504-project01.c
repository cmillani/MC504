#include <pthread.h>
#include <stdio.h>
#include <inttypes.h>

/*
	Contador de thread atual e cada thread tem seu numero -- como thread vai saber seu numero? -- CONTADOR DE RODAAANDO, RODANDO, piao do baú
	A fila basicamente chama numero da thread++
	Cada fila tem que ser adicionada de elementos, e o carrinho tem que saber se esta travado ou nao
	O batman controla a sequencia e sinaliza corrinhos de impasses e os carrinhos sinalizam o outro para passar caso a fila fique muuito grande
*/
uint8_t n_size, n_now, w_size, w_now, s_size, s_now, e_size, r_now; //Tudo inicia igual a zero, espera condition 
pthread_t n_queue[40], w_queue[40], s_queue4[40], e_queue[40]; //One set of threads for each of the sides of the cross
pthread_mutex_t n_insert_mutex, s_insert_mutex, w_insert_mutex, l_insert_mutex;

void *batman(void* tid)
{
	
}

int main()
{
	return 0;
}