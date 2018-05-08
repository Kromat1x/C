#include "main.h"

void run_simulation(int num_lines, int num_cols, int **world, int num_snakes,
	struct snake *snakes, int step_count, char *file_name) 
{
	
	// TODO: Implement Parallel Snake simulation using the default (env. OMP_NUM_THREADS) 
	// number of threads.
	//
	// DO NOT include any I/O stuff here, but make sure that world and snakes
	// parameters are updated as required for the final state.
	int i, j, k, index, lastDir, count = 0;
	struct coord aux;
	struct coord saved;
	
	//Save the tail for each snake
	for (i = 0; i < num_lines; i++) {
		for (j = 0; j < num_cols; j++) {
			if(world[i][j] != 0) {
				
				if(i == 0) {
					if(world[num_lines - 1][j] == world[i][j]) {
						count++;
					}
				} else if(world[i - 1][j] == world[i][j]) {
					count++;
				}
				
				if(i == num_lines - 1) {
					if(world[0][j] == world[i][j]) {
						count++;
					}
				} else if(world[i + 1][j] == world[i][j]) {
					count++;
				}
				
				if(j == num_cols - 1) {
					if(world[i][0] == world[i][j]) {
						count++;
					}
				} else if(world[i][j + 1] == world[i][j]) {
					count++;
				}
				
				if(j == 0) {
					if(world[i][num_cols - 1] == world[i][j]) {
						count++;
					}
				} else if(world[i][j - 1] == world[i][j]) {
					count++;
				}
				
				for(k = 0; k < num_snakes; k++) {
					if(snakes[k].encoding == world[i][j]) {
						index = k;
						break;
					}
				}
				
				if(count == 1 && !(i == snakes[index].head.line && j == snakes[index].head.col)) {
					//Add search for the snake with this tail
					snakes[index].tail.line = i;
					snakes[index].tail.col = j;
				}
				
				count = 0;
			}
		}
	}
	
	for (i = 0; i < step_count; i++) {
		int finish = 0;
		#pragma omp parallel for
		for (j = 0; j < num_snakes; j++) {
			snakes[j].newHead.col = snakes[j].head.col;
			snakes[j].newHead.line = snakes[j].head.line;
			
			if(snakes[j].direction == 'N') {
				
				if(snakes[j].newHead.line== 0) {
					snakes[j].newHead.line = num_lines - 1;
				} else {
					snakes[j].newHead.line--;
				}
				
			} else if(snakes[j].direction == 'S') {
				if(snakes[j].newHead.line == num_lines - 1) {
					snakes[j].newHead.line = 0;
				} else {
					snakes[j].newHead.line++;
				}
				
			} else if(snakes[j].direction == 'E') {
				if(snakes[j].newHead.col == num_cols - 1) {
					snakes[j].newHead.col = 0;
				} else {
					snakes[j].newHead.col++;
				}
				
			} else if(snakes[j].direction == 'V') {
				if(snakes[j].newHead.col == 0) {
					snakes[j].newHead.col = num_cols - 1;
				} else {
					snakes[j].newHead.col--;
				}	
			}
			
			//Get the new tail
			int newTailCol = snakes[j].tail.col;
			int newTailLine = snakes[j].tail.line;
			int newTailFound = 0;
			
			
			if(!newTailFound && snakes[j].tail.line == 0) {
				if(world[num_lines - 1][snakes[j].tail.col] == world[snakes[j].tail.line][snakes[j].tail.col]) {
					newTailLine = num_lines - 1;
					newTailFound = 1;
				}
			}
			
			if(!newTailFound && snakes[j].tail.line == num_lines - 1) {
				if(world[0][snakes[j].tail.col] == world[snakes[j].tail.line][snakes[j].tail.col]) {
					newTailLine = 0;
					newTailFound = 1;
				}
			}
			
			if(!newTailFound && snakes[j].tail.col == 0) {
				if(world[snakes[j].tail.line][num_cols - 1] == world[snakes[j].tail.line][snakes[j].tail.col]) {
					newTailCol = num_cols - 1;
					newTailFound = 1;
				}
			}
			
			if(!newTailFound && snakes[j].tail.col == num_cols - 1) {
				if(world[snakes[j].tail.line][0] == world[snakes[j].tail.line][snakes[j].tail.col]) {
					newTailCol = 0;
					newTailFound = 1;
				}
			}
			if(!newTailFound) {
				if((snakes[j].tail.line > 0) && world[snakes[j].tail.line - 1][snakes[j].tail.col] == world[snakes[j].tail.line][snakes[j].tail.col]) {
					newTailLine--;
				} else if((snakes[j].tail.line < num_lines - 1) && world[snakes[j].tail.line + 1][snakes[j].tail.col] == world[snakes[j].tail.line][snakes[j].tail.col]) {
					newTailLine++;
				} else if((snakes[j].tail.col < num_cols - 1) && world[snakes[j].tail.line][snakes[j].tail.col + 1] == world[snakes[j].tail.line][snakes[j].tail.col]) {
					newTailCol++;
				} else if((snakes[j].tail.col > 0) && world[snakes[j].tail.line][snakes[j].tail.col - 1] == world[snakes[j].tail.line][snakes[j].tail.col]) {
					newTailCol--;
				}
			}
			newTailFound = 0;
			snakes[j].oldTail.col = snakes[j].tail.col;
			snakes[j].oldTail.line = snakes[j].tail.line;
			snakes[j].tail.col = newTailCol;
			snakes[j].tail.line = newTailLine;
		}//snakes for
		
		int snakeCountBeforeCollision = 0;
		for(j = 0; j < num_snakes; j++) {
			//printf("I am %d, my new tail is : (%d %d)\n", snakes[j].encoding, snakes[j].tail.line, snakes[j].tail.col);
			if(world[snakes[j].newHead.line][snakes[j].newHead.col] != 0) {
				//printf("Collision detected between : %d and %d !\n", snakes[j].encoding, world[snakes[j].newHead.line][snakes[j].newHead.col]);
				count = 0;
				for(k = 0; k < num_snakes; k++) {
					if(snakes[k].oldTail.col == snakes[j].newHead.col && snakes[k].oldTail.line == snakes[j].newHead.line) {
						count++;
						break;
					}
				}
				if(!count) {
					finish = 1;
					snakeCountBeforeCollision = j;
					break;
				}
				count = 0;
			} else {
				world[snakes[j].newHead.line][snakes[j].newHead.col] = snakes[j].encoding;
			}
		}	
		if(finish){
			for(j = 0; j < snakeCountBeforeCollision; j++) {
				world[snakes[j].newHead.line][snakes[j].newHead.col] = 0;
			}
			break;
		} else {
			//no collision :
			for(j = 0; j < num_snakes; j++) {
				snakes[j].head.col = snakes[j].newHead.col;
				snakes[j].head.line = snakes[j].newHead.line;
				
				if(world[snakes[j].oldTail.line][snakes[j].oldTail.col] == snakes[j].encoding) {
					world[snakes[j].oldTail.line][snakes[j].oldTail.col] = 0;
				}
			}
		}	
	}
}