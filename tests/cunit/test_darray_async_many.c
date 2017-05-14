/*
 * This program tests darrays with async. This tests uses many types
 * of vars and iodesc's, all in the same file.
 *
 * Ed Hartnett, 5/10/17
 */
#include <pio.h>
#include <pio_tests.h>
#include <pio_internal.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The minimum number of tasks this test should run on. */
#define MIN_NTASKS 1

/* The name of this test. */
#define TEST_NAME "test_darray_async_many"

/* For 1-D use. */
#define NDIM1 1

/* For 2-D use. */
#define NDIM2 2

/* For 3-D use. */
#define NDIM3 3

/* For 4-D use. */
#define NDIM4 4

/* For maplens of 2. */
#define MAPLEN2 2

/* Lengths of non-unlimited dimensions. */
#define LAT_LEN 2
#define LON_LEN 3

/* Number of vars in test file. */
#ifdef _NETCDF4
#define NTYPE 11
int my_type[NTYPE] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT,
                      PIO_DOUBLE, PIO_UBYTE, PIO_USHORT, PIO_UINT, PIO_INT64, PIO_UINT64};
#else
#define NTYPE NUM_CLASSIC_TYPES
int my_type[NTYPE] = {PIO_BYTE, PIO_CHAR, PIO_SHORT, PIO_INT, PIO_FLOAT,
                       PIO_DOUBLE};
#endif /* _NETCDF4 */

/* We will have one record, and one non-record var of each type. */
#define NVAR (NTYPE * 2)

/* Number of records written for record var. */
#define NREC 3

/* Names of the dimensions. */
char dim_name[NDIM4][PIO_MAX_NAME + 1] = {"time", "vert_level", "lat", "lon"};

/* Check the file that was created in this test. */
int check_darray_file(int iosysid, char *data_filename, int iotype, int my_rank,
                      int *rec_varid, int *norec_varid, int num_types)
{
    int ncid;
    int ret;

    /* These are the values we expect. */
    signed char expected_byte[LAT_LEN * LON_LEN] = {1, 2, 2, 3, 3, 4};
    char expected_char[LAT_LEN * LON_LEN] = {65, 97, 66, 98, 67, 99};
    short expected_short[LAT_LEN * LON_LEN] = {-10, -9, -20, -19, -30, -29};
    int expected_int[LAT_LEN * LON_LEN] = {-NC_MAX_SHORT - 1, NC_MAX_SHORT + 2, -NC_MAX_SHORT - 2,
                                           NC_MAX_SHORT + 3, -NC_MAX_SHORT - 3, NC_MAX_SHORT + 4};
    float expected_float[LAT_LEN * LON_LEN] = {10.5, 11.5, 21, 22, 31.5, 32.5};
    double expected_double[LAT_LEN * LON_LEN] = {NC_MAX_FLOAT + 0.5, NC_MAX_FLOAT + 1.5, NC_MAX_FLOAT + 1.5,
                                                 NC_MAX_FLOAT + 2.5, NC_MAX_FLOAT + 2.5, NC_MAX_FLOAT + 3.5};
#ifdef _NETCDF4
    unsigned char expected_ubyte[LAT_LEN * LON_LEN] = {10, 11, 20, 21, 30, 31};
    unsigned short expected_ushort[LAT_LEN * LON_LEN] = {1000, 1001, 2000, 2001, 3000, 3001};
    unsigned int expected_uint[LAT_LEN * LON_LEN] = {(unsigned short)32777, (unsigned short)32778, (unsigned short)32787, (unsigned short)32788, (unsigned short)32797, (unsigned short)32798};
    long long expected_int64[LAT_LEN * LON_LEN] = {-2147483639LL, -2147483637LL, -2147483629LL,
                                                   -2147483627LL, -2147483619LL, -2147483617LL};
    unsigned long long expected_uint64[LAT_LEN * LON_LEN] = {9223372036854775817ULL, 9223372036854775818ULL,
                                                             9223372036854775827ULL, 9223372036854775828ULL,
                                                             9223372036854775837ULL, 9223372036854775838ULL};
#endif /* _NETCDF4 */

    /* Reopen the file. */
    if ((ret = PIOc_openfile(iosysid, &ncid, &iotype, data_filename, NC_NOWRITE)))
        ERR(ret);

    /* Check metadata. */
    int ndims_in, nvars_in, ngatts_in, unlimdimid_in;
    if ((ret = PIOc_inq(ncid, &ndims_in, &nvars_in, &ngatts_in, &unlimdimid_in)))
        ERR(ret);
    if (ndims_in != NDIM4 || nvars_in != num_types * 2 || ngatts_in != 0 || unlimdimid_in != 0)
        ERR(ERR_WRONG);

    /* Check the vars. */
    for (int t = 0; t < num_types; t++)
    {
        void *data_in;
        void *norec_data_in;
        PIO_Offset type_size;

        /* Find size of type. */
        if ((ret = PIOc_inq_type(ncid, my_type[t], NULL, &type_size)))
            ERR(ret);

        /* Allocate buffers to hold data. */
        if (!(data_in = malloc(LAT_LEN * LON_LEN * NREC * type_size)))
            ERR(PIO_ENOMEM);
        if (!(norec_data_in = malloc(LAT_LEN * LON_LEN * type_size)))
            ERR(PIO_ENOMEM);

        /* Read record and non-record vars for this type. */
        if ((ret = PIOc_get_var(ncid, rec_varid[t], data_in)))
            ERR(ret);
        if ((ret = PIOc_get_var(ncid, norec_varid[t], norec_data_in)))
            ERR(ret);

        /* Check each value of non-record data. */
        for (int r = 0; r < LAT_LEN * LON_LEN; r++)
        {
            switch (my_type[t])
            {
            case PIO_BYTE:
                if (((signed char *)norec_data_in)[r] != expected_byte[r])
                    ERR(ERR_WRONG);
                break;
            case PIO_CHAR:
                if (((char *)norec_data_in)[r] != expected_char[r])
                    ERR(ERR_WRONG);
                break;
            case PIO_SHORT:
                if (((short *)norec_data_in)[r] != expected_short[r])
                    ERR(ERR_WRONG);
                break;
            case PIO_INT:
                if (((int *)norec_data_in)[r] != expected_int[r])
                    ERR(ERR_WRONG);
                break;
            case PIO_FLOAT:
                if (((float *)norec_data_in)[r] != expected_float[r])
                    ERR(ERR_WRONG);
                break;
            case PIO_DOUBLE:
                if (((double *)norec_data_in)[r] != expected_double[r])
                    ERR(ERR_WRONG);
                break;
#ifdef _NETCDF4
            case PIO_UBYTE:
                if (((unsigned char *)norec_data_in)[r] != expected_ubyte[r])
                    ERR(ERR_WRONG);
                break;
            case PIO_USHORT:
                if (((unsigned short *)norec_data_in)[r] != expected_ushort[r])
                    ERR(ERR_WRONG);
                break;
            case PIO_UINT:
                if (((unsigned int *)norec_data_in)[r] != expected_uint[r])
                    ERR(ERR_WRONG);
                break;
            case PIO_INT64:
                if (((long long *)norec_data_in)[r] != expected_int64[r])
                    ERR(ERR_WRONG);
                break;
            case PIO_UINT64:
                if (((unsigned long long *)norec_data_in)[r] != expected_uint64[r])
                    ERR(ERR_WRONG);
                break;
#endif /* _NETCDF4 */
            default:
                ERR(ERR_WRONG);
            }
        }
        
        /* Check each value of record data. */
        for (int r = 0; r < LAT_LEN * LON_LEN * NREC; r++)
        {
            switch (my_type[t])
            {
            case PIO_BYTE:
                if (((signed char *)data_in)[r] != expected_byte[r % (LAT_LEN * LON_LEN)])
                    ERR(ERR_WRONG);
                break;
            case PIO_CHAR:
                if (((char *)data_in)[r] != expected_char[r % (LAT_LEN * LON_LEN)])
                    ERR(ERR_WRONG);
                break;
            case PIO_SHORT:
                if (((short *)data_in)[r] != expected_short[r % (LAT_LEN * LON_LEN)])
                    ERR(ERR_WRONG);
                break;
            case PIO_INT:
                if (((int *)data_in)[r] != expected_int[r % (LAT_LEN * LON_LEN)])
                    ERR(ERR_WRONG);
                break;
            case PIO_FLOAT:
                if (((float *)data_in)[r] != expected_float[r % (LAT_LEN * LON_LEN)])
                    ERR(ERR_WRONG);
                break;
            case PIO_DOUBLE:
                if (((double *)data_in)[r] != expected_double[r % (LAT_LEN * LON_LEN)])
                    ERR(ERR_WRONG);
                break;
#ifdef _NETCDF4
            case PIO_UBYTE:
                if (((unsigned char *)data_in)[r] != expected_ubyte[r % (LAT_LEN * LON_LEN)])
                    ERR(ERR_WRONG);
                break;
            case PIO_USHORT:
                if (((unsigned short *)data_in)[r] != expected_ushort[r % (LAT_LEN * LON_LEN)])
                    ERR(ERR_WRONG);
                break;
            case PIO_UINT:
                if (((unsigned int *)data_in)[r] != expected_uint[r % (LAT_LEN * LON_LEN)])
                    ERR(ERR_WRONG);
                break;
            case PIO_INT64:
                if (((long long *)data_in)[r] != expected_int64[r % (LAT_LEN * LON_LEN)])
                    ERR(ERR_WRONG);
                break;
            case PIO_UINT64:
                if (((unsigned long long *)data_in)[r] != expected_uint64[r % (LAT_LEN * LON_LEN)])
                    ERR(ERR_WRONG);
                break;
#endif /* _NETCDF4 */
            default:
                ERR(ERR_WRONG);
            }
        }
        
        free(data_in);
        free(norec_data_in);
    }
            
    /* Close the file. */
    if ((ret = PIOc_closefile(ncid)))
        ERR(ret);

    return 0;
}

/* Run a simple test using darrays with async. */
int run_darray_async_test(int iosysid, int my_rank, MPI_Comm test_comm,
                          int num_flavors, int *flavor)
{
    int ioid_1byte;
    int ioid_2byte;
    int ioid_4byte;
    int ioid_8byte;
    int dim_len[NDIM4] = {NC_UNLIMITED, 2, 2, 3};
    int dimids_3d[NDIM3] = {0, 2, 3};
    int dimids_2d[NDIM2] = {2, 3};
    PIO_Offset elements_per_pe = LAT_LEN;
    PIO_Offset compdof[LAT_LEN] = {my_rank * 2 - 2, my_rank * 2 - 1};
    char decomp_filename[PIO_MAX_NAME + 1];

    /* Test data. */
    signed char my_data_byte[LAT_LEN] = {my_rank, my_rank + 1};
    char my_data_char[LAT_LEN] = {64 + my_rank, 96 + my_rank};
    short my_data_short[LAT_LEN] = {-my_rank * 10, -my_rank * 10 + 1};
    int my_data_int[LAT_LEN] = {-NC_MAX_SHORT - my_rank, NC_MAX_SHORT + my_rank + 1};
    float my_data_float[LAT_LEN] = {my_rank * 10.5, my_rank * 10.5 + 1};
    double my_data_double[LAT_LEN] = {NC_MAX_FLOAT + my_rank + 0.5, NC_MAX_FLOAT + my_rank * 1.5};
#ifdef _NETCDF4
    unsigned char my_data_ubyte[LAT_LEN] = {my_rank * 10, my_rank * 10 + 1};
    unsigned short my_data_ushort[LAT_LEN] = {my_rank * 1000, my_rank * 1000 + 1};
    unsigned int my_data_uint[LAT_LEN] = {NC_MAX_SHORT + my_rank * 10, NC_MAX_SHORT + my_rank * 10 + 1};
    long long my_data_int64[LAT_LEN] = {NC_MAX_INT + my_rank * 10, -NC_MAX_INT + my_rank * 10};
    unsigned long long my_data_uint64[LAT_LEN] = {NC_MAX_INT64 + my_rank * 10, NC_MAX_INT64 + my_rank * 10 + 1};
#endif /* _NETCDF4 */

#ifdef _NETCDF4
    void *my_data[NTYPE] = {my_data_byte, my_data_char, my_data_short, my_data_int, my_data_float, my_data_double,
                            my_data_ubyte, my_data_ushort, my_data_uint, my_data_int64, my_data_uint64};
#else
    void *my_data[NTYPE] = {my_data_byte, my_data_char, my_data_short, my_data_int, my_data_float, my_data_double};
#endif /* _NETCDF4 */
    int ret;

    sprintf(decomp_filename, "decomp_%s_rank_%d.nc", TEST_NAME, my_rank);

    /* Create the PIO decompositions for this test. */
    if ((ret = PIOc_init_decomp(iosysid, PIO_BYTE, NDIM2, &dim_len[2], elements_per_pe,
                                compdof, &ioid_1byte, PIO_REARR_BOX, NULL, NULL)))
        ERR(ret);
    if ((ret = PIOc_init_decomp(iosysid, PIO_SHORT, NDIM2, &dim_len[2], elements_per_pe,
                                compdof, &ioid_2byte, PIO_REARR_BOX, NULL, NULL)))
        ERR(ret);
    if ((ret = PIOc_init_decomp(iosysid, PIO_INT, NDIM2, &dim_len[2], elements_per_pe,
                                compdof, &ioid_4byte, PIO_REARR_BOX, NULL, NULL)))
        ERR(ret);
    if ((ret = PIOc_init_decomp(iosysid, PIO_DOUBLE, NDIM2, &dim_len[2], elements_per_pe,
                                compdof, &ioid_8byte, PIO_REARR_BOX, NULL, NULL)))
        ERR(ret);

    /* These are the decompositions associated with each type. */
#ifdef _NETCDF4
    int var_ioid[NTYPE] = {ioid_1byte, ioid_1byte, ioid_2byte, ioid_4byte, ioid_4byte,
                           ioid_8byte, ioid_1byte, ioid_2byte, ioid_4byte, ioid_8byte,
                           ioid_8byte};
#else
    int var_ioid[NTYPE] = {ioid_1byte, ioid_1byte, ioid_2byte, ioid_4byte, ioid_4byte,
                           ioid_8byte};
#endif /* _NETCDF4 */    
    
    /* Write the decomp file (on appropriate tasks). */
    if ((ret = PIOc_write_nc_decomp(iosysid, decomp_filename, 0, ioid_1byte, NULL, NULL, 0)))
        return ret;

    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        int ncid;
        int dimid[NDIM3];
        char data_filename[PIO_MAX_NAME + 1];
        int num_types = flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P ?
            NUM_NETCDF4_TYPES - 1 : NUM_CLASSIC_TYPES;
        int rec_varid[num_types];
        int norec_varid[num_types];

        /* For now, only serial iotypes work. Parallel coming soon! */
        if (flavor[fmt] == PIO_IOTYPE_PNETCDF)
            continue;

        /* Create sample output file. */
        sprintf(data_filename, "data_%s_iotype_%d.nc", TEST_NAME, flavor[fmt]);
        if ((ret = PIOc_createfile(iosysid, &ncid, &flavor[fmt], data_filename,
                                   NC_CLOBBER)))
            ERR(ret);

        /* Define dimensions. */
        for (int d = 0; d < NDIM4; d++)
            if ((ret = PIOc_def_dim(ncid, dim_name[d], dim_len[d], &dimid[d])))
                ERR(ret);

        /* Define variables. */
        char var_name[PIO_MAX_NAME + 1];
        char var_norec_name[PIO_MAX_NAME + 1];
        for (int t = 0; t < num_types; t++)
        {

            sprintf(var_name, "var_%d", t);
            sprintf(var_norec_name, "var_norec_%d", t);
            if ((ret = PIOc_def_var(ncid, var_name, my_type[t], NDIM3, dimids_3d, &rec_varid[t])))
                ERR(ret);
            if ((ret = PIOc_def_var(ncid, var_norec_name, my_type[t], NDIM2, dimids_2d,
                                    &norec_varid[t])))
            ERR(ret);
        }

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Write a record and non-record var for each type. */
        for (int t = 0; t < num_types; t++)
        {
            /* Write record data to the record vars. */
            for (int r = 0; r < NREC; r++)
            {
                /* Set or advance the record number. */
                if (!r)
                {
                    if ((ret = PIOc_setframe(ncid, rec_varid[t], 0)))
                        ERR(ret);
                }
                else
                {
                    if ((ret = PIOc_advanceframe(ncid, rec_varid[t])))
                        ERR(ret);
                }

                /* Write a record of data. */
                if ((ret = PIOc_write_darray(ncid, rec_varid[t], var_ioid[t], elements_per_pe,
                                             my_data[t], NULL)))
                    ERR(ret);

                /* Sync the file. */
                if ((ret = PIOc_sync(ncid)))
                    ERR(ret);
            } /* next record. */

        } /* next record var */
            
        /* Write some data to the non-record vars. */
        for (int t = 0; t < num_types; t++)
        {
            if ((ret = PIOc_write_darray(ncid, norec_varid[t], var_ioid[t], elements_per_pe, my_data[t], NULL)))
                ERR(ret);
        }

        /* Close the file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Check the file for correctness. */
        if ((ret = check_darray_file(iosysid, data_filename, PIO_IOTYPE_NETCDF, my_rank,
                                     rec_varid, norec_varid, num_types)))
            ERR(ret);

    } /* next iotype */

    /* Free the decompositions. */
    if ((ret = PIOc_freedecomp(iosysid, ioid_1byte)))
        ERR(ret);
    if ((ret = PIOc_freedecomp(iosysid, ioid_2byte)))
        ERR(ret);
    if ((ret = PIOc_freedecomp(iosysid, ioid_4byte)))
        ERR(ret);
    if ((ret = PIOc_freedecomp(iosysid, ioid_8byte)))
        ERR(ret);

    return 0;
}

/* Run Tests for pio_spmd.c functions. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks;  /* Number of processors involved in current execution. */
    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    MPI_Comm test_comm; /* A communicator for this test. */
    int ret;     /* Return code. */

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, MIN_NTASKS,
                              TARGET_NTASKS, 3, &test_comm)))
        ERR(ERR_INIT);
    if ((ret = PIOc_set_iosystem_error_handling(PIO_DEFAULT, PIO_RETURN_ERROR, NULL)))
        return ret;

    /* Figure out iotypes. */
    if ((ret = get_iotypes(&num_flavors, flavor)))
        ERR(ret);
    printf("Runnings tests for %d flavors\n", num_flavors);

    /* Test code runs on TARGET_NTASKS tasks. The left over tasks do
     * nothing. */
    if (my_rank < TARGET_NTASKS)
    {
        int iosysid;

        /* Initialize with task 0 as IO task, tasks 1-3 as a
         * computation component. */
#define NUM_IO_PROCS 1
#define NUM_COMPUTATION_PROCS 3
#define COMPONENT_COUNT 1
        int num_computation_procs = NUM_COMPUTATION_PROCS;
        MPI_Comm io_comm;              /* Will get a duplicate of IO communicator. */
        MPI_Comm comp_comm[COMPONENT_COUNT]; /* Will get duplicates of computation communicators. */
        int mpierr;

        if ((ret = PIOc_init_async(test_comm, NUM_IO_PROCS, NULL, COMPONENT_COUNT,
                                   &num_computation_procs, NULL, &io_comm, comp_comm,
                                   PIO_REARR_BOX, &iosysid)))
            ERR(ERR_INIT);
        
        /* This code runs only on computation components. */
        if (my_rank)
        {
            /* Run the simple darray async test. */
            if ((ret = run_darray_async_test(iosysid, my_rank, test_comm, num_flavors, flavor)))
                return ret;
            
            /* Finalize PIO system. */
            if ((ret = PIOc_finalize(iosysid)))
                return ret;
            
            /* Free the computation conomponent communicator. */
            if ((mpierr = MPI_Comm_free(comp_comm)))
                MPIERR(mpierr);
        }
        else
        {
            /* Free the IO communicator. */
            if ((mpierr = MPI_Comm_free(&io_comm)))
                MPIERR(mpierr);
        }
    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize the MPI library. */
    printf("%d %s Finalizing...\n", my_rank, TEST_NAME);
    if ((ret = pio_test_finalize(&test_comm)))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
