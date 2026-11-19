#include <assert.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <unistd.h>

static_assert(MAXPATHLEN == PATH_MAX, "MAXPATHLEN must match PATH_MAX");

static int enabled(void)
{
    int evaluations = 0;
    assert(++evaluations == 1);
    return evaluations;
}

#define NDEBUG
#include <assert.h>

static int disabled(void)
{
    int evaluations = 0;
    assert(++evaluations == 1);
    return evaluations;
}

#undef NDEBUG
#include <assert.h>

int main(void)
{
    if (enabled() != 1 || disabled() != 0) return 1;

    pid_t child = fork();
    if (child < 0) return 2;
    if (child == 0) {
        assert(0 && "standalone assertion smoke");
        _exit(3);
    }

    int status;
    if (waitpid(child, &status, 0) != child) return 4;
    return !(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}
