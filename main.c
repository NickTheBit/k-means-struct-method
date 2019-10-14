#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define BUFFER_SIZE 10

struct clusters {
	int space;
	int elementNum;
	double *sum;
	double *Centroid;
	double *exCentroid;
	int *elements;
};

int main(int argc,char **argv) {
	int i,j,d;				// Standart loop iterators
    FILE *f;				// Primary file pointer
	char filepath[100];		// File path 
	char temp;				// Temporary var for chars
	double tempDbl;			// Temporary var for doubles
	double minDistance;		// Used for cluster allocation
	int clusterNum;			// Amount of clusters
	int dimentions = 1;		// Amount of dimentions
	int elementCounter = 0;	// Amount of elements
	int finished = 0;		// Variable for termination
	int minCluster;			// Temporary storage of the closest cluster
	int iterCounter = 0;	// Iterations counter
	clock_t recs[2];		// Vars for timing

	// File retrieval
	if (argc == 3) {
		f = fopen(argv[1],"r");
		clusterNum = atoi(argv[2]);
	} else {
		printf("Insert filepath: ");
		scanf("%s",filepath);
		f = fopen(filepath,"r");
		printf("Insert amount of clusters: ");
		scanf("%d",&clusterNum);
	}
	if (!f) {
		printf("File invalid/n");
		return 0;
	}

	// Reading dataset
	fscanf(f,"%c",&temp);
	while (temp != '\n') {
		if ((temp == '\n')||(temp == '\t')||(temp == ' ')) {
			dimentions ++;
		}
		fscanf(f,"%c",&temp);
	}
	rewind(f);
	fscanf(f,"%c",&temp);
	while (!feof(f)) {
		if (temp == '\n')
			elementCounter++;
		fscanf(f,"%c",&temp);
	}
	rewind(f);

	printf("Detected:\n\tElements: %d\n\tDimentions: %d\n",elementCounter,dimentions);

	// Starting timing
	recs[0] = clock();
	
	// Allocating space
	double **elements = malloc(elementCounter * sizeof(double));
	for (i=0; i<elementCounter; i++)
		elements[i] = malloc(dimentions * sizeof(double));

	// Inserting data in RAM
	for (i = 0; i < elementCounter; i++) {
		for (j=0; j<dimentions; j++)
			fscanf(f,"%lf",&elements[i][j]);
	}

	// Creating and initialising cluster vars
	struct clusters clu[clusterNum];
	for (i=0; i<clusterNum; i++) {
		clu[i].space = BUFFER_SIZE;
		clu[i].elementNum = 0;
		clu[i].sum = calloc(dimentions,sizeof(double));
		clu[i].elements = malloc(BUFFER_SIZE * sizeof(int));
	}

	// Generating initial centroids and priming exCentroid for termination check
	for (i=0; i<clusterNum; i++) {
		clu[i].Centroid = calloc(dimentions,sizeof(double));
		clu[i].exCentroid = calloc(dimentions,sizeof(double));
		for (j=0; j<dimentions; j++)
			clu[i].Centroid[j] = elements[rand() % elementCounter][j];
	}

	// Starting main loop ( Parralel part should start here)
	do {
		// Preparing cluster vars (also resets them after 1st iteration)
		for (i=0; i<clusterNum; i++) {
			clu[i].elementNum = 0;
			for (d=0; d<dimentions; d++)
				clu[i].sum[d] = 0;
		}

		for (i = 0; i < elementCounter; i++) {
			tempDbl = 0;
			minDistance = 99999999;
			for (j = 0; j < clusterNum; j++) {
				for (d = 0; d < dimentions; d++) {
					tempDbl += pow(elements[i][d] - clu[j].Centroid[d],2);
				}
				tempDbl = sqrt(tempDbl);
				if (tempDbl <= minDistance) {
					minDistance = tempDbl;
					minCluster = j;
				}
			}
			// If the cluster doesnt have enouph space we are allocating some
			if (clu[minCluster].space < clu[minCluster].elementNum) {
				clu[minCluster].elements = calloc(BUFFER_SIZE,sizeof(int));
				clu[minCluster].space += BUFFER_SIZE;
			}

			// Adding the new element and increasing the counter
			clu[minCluster].elements[clu[minCluster].elementNum ++] = i;
			// also adding to the total sum of said cluster
			for (d=0; d<dimentions; d++)
				clu[minCluster].sum[d] += elements[i][d];
		}

		// Determining new centroids
		for (i=0; i<clusterNum; i++) {
			for (d=0; d<dimentions; d++) {
				clu[i].Centroid[d] = clu[i].sum[d] / clu[i].elementNum;
			}
		}

		iterCounter ++;

		// Checking for termination (comparing ex centroids with current ones)
		finished = 1;
		for (i=0; i<clusterNum; i++)
			for (d=0; d<dimentions; d++) {
				if (clu[i].Centroid[d] != clu[i].exCentroid[d])
					finished = 0;
				clu[i].exCentroid[d] = clu[i].Centroid[d];
			}

	} while (finished == 0);

	// Ending and printing time
	recs[1] = clock();
	printf("\nTime of execution: %f secs\n",(double)(recs[1]-recs[0])/CLOCKS_PER_SEC);
	printf("Iterations: %d\n",iterCounter);

	// Closing primary dataset
	fclose(f);

	// Writing results to file
	for (i=0; i<clusterNum; i++) {
		sprintf(filepath,"cluster%d.csv",i);
		f = fopen(filepath,"w");
		for (j=0; j<clu[i].elementNum; j++) {
			for (d=0; d<dimentions; d++)
				fprintf(f,"%lf ",elements[clu[i].elements[j]][d]);
			fprintf(f,"\n");
		}
		fclose(f);
	}
	return 0;
}