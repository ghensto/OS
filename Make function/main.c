#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "util.h"

/* Flag option 
if n_flag = 1: only display the commands, doesn't execute
if b_flag = 1: do not check timestamps, always recompile */

int b_flag = 0; 
int n_flag = 0;

void show_error_message(char * lpszFileName)
{
	fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", lpszFileName);
	fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	fprintf(stderr, "-n\t\tDon't actually execute commands, just print them.\n");
	fprintf(stderr, "-B\t\tDon't check files timestamps.\n");
	exit(0);
}

/* Taken from Unix Systems Programming, Robbins & Robbins, p72 */
pid_t r_wait(int *stat_loc) {
	int retval; 

	while(((retval = wait(stat_loc)) == -1) && (errno == EINTR)); 
	return retval; 
}
//Check if all the source files in the DAG exist
void check_if_all_files_exist(target_t * const target, target_t *targetsList, int const nTargetCount) {
	for (int i = 0; i < target->nDependencyCount; i++) {
		// Get a dependency and check if it is a target
		int dependencyIndex = find_target(target->szDependencies[i], targetsList, nTargetCount);
		if (dependencyIndex != -1) {	
			target_t dependency = targetsList[dependencyIndex]; 
			int compare = compare_modification_time(target->szTarget, dependency.szTarget);
			check_if_all_files_exist(&dependency, targetsList, nTargetCount);	
		}
		else {
			// not a target, so check if file exists
			// return error if file does not exist (exit())
			int check_exist = does_file_exist(target->szDependencies[i]);
			if (check_exist == -1) {
				printf("%s does not exist\n", target->szDependencies[i]); 
				exit(1); 
			}
		} 
	}
}

/* Builds a target and its dependencies.
 * param target: The target to build. 
 * param targetsList: The list of all targets in the makefile. 
 * param nTargetCount: The number of targets in the makefile. 
 */
void build_target(target_t * const target, target_t *targetsList, int const nTargetCount) {
	// Build each dependency first
	for (int i = 0; i < target->nDependencyCount; i++) {
		// Get a dependency and check if it is a target
		int dependencyIndex = find_target(target->szDependencies[i], targetsList, nTargetCount);
		if (dependencyIndex != -1) {	
			target_t dependency = targetsList[dependencyIndex]; 
			int compare = compare_modification_time(target->szTarget, dependency.szTarget);
			build_target(&dependency, targetsList, nTargetCount);	
		}
	}


	int shouldBuild = 0; 

	// Check if there is a command. If not, skip to the end
	if (strlen(target->szCommand) != 0) {

		// Check for b_flag. If b_flag is on, skip checking timestamp
		if (b_flag) {
			shouldBuild = 1;
		}
		else {
			// For a target that has no dependency, skip checking timestamp
			if (!target->nDependencyCount) {
				shouldBuild = 1;
			}
			else {
				for (int i = 0; i < target->nDependencyCount; i++) {
					int compare = compare_modification_time(target->szTarget, target->szDependencies[i]);
					// Target or dependency wasn't built or doesn't exist
					if (compare == -1) {
						shouldBuild = 1;
						break;
					}
					if (compare == 2) {
						// DependencyTime > TargetTime : out-dated
						// build Target(parent)
				 		shouldBuild = 1;
				 		break;
					}
				}
					
			}
		}

		if (shouldBuild) {
			//check n_flag. If n_flag is on, skip building, only print out command
			if (n_flag) {
				fprintf(stdout,"%s\n",target->szCommand);
			}
			else {
				pid_t childpid = fork(); 
				if (childpid == -1) {
					perror("Failed to fork"); 
					exit(1); 
				}
				if (childpid == 0) {
					execvp(target->prog_args[0], &(target->prog_args[0])); 
					perror("Child failed to execvp the command"); 
					exit(1); 
				}
				else { 
					int wstatus = 0; 
					wait(&wstatus);
					if (WEXITSTATUS(wstatus) != 0) {
						printf("Child exited with error code=%d\n", WEXITSTATUS(wstatus));
						exit(-1); 
					}
				}
			}
		}
	}
}

int main(int argc, char **argv) 
{
	target_t targets[MAX_NODES]; //List of all the targets. Check structure target_t in util.h to understand what each target will contain.
	int nTargetCount = 0;

	// Declarations for getopt
	extern int optind;
	extern char * optarg;
	int ch;
	char * format = "f:hnB";
	
	// Variables you'll want to use
	char szMakefile[64] = "Makefile";
	char szTarget[64] = "";
	int i=0;

	// FLAGS
	// int **** =
	
	//init Targets 
	for(i=0;i<MAX_NODES;i++)
	{
		targets[i].pid=0 ;
		targets[i].nDependencyCount = 0;
		strcpy(targets[i].szTarget, "");
		strcpy(targets[i].szCommand, "");
		targets[i].nStatus = FINISHED;
	}

	while((ch = getopt(argc, argv, format)) != -1) 
	{
		switch(ch) 
		{
			case 'f':
				strcpy(szMakefile, strdup(optarg));
				break;
			case 'n':
				n_flag = 1;
				break;
			case 'B':
				b_flag = 1;
				break;
			case 'h':
			default:
				show_error_message(argv[0]);
				exit(1);
		}
	}

	argc -= optind;
	argv += optind;

	if(argc > 1)
	{
		show_error_message(argv[0]);
		return EXIT_FAILURE;
	}

	/* Parse graph file or die */
	if((nTargetCount = parse(szMakefile, targets)) == -1) 
	{
		return EXIT_FAILURE;
	}

	//Setting Targetname
	//if target is not set, set it to default (first target from makefile)
	if(argc == 1)
	{
		strcpy(szTarget, argv[0]);
	}
	else
	{
		strcpy(szTarget, targets[0].szTarget);
	}

	// Get the first target
	// Check if all files in the DAG exist. If not, terminate program
	// If all files exist, build the first target as well as its dependencies
	int targetIndex = find_target(szTarget, targets, nTargetCount); 
	if (targetIndex != -1) {
		target_t target = targets[targetIndex];
		target_t target_1 = targets[0]; 
		check_if_all_files_exist(&target_1, targets, nTargetCount); 
		build_target(&target, targets, nTargetCount); 
	}
	
	return EXIT_SUCCESS;
}
