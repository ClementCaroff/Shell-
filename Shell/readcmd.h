#ifndef __READCMD_H
#define __READCMD_H

/* Lit une ligne de commande depuis l'entrée standard.
 * Renvoie null si l'entrée standard est fermée.
 * Dans le cas où suffisamment de mémoire ne peut être allouée pour le résultat, 
 * affiche un message d'erreur et appelle exit().
 */
struct cmdline *readcmd(void);


/* Structure retournée par readcmd() */
struct cmdline {
	char *err;	/* Si non null : message d'erreur à afficher. 
				 * Dans ce cas, les autres champs sont nuls. */
	char *in;	/* Si non null : nom du fichier vers lequel l'entrée doit être redirigée. */
	char *out;	/* Si non null : nom du fichier vers lequel la sortie doit être redirigée. */
	char *backgrounded; /* Si non null : commande en tâche de fond */ 
	char ***seq;	/* Une ligne de commande est une suite de commandes liées par des tubes
					 * Ainsi, 
					 * - une commande est un tableau de chaînes de caractères (char **), 
					 *   dont le dernier élément est un pointeur null.
					 * - une séquence est un tableau de commandes (char ***)
					 *   dont le dernier élément est un pointeur null.
					 * La struct cmdline renvoyée par readcmd() a au moins un élément 
					 * non nul (seq[0]).
					 */
};
#endif

