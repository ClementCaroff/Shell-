#define _POSIX_SOURCE
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include "processus.h"
#include "readcmd.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFFER 200


/*Pid du processus courant relatif au shell.*/
int pid_courant = 0;
/*Pid du processus courant réel*/
int pid_courant_reel;
 /*Pour stocker la ligne de commande lancée*/
char *ligne_cmd;
/*Liste des processus*/
List_proc liste_processus;



/*====================================================================*/
/*Traitant pour SIGSTP*/

void traitSTP () {
	kill(pid_courant_reel,SIGSTOP);
	modif_etat(liste_processus,shellpid(pid_courant_reel,liste_processus),SUSPENDU);
	printf("\n");
}
/*====================================================================*/
/*Traitant pour SIGINT*/

void traitINT () {
	
	kill(pid_courant_reel,SIGINT);
	supprimer(shellpid(pid_courant_reel,liste_processus),&liste_processus);
	printf("\n");
}
/*====================================================================*/
/*Initialise les traitants pour les signaux gérés*/
void init_traitant() {
	signal(SIGTSTP,traitSTP);
	signal(SIGINT,traitINT);
}
/*====================================================================*/    
  /* Fonction qui éxecute la commande exit
  * Paramètres : /
  * Préconditions : cmd -> seq[0][0] = 'exit'
  * Postconditions : /
  * Exceptions : /
  */ 
void exec_exit() {
	printf("Au revoir. \n");
	vider(&liste_processus);	 
	exit(0);
	}
/*====================================================================*/ 
  /* Fonction qui éxecute la commande stop
  * Paramètres : -struct cmdline* cmd : la commande
  * Retour : 0 pour indiquer au programme appelant qu'une commande interne
  * a été éxécutée
  * Préconditions : cmd -> seq[0][0] = 'stop'
  * Postconditions : /
  * Exceptions : /
  */ 
int exec_stop(struct cmdline * cmd) {
	int pid_sus = atol(cmd -> seq [0][1]);
	kill(realpid(pid_sus, liste_processus),SIGSTOP);
	modif_etat(liste_processus,pid_sus,SUSPENDU);
	return 0;
	}
	
/*====================================================================*/     
 /* Fonction qui éxecute la commande cd
  * Paramètres : -struct cmdline* cmd : la commande
  * Retour : 0 pour indiquer au programme appelant qu'une commande interne
  * a été éxécutée
  * Préconditions : cmd -> seq[0][0] = 'cd'
  * Postconditions : /
  * Exceptions : /
  */ 
 
 int exec_cd (struct cmdline * cmd) {
	int code_dir;
	/*Test des variantes de cd*/
		if (cmd -> seq[0][1] == NULL) {
			code_dir = chdir("/home");
		}
		else { 
			code_dir=chdir(cmd -> seq[0][1]);
			if (code_dir != 0) {
				printf("Répértoire inconnu \n");
			}
		}
		return 0;
	}	 
/*====================================================================*/

 /* Fonction qui éxecute la commande jobs
  * Paramètres : -struct cmdline* cmd : la commande
  * Retour : 0 pour indiquer au programme appelant qu'une commande interne
  * a été éxécutée
  * Préconditions : cmd -> seq[0][0] = 'jobs'
  * Postconditions : /
  * Exceptions : /
  */ 
  
int exec_jobs () {
	afficher(liste_processus);
	return 0;
}

/*====================================================================*/
 /* Fonction qui éxecute la commande cont
  * Paramètres : -struct cmdline* cmd : la commande
  * Retour : 0 pour indiquer au programme appelant qu'une commande interne
  * a été éxécutée
  * Préconditions : cmd -> seq[0][0] = 'cont'
  * Postconditions : /
  * Exceptions : /
  */ 
int exec_cont (struct cmdline *cmd) {
	int pid_act = atol(cmd -> seq[0][1]);
	kill(realpid(pid_act,liste_processus),SIGCONT);
	modif_etat(liste_processus,pid_act,ACTIF);
	return 0;
}
/*====================================================================*/
/*Fonction qui ouvre un fichier en cas de redirection.*/

void creatFileOfRed (struct cmdline *cmd, int *in, int *out) {
	if (cmd -> in != NULL) {
		if((*in = open(cmd -> in,O_CREAT | O_RDONLY ,00777))==-1) {
			perror("Erreur : ");
			exit(1);
		}
	} else {
		*in =0;
	}
	
	if (cmd -> out != NULL) {
		if((*out = open(cmd -> out,O_CREAT | O_WRONLY | O_TRUNC,00777))==-1) {
			perror("Erreur : ");
			exit(1);
		}
	} else {
		*out = 1;
	}
}
/*====================================================================*/
/*Fonction qui crèent une redirection entre 2 descripteur de fichier*/

void creatRed(int descred, int desc) {
	if (dup2(descred,desc) == -1) {
		perror("Erreur : ");
		exit(1);
		}
	}
/*====================================================================*/	
/*Fonction qui ferme un descripteur de fichier redirigé.*/

void closeRed( int descred) {
	if ( close(descred) == -1) {
		perror("Erreur : ");
		exit(1);
		}
	}
/*====================================================================*/
/*Fonction qui gèrent les redirections d'entrée sortie.*/

void gererRed( struct cmdline *cmd, int in, int out) {
	if (cmd -> in != NULL) {
		creatRed(in,0);
	}
	if (cmd -> out != NULL) {
		creatRed(out,1);
	}
}	
/*====================================================================*/
/*Fonction qui Identifie et éxécute une commande interne
 * Paramètres : struct cmdline * cmd : la commande
 * Retour : 0 ou 1 pour indiquer au programme appelant si une commande interne
 * a été éxecuté ou non.
 * Préconditions : /
 * Postcondition : /
 * Exception : /
 */ 

int exec_cmd_in (struct cmdline* cmd) {
	int code_retour;
	
	/*Filtrage pour reconnaitre la commande interne*/
	char *cmd_char = cmd -> seq[0][0];
	if (strcmp(cmd_char,"exit")==0) {
		code_retour = 0;
		exec_exit();
	} else if (strcmp(cmd_char,"cd")==0) {
		code_retour = exec_cd(cmd);			
	} else if (strcmp(cmd_char,"jobs")==0) {
		code_retour=exec_jobs();
	} else if  (strcmp(cmd_char,"cont")==0) {
		code_retour=exec_cont(cmd);
	}else if  (strcmp(cmd_char,"stop")==0) {
		code_retour=exec_stop(cmd);
	}	else {
		code_retour = 1;
	}
	return code_retour;
}

/*====================================================================*/
/*Fonction qui éxécute une commande non interne
 * Paramètres : struct cmdline * cmd : la commande
 * 			  : int position la position de la commande dans cmd -> seq
 * 			  : int *desc: descripteur de redirection
 * 			  : int * descred : descripteur à rediriger
 * Postcondition : /
 * Exception : /
 */ 
 
/*Executer commande non interne*/
void exec_cmd ( struct cmdline* cmd, int position, int descred, int desc) {
	int status; /*Status du processus*/
			pid_courant++;
			pid_courant_reel = fork();
			
		switch(pid_courant_reel) {
		case 0  :/*Gestion des redirections.*/
				if (cmd -> seq[1] == NULL) {
					/*Cas une seule commande.*/
					gererRed(cmd,descred,desc);
					closeRed(descred);
					closeRed(desc);
				} else {
				creatRed(descred,desc);
			}
				execvp(cmd -> seq[position][0],cmd -> seq[position]);
				exit(1);
		  
		case -1 :printf("Erreur Mémoire \n");
			exit(2);
		  
		default :
		inserer_en_tete(pid_courant,pid_courant_reel,ligne_cmd,ACTIF,&liste_processus);
		/*Test si la commande est en arrière plan.*/
		if (cmd -> backgrounded == NULL)  {
			waitpid(pid_courant_reel,&status,WUNTRACED);
		}
		
		 if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) != 0) {
				printf("Commande incorrecte \n");
				}else{
				printf("Execution réussie \n");
				supprimer(pid_courant,&liste_processus);
				}
			}
			break;
		}
    }


/*====================================================================*/
/*Gestion des pipes.*/
void exec_pipe( struct cmdline *cmd) {
	
	int i; /*La position de la commande à éxécuter.*/
	int outOfPipe; /*L'entrée à définir pour la commande à l'étape suivante.*/
	int p[2]; /*Pipe à définir.*/
	
	int in; /*L'entrée redirigée pour la première commande.*/
	int out; /* La sortie redirigée pour la dernière commande.*/
	
	/*Ouverture des fichiers de redirection en cas de redirections.*/
	creatFileOfRed(cmd,&in,&out);
	
	/*Exécution des commandes.*/
	if (cmd -> seq[1] == NULL) {
		/*Cas : une seule commande.*/
		exec_cmd(cmd,0,in,out);
	} else {
		/*Cas plusieurs commandes.*/
		
	
	i=1;
	outOfPipe = in;
	
	while (cmd->seq[i+1] != NULL) {
		
		if (pipe(p) == -1) {
			perror("Erreur pipe : ");
			exit(1);
		}
		
		exec_cmd(cmd,i,outOfPipe,p[1]);
		closeRed(p[1]);
		outOfPipe = p[0];
		i++;
	}
	
	/*Exécution de la dernière commande des pipes.*/
	exec_cmd(cmd,i,out,outOfPipe);
	closeRed(outOfPipe);
	}
}
	

/*====================================================================*/

	/*Boucle du shell Principale*/
    int main(){
   
   struct cmdline *cmd; /*Pour lire la ligne de commande.*/
   int interne; /*Gerer le code retour d'execution de commande interne*/
   char *rep;
   
   /*Initialisation de la liste de processus*/
   
   liste_processus = nouvelle_liste();
   

   
   /*Allocation de mémoire pour récupérer le répertoire courant*/
   if ((rep = malloc(sizeof(char) * BUFFER)) == NULL) {
	exit(1);
   }
   
   printf("Bienvenue. \n");
    /*Boucle infinie*/
   do {   
	   signal(SIGINT, SIG_IGN);
	   signal(SIGTSTP, SIG_IGN);
	   /*Récupération et affichage du répértoire courant*/
	   rep[0] = '\0';
	   printf("Répértoire courant : %s \n",getcwd(rep,200));
      
      /*Lecture de la commande*/
      printf("Veuillez entrer une commande : ");
      cmd = readcmd();
      if (cmd != NULL && cmd->seq[0] != NULL) {
		  
			/*Initialisation des traitants*/
			init_traitant();
		  
		  /*Stockage de la ligne de commande*/
		  if ((ligne_cmd = malloc(sizeof(char) * BUFFER)) == NULL) {
			  exit(1);
		  }
		  ligne_cmd[0] = '\0';
		  strcat(ligne_cmd,cmd -> seq[0][0]);
		  
		  
		  
		  /*Test si commande interne et éxécution*/
		  interne = exec_cmd_in(cmd);
		  if (interne != 0) {

		  /*Execution de la commande non interne avec gestion des pipes*/
		  exec_pipe(cmd);

	}
		}   
   } while(1);
   /*Libération de mémoire*/
   free(ligne_cmd);
   free(rep);  
}

/*====================================================================*/
	
