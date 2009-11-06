#include "mod2.h"

#ifdef HAVE_MINOR

#include "structs.h"
#include "polys.h"
#include "ideals.h"
#include <Cache.h>
#include <Minor.h>
#include <MinorProcessor.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include "TestMinors.h"
#include <stdio.h>
#include "PrettyPrinter.h"
#include "febase.h"

using namespace std;

poly zeroPoly = pISet(0);
int nonZeroCounter = 0;

void testOneMinor(PrettyPrinter& prpr, string testHeader, int rowCount, int columnCount, int entryBound, int zeroPercentage,
                  int minorSize, int randomSeed, int cacheEntries, int cacheWeight);
void testAllMinors(PrettyPrinter& prpr, string testHeader, int rowCount, int columnCount, int entryBound, int zeroPercentage,
                   int minorRows, int minorColumns, int minorSize, int randomSeed, int cacheEntries, int cacheWeight);
void testAllMinorsUntil(PrettyPrinter& prpr, string testHeader, int rowCount, int columnCount, int entryBound, int zeroPercentage,
                        int minorSize, int randomSeed, int cacheEntries, int cacheWeight, int targetMinor,
                        bool checkForEquality, int maxLoops);

void minorUsageInfo()
{
  PrintS("\nType 'system(\"minors\", 0);' to run 5 default tests with a random");
  PrintS("\ninteger matrix. This test, for which no ring needs to be declared");
  PrintS("\nbeforehand, will generate the file 'minor_output_results_ints.txt'");
  PrintS("\nincluding all results and runtimes, and a much more detailed file");
  PrintS("\n'minor_output_complete_ints.txt' (both in the folder of the SIN-");
  PrintS("\nGULAR executable).");
  PrintS("\n\nType 'system(\"minors\", m, k, strategies, nCache, wCache,");
  PrintS("\n             dumpMinors, dumpResults, dumpComplete, dumpConsole);'");
  PrintS("\nto compute all k x k minors of the poly matrix m. Depending on the");
  PrintS("\ninteger 'strategies', e.g. = '420', these tests will first be per-");
  PrintS("\nformed without a cache (due to lowest digit '0') and afterwards");
  PrintS("\nwith a cache, deploying the strategies '2' and '4'. (There are five");
  PrintS("\nstrategies available; always sort strategies from highest to lowest,");
  PrintS("\ni.e., use '5210' instead of '2501'.");
  PrintS("\nWhen a cache is used, it has a maximum number of 'nCache' entries");
  PrintS("\nand a maximum weight of 'wCache'. (The weight of the cache equals");
  PrintS("\nthe total number of monomials counted over all cached polynomials.)");
  PrintS("\nNote that a ring needs to be defined beforehand, and the poly matrix");
  PrintS("\nm. Set 'dumpMinors' to 1 in order to write all non-zero minors to");
  PrintS("\nthe file 'non_zero_poly_minors.txt'. When 'dumpResults' equals 1,");
  PrintS("\nan additional file 'minor_output_results_polys.txt' will be written");
  PrintS("\nwhich contains an overview over all results and runtimes. Set");
  PrintS("\n'dumpComplete' to 1 in order to obtain a very detailed file output");
  PrintS("\nin 'minor_output_complete_polys.txt'. (All files will be created in");
  PrintS("\nthe folder of the SINGULAR executable.)");
  PrintS("\nSet 'dumpConsole' to 1 in order to write everything also to the con-");
  PrintS("\nsole.");
  PrintS("\n\nType 'ideal i = system(\"minors\", m, k, strategy, nCache, wCache);'");
  PrintS("\nto obtain the ideal generated by all k x k non-zero minors of the");
  PrintS("\npoly matrix m. Note again that both ring and matrix must be defined");
  PrintS("\nbeforehand. (Note that no checks for duplicate ideal generators will");
  PrintS("\nbe performed.) With 'strategy' == 0, the computation will be run with-");
  PrintS("\nout cache; otherwise a cache with the respective strategy (1, ..., 5)");
  PrintS("\nwill be used. In this case, the cache will have at most 'nCache' en-");
  PrintS("\ntries, i.e. cached sub-minors, and a maximum weight of 'wCache'. (The");
  PrintS("\nweight of the cache equals the total number of monomials counted over");
  PrintS("\nall cached polynomials.)");
  PrintS("\nFor this command, there will be no file or console outputs.");
  PrintS("\n\nType 'ideal i = system(\"minors\", m, k, strategy, nCache, wCache, ch);'");
  PrintS("\nto obtain the ideal generated by all k x k non-zero minors of the");
  PrintS("\ninteger matrix m modulo ch. (For the remaining parameters, see the");
  PrintS("\nprevious call.)\n\n");
}

/**
* A method for obtaining a random matrix with specified properties.
* The returned structure is 1-dimensional, i.e., the matrix from
* top to bottom, left to right.
*/
void fillRandomMatrix(const int rowCount, const int columnCount, const int randomSeed,
                      const int zeroPercentage, const int entryBound, int* theMatrix) {
    int theSize = rowCount * columnCount;
    siSeed = randomSeed; // random seed for ensuring reproducability of experiments
    for (int i = 0; i < theSize; i++) {
        if ((siRand() % 100) < zeroPercentage)
            theMatrix[i] = 0;
        else
            theMatrix[i] = 1 + (siRand() % entryBound); // ranges from 1 to entryBound, including both
    }
}

void writeTheMinorIfNonZero(PrettyPrinter& pm, const PolyMinorValue& pmv) {
    if (!pEqualPolys(pmv.getResult(), zeroPoly))
    {
      nonZeroCounter++;
      +pm < nonZeroCounter < ". " < pString(pmv.getResult());
    }
}

/**
* A method for testing the implementation.<br>
* All intermediate and final results will be printed to both the console and
* into files with the names specified by filenameForCompleteOutput and
* filenameForResultOutput, which both reside in the path of the compiled executable.
* @param argc not used
* @param *argv[] not used
*/
int testIntMinors (const int dummy) {
  // for output of non-zero minors into file
  PrettyPrinter prpr("minor_output_complete_ints.txt", "minor_output_results_ints.txt", false, false, -1, "   ");

  // computes just one minor:
  testOneMinor(prpr, "Test I", 7, 10, 50, 20, 5, 471, 70, 1000);
  +prpr; +prpr;

  // computes all (1470) minors of a specified size (6x6):
  testAllMinors(prpr, "Test II", 7, 10, 50, 20, 7, 10, 6, 471, 200, 10000);
  +prpr; +prpr;

  // looks for minor == 92868; to this end, it needs to compute 1632 minors:
  testAllMinorsUntil(prpr, "Test III", 100, 60, 10, 10, 6, 471, 300, 10000, 92868, true, 10000);
  +prpr; +prpr;

  // looks for the first non-zero minor (6x6); to this end, it needs to compute 229 minors:
  testAllMinorsUntil(prpr, "Test IV", 100, 60, 10, 75, 6, 4712, 300, 10000, 0, false, 10000);
  +prpr; +prpr;

  // looks for minor == -43065; to this end, it needs to compute 23 minors:
  testAllMinorsUntil(prpr, "Test V", 100, 60, 10, 10, 6, 471, 300, 10000, -43065, true, 10000);

  return 0;
}

void testStuff (const poly p)
{
  PrintLn(); PrintS("poly = "); PrintS(pString(p));
  PrintLn(); PrintS("length of poly = ");
  char h[10];
  sprintf(h, "%d", pLength(p));
  PrintS(h);
}

int testAllPolyMinors(matrix mat, int minorSize, int strategies, int cacheEntries, int cacheWeight,
                      int dumpMinors, int dumpResults, int dumpComplete, int dumpConsole) {
  // for pretty printing and file output of results and runtimes
  PrettyPrinter prpr(dumpComplete == 1 ? "minor_output_complete_polys.txt" : "",
                     dumpResults == 1 ? "minor_output_results_polys.txt" : "",
                     false, false, dumpConsole == 1 ? 0 : -1, "   ");

  // for output of non-zero minors into file
  PrettyPrinter pm(dumpMinors == 1 ? "non_zero_poly_minors.txt" : "", "", false, false, -1, "   ");

  int rowCount = mat->nrows;
  int columnCount = mat->ncols;
  clock_t totalTimeStart, totalTime, printTimeStart, printTime;
  string testHeader = "COMPUTE ALL MINORS IN A POLY MATRIX";

  prpr < testHeader;
  +prpr; for (int i = 0; i < int(testHeader.size()); i++) prpr < "="; // underlines the header string
  +prpr < "Testing the successive computation of all minors of a given size without and with cache, respectively.";
  +prpr < "In the case of computing with cache, different caching strategies may be deployed.";
  +prpr < "The provided matrix is expected to have SINGULAR polys as entries.";

  poly* myPolyMatrix = (poly*)(mat->m);
  PolyMinorProcessor mp;
  mp.defineMatrix(rowCount, columnCount, myPolyMatrix);

  /* The next lines are for defining the sub-matrix of myPolyMatrix
     from which we want to compute all k x k - minors.
     In the given setting, we want the entire matrix to form the sub-matrix. */
  int minorRows = rowCount;
  int minorColumns = columnCount;
  int myRowIndices[minorRows];  for (int i = 0; i < minorRows; i++) myRowIndices[i] = i;
  int myColumnIndices[minorColumns]; for (int i = 0; i < minorColumns; i++) myColumnIndices[i] = i;

  // setting sub-matrix and size of minors of interest within that sub-matrix:
  mp.defineSubMatrix(minorRows, myRowIndices, minorColumns, myColumnIndices);
  mp.setMinorSize(minorSize);

  // define the cache:
  Cache<MinorKey, PolyMinorValue> cch(cacheEntries, cacheWeight);

  // container for all upcoming results
  PolyMinorValue theMinor;

  // counters...
  int k = 1;
  int totalMultiplications = 0;
  int totalAdditions = 0;
  int totalMultiplicationsAccumulated = 0;
  int totalAdditionsAccumulated = 0;

  // target for retrieving and writing momentary row and column indices:
  int myIndexArray[32];

  if (strategies % 10 == 0)
  {
    strategies = strategies / 10;
    +prpr; +prpr < "Results - " < testHeader < " - no cache";
    +prpr < "computing all minors of size " < minorSize < "x" < minorSize;

    pm < "non-zero minors - no cache\n==========================";
    nonZeroCounter = 0;

    printTime = 0; totalTimeStart = clock();
    // iteration over all minors of size "minorSize x minorSize"
    while (mp.hasNextMinor()) {
        // retrieving the minor:
        theMinor = mp.getNextMinor();
        printTimeStart = clock();
        writeTheMinorIfNonZero(pm, theMinor);

        // updating counters:
        totalMultiplications += theMinor.getMultiplications();
        totalAdditions += theMinor.getAdditions();
        totalMultiplicationsAccumulated += theMinor.getAccumulatedMultiplications();
        totalAdditionsAccumulated += theMinor.getAccumulatedAdditions();
  
        // writing current row indices:
        mp.getCurrentRowIndices(myIndexArray);
        +prpr; prpr < k++ < ". minor (rows: ";
        for (int i = 0; i < minorSize; i++) {
            if (i != 0) prpr < ", ";
            prpr < myIndexArray[i];
        };
  
        // writing current column indices:
        mp.getCurrentColumnIndices(myIndexArray);
        prpr < "; columns: ";
        for (int i = 0; i < minorSize; i++) {
            if (i != 0) prpr < ", ";
            prpr < myIndexArray[i];
        };
        prpr < ") = ";
  
        // write the actual value of the minor:
        prpr < theMinor.toString();
        printTime += clock() - printTimeStart;
    };
    totalTime = clock() - totalTimeStart - printTime;

    // writing summarized information
    ++prpr; ++prpr << "Operation counters - " << testHeader << " - no cache";
    ++prpr << "performed in total " << totalMultiplications << " multiplications and ";
    prpr << totalAdditions << " additions";
    ++prpr << "number of non-zero minors = " << nonZeroCounter;
    ++prpr << "(time = " << ((totalTime * 1000) / CLOCKS_PER_SEC) << " msec)";
  }

  for (int strategy = 1; strategy <= 5; strategy++) {
    if (strategies % 10 == strategy)
    {
      strategies = strategies / 10;
      // setting sub-matrix, size of minors of interest within that sub-matrix, and strategy:
      mp.defineSubMatrix(minorRows, myRowIndices, minorColumns, myColumnIndices);
      mp.setMinorSize(minorSize);
      MinorValue::SetRankingStrategy(strategy);
  
      // counters...
      k = 1;
      totalMultiplications = 0;
      totalAdditions = 0;
      totalMultiplicationsAccumulated = 0;
      totalAdditionsAccumulated = 0;
  
      // cleaning up and redefinition of the cache:
      cch.clear();
      Cache<MinorKey, PolyMinorValue> cch(cacheEntries, cacheWeight);
  
      +prpr; +prpr <  "Results - " < testHeader < " - using cache - deploying caching strategy #" < strategy;
      +prpr < "computing all minors of size " < minorSize < "x" < minorSize;

      pm < "\n\nnon-zero minors - using cache - deploying caching strategy #" < strategy
         < "\n=============================================================";
      nonZeroCounter = 0;
  
      printTime = 0; totalTimeStart = clock();
      // iteration over all minors of size "minorSize x minorSize"
      while (mp.hasNextMinor()) {
          // retrieving the minor:
          theMinor = mp.getNextMinor(cch);
          printTimeStart = clock();
          writeTheMinorIfNonZero(pm, theMinor);
  
          // updating counters:
          totalMultiplications += theMinor.getMultiplications();
          totalAdditions += theMinor.getAdditions();
          totalMultiplicationsAccumulated += theMinor.getAccumulatedMultiplications();
          totalAdditionsAccumulated += theMinor.getAccumulatedAdditions();

          // writing current row indices:
          mp.getCurrentRowIndices(myIndexArray);
          +prpr < k++ < ". minor (rows: ";
          for (int i = 0; i < minorSize; i++) {
              if (i != 0) prpr < ", ";
              prpr < myIndexArray[i];
          };
  
          // writing current column indices:
          mp.getCurrentColumnIndices(myIndexArray);
          prpr < "; columns: ";
          for (int i = 0; i < minorSize; i++) {
              if (i != 0) prpr < ", ";
              prpr < myIndexArray[i];
          };
          prpr < ") = ";
  
          // writeMinor the actual value of the minor:
          prpr < theMinor.toString();
          printTime += clock() - printTimeStart;
      };
      totalTime = clock() - totalTimeStart - printTime;
  
      // writing summarized information
      ++prpr; ++prpr << "Operation counters - " << testHeader << " - using cache - deploying caching strategy #" << strategy;
      ++prpr << "performed in total " << totalMultiplications << " multiplications and ";
      prpr << totalAdditions << " additions";
      ++prpr << "(computation without reuse would need " << totalMultiplicationsAccumulated << " and ";
      prpr << totalAdditionsAccumulated << " additions)";
      ++prpr << "number of non-zero minors = " << nonZeroCounter;
      ++prpr << "(time = " << ((totalTime * 1000) / CLOCKS_PER_SEC) << " msec)";
      +prpr < "The cache looks like this:";
      +prpr < cch.toString();
    }
  }

  return 0;
}

ideal testAllPolyMinorsAsIdeal(matrix mat, int minorSize, int strategy, int cacheEntries, int cacheWeight)
{
    // counters + auxiliary stuff
  int totalMultiplications = 0;
  int totalAdditions = 0;
  int totalMultiplicationsAccumulated = 0;
  int totalAdditionsAccumulated = 0;
  char h[30];

  int rowCount = mat->nrows;
  int columnCount = mat->ncols;

  poly* myPolyMatrix = (poly*)(mat->m);
  PolyMinorProcessor mp;
  mp.defineMatrix(rowCount, columnCount, myPolyMatrix);

  /* The next lines are for defining the sub-matrix of myPolyMatrix
     from which we want to compute all k x k - minors.
     In the given setting, we want the entire matrix to form the sub-matrix. */
  int minorRows = rowCount;
  int minorColumns = columnCount;
  int myRowIndices[minorRows];  for (int i = 0; i < minorRows; i++) myRowIndices[i] = i;
  int myColumnIndices[minorColumns]; for (int i = 0; i < minorColumns; i++) myColumnIndices[i] = i;

  // setting sub-matrix and size of minors of interest within that sub-matrix:
  mp.defineSubMatrix(minorRows, myRowIndices, minorColumns, myColumnIndices);
  mp.setMinorSize(minorSize);

  // containers for all upcoming results
  PolyMinorValue theMinor;
  poly po = NULL;
  ideal iii = idInit(1, 0);

  if (strategy == 0)
  {
    PrintLn(); PrintS("new code uses no cache");
    // iteration over all minors of size "minorSize x minorSize"
    while (mp.hasNextMinor()) {
      // retrieving the minor:
      theMinor = mp.getNextMinor();
      po = theMinor.getResult();
      totalMultiplicationsAccumulated += theMinor.getAccumulatedMultiplications();
      totalMultiplications += theMinor.getMultiplications();
      totalAdditionsAccumulated += theMinor.getAccumulatedAdditions();
      totalAdditions += theMinor.getAdditions();
      idInsertPoly(iii, pCopy(po)); // will include po only if it is not the zero polynomial
    }
  }
  else
  {
    PrintLn(); PrintS("new code uses cache with caching strategy ");
    sprintf(h, "%d", strategy); PrintS(h);
    MinorValue::SetRankingStrategy(strategy);
    Cache<MinorKey, PolyMinorValue> cch(cacheEntries, cacheWeight);
    // iteration over all minors of size "minorSize x minorSize"
    while (mp.hasNextMinor()) {
      // retrieving the minor:
      theMinor = mp.getNextMinor(cch);
      po = theMinor.getResult();
      totalMultiplicationsAccumulated += theMinor.getAccumulatedMultiplications();
      totalMultiplications += theMinor.getMultiplications();
      totalAdditionsAccumulated += theMinor.getAccumulatedAdditions();
      totalAdditions += theMinor.getAdditions();
      idInsertPoly(iii, pCopy(po)); // will include po only if it is not the zero polynomial
    }
  }
  idSkipZeroes(iii);  // remove zero generators (resulting from block-wise allocation of memory)
  PrintLn(); PrintS("numbers of performed operations");
  PrintLn(); PrintS("   polynomial-to-polynomial multiplications: ");
  sprintf(h, "%d", totalMultiplications); PrintS(h);
  PrintLn(); PrintS("   polynomial-to-polynomial additions: ");
  sprintf(h, "%d", totalAdditions); PrintS(h);
  PrintLn(); PrintS("   (polynomial-to-polynomial multiplications without cache would be: ");
  sprintf(h, "%d", totalMultiplicationsAccumulated); PrintS(h); PrintS(")");
  PrintLn(); PrintS("   (polynomial-to-polynomial additions without cache would be: ");
  sprintf(h, "%d", totalAdditionsAccumulated); PrintS(h); PrintS(")");
  PrintLn(); PrintLn();
  return iii;
}

ideal testAllIntMinorsAsIdeal(matrix mat, int minorSize, int strategy, int cacheEntries, int cacheWeight, int characteristic)
{
    // counters + auxiliary stuff
  int totalMultiplications = 0;
  int totalAdditions = 0;
  int totalMultiplicationsAccumulated = 0;
  int totalAdditionsAccumulated = 0;
  char h[30];

  int rowCount = mat->nrows;
  int columnCount = mat->ncols;

  poly* myPolyMatrix = (poly*)(mat->m);
  int theSize = rowCount * columnCount;
  int myIntMatrix[theSize]; int vv;
  for (int i = 0; i < theSize; i++)
  {
    vv = 0;
    if (myPolyMatrix[i] != NULL)
    {
      vv = n_Int(pGetCoeff(myPolyMatrix[i]), currRing);
      if (characteristic != 0) vv = vv % characteristic;
    }
    myIntMatrix[i] = vv;
  }

  IntMinorProcessor mp;
  mp.defineMatrix(rowCount, columnCount, myIntMatrix);

  /* The next lines are for defining the sub-matrix of myIntMatrix
     from which we want to compute all k x k - minors.
     In the given setting, we want the entire matrix to form the sub-matrix. */
  int minorRows = rowCount;
  int minorColumns = columnCount;
  int myRowIndices[minorRows];  for (int i = 0; i < minorRows; i++) myRowIndices[i] = i;
  int myColumnIndices[minorColumns]; for (int i = 0; i < minorColumns; i++) myColumnIndices[i] = i;

  // setting sub-matrix and size of minors of interest within that sub-matrix:
  mp.defineSubMatrix(minorRows, myRowIndices, minorColumns, myColumnIndices);
  mp.setMinorSize(minorSize);

  // containers for all upcoming results
  IntMinorValue theMinor;
  int theValue;
  ideal iii = idInit(1, 0);

  if (strategy == 0)
  {
    PrintLn(); PrintS("new code uses no cache");
    // iteration over all minors of size "minorSize x minorSize"
    while (mp.hasNextMinor()) {
      // retrieving the minor:
      theMinor = mp.getNextMinor(characteristic);
      theValue = theMinor.getResult();
      totalMultiplicationsAccumulated += theMinor.getAccumulatedMultiplications();
      totalMultiplications += theMinor.getMultiplications();
      totalAdditionsAccumulated += theMinor.getAccumulatedAdditions();
      totalAdditions += theMinor.getAdditions();
      idInsertPoly(iii, pISet(theValue)); // will include theValue only if it is not zero
    }
  }
  else
  {
    PrintLn(); PrintS("new code uses cache with caching strategy ");
    sprintf(h, "%d", strategy); PrintS(h);
    MinorValue::SetRankingStrategy(strategy);
    Cache<MinorKey, IntMinorValue> cch(cacheEntries, cacheWeight);
    // iteration over all minors of size "minorSize x minorSize"
    while (mp.hasNextMinor()) {
      // retrieving the minor:
      theMinor = mp.getNextMinor(cch, characteristic);
      theValue = theMinor.getResult();
      totalMultiplicationsAccumulated += theMinor.getAccumulatedMultiplications();
      totalMultiplications += theMinor.getMultiplications();
      totalAdditionsAccumulated += theMinor.getAccumulatedAdditions();
      totalAdditions += theMinor.getAdditions();
      idInsertPoly(iii, pISet(theValue)); // will include theValue only if it is not zero
    }
  }
  idSkipZeroes(iii);  // remove zero generators (resulting from block-wise allocation of memory)

  PrintLn(); PrintS("numbers of performed operations");
  PrintLn(); PrintS("   multiplications: ");
  sprintf(h, "%d", totalMultiplications); PrintS(h);
  PrintLn(); PrintS("   additions: ");
  sprintf(h, "%d", totalAdditions); PrintS(h);
  PrintLn(); PrintS("   (multiplications without cache would be: ");
  sprintf(h, "%d", totalMultiplicationsAccumulated); PrintS(h); PrintS(")");
  PrintLn(); PrintS("   (additions without cache would be: ");
  sprintf(h, "%d", totalAdditionsAccumulated); PrintS(h); PrintS(")");
  PrintLn(); PrintLn();

  return iii;
}

/**
* A method for testing the computation of one minor; without and with cache, respectively.<br>
* All results should be equal no matter whether we do or do not use a cache. Neither should
* the cache strategy influence the mathematical value of the minor.
*/
void testOneMinor(PrettyPrinter& prpr, string testHeader, int rowCount, int columnCount, int entryBound, int zeroPercentage,
                  int minorSize, int randomSeed, int cacheEntries, int cacheWeight) {
    int start, end;

    prpr < testHeader;
    +prpr; for (int i = 0; i < int(testHeader.size()); i++) prpr < "="; /* underlines the header string */
    +prpr < "Testing the computation of one minor without and with cache, respectively.";
    +prpr < "For computing with cache, 5 different caching strategies will be deployed.";

    int* myMatrix = new int[rowCount * columnCount];
    fillRandomMatrix(rowCount, columnCount, randomSeed, zeroPercentage, entryBound, myMatrix);

    IntMinorProcessor mp;
    mp.defineMatrix(rowCount, columnCount, myMatrix);

    int myRowIndices[minorSize];  for (int i = 0; i < minorSize; i++) myRowIndices[i] = i;
    int myColumnIndices[minorSize]; for (int i = 0; i < minorSize; i++) myColumnIndices[i] = columnCount - minorSize + i;

    // We would like to printout mp. For that, we need to provide complete information of
    // what minors we intend to compute later on.
    mp.defineSubMatrix(minorSize, myRowIndices, minorSize, myColumnIndices);
    mp.setMinorSize(minorSize);

    +prpr; +prpr < mp.toString(); +prpr; +prpr;

    // compute the minor without cache:
    prpr << "Results - " << testHeader << " - no cache";
    start = clock();
    IntMinorValue mv = mp.getMinor(minorSize, myRowIndices, myColumnIndices, 0);
    end = clock();
    ++prpr << "value of minor = " << mv.toString();
    ++prpr << "(time = " << (end - start) << " msec)";

    // define the cache:
    Cache<MinorKey, IntMinorValue> cch = Cache<MinorKey, IntMinorValue>(cacheEntries, cacheWeight);

    // compute minor using the cache, for all implemented caching strategies:
    for (int strategy = 1; strategy <= 5; strategy++) {
        // clear cache:
        cch.clear();
        mp.defineSubMatrix(minorSize, myRowIndices, minorSize, myColumnIndices);

        // compute the minor using the cache and current strategy
        IntMinorValue::SetRankingStrategy(strategy);
        start = clock();
        mv = mp.getMinor(minorSize, myRowIndices, myColumnIndices, cch, 0);
        end = clock();

        ++prpr; ++prpr << "Results - " << testHeader;
        prpr << " - using cache - deploying caching strategy #" << strategy;
        ++prpr << "value of minor = " << mv.toString();
        ++prpr << "(time = " << (end - start) << " msec)";
        +prpr < "The cache looks like this:";
        +prpr < cch.toString();
    }
    delete [] myMatrix;
}

/**
* A method for testing the computation of all minors of a given size within a pre-defined
* sub-matrix of an underlying matrix.<br>
* Again, we do this first without cache, and later using a cache, respectively.<br>
* All results should be equal no matter whether we do or do not use a cache. Neither should the cache strategy
* influence the mathematical value of the minor.
*/
void testAllMinors(PrettyPrinter& prpr, string testHeader, int rowCount, int columnCount, int entryBound, int zeroPercentage,
                   int minorRows, int minorColumns, int minorSize, int randomSeed, int cacheEntries, int cacheWeight) {
    clock_t totalTimeStart, totalTime, printTimeStart, printTime;

    prpr < testHeader;
    +prpr; for (int i = 0; i < int(testHeader.size()); i++) prpr < "="; // underlines the header string
    +prpr < "Testing the successive computation of all minors of a given size without and with cache, respectively.";
    +prpr < "In the case of computing with cache, 5 different caching strategies will be deployed.";

    int* myMatrix = new int[rowCount * columnCount];
    fillRandomMatrix(rowCount, columnCount, randomSeed, zeroPercentage, entryBound, myMatrix);

    IntMinorProcessor mp;
    mp.defineMatrix(rowCount, columnCount, myMatrix);

    int myRowIndices[minorRows];  for (int i = 0; i < minorRows; i++) myRowIndices[i] = i;
    int myColumnIndices[minorColumns]; for (int i = 0; i < minorColumns; i++) myColumnIndices[i] = columnCount - minorColumns + i;

    // setting sub-matrix and size of minors of interest within that sub-matrix:
    mp.defineSubMatrix(minorRows, myRowIndices, minorColumns, myColumnIndices);
    mp.setMinorSize(minorSize);

    +prpr; +prpr < mp.toString();

    // define the cache:
    Cache<MinorKey, IntMinorValue> cch = Cache<MinorKey, IntMinorValue>(cacheEntries, cacheWeight);

    // container for all upcoming results
    IntMinorValue theMinor;

    // counters...
    int k = 1;
    int totalMultiplications = 0;
    int totalAdditions = 0;
    int totalMultiplicationsAccumulated = 0;
    int totalAdditionsAccumulated = 0;

    // target for retrieving and writing momentary row and column indices:
    int myIndexArray[32];

    +prpr; +prpr < "Results - " < testHeader < " - no cache";
    +prpr < "computing all minors of size " < minorSize < "x" < minorSize;

    printTime = 0; totalTimeStart = clock();
    // iteration over all minors of size "minorSize x minorSize"
    while (mp.hasNextMinor()) {
        // retrieving the minor:
        theMinor = mp.getNextMinor(0);
        printTimeStart = clock();

        // updating counters:
        totalMultiplications += theMinor.getMultiplications();
        totalAdditions += theMinor.getAdditions();
        totalMultiplicationsAccumulated += theMinor.getAccumulatedMultiplications();
        totalAdditionsAccumulated += theMinor.getAccumulatedAdditions();

        // writing current row indices:
        mp.getCurrentRowIndices(myIndexArray);
        +prpr < k++ < ". minor (rows: ";
        for (int i = 0; i < minorSize; i++) {
            if (i != 0) prpr < ", ";
            prpr < myIndexArray[i];
        };

        // writing current column indices:
        mp.getCurrentColumnIndices(myIndexArray);
        prpr < "; columns: ";
        for (int i = 0; i < minorSize; i++) {
            if (i != 0) prpr < ", ";
            prpr < myIndexArray[i];
        };
        prpr < ") = ";

        // write the actual value of the minor:
        prpr < theMinor.toString();
        printTime += clock() - printTimeStart;
    };
    totalTime = clock() - totalTimeStart - printTime;
    // writing summarized information
    ++prpr; ++prpr << "Operation counters - " << testHeader << " - no cache";
    ++prpr << "performed in total " << totalMultiplications << " multiplications and ";
    prpr << totalAdditions << " additions";
    ++prpr << "(time = " << ((totalTime * 1000) / CLOCKS_PER_SEC) << " msec)";

    for (int strategy = 1; strategy <= 5; strategy++) {
        // setting sub-matrix, size of minors of interest within that sub-matrix, and strategy:
        mp.defineSubMatrix(minorRows, myRowIndices, minorColumns, myColumnIndices);
        mp.setMinorSize(minorSize);
        IntMinorValue::SetRankingStrategy(strategy);

        // counters...
        k = 1;
        totalMultiplications = 0;
        totalAdditions = 0;
        totalMultiplicationsAccumulated = 0;
        totalAdditionsAccumulated = 0;

        // cleaning up and redefinition of the cache:
        cch.clear();
        cch = Cache<MinorKey, IntMinorValue>(cacheEntries, cacheWeight);

        +prpr; +prpr < "Results - " < testHeader < " - using cache - deploying caching strategy #" < strategy;
        +prpr < "computing all minors of size " < minorSize < "x" < minorSize;

        printTime = 0; totalTimeStart = clock();
        // iteration over all minors of size "minorSize x minorSize"
        while (mp.hasNextMinor()) {
            // retrieving the minor:
            theMinor = mp.getNextMinor(cch, 0);
            printTimeStart = clock();

            // updating counters:
            totalMultiplications += theMinor.getMultiplications();
            totalAdditions += theMinor.getAdditions();
            totalMultiplicationsAccumulated += theMinor.getAccumulatedMultiplications();
            totalAdditionsAccumulated += theMinor.getAccumulatedAdditions();

            // writing current row indices:
            mp.getCurrentRowIndices(myIndexArray);
            +prpr < k++ < ". minor (rows: ";
            for (int i = 0; i < minorSize; i++) {
                if (i != 0) prpr < ", ";
                prpr < myIndexArray[i];
            };

            // writing current column indices:
            mp.getCurrentColumnIndices(myIndexArray);
            prpr < "; columns: ";
            for (int i = 0; i < minorSize; i++) {
                if (i != 0) prpr < ", ";
                prpr < myIndexArray[i];
            };
            prpr < ") = ";

            // writeMinor the actual value of the minor:
            prpr < theMinor.toString();
            printTime += clock() - printTimeStart;
        };
        totalTime = clock() - totalTimeStart - printTime;
        // writing summarized information
        ++prpr; ++prpr << "Operation counters - " << testHeader << " - using cache - deploying caching strategy #" << strategy;
        ++prpr << "performed in total " << totalMultiplications << " multiplications and ";
        prpr << totalAdditions << " additions";
        ++prpr << "(computation without reuse would need " << totalMultiplicationsAccumulated << " and ";
        prpr << totalAdditionsAccumulated << " additions)";
        ++prpr << "(time = " << ((totalTime * 1000) / CLOCKS_PER_SEC) << " msec)";
        +prpr < "The cache looks like this:";
        +prpr < cch.toString();
    }
    delete [] myMatrix;
}

/**
* A method for testing the computation of all minors of a given size within a pre-defined
* sub-matrix of an underlying large matrix.<br>
* Again, we do this first without cache, and later using a cache, respectively.<br>
* All results should be equal no matter whether we do or do not use a cache. Neither should the cache strategy
* influence the mathematical value of the minor.
* zeroP: this is the probability for zero entries in the matrix;
*        all other matrix entries will range from 1 to entryBound and be equally distributed
*/
void testAllMinorsUntil(PrettyPrinter& prpr, string testHeader, int rowCount, int columnCount, int entryBound, int zeroPercentage,
                        int minorSize, int randomSeed, int cacheEntries, int cacheWeight, int targetMinor,
                        bool checkForEquality, int maxLoops) {
    clock_t totalTimeStart, totalTime, printTimeStart, printTime;

    prpr < testHeader;
    +prpr; for (int i = 0; i < int(testHeader.size()); i++) prpr < "="; // underlines the header string
    +prpr < "Testing the successive computation of minors of a given size without and with ";
    +prpr < "cache, respectively, until a minor with a certain value is found.";
    +prpr < "In the case of computing with cache, 5 different caching strategies will be deployed.";

    +prpr; +prpr < "In this test, the matrix is " < rowCount < " x " < columnCount < ".";
    +prpr < "The minor we are looking for is " < minorSize < " x " < minorSize < ", and ";
    prpr < "is supposed to have a value of ";
    if (!checkForEquality) prpr < "<> ";
    prpr < targetMinor < ".";
    +prpr < "As an upper bound for the number of loops, at most " < maxLoops < " minors will be computed.";

    int* myMatrix = new int[rowCount * columnCount];
    fillRandomMatrix(rowCount, columnCount, randomSeed, zeroPercentage, entryBound, myMatrix);

    IntMinorProcessor mp;
    mp.defineMatrix(rowCount, columnCount, myMatrix);

    int myRowIndices[rowCount]; for (int i = 0; i < rowCount; i++) myRowIndices[i] = i; // choosing all rows
    int myColumnIndices[columnCount]; for (int i = 0; i < columnCount; i++) myColumnIndices[i] = i; // choosing all columns

    // define the cache:
    Cache<MinorKey, IntMinorValue> cch = Cache<MinorKey, IntMinorValue>(cacheEntries, cacheWeight);

    // container for all upcoming results
    IntMinorValue theMinor;

    // counters...
    int k = 1;
    int totalMultiplications = 0;
    int totalAdditions = 0;
    int totalMultiplicationsAccumulated = 0;
    int totalAdditionsAccumulated = 0;

    // setting sub-matrix and size of minors of interest within that sub-matrix:
    mp.defineSubMatrix(rowCount, myRowIndices, columnCount, myColumnIndices);
    mp.setMinorSize(minorSize);

    +prpr; +prpr < mp.toString();

    +prpr < "Results - " < testHeader < " - no cache";
    +prpr < "computing all minors of size " < minorSize < "x" < minorSize;
    prpr < " until first minor with specified value is found:";

    bool minorFound = false;
    int loops = 0;
    printTime = 0; totalTimeStart = clock();
    // iteration over all minors of size "minorSize x minorSize"
    while (mp.hasNextMinor() && (!minorFound) && (loops < maxLoops)) {

        // retrieving the minor:
        theMinor = mp.getNextMinor(0);
        printTimeStart = clock();
        minorFound = (checkForEquality ? (theMinor.getResult() == targetMinor) : (theMinor.getResult() != targetMinor));

        // updating counters:
        totalMultiplications += theMinor.getMultiplications();
        totalAdditions += theMinor.getAdditions();
        totalMultiplicationsAccumulated += theMinor.getAccumulatedMultiplications();
        totalAdditionsAccumulated += theMinor.getAccumulatedAdditions();

        // writing current minor
        +prpr < k++ < ". minor = " < theMinor.getResult();

        loops++;
        printTime += clock() - printTimeStart;
    };
    totalTime = clock() - totalTimeStart - printTime;
    // writing summarized information
    ++prpr; ++prpr << "Operation counters - " << testHeader << " - no cache";
    ++prpr << "performed in total " << totalMultiplications << " multiplications and ";
    prpr << totalAdditions << " additions";
    ++prpr << "(time = " << ((totalTime * 1000) / CLOCKS_PER_SEC) << " msec)";

    for (int strategy = 1; strategy <= 5; strategy++) {
        // setting sub-matrix, size of minors of interest within that sub-matrix, and strategy:
        mp.defineSubMatrix(rowCount, myRowIndices, columnCount, myColumnIndices);
        mp.setMinorSize(minorSize);
        IntMinorValue::SetRankingStrategy(strategy);

        // counters...
        k = 1;
        totalMultiplications = 0;
        totalAdditions = 0;
        totalMultiplicationsAccumulated = 0;
        totalAdditionsAccumulated = 0;

        // cleaning up and redefinition of the cache:
        cch.clear();
        cch = Cache<MinorKey, IntMinorValue>(cacheEntries, cacheWeight);

        +prpr; +prpr < testHeader < " - using cache - deploying caching strategy #" < strategy;
        +prpr < "computing all minors of size " < minorSize < "x" < minorSize;
        prpr < " until first minor with specified value is found:";

        int loops = 0;
        bool minorFound = false;
        printTime = 0; totalTimeStart = clock();
        // iteration over all minors of size "minorSize x minorSize"
        while (mp.hasNextMinor() && (!minorFound) && (loops < maxLoops)) {

            // retrieving the minor:
            theMinor = mp.getNextMinor(cch, 0);
            printTimeStart = clock();
            minorFound = (checkForEquality ? (theMinor.getResult() == targetMinor) : (theMinor.getResult() != targetMinor));

            // updating counters:
            totalMultiplications += theMinor.getMultiplications();
            totalAdditions += theMinor.getAdditions();
            totalMultiplicationsAccumulated += theMinor.getAccumulatedMultiplications();
            totalAdditionsAccumulated += theMinor.getAccumulatedAdditions();

            // writing current minor
            +prpr < k++ < ". minor = " < theMinor.getResult();

            loops++;
            printTime += clock() - printTimeStart;
        };
        totalTime = clock() - totalTimeStart - printTime;
        // writing summarized information
        ++prpr; ++prpr << "Operation counters - " << testHeader << " - using cache - deploying caching strategy #" << strategy;
        ++prpr << "performed in total " << totalMultiplications << " multiplications and ";
        prpr << totalAdditions << " additions";
        ++prpr << "(computation without reuse would need " << totalMultiplicationsAccumulated << " and ";
        prpr << totalAdditionsAccumulated << " additions)";
        ++prpr << "(time = " << ((totalTime * 1000) / CLOCKS_PER_SEC) << " msec)";
        +prpr < "The cache has " < cch.getNumberOfEntries() < " (of max. " < cacheEntries < ") entries and a weight of ";
        prpr < cch.getWeight() < " (of max. " < cacheWeight < ").";
    }
    delete [] myMatrix;
}

#endif // HAVE_MINOR