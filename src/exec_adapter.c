/* Variadic process functions for the standalone profile, after musl 1.2.5
 * src/process: execl, execle, execlp gather their arguments into an argv on
 * the stack and call execve/execv/execvp; vfork behaves as fork (POSIX allows
 * it; musl uses clone(CLONE_VM|CLONE_VFORK) on x86-64). */
#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>

extern char **environ;
int execve(const char *, char *const[], char *const[]);
int execvp(const char *, char *const[]);
pid_t fork(void);

int execl(const char *path, const char *argv0, ...)
{
	int argc;
	va_list ap;
	va_start(ap, argv0);
	for (argc = 1; va_arg(ap, const char *); argc++);
	va_end(ap);
	{
		int i;
		char *argv[argc + 1];
		va_start(ap, argv0);
		argv[0] = (char *)argv0;
		for (i = 1; i < argc; i++) argv[i] = va_arg(ap, char *);
		argv[i] = NULL;
		va_end(ap);
		return execve(path, argv, environ);
	}
}

int execle(const char *path, const char *argv0, ...)
{
	int argc;
	va_list ap;
	va_start(ap, argv0);
	for (argc = 1; va_arg(ap, const char *); argc++);
	va_end(ap);
	{
		int i;
		char *argv[argc + 1];
		char **envp;
		va_start(ap, argv0);
		argv[0] = (char *)argv0;
		for (i = 1; i <= argc; i++) argv[i] = va_arg(ap, char *);
		envp = va_arg(ap, char **);
		va_end(ap);
		return execve(path, argv, envp);
	}
}

int execlp(const char *file, const char *argv0, ...)
{
	int argc;
	va_list ap;
	va_start(ap, argv0);
	for (argc = 1; va_arg(ap, const char *); argc++);
	va_end(ap);
	{
		int i;
		char *argv[argc + 1];
		va_start(ap, argv0);
		argv[0] = (char *)argv0;
		for (i = 1; i < argc; i++) argv[i] = va_arg(ap, char *);
		argv[i] = NULL;
		va_end(ap);
		return execvp(file, argv);
	}
}

pid_t vfork(void)
{
	return fork();
}
