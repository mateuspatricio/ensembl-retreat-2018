# include <mpi.h>
# include <mysql.h>
# include <stdlib.h> 
# include <iostream>
# include <unistd.h>

/* MySQL Connector/C++ specific headers */
#include <mysql.h>

#define  SERVER_NODE     0

MYSQL *connection, mysql;
MYSQL_RES *result;
MYSQL_ROW row;


using namespace std;

//Prototypes
int main ( int argc, char *argv[] );

int main ( int argc, char *argv[] ) {

    //Start with MPI
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
            cerr << "Usage: " << argv[0] << " chunck_size" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        int chunksize = atoi(argv[1]);

        // Send each job the interval of sequences they have to query
        int from = 0;
        int to = chunksize;
        for (int destination_id=1; destination_id<numprocs; destination_id++) {
            cout << "From:" << from << " - To: " << to << endl;
            MPI_Send(&from, 1, MPI_INT, destination_id, tag_chunksize, MPI_COMM_WORLD);
            MPI_Send(&to, 1, MPI_INT, destination_id, tag_data, MPI_COMM_WORLD);
            from = to;
            to = from + chunksize;
            from++;
        }

        // Ther server node will rest this time :)

        // Reduce data from workers
        float avg = -1;
        float myavg;

        MPI_Reduce(&myavg, &avg, 1, MPI_FLOAT, MPI_SUM, SERVER_NODE, MPI_COMM_WORLD);

        avg = avg/(numprocs-1);
        cout << "Sequence average length is: " << avg << endl;

    }
    else {

        MYSQL *connect;
        connect=mysql_init(NULL);
        if (!connect) {
            cout<<"MySQL Initialization failed";
            return 1;
        }
        connect=mysql_real_connect(connect, "mysql-ensembl-mirror", "ensro", "" , "ensembl_compara_92" , 4240,NULL,0);
        if (connect) {
            //cout<<"connection succeeded\n";
        } 
        else {
            cout<<"connection failed\n";
        }

        // Worker nodes
        // Receive fraction of array sent by the server node
        int from;
        int to;
        MPI_Recv(&from, 1, MPI_INT, SERVER_NODE, tag_chunksize, MPI_COMM_WORLD, &status);
        MPI_Recv(&to, 1, MPI_INT, SERVER_NODE, tag_data, MPI_COMM_WORLD, &status);

        float myavg;

        //Running queries on the clients:
        MYSQL_RES *res_set;
        MYSQL_ROW row;

        //int from = 1;
        //int to = 1;
        char query_string[50];
        sprintf(query_string, "SELECT AVG(LENGTH(sequence)) FROM sequence WHERE sequence_id BETWEEN %d AND %d;", from, to);
        mysql_query (connect,query_string);

        unsigned int i =0;
        res_set = mysql_store_result(connect);
        unsigned int numrows = mysql_num_rows(res_set);
        while (((row= mysql_fetch_row(res_set)) !=NULL )) { 
            myavg = atof(row[i]);
        }

        cout << "my avg: " << myavg << endl;

        /* Send my results back to the the server task */
        MPI_Reduce(&myavg, NULL, 1, MPI_FLOAT, MPI_SUM, SERVER_NODE, MPI_COMM_WORLD);

        mysql_close (connect);
    }


    MPI_Finalize();
}
