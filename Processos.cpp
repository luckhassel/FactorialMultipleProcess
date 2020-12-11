#include <stdlib.h>
#include <stdio.h> 
#include <errno.h> 
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h> 
#include <sys/types.h> 
#include <time.h> 
#include <iostream>

// Cria uma struct para passar e armazenar dados dos processos
struct argumentos {   
	int num_inicio, num_fim;  
	long double  resultado;
	int pipefd[2];   
};

int main(int argc , char *argv[] ) 
{ 
    
    //Verifica se a quantidade de argumentos está correta
    if (argc != 3) 
    { 
        fprintf(stderr,"Erro - Entre com o valor do fatorial e processo\n");
        return -EINVAL; 
    }
    
    // Recebe os valores de fatorial e processos da linha de comando
    int fatorial = atoi(argv[1]);          
    int processos = atoi(argv[2]);
    
    // Verifica se o fatorial está dentro do intervalo
    if ( fatorial < 0 || fatorial > 1000000000) 
    { 
        fprintf(stderr, "Numero fatorial deve estar entre 0 e 1000000000\n");
        return -EDOM;
    }  
    
     //Verifica se o numero de processos está dentro do intervalo
    if ( processos < 0 || processos > 1000) 
    { 
        fprintf(stderr, "Numero de processos deve estar entre 1 e 10000\n"); 
        return -EDOM;
    } 
    
    // Calcula a quantidade de números por processo
    int fatorial_por_processo = fatorial/processos; 
    
    printf("Fatorial escolhido: %d \n",fatorial); 
    printf("Quantidade de processos: %d \n\n",processos);
	
    // Criação de um vetor de estrutura argumentos
    argumentos resultados[processos];
    
    int cont = 0;

    //Receberá os valores dos ids de processos
    int pid[processos];

    resultados[0].num_inicio = fatorial;
    resultados[0].num_fim = fatorial - fatorial_por_processo;

    for(int a = 0; a < processos; a++ )
    {
    	if( pipe(resultados[cont].pipefd) < 0) // Verifica erro no pipe
      	{
    		fprintf(stderr, "Nao foi possível criar um pipe entre os processos.\n"); 
        	return -EPIPE;
        }
        
        pid[cont]= fork(); // Cria um processo filho e armazena seu id no vetor
    	
    	if ( pid[cont] < 0 ) 
    	{ 
    		fprintf(stderr, "ERRO - processo filho nao pode ser criado.\n"); 
                return -ECHILD;
    	} 
  
        // Loop do processo filho
    	if ( pid[cont] == 0 )
    	{	 
            // Inicializa os dados da estrutura	
            resultados[cont].resultado = 1;
            if (a!=0)
                resultados[cont].num_inicio=fatorial-a*fatorial_por_processo - a;
                resultados[cont].num_fim= resultados[cont].num_inicio - fatorial_por_processo;
    		
            // Faz o calculo do fatorial daquele processo filho

            if (resultados[cont].num_fim <= 0) resultados[cont].resultado = 1;

            else
            {
                for (int c = resultados[cont].num_inicio; c >= resultados[cont].num_fim; c--)
 			    resultados[cont].resultado = resultados[cont].resultado * c;
            }
    			
	        
	        printf("Processo: %d \n", getpid());
            std::cout << "Resultado parcial --> " << resultados[cont].resultado << std::endl;
	        // printf("Resultado parcial --> %f\n",resultados[cont].resultado);
	        
	        close(resultados[cont].pipefd[0]); // Fecha área de leitura (boa prática)
	        write(resultados[cont].pipefd[1], &resultados[cont].resultado, sizeof(long double)); //Escreve no pipe o valor do resultado do processo
	        close(resultados[cont].pipefd[1]); // Fecha a escrita no pipe
	        exit(0); // Mata o processo filho
        } 
  
        // Processo pai apenas incrementa o contador
    	else
    	{ 		
     	   	cont++;   	
   	} 	
  }
  
  while(wait(NULL) > 0) {} // Aguarda todos os processos filhos terminarem
  
  printf("\n\nID do pai : %d ", getpid());
 
  long double resultados_final [processos]; // Recebe os resultados pelo pipe
  long double fat=1;

  // Realizada a leitura de todos os resultados no pipe e armazenamento no vetor resultado final
  for(int aux = 0; aux < processos; aux++)
  {
   	close(resultados[aux].pipefd[1]);
  	read(resultados[aux].pipefd[0], &resultados_final[aux], sizeof(long double));
  	close(resultados[aux].pipefd[0]);
  }

  // Calcula o fatorial      
  for(int r = 0 ; r < processos; r++){
 	fat = fat*resultados_final[r];
  }
 
  std::cout << "\n\nResultado final --> " << fat << std::endl;
  
  return 0;  
}
