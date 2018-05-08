/**
 * Operating Systems 2013-2017 - Assignment 2
 *
 * Bînă Marius Andrei 336CC
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "cmd.h"
#include "utils.h"

#define READ		0
#define WRITE		1

/**
 * Functie de redirectare din laborator la care s-a adaugat parametrul
 * de io_flags pentru a cunoaste modul de deschidere al fisierelor pentru
 * redirectare
 */

static void do_redirect(int filedes, const char *filename, int io_flags)
{
	int ret;
	int fd;
	int flags_app = O_WRONLY | O_CREAT | O_APPEND;
	int flags_tr = O_WRONLY | O_CREAT | O_TRUNC;

	if (filedes == STDIN_FILENO) {
		fd = open(filename, O_RDONLY, 0644);
	} else if (filedes == STDOUT_FILENO) {
		if (io_flags & IO_OUT_APPEND) {
			fd = open(filename, flags_app, 0644);
			DIE(fd < 0, "open");
		} else {
			fd = open(filename, flags_tr, 0644);
			DIE(fd < 0, "open");
		}
	} else if (filedes == STDERR_FILENO) {
		if (io_flags & IO_ERR_APPEND) {
			fd = open(filename, flags_app, 0644);
			DIE(fd < 0, "open");
		} else {
			fd = open(filename, flags_tr, 0644);
			DIE(fd < 0, "open");
		}
	}
	ret = dup2(fd, filedes);
	DIE(ret < 0, "dup2");
	close(fd);
}

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	return chdir(get_word(dir));
}

/**
 * Internal exit/quit command.
 */
static int shell_exit(void)
{
	return SHELL_EXIT;
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	int status, arg_size, wait_ret;
	pid_t kiddo;
	char **args;
	int saved_in, saved_out, saved_err;
	int ap = IO_OUT_APPEND;

	/*Sanity checks*/
	DIE(s->up != father, "Wrong daddy");
	DIE(s == NULL, "Null simple command");
	if (strcmp(get_word(s->verb), "exit") == 0)
		return shell_exit();
	else if (strcmp(get_word(s->verb), "quit") == 0)
		return shell_exit();
	if (strcmp(get_word(s->verb), "cd") == 0) {
		/*Se salveaza descriptorii initiali*/
		saved_in = dup(STDIN_FILENO);
		saved_out = dup(STDOUT_FILENO);
		saved_err = dup(STDERR_FILENO);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		/**
		 * Se efectueaza redirectari unde este cazul,
		 * adica daca s->in diferit de NULL atunci exista fisier
		 * de unde trebuie luat inputul si astfel STDIN trebuie
		 * redirectat, pentru asta se foloseste functia definita mai
		 * sus
		 */
		if (s->in != NULL)
			do_redirect(STDIN_FILENO, get_word(s->in), IO_REGULAR);

		if (s->out != NULL && s->err != NULL)
			if (strcmp(get_word(s->out), get_word(s->err)) == 0) {
				char *s_err = get_word(s->err);
				char *s_out = get_word(s->out);

				do_redirect(STDERR_FILENO, s_err, s->io_flags);
				do_redirect(STDOUT_FILENO, s_out, ap);
				free(s_err);
				free(s_out);
			} else {
				char *s_err = get_word(s->err);
				char *s_out = get_word(s->out);

				do_redirect(STDOUT_FILENO, s_err, s->io_flags);
				do_redirect(STDERR_FILENO, s_out, s->io_flags);
				free(s_err);
				free(s_out);
			}
		else
			if (s->out != NULL) {
				char *aux = get_word(s->out);

				do_redirect(STDOUT_FILENO, aux, s->io_flags);
				free(aux);
			} else if (s->err != NULL) {
				char *aux = get_word(s->out);

				do_redirect(STDERR_FILENO, aux, s->io_flags);
			}
		dup2(saved_in, STDIN_FILENO);
		dup2(saved_out, STDOUT_FILENO);
		dup2(saved_err, STDERR_FILENO);
		close(saved_in);
		close(saved_out);
		close(saved_err);
		/*Se apeleaza shell_cd cu calea dorita*/
		return shell_cd(s->params);
	}

	/**
	 * Daca next_part nu e NULL si e "=" atunci avem
	 * modificare sau adaugare de variabila de mediu
	 */
	if (s->verb->next_part != NULL) {
		if (strcmp(s->verb->next_part->string, "=") == 0) {
			if (s->verb->next_part->next_part == NULL) {
				fprintf(stderr, "invalid");
				exit(EXIT_FAILURE);
			} else {
				word_t *aux = s->verb;
				word_t *val = aux->next_part->next_part;

				/**
				 * Se apeleaza setenv folosind neaparat get_word
				 * deoarece la atribuirea unei valori pentru
				 * o variabila
				 * de mediu, chiar si valoarea respectiva poate
				 * contine o variabila de mediu care trebuie
				 * expandata.
				 * Acest expand se face in interiorul get_word
				 */
				return setenv(aux->string, get_word(val), 1);
			}
		}
	}

	kiddo = fork();

	switch (kiddo) {
	case 0:
	{
		/*Aceleasi redirectari ca la cd, daca este cazul*/
		if (s->in != NULL)
			do_redirect(STDIN_FILENO, get_word(s->in), IO_REGULAR);

		if (s->out != NULL && s->err != NULL)
			if (strcmp(get_word(s->out), get_word(s->err)) == 0) {
				char *s_err = get_word(s->err);
				char *s_out = get_word(s->out);

				do_redirect(STDERR_FILENO, s_err, s->io_flags);
				do_redirect(STDOUT_FILENO, s_out, ap);
				free(s_err);
				free(s_out);
			} else {
				char *s_err = get_word(s->err);
				char *s_out = get_word(s->out);

				do_redirect(STDOUT_FILENO, s_out, s->io_flags);
				do_redirect(STDERR_FILENO, s_err, s->io_flags);
				free(s_err);
				free(s_out);
			}
		else
			if (s->out != NULL) {
				char *s_out = get_word(s->out);

				do_redirect(STDOUT_FILENO, s_out, s->io_flags);
				free(s_out);
			} else if (s->err != NULL) {
				char *s_err = get_word(s->err);

				do_redirect(STDERR_FILENO, s_err, s->io_flags);
				free(s_err);
			}
		/**
		 * Se iau argumentele pentru executarea comenzii si se folosesc
		 * in execvp, care daca da -1 se afiseaza mesaj de eroare
		 */
		args = get_argv(s, &arg_size);
		if (execvp(get_word(s->verb), args) == -1) {
			char *x = get_word(s->verb);

			fprintf(stderr, "Execution failed for '%s'\n", x);
		}
		exit(1);
	}
	case -1:
		DIE(1, "Fork in parse_simple");
		break;
	default:
		/**
		 * Procesul parinte asteapta terminarea
		 * executiei procesului copil
		 */
		wait_ret = waitpid(kiddo, &status, 0);
		DIE(wait_ret < 0, "Waitpid in parse_simple");
	}


	if (!WIFEXITED(status))
		exit(EXIT_FAILURE);
	else
		return WEXITSTATUS(status);
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool do_in_parallel(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	pid_t kiddo1, kiddo2;
	int status, wait_ret;

	kiddo1 = fork();

	/**
	 * Executia in paralel a doua comenzi se face folosind doua procese copil
	 * ale aceluiasi parinte, acestea executa cele doua comenzi in paralel
	 */
	switch (kiddo1) {
	case 0:
		//Baietul 1
		exit(parse_command(cmd1, level+1, father));
		break;
	case -1:
		DIE(1, "Do_in_parallel - first fork");
		break;
	default:
		//El padre
		kiddo2 = fork();
		switch (kiddo2) {
		case 0:
			//Baietul 2
			exit(parse_command(cmd2, level+1, father));
			break;
		case -1:
			//Ceva grav 2
			DIE(1, "Do_in_parallel - second fork");
			break;
		default:
			//El padre 2
			waitpid(kiddo2, &status, 0);
			break;
		}
		wait_ret = waitpid(kiddo1, &status, 0);
		DIE(wait_ret < 0, "Waitpid in parallel");
		if (WIFEXITED(status))
			return WEXITSTATUS(status);
		break;
	}

	return WEXITSTATUS(status);
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2)
 */
static bool do_on_pipe(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	int status, my_pipe, rc, wait_ret;
	int pipe_fds[2];
	pid_t kiddo1, kiddo2;

	/**
	 * Asemenator cu executia in paralel, in pipe se
	 * folosesc tot doua procese copil
	 * acestea comunica printr-un pipe a carui
	 * functionalitate a fost descrisa mai
	 * detaliat in README
	 */
	my_pipe = pipe(pipe_fds);
	if (my_pipe == -1)
		DIE(1, "open_pipe");
	kiddo1 = fork();

	switch (kiddo1) {
	case 0:
	{
		//Baietul 1
		if (close(pipe_fds[0]) == -1)
			DIE(1, "close pipe1");
		rc = dup2(pipe_fds[1], STDOUT_FILENO);
		DIE(rc < 0, "dup2 stdout pipe");
		if (close(pipe_fds[1]) == -1)
			DIE(1, "close pipe2");
		exit(parse_command(cmd1, level+1, father));
		break;
	}
	case -1:
	{
		//Ceva grav
		DIE(1, "Do_on_pipe - first fork");
		break;
	}
	default:
		break;
	}

	kiddo2 = fork();

	switch (kiddo2) {
	case 0:
	{
		if (close(pipe_fds[1]) == -1)
			DIE(1, "close pipe3");
		rc = dup2(pipe_fds[0], STDIN_FILENO);
		DIE(rc < 0, "dup2 stdin pipe");
		if (close(pipe_fds[0]) == -1)
			DIE(1, "close pipe4");
		exit(parse_command(cmd2, level+1, father));
	}
	case -1:
		DIE(1, "Do_on_pipe - second fork");

	default:
		break;
	}

	/**
	 * Parintele inchide ambele capete de pipe
	 * si asteapta terminarea copiilor
	 */
	if (close(pipe_fds[0]) == -1)
		DIE(1, "close pipe5");
	if (close(pipe_fds[1]) == -1)
		DIE(1, "close pipe6");
	wait_ret = waitpid(kiddo1, &status, 0);
			DIE(wait_ret < 0, "Waitpid in pipe");
	wait_ret = waitpid(kiddo2, &status, 0);
			DIE(wait_ret < 0, "Waitpid in pipe");

	if (!WIFEXITED(status))
		exit(EXIT_FAILURE);
	else
		return WEXITSTATUS(status);
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	int ret;

	DIE(c->up != father, "Parse command wrong father");
	DIE(c == NULL, "parse command null");
	/*Executia unei comenzi simple*/
	if (c->op == OP_NONE)
		return parse_simple(c->scmd, level+1, c);

	switch (c->op) {
	case OP_SEQUENTIAL:
		/*Executia a doua comenzi una dupa cealalta*/
		parse_command(c->cmd1, level+1, c);
		ret = parse_command(c->cmd2, level+1, c);
		return ret;

	case OP_PARALLEL:
		/*Executia a doua comenzi in paralel*/
		return do_in_parallel(c->cmd1, c->cmd2, level+1, c);

	case OP_CONDITIONAL_NZERO:
		/**
		 * Executie conditionata pentru a doua comanda,
		 * se executa doar in cazul in care prima comanda
		 * nu returneaza 0
		 */
		ret = parse_command(c->cmd1, level+1, c);
		if (ret != 0)
			return parse_command(c->cmd2, level+1, c);
		break;

	case OP_CONDITIONAL_ZERO:
		/**
		 * Executie conditionata pentru a doua comanda,
		 * se executa doar in cazul in care prima comanda
		 * returneaza 0
		 */
		ret = parse_command(c->cmd1, level+1, c);
		if (ret == 0)
			return parse_command(c->cmd2, level+1, c);
		break;

	case OP_PIPE:
		/**
		 * Executie in pipe, outputul primei comenzi
		 * devine input pentru a doua comanda
		 */
		return do_on_pipe(c->cmd1, c->cmd2, level+1, c);

	default:
		return SHELL_EXIT;
	}

	return 0;
}
