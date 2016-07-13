/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

#include "sparspaksolver.h"
#include "sparspak.h"

#include <cstring>
#include <limits>
#include <iostream>
#include <ctime>
using namespace std;

// Local module-level functions
//-----------------------------------------------------------------------------
int  compress(
        int n, int nnz, int* xrow, int* xcol, int* xadj, int* adjncy, int* xaij);
void buildAdjncy(
        int n, int nnz, int* xrow, int* xcol, int* xadj,
        int* adjncy, int* adjncy2, int* xaij, int* nz);
void sortAdjncy(
        int n, int nnz, int* xadj, int* adjncy, int* xadj2, int* adjncy2, int* nz);
void transpose(
        int n, int* xadj1, int* adjncy1, int* xadj2, int* adjncy2, int* nz);
int  reorder(
        int n, int* xadj, int* adjncy, int* perm, int* invp, int& nnzl);
int  factorize(
        int n, int& nnzl, int* xadj, int* adjncy, int* perm,
        int* invp, int* xlnz, int* xnzsub, int* nzsub);
void aij2lnz(
        int nnz, int* xrow, int* xcol, int* invp, int* xlnz, int* xnzsub,
        int* nzsub, int* xaij);

//-----------------------------------------------------------------------------

SparspakSolver::SparspakSolver(ostream& logger) :
    nrows(0), nnz(0), nnzl(0), perm(0), invp(0), xlnz(0), xnzsub(0),
    nzsub(0), xaij(0), link(0), first(0), lnz(0), diag(0), rhs(0), temp(0),
    msgLog(logger)
{}

//-----------------------------------------------------------------------------

SparspakSolver::~SparspakSolver()
{
    delete [] perm;
    delete [] invp;
    delete [] xlnz;
    delete [] xnzsub;
    delete [] nzsub;
    delete [] xaij;
    delete [] link;
    delete [] first;
    delete [] lnz;
    delete [] diag;
    delete [] rhs;
    delete [] temp;
}

//-----------------------------------------------------------------------------

int SparspakSolver::init(int nrows_, int nnz_, int* xrow, int* xcol)
{
    // ... save number of equations and number of off-diagonal coeffs.
    nrows = nrows_;
    nnz = nnz_;

    // ... allocate space for pointers from Aij to lnz
    xaij = new int[nnz];
    if ( !xaij ) return 0;
    memset(xaij, 0, nnz*sizeof(int));

    // ... allocate space for row re-ordering
    perm = new int[nrows];
    invp = new int[nrows];
    if ( !perm || !invp ) return 0;


    // ... compress, re-order, and factorize coeff. matrix A
    int* xadj;
    int* adjncy;
    int flag = 0;
    for (;;)
    {
        // ... allocate space for adjacency lists
        xadj = new int[nrows+1];
        adjncy = new int[2*nnz];
        if ( !xadj || !adjncy ) break;

        // ... store matrix A in compressed format
        if ( !compress(nrows, nnz, xrow, xcol, xadj, adjncy, xaij) ) break;

        // ... re-order the rows of A to minimize fill-in
        //clock_t startTime = clock();
        if ( !reorder(nrows, xadj, adjncy, perm, invp, nnzl) ) break;

/************ DEBUG  ******************
    cout << "\n nnzl = " << nnzl;

    for (int i = 0; i < nrows; i++)
    {
        cout << "\n i = " << i << "  perm[i] = " << perm[i] << "  invp[i] = " << invp[i];
    }

*****************************************/

        // ... allocate space for compressed storage of factorized matrix
        xlnz = new int[nrows+1];
        xnzsub = new int[nrows+1];
        nzsub = new int[nnzl];
        if ( !xlnz || !xnzsub || !nzsub ) break;

        // ... symbolically factorize A to produce L
        if ( !factorize(nrows, nnzl, xadj, adjncy, perm, invp, xlnz,
                        xnzsub, nzsub) ) break;

/*************  DEBUG  ********************
        // ... report factorization results

        int nnz0 = xadj[nrows] / 2;
        double procTime = (double)(clock() - startTime) /
                          (double)CLOCKS_PER_SEC * 1000.0;

        msgLog << endl;
        msgLog << "  Hydraulic Solution Matrix:" << endl;
        msgLog << "  Number of rows          " << nrows << endl;
        msgLog << "  Off-diagonal non-zeros  " << nnz << endl;
        msgLog << "  Duplicate non-zeros     " << nnz - nnz0 << endl;
        msgLog << "  Amount of fill-in       " << nnzl - nnz0 << endl;
        msgLog << "  Processing time (msec)  " << procTime << endl;
********************************************/

        // ... all steps were successful
        flag = 1;
        break;
    }

    // ... free memory used for adjacency lists
    delete [] xadj;
    delete [] adjncy;

    // ... return if error condition
    if ( !flag ) return flag;

    // ... map off-diag coeffs. of A to positions in xlnz
    aij2lnz(nnz, xrow, xcol, invp, xlnz, xnzsub, nzsub, xaij);

    // ... allocate space for coeffs. of L and r.h.s vector
    lnz = new double[nnzl];
    diag = new double[nrows];
    rhs = new double[nrows];
    if ( !lnz || ! diag || !rhs ) return 0;

    // ... allocate space for work arrays used by the solve() method
    temp = new double[nrows];
    first = new int[nrows];
    link = new int[nrows];
    if ( !temp || !first || !link ) return 0;
    return 1;
}

//-----------------------------------------------------------------------------

int SparspakSolver::solve(int n, double x[])
{
    // ... call sp_numfct to numerically evaluate the factorized matrix L

/*********  DEBUG  ****************************
    --diag;  --rhs; --invp;
    cout << "\n Before call to numfct:";
    for (int i = 1; i <= nrows; i++)
    {
        int j = invp[i] - 1;
        cout << "\n diag[" << j << "] = " << diag[i] << ",  rhs[" << j << "] = " << rhs[i];
    }
    ++diag;  ++rhs;  ++invp;
*********************************************/

    int flag;
    sp_numfct(nrows, xlnz, lnz, xnzsub, nzsub, diag, link, first, temp, flag);

    // if the matrix was ill-conditioned, return the problematic row
    if ( flag )
    {
        --invp;
        flag = invp[flag] - 1;
        ++invp;
        return flag;
    }

    // call sp_solve() to solve the system LDL'x = b
    sp_solve(nrows, xlnz, lnz, xnzsub, nzsub, diag, rhs);

    // transfer results from rhs to x (recognizing that rhs
    // arrays are offset by 1)
    --x; --rhs; --invp;
    for (int i = 1; i <= nrows; i++)
    {
        x[i] = rhs[invp[i]];
    }
    ++x; ++rhs; ++invp;
    return -1;
}

//-----------------------------------------------------------------------------

void SparspakSolver::reset()
{
    memset(diag, 0, (nrows)*sizeof(double));
    memset(lnz,  0, (nnzl)*sizeof(double));
    memset(rhs,  0, (nrows)*sizeof(double));
}

//-----------------------------------------------------------------------------

double SparspakSolver::getDiag(int i)
{
    int k = invp[i] - 1;
    return diag[k];
}

//-----------------------------------------------------------------------------

double SparspakSolver::getOffDiag(int i)
{
    int k = xaij[i] - 1;
    return lnz[k];
}

//-----------------------------------------------------------------------------

double SparspakSolver::getRhs(int i)
{
    int k = invp[i] - 1;
    return rhs[k];
}

//-----------------------------------------------------------------------------

void SparspakSolver::setDiag(int i, double value)
{
    int k = invp[i] - 1;
    diag[k] = value;
}

//-----------------------------------------------------------------------------

void SparspakSolver::setRhs(int i, double value)
{
    int k = invp[i] - 1;
    rhs[k] = value;
}

//-----------------------------------------------------------------------------

void SparspakSolver::addToDiag(int i, double value)
{
    int k = invp[i] - 1;
    diag[k] += value;
}

//-----------------------------------------------------------------------------

void SparspakSolver::addToOffDiag(int j, double value)
{
    int k = xaij[j] - 1;
    lnz[k] += value;
}

//-----------------------------------------------------------------------------

void SparspakSolver::addToRhs(int i, double value)
{
    int k = invp[i] - 1;
    rhs[k] += value;
}

//=============================================================================

//  Store the matrix non-zero structure in a set of compressed adjacency lists
//  adjncy, where xadj has pointers to the starting index of each list in adjncy.

int compress(int n, int nnz, int* xrow, int* xcol, int* xadj, int* adjncy,
             int* xaij)
{
    int  flag = 0;
    int  *xadj2 = 0;
    int  *adjncy2 = 0;
    int  *nz = 0;

    // ... allocate memory
    xadj2 = new int[n+1];
    adjncy2 = new int[2*nnz];
    nz = new int[n];
    if ( xadj2 && adjncy2 && nz )
    {
        // ... build adjacency lists for the columns of A
        //     (for each column, store the rows indexes with non-zero coeffs.)
        buildAdjncy(n, nnz, xrow, xcol, xadj, adjncy, adjncy2, xaij, nz);

        // ... sort entries stored in each adjacency list
        sortAdjncy(n, nnz, xadj, adjncy, xadj2, adjncy2, nz);

        // ... re-label all row/col indexes to be 1-based
        //     (since the original Sparspak was written in Fortran)
        for (int i = 0; i < 2*nnz; i++) adjncy[i]++;
        for (int i = 0; i <= n; i++) xadj[i]++;
        flag = 1;
    }
    delete [] xadj2;
    delete [] adjncy2;
    delete [] nz;
    return flag;
}

//-----------------------------------------------------------------------------

//  Save the column index of each non-zero coefficient in a list for each row.

void buildAdjncy(
        int n, int nnz, int* xrow, int* xcol, int* xadj,
        int* adjncy, int* adjncy2, int* xaij, int* nz)
{
    int i, j, k, m, dup = 0;

    // ... use adjncy to temporarily store non-duplicate coeffs.
    //     (parallel links create duplicates)
    int* nondup = adjncy;

    // ... count number of off-diagonal coeffs. in each column of A
    for (i = 0; i < n; i++) nz[i] = 0;
    for (k = 0; k < nnz; k++)
    {
        nz[xrow[k]]++;
        nz[xcol[k]]++;
    }

    // ... initialize adjncy2 to -1 (signifying an empty entry)
    for (i = 0; i < 2*nnz; i++) adjncy2[i] = -1;


    // ... make xadj array point to location in adjncy array where
    //     adjacency list for each column begins
    xadj[0] = 0;
    for (i = 0; i < n; i++)
    {
        xadj[i+1] = xadj[i] + nz[i];
        nz[i] = 0;
    }

    // ... fill adjncy2 array with non-zero row indexes for each column
    for (k = 0; k < nnz; k++)
    {
        i = xrow[k];
        j = xcol[k];

        // ... check for duplicate row/col
        dup = 0;
        for (m = xadj[i]; m < xadj[i]+nz[i]; m++)
        {
            if ( j == adjncy2[m] )
            {
                dup = 1;

                // ... mark xaij with negative of original coeff. index
                xaij[k] = -nondup[m];
                break;
            }
        }

        // ... if not a duplicate, add i and j to adjncy2
        if ( !dup )
        {
            m = xadj[i] + nz[i];
            adjncy2[m] = j;
            nondup[m] = k;
            nz[i]++;
            m = xadj[j] + nz[j];
            adjncy2[m] = i;
            nondup[m] = k;
            nz[j]++;
        }
    }

    // ... re-construct xadj with duplicates removed
    for (i = 0; i < n; i++) xadj[i+1] = xadj[i] + nz[i];

    // ... transfer from adjncy2 to adjncy with duplicates removed
    k = 0;
    for (i = 0; i < 2*nnz; i++)
    {
        if ( adjncy2[i] >= 0 )
        {
            adjncy[k] = adjncy2[i];
            k++;
        }
    }
}

//-----------------------------------------------------------------------------

//  Sort the column indexes stored in each row's adjacency list.

void sortAdjncy(
        int n, int nnz, int* xadj, int* adjncy, int* xadj2,
        int* adjncy2, int* nz)
{
    // ... count number of non-zeros in each row
    //     (xadj[] holds # non-zeros in each column)
    for (int j = 0; j < n; j++) nz[j] = 0;
    for (int i = 0; i < n; i++)
    {
        for (int k = xadj[i]; k < xadj[i+1]; k++)
        {
            int j = adjncy[k];
            nz[j]++;
        }
    }

    // ... fill xadj2 with cumulative # non-zeros in each row
    xadj2[0] = 0;
    for (int i = 0; i < n; i++)
    {
        xadj2[i+1] = xadj2[i] + nz[i];
    }

    // ... transpose adjncy twice to order column indices
    transpose(n, xadj, adjncy, xadj2, adjncy2, nz);
    transpose(n, xadj2, adjncy2, xadj, adjncy, nz);
}

//-----------------------------------------------------------------------------

void transpose(int n, int* xadj1, int* adjncy1, int* xadj2, int* adjncy2, int* nz)
{
     for (int j = 0; j < n; j++) nz[j] = 0;
     for (int i = 0; i < n; i++)
     {
         for (int k = xadj1[i]; k < xadj1[i+1]; k++)
         {
             int j = adjncy1[k];
             int kk = xadj2[j] + nz[j];
             adjncy2[kk] = i;
             nz[j]++;
         }
     }
}

//-----------------------------------------------------------------------------

//  Apply the Multiple Minimum Degree algorithm to re-order the rows of the
//  matrix to minimize the amount of fill-in when the matrix is factorized.

int reorder(int n, int* xadj, int* adjncy, int* perm, int* invp, int& nnzl)
{
    // ... make a copy of the adjacency list
    int nnz2 = xadj[n];
    int* adjncy2 = new int[nnz2];
    if ( ! adjncy2 ) return 0;
    for (int i = 0; i < nnz2; i++)
    {
        adjncy2[i] = adjncy[i];
    }

    // ... create work arrays for row re-ordering
    int flag = 0;
    int *qsize = 0;
    int *llist = 0;
    int *marker = 0;
    int *dhead = 0;
    qsize = new int[n];
    llist = new int[n];
    marker = new int[n];
    dhead = new int[n];
    if ( qsize && llist && marker && dhead )
    {
        // ... call Sparspak sp_genmmd to apply multiple
        //     minimum degree re-ordering to A
        int delta = -1;
        int nofsub = 0;
        int maxint = std::numeric_limits<int>::max();
        sp_genmmd(&n, xadj, adjncy2, invp, perm, &delta, dhead, qsize,
                  llist, marker, &maxint, &nofsub);
        nnzl = nofsub;
        flag = 1;
    }

    // ... delete work arrays
    delete [] adjncy2;
    delete [] qsize;
    delete [] llist;
    delete [] marker;
    delete [] dhead;
    return flag;
}

//-----------------------------------------------------------------------------

//  Symbolically factorize the matrix

int factorize(
        int n, int& nnzl, int* xadj, int* adjncy, int* perm,
        int* invp, int* xlnz, int* xnzsub, int* nzsub)
{
    // ... create work arrays
    int flag = 0;
    int *mrglnk = 0;
    int *rchlnk = 0;
    int *marker = 0;
    mrglnk = new int[n];
    rchlnk = new int[n];
    marker = new int[n];

    // ... call Sparspak sp_smbfct routine
    if ( mrglnk && rchlnk && marker )
    {
        int maxlnz, maxsub = nnzl;
        sp_smbfct(n, xadj, adjncy, perm, invp, xlnz, maxlnz, xnzsub,
                  nzsub, maxsub, mrglnk, rchlnk, marker, flag);

        // ... update nnzl with size needed for lnz
        nnzl = maxlnz;

        // ... a return flag > 0 indicates insufficient memory;
        //     convert it to an error flag
        if ( flag > 0 ) flag = 0;
        else flag = 1;
    }
    delete [] mrglnk;
    delete [] rchlnk;
    delete [] marker;
    return flag;
}

//-----------------------------------------------------------------------------

//  Map the original off-diagonal coeffs. of the matrix to its factorized form.

void aij2lnz(
        int nnz, int* xrow, int* xcol, int* invp, int* xlnz, int* xnzsub,
        int* nzsub, int* xaij)
{
    int i, j, ksub;

    // ... adjust arrays for non-zero offset
    --xlnz; --xnzsub; --nzsub;

    // ... examine each non-zero coefficient
    for (int m = 0; m < nnz; m++)
    {
        // ... skip coeff. if it is marked as being a duplicate
        if ( xaij[m] < 0 ) continue;

        // ... determine its offset row & column below the diagonal
        //     (j is a column index and i is a row index with i > j)
        i = invp[xrow[m]];   // these return indexes starting from 1
        j = invp[xcol[m]];
        if ( i < j )
        {
            ksub = j;
            j = i;
            i = ksub;
        }

        // ... search for row index in nzsub
        ksub = xnzsub[j];
        for (int k = xlnz[j]; k < xlnz[j+1]; k++)
        {
            if ( nzsub[ksub] == i )
            {
                xaij[m] = k;
                break;
            }
            ksub++;
        }
    }

    // ... map any duplicate coeffs. (marked by the negative
    //     of the coeff. index they duplicate)
    for (int m = 0; m < nnz; m++)
    {
        if ( xaij[m] < 0 ) xaij[m] = xaij[-xaij[m]];
    }

    // ... reset arrays for zero offset
    ++xlnz; ++xnzsub; ++nzsub;
}
