# include <mpi.h>
# include <stdlib.h> 
# include <iostream>
# include <unistd.h>

//# include <string>
//# include <fstream>

#define  SERVER_NODE     0

using namespace std;

//Prototypes
int main ( int argc, char *argv[] );
long make_operation(int data[], int n_elements, int myid);

int main ( int argc, char *argv[] ) {

    MPI_Status status;
    int numprocs, my_id;

    //Initialize MPI.
    MPI_Init ( &argc, &argv );

    //Get the number of processes.
    MPI_Comm_size ( MPI_COMM_WORLD, &numprocs );

    //Determine the rank of this process.
    MPI_Comm_rank ( MPI_COMM_WORLD, &my_id );

    int tag_data = 1;
    int tag_chunksize = 2;

    // Master task only 
    if (my_id == SERVER_NODE){

        //Initialize the array
        if (argc != 2) {
            cerr << "Usage: " << argv[0] << " n_elements" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        int array_size = atoi(argv[1]);

        if (array_size % numprocs != 0) {
            cout << "Quitting. Array size (" << array_size << ") must be divisible by numprocs (" << numprocs << ").\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        int* data = new int[array_size];
        long expected_sum = 0;
        for(int i=0; i<array_size; i++) {
            //data[i] =  i * 1.0;
            data[i] =  1;
            expected_sum = expected_sum + data[i];
        }
        cout << "Initialized array sum = " << expected_sum << endl;

        // Send each job a a fraction of the array, keeping the first for the server node
        int chunksize = (array_size / numprocs);
        int offset = chunksize;
        for (int destination_id=1; destination_id<numprocs; destination_id++) {
            MPI_Send(&chunksize, 1, MPI_INT, destination_id, tag_chunksize, MPI_COMM_WORLD);
            MPI_Send(&data[offset], chunksize, MPI_INT, destination_id, tag_data, MPI_COMM_WORLD);
            cout << "Sent " << chunksize << " elements to task " << destination_id << " offset= " << offset << endl;
            offset = offset + chunksize;
        }

        // Master does its share of the work
        cout << "Sent " << chunksize << " elements to task SERVER no offset" << endl;
        long mysum = make_operation(data, chunksize, my_id);

        // Reduce data from workers
        long sum = -1;
        MPI_Reduce(&mysum, &sum, 1, MPI_LONG, MPI_SUM, SERVER_NODE, MPI_COMM_WORLD);

        cout << "Sample data:" << endl;
        offset = 0;
        for (int i=0; i<numprocs; i++) {
            for (int j=0; j<5; j++) 
                cout << "  " << data[offset+j];
            cout << endl;
            offset = offset + chunksize;
        }
        cout << "Final sum = " << sum << " " << (sum == expected_sum ? "OK" : "ERROR") << endl;

        delete [] data;

    }
    else {
        // Worker nodes
        // Receive fraction of array sent by the server node
        int chunksize;
        MPI_Recv(&chunksize, 1, MPI_INT, SERVER_NODE, tag_chunksize, MPI_COMM_WORLD, &status);
        int* data = new int[chunksize];
        MPI_Recv(data, chunksize, MPI_INT, SERVER_NODE, tag_data, MPI_COMM_WORLD, &status);

        long mysum = make_operation(data, chunksize, my_id);

        /* Send my results back to the the server task */
        MPI_Reduce(&mysum, NULL, 1, MPI_LONG, MPI_SUM, SERVER_NODE, MPI_COMM_WORLD);

        delete [] data;
    }

    MPI_Finalize();
}


long make_operation(int data[], int n_elements, int myid) {
    // Perform some operations with the array

    long mysum = 0;
    float* waster_array;
    waster_array = new float[10000];

    for(int i=0; i<n_elements; i++) {
        mysum = mysum + data[i];

        //Do the same amout of work per each position of the array
        for(int j=0; j < 10000; j++) {
            for(int k=0; k < 10000; k++) {
                waster_array[j] = waster_array[i] + data[i] + 1 * 1.0;
            }
        }

    }
    delete [] waster_array;

    cout << "proc [" << myid << "] sum = " << mysum << endl;
    return(mysum);
}
