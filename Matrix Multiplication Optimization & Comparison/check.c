#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>

int main(int argc, char **argv) {
	int fp1, fp2, ok = 1;
	double value_1, value_2;
	
	fp1 = open(argv[1], O_RDONLY, 0644);
	fp2 = open(argv[2], O_RDONLY, 0644);
	
	
	while (read(fp1, &value_1, sizeof(double)) != 0 && read(fp2, &value_2, sizeof(double)) != 0) {
		//printf("Ref: %f - My Value: %f\n", value_1, value_2);
		if(abs(value_1 - value_2) > 0.001)
			ok = 0;
	}
	
	if(ok == 1){
		printf("SUCCESS!\n");
	} else {
		printf("FAIL!\n");
	}
	return 0;
}
