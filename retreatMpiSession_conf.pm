=head1 LICENSE

Copyright [1999-2015] Wellcome Trust Sanger Institute and the EMBL-European Bioinformatics Institute
Copyright [2016-2018] EMBL-European Bioinformatics Institute

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=cut


=head1 CONTACT

  Please email comments or questions to the public Ensembl
  developers list at <http://lists.ensembl.org/mailman/listinfo/dev>.

  Questions may also be sent to the Ensembl help desk at
  <http://www.ensembl.org/Help/Contact>.

=head1 NAME

Bio::EnsEMBL::Compara::PipeConfig::retreatMpiSession_conf

=head1 DESCRIPTION

    The PipeConfig file for the retreat MPI/multi-threaded software in eHive/LSF session.

=head1 SYNOPSIS

    #1. generate list of jobs .....

    #2. run these jobs in different classes .....


=head1 AUTHORSHIP

Mateus Patricio and Matthieu Muffato

=head1 APPENDIX

The rest of the documentation details each of the object methods.
Internal methods are usually preceded with an underscore (_)

=cut

package Bio::EnsEMBL::Hive::Examples::Factories::PipeConfig::retreatMpiSession_conf;

use strict;
use warnings;

use base ('Bio::EnsEMBL::Hive::PipeConfig::HiveGeneric_conf');

sub default_options {
    my ($self) = @_;

    return {
        %{$self->SUPER::default_options},   # inherit the generic ones
        'host'          => 'mysql-ens-compara-prod-1:4485',
        'user'          => 'ensadmin',
        'password'      => $ENV{ENSADMIN_PSW},
        'mpi_code_exe'  => '/homes/mateus/mpi/distribute_array',
        'mpirun_exe'    => '/usr/lib64/mpich-3.2/bin/mpirun',
        'array_size'    => 500,
    };
}

sub resource_classes {
    my ($self) = @_;
    return {
        %{$self->SUPER::resource_classes},  # inherit 'default' from the parent class
        '500Mb_4c_ptile_1_mpi'    => {'LSF' => '-q mpi-rh7 -n 4 -M500 -R"select[mem>500] rusage[mem=500] span[ptile=1]"' },
        '500Mb_4c_ptile_2_mpi'    => {'LSF' => '-q mpi-rh7 -n 4 -M500 -R"select[mem>500] rusage[mem=500] span[ptile=2]"' },

        '500Mb_10c_ptile_1_mpi'   => {'LSF' => '-q mpi-rh7 -n 10 -M500 -R"select[mem>500] rusage[mem=500] span[ptile=1]"' },
        '500Mb_10c_ptile_2_mpi'   => {'LSF' => '-q mpi-rh7 -n 10 -M500 -R"select[mem>500] rusage[mem=500] span[ptile=2]"' },

        '500Mb_20c_ptile_1_mpi'   => {'LSF' => '-q mpi-rh7 -n 20 -M500 -R"select[mem>500] rusage[mem=500] span[ptile=1]"' },
        '500Mb_20c_ptile_5_mpi'   => {'LSF' => '-q mpi-rh7 -n 20 -M500 -R"select[mem>500] rusage[mem=500] span[ptile=5]"' },

        '500Mb_50c_ptile_2_mpi'   => {'LSF' => '-q mpi-rh7 -n 50 -M500 -R"select[mem>500] rusage[mem=500] span[ptile=2]"' },
        '500Mb_50c_ptile_10_mpi'  => {'LSF' => '-q mpi-rh7 -n 50 -M500 -R"select[mem>500] rusage[mem=500] span[ptile=10]"' },

        '500Mb_100c_ptile_2_mpi'  => {'LSF' => '-q mpi-rh7 -n 100 -M500 -R"select[mem>500] rusage[mem=500] span[ptile=2]"' },
        '500Mb_100c_ptile_10_mpi' => {'LSF' => '-q mpi-rh7 -n 100 -M500 -R"select[mem>500] rusage[mem=500] span[ptile=10]"' },
    }
}

sub pipeline_wide_parameters {  # these parameter values are visible to all analyses, can be overridden by parameters{} and input_id{}
    my ($self) = @_;
    return {
        %{$self->SUPER::pipeline_wide_parameters},          # here we inherit anything from the base class

        'mpi_code_exe'     => $self->o('mpi_code_exe'),
        'mpirun_exe'       => $self->o('mpirun_exe'),
        'array_size'       => $self->o('array_size'),
    }
}

sub pipeline_analyses {
    my ($self) = @_;

    return [

# ---------------------------------------------[ create jobs ]--------------------------------------------------------------------------------

        {   -logic_name => 'create_jobs',
            -module     => 'Bio::EnsEMBL::Hive::RunnableDB::Dummy',
            -input_ids => [
                {},
            ],
            -flow_into => {
                1 => [ 
                            'run_cmd_4_cores_ptile_1', 'run_cmd_4_cores_ptile_2',
                            'run_cmd_10_cores_ptile_1', 'run_cmd_10_cores_ptile_2',
                            'run_cmd_20_cores_ptile_1', 'run_cmd_20_cores_ptile_5',
                            'run_cmd_50_cores_ptile_2', 'run_cmd_50_cores_ptile_10',
                            'run_cmd_100_cores_ptile_2', 'run_cmd_100_cores_ptile_10'
                    ],
            },
        },

# ---------------------------------------------[ run commands ]-------------------------------------------------------------

        {   -logic_name    => 'run_cmd_4_cores_ptile_1',
            -module        => 'Bio::EnsEMBL::Hive::RunnableDB::SystemCmd',
            -parameters    => {
                'cmd'       => '#mpirun_exe# #mpi_code_exe# #array_size#',
            },
            -rc_name => '500Mb_4c_ptile_1_mpi',
        },

        {   -logic_name    => 'run_cmd_4_cores_ptile_2',
            -module        => 'Bio::EnsEMBL::Hive::RunnableDB::SystemCmd',
            -parameters    => {
                'cmd'       => '#mpirun_exe# #mpi_code_exe# #array_size#',
            },
            -rc_name => '500Mb_4c_ptile_2_mpi',
        },

        {   -logic_name    => 'run_cmd_10_cores_ptile_1',
            -module        => 'Bio::EnsEMBL::Hive::RunnableDB::SystemCmd',
            -parameters    => {
                'cmd'       => '#mpirun_exe# #mpi_code_exe# #array_size#',
            },
            -rc_name => '500Mb_10c_ptile_1_mpi',
        },

        {   -logic_name    => 'run_cmd_10_cores_ptile_2',
            -module        => 'Bio::EnsEMBL::Hive::RunnableDB::SystemCmd',
            -parameters    => {
                'cmd'       => '#mpirun_exe# #mpi_code_exe# #array_size#',
            },
            -rc_name => '500Mb_10c_ptile_2_mpi',
        },

        {   -logic_name    => 'run_cmd_20_cores_ptile_1',
            -module        => 'Bio::EnsEMBL::Hive::RunnableDB::SystemCmd',
            -parameters    => {
                'cmd'       => '#mpirun_exe# #mpi_code_exe# #array_size#',
            },
            -rc_name => '500Mb_20c_ptile_1_mpi',
        },

        {   -logic_name    => 'run_cmd_20_cores_ptile_5',
            -module        => 'Bio::EnsEMBL::Hive::RunnableDB::SystemCmd',
            -parameters    => {
                'cmd'       => '#mpirun_exe# #mpi_code_exe# #array_size#',
            },
            -rc_name => '500Mb_20c_ptile_5_mpi',
        },

        {   -logic_name    => 'run_cmd_50_cores_ptile_2',
            -module        => 'Bio::EnsEMBL::Hive::RunnableDB::SystemCmd',
            -parameters    => {
                'cmd'       => '#mpirun_exe# #mpi_code_exe# #array_size#',
            },
            -rc_name => '500Mb_50c_ptile_2_mpi',
        },

        {   -logic_name    => 'run_cmd_50_cores_ptile_10',
            -module        => 'Bio::EnsEMBL::Hive::RunnableDB::SystemCmd',
            -parameters    => {
                'cmd'       => '#mpirun_exe# #mpi_code_exe# #array_size#',
            },
            -rc_name => '500Mb_50c_ptile_10_mpi',
        },

        {   -logic_name    => 'run_cmd_100_cores_ptile_2',
            -module        => 'Bio::EnsEMBL::Hive::RunnableDB::SystemCmd',
            -parameters    => {
                'cmd'       => '#mpirun_exe# #mpi_code_exe# #array_size#',
            },
            -rc_name => '500Mb_100c_ptile_2_mpi',
        },

        {   -logic_name    => 'run_cmd_100_cores_ptile_10',
            -module        => 'Bio::EnsEMBL::Hive::RunnableDB::SystemCmd',
            -parameters    => {
                'cmd'       => '#mpirun_exe# #mpi_code_exe# #array_size#',
            },
            -rc_name => '500Mb_100c_ptile_10_mpi',
        },
    ];
}

1;

