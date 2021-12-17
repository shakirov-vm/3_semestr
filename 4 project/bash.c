
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void sigf_handl(int sign) {
	printf("Segfolt\n");
	sleep(10);
	exit(131);
}

int main() {

	printf("pid - %d\n", getpid());

	sigset_t set_sigf;
	sigemptyset(&set_sigf);

	struct sigaction segf;
	segf.sa_handler = sigf_handl;
	segf.sa_mask = set_sigf;
	segf.sa_flags = SA_NOCLDWAIT;
	sigaction(SIGSEGV, &segf, NULL);

	kill(getpid(), SIGSEGV);
}