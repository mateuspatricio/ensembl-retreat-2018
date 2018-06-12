# include <mpi.h>
# include <stdlib.h> 
# include <iostream>
# include <unistd.h>

//# include <string>
//# include <fstream>

#define  SERVER_NODE     0

//data to be worked by the script
//float  data[ARRAYSIZE];

using namespace std;

//Prototypes
int main ( int argc, char *argv[] );
float make_operation(int myoffset, int chunk, int myid, float data[]);
float make_operation_linear(int myoffset, int chunk, int myid, float data[]);

int main ( int argc, char *argv[] ) {

    //int array_size = 100;
    int array_size;
    array_size = atoi(argv[1]);

    float* data;
    data = new float[array_size];

    MPI_Status status;
    
    int numprocs, my_id, chunksize, tag1, tag2, offset, source;
    //time_t start, end;
    float sum, mysum;

    //Initialize MPI.
    MPI_Init ( &argc, &argv );

    //Get the number of processes.
    MPI_Comm_size ( MPI_COMM_WORLD, &numprocs );

    //Determine the rank of this process.
    MPI_Comm_rank ( MPI_COMM_WORLD, &my_id );

    if (array_size % numprocs != 0) {
        cout << "Quitting. Array size (" << array_size << ") must be divisible by numprocs (" << numprocs << ").\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(0);
    }

    //chunksize = (ARRAYSIZE / numprocs);
    chunksize = (array_size / numprocs);
    tag2 = 1;
    tag1 = 2;

    // Master task only 
    if (my_id == SERVER_NODE){

        //get start time
        //time (&start);

        //Initialize the array
        sum = 0;
        for(int i=0; i<array_size; i++) {
            //data[i] =  i * 1.0;
            data[i] =  1;
            sum = sum + data[i];
        }
        cout << "Initialized array sum = " << sum << endl;

        // Send each job a a fraction of the array, keeping the first for the server node
        offset = chunksize;
        for (int destination_id=1; destination_id<numprocs; destination_id++) {
            MPI_Send(&offset, 1, MPI_INT, destination_id, tag1, MPI_COMM_WORLD);
            MPI_Send(&data[offset], chunksize, MPI_FLOAT, destination_id, tag2, MPI_COMM_WORLD);
            cout << "Sent " << chunksize << " elements to task " << destination_id << " offset= " << offset << endl;
            offset = offset + chunksize;
        }

        // Master does it's share of the work
        offset = 0;
        cout << "Sent " << chunksize << " elements to task SERVER offset= " << offset << endl;
        //mysum = make_operation(offset, chunksize, my_id, data);
        mysum = make_operation_linear(offset, chunksize, my_id, data);

        // Reduce data from workers
        MPI_Reduce(&mysum, &sum, 1, MPI_FLOAT, MPI_SUM, SERVER_NODE, MPI_COMM_WORLD);
        cout << "Sample results:" << endl;
        offset = 0;
        for (int i=0; i<numprocs; i++) {
            for (int j=0; j<5; j++) 
                cout << "  " << data[offset+j];
            cout << endl;
            offset = offset + chunksize;
        }
        cout << "Final sum = " << sum << endl;

    }
    else {
        // Server nodes
        // Receive fraction of array sent by the server node
        source = SERVER_NODE;
        MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
        MPI_Recv(&data[offset], chunksize, MPI_FLOAT, source, tag2, MPI_COMM_WORLD, &status);

        //mysum = make_operation(offset, chunksize, my_id, data);
        mysum = make_operation_linear(offset, chunksize, my_id, data);

        /* Send my results back to the the server task */
        MPI_Reduce(&mysum, NULL, 1, MPI_FLOAT, MPI_SUM, SERVER_NODE, MPI_COMM_WORLD);

    }

    MPI_Finalize();

    if (my_id == SERVER_NODE){
        //time (&end);
        //double diff = difftime(end,start);
        //cout << "Runtime:" << diff << " seconds\n";
        //cout << "\'{\"sum\":" << sum << ",\"time\":" << diff << "}\'" << endl;

        //ofstream myfile;
        //initialize random seed
        //srand (time(NULL));
        //int file_index = rand();

        //char file_name[50];
        //sprintf(file_name, "example_%d.txt", file_index);
        //cout << endl << "|" << file_name << "|" << endl;

        //myfile.open (file_name);
        //myfile << "\'{\"sum\":" << sum << ",\"time\":" << diff << "}\'" << endl; 
        //myfile.close();

        delete [] data;
    }


}

float make_operation(int myoffset, int chunk, int myid, float data[]) {
    // Perform some operations with the array

    float waister_array[myoffset + chunk];

    float mysum = 0;
    for(int i=myoffset; i < myoffset + chunk; i++) {
        mysum = mysum + data[i] + 1;
        for(int j=myoffset; j < myoffset + chunk; j++) {
            for(int k=myoffset; k < myoffset + chunk; k++) {
                //int time_consumer = j*k; 
                //int waister_1 = data[i] + i * 1.0 + time_consumer;
                //int waister_2 = data[i] + 1;
                waister_array[i] = waister_array[i] + data[i] + 1 * 1.0;
            }
        }
    }
    cout << "proc [" << myid << "] sum = " << mysum << endl;
    return(mysum);
}

float make_operation_linear(int myoffset, int chunk, int myid, float data[]) {
    // Perform some operations with the array

    float mysum = 0;
    float* waister_array;
    waister_array = new float[10000];

    //usleep(chunk/2);
    for(int i=myoffset; i < myoffset + chunk; i++) {
        mysum = mysum + data[i] + 1;

        //Do the same amout of work per each position of the array
        for(int j=0; j < 10000; j++) {
            for(int k=0; k < 10000; k++) {
                waister_array[j] = waister_array[i] + data[i] + 1 * 1.0;
            }
        }

    }

    cout << "proc [" << myid << "] sum = " << mysum << endl;
    return(mysum);
}
