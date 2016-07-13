
//  !!!!  THIS IS LEGACY CODE THAT SHOULD BE TREATED AS AN  !!!!
//  !!!!  EXTERNAL LIBRARY AND NOT BE MODIFIED.             !!!!


/*****************************************************************
**********     SPARSPAK ..... SOLUTION OF SPARSE         *********
**********     SYMMETRIC SYSTEMS OF LINEAR EQUATIONS     *********
**********     BY LDL FACTORIZATION WITH MMD RE-ORDERING *********
******************************************************************
*
*******************************************************************/
#ifndef SPARSPAK_H_
#define SPARSPAK_H_

int sp_genmmd(int* neqns, int* xadj, int* adjncy, int* invp, int* perm,
           int* delta, int* dhead, int* qsize, int* llist, int* marker,
           int*  maxint, int* nofsub);
    // Re-orders rows of matrix A to reduce fill-in when A is
    // factorized to produce L.

void sp_smbfct(int neqns, int* xadj, int* adjncy, int* perm, int* invp,
            int *xlnz, int& maxlnz, int* xnzsub, int* nzsub,
            int& maxsub, int* rchlnk, int* mrglnk, int* marker,
            int& flag);
    // Performs symbolic factorization of matrix A to identify non-zero
    // entries of L.

void sp_numfct(int neqns, int* xlnz, double* lnz, int* xnzsub, int* nzsub,
            double* diag, int* link, int* first, double* temp,
            int& iflag);
    // Performs numerical factorization of A to compute the non-zero
    // values in L.

void sp_solve(int neqns, int* xlnz, double* lnz, int* xnzsub,
           int* nzsub, double* diag, double* rhs);
    // Solves the factorized system LDL'x = b.

#endif
