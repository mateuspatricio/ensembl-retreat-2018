# ensembl-retreat-2018
Some files used by the "MPI/multi-threaded software in eHive/LSF" session during the Ensembl retreat 2018

## Compiling preparations
Currently LSF is not working properly with our MPI implementation, so we should stick to use the one supplied by the EBI system team.
Assuming that most of us are sourcing `basic.sh` we should get rid of the unwanted stuff.

For that you could try to run the following recipe:

```bash
unset PROMPT_COMMAND
unset $(env | grep /nfs/software/ensembl | grep -v : | cut -d= -f1 | grep -v '^_$')
LD_LIBRARY_PATH=$(echo $LD_LIBRARY_PATH | sed 's/:/\n/g' | grep -v /nfs/software/ensembl | xargs echo | sed 's/ /:/g')
MANPATH=$(echo $MANPATH | sed 's/:/\n/g' | grep -v /nfs/software/ensembl | xargs echo | sed 's/ /:/g')
INFOPATH=$(echo $INFOPATH | sed 's/:/\n/g' | grep -v /nfs/software/ensembl | xargs echo | sed 's/ /:/g')
PATH=$(echo $PATH | sed 's/:/\n/g' | grep -v /nfs/software/ensembl | xargs echo | sed 's/ /:/g')
source /nfs/software/ensembl/RHEL7-JUL2017-core2/envs/plenv.sh
module load mpi/mpich-3.2-x86_64
```

### Compiling
`mpic++ -o distribute_array distribute_array.cc`

### Testing
Using the default mpirun:
`/usr/lib64/mpich-3.2/bin/mpirun -n 4 /homes/mateus/mpi/distribute_array 100`

## Initiating the pipeline
`init_pipeline.pl Bio::EnsEMBL::Hive::Examples::Factories::PipeConfig::retreatMpiSession_conf -pipeline_name retreat_mpi_session_500`

## Querying the results
```SQL
SELECT logic_name, input_id, runtime_msec, cpu_sec, lifespan_sec, mem_megs FROM job JOIN analysis_base USING (analysis_id) JOIN role USING (role_id) JOIN worker USING (worker_id) JOIN worker_resource_usage USING (worker_id) WHERE logic_name like "run_cmd%" AND job.status = "DONE" ORDER BY (cpu_sec);
```
