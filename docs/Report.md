# Project Report 
# Phase 1 - Implementing Matrices

## Initial Page Design
To store the matrices as pages, we initially thought of imagining the matrix as just a linear array of N x N integers.
If a block could store say x integers (In the default configuration it's 1000/4 = 250), it would store 250 integers in
each block, regardless of if it goes into multiple rows, etc.
There were various benefits to such a page design

1. Simplicity of implementation - This would allow the page design to be very simple, and solves multiple issues like a row being too big for the given block size. This design also allows us to query the cell that we might need very easily. 
2. Internal Fragmentation - With this approach, we would have absolutely no internal fragmentation in the pages (except for maybe the last page). This page design is therefore storage efficient.

However, when we began to implement this page design though, we came across a major flaw that had to be addressed. Commands like TRANSPOSE, COMPUTE and CHECKSYMMETRY have to end up "randomly" accessing values causing a lot of page faults.

## Actual Page Design
We instead decided to go for a submatrix approach, where instead of storing integers in a linear fashion, we divide the NxN matrix into several MxM matrices. When we create a matrix, we 
compute what is the largest square submatrix that could fit in a single page, and then divide the matrix accordingly.

Of course, there will be some matrices at the edges which aren't complete squares, but we keep track of them by storing the dimensions of 
each submatrix in the matrix object.

In addition to the default page features, we also store an integer in pages that indicates if a page is "dirty" or not.
Several of the commands implemented change the values in pages, and to simplify the flow of saving the pages to disk,
we have a dirty bit indicating that it's been modified, and it'll be written to the disk when it's popped out.

## LOAD MATRIX
This was implemented in the pre-existing LOAD command that was implemented for tables.
### Error Handling
1. Syntactic - We ensure that if the LOAD query has 3 tokens, the second one is "MATRIX", and we then take the third token as the name of the matrix to be loaded.
2. Semantic - We first check if the matrix already exists in the catalogue or if the file doesn't exist on the disk.
### Implementation
Firstly, we created a Matrix object that would store information related to matrices that are loaded. We then added some functionality to the tableCatalogue
to support keeping track of loaded Matrices as well.

We then went onto implementing the blockify function for matrix. We get the dimensions fromm the first line, and then computed the largest MxM matrix that can fit inside the page, and find how many such submatrices can fit in a row.
Once that is done, we'd read M rows at a time, and then create Pages of them, while keeping track of their dimensions in the Matrix object. Some changes were made to the Page constructor to allow specifying the columncount as well.

## PRINT MATRIX 
This was implemented in the pre-existing PRINT command that was implemented for tables.
### Error Handling
1. Syntactic - We ensure that if the PRINT query has 3 tokens, the second one is "MATRIX", and we then take the third token as the name of the matrix to be printed.
2. Semantic - We first check if the matrix exists in the catalogue, else print an error.
### Implementation 
The idea of writing pages is similar to how we read pages in LOAD. We take M rows at a time by iterating through the blocks in those M rows once by one, and then write to output once we have it

We defined another function `writeRows` that simplifies writing multiple rows to an output stream.

## TRANSPOSE 
### Error Handling
1. Syntactic - We ensure that if the TRANSPOSE query has 3 tokens, the second one is "MATRIX", and we then take the third token as the name of the matrix to be transposed.
2. Semantic - We first check if the matrix exists in the catalogue, else print an error.

### Implementation
This is where using submatrices made the implementation, as well as the efficiency a lot better. We'd find the two corresponding submatrices that have to be transposed and then swapped. If it's a submatrix on the diagonal, it would only need to be transposed. 
To easily transpose the matrices, we implemented two functions, a function for submatrices on the diagonal and one for those not on it.
For the submatrices on the diagonal, it's as simple as just transposing the submatrix, and marking the page as dirty to not lose the changes.
For the submatrices not on the diagonal, we'd pass one page to the other, and simultaneously swap and transpose the values in them. By doing this, there's no need of creating any new pages, changing any dimensions or pageIndices, and leads to a cleaner implementation.

Using this, we just iterated through the blocks and the matrix and transposed them without using more than 2 blocks of main memory at a time.

If the symmetry of the matrix was checked before, and it is symmetric, we don't perform any transpose to avoid redundancy.

## EXPORT 
This was implemented in the pre-existing EXPORT command that was implemented for tables.
### Error Handling
1. Syntactic - We ensure that if the EXPORT query has 3 tokens, the second one is "MATRIX", and we then take the third token as the name of the matrix to be exported.
2. Semantic - We first check if the matrix exists in the catalogue, else print an error.
### Implementation
This implementation is very similar to how we print matrices, and the saving of it on disk is largely similar to how tables are made permanent.
We iterate through M rows at a time in this as well, write down each block in the row, and then write the rows to the output stream.
## RENAME 
This was implemented in the pre-existing RENAME command that was implemented for tables.
### Error Handling
1. Syntactic - We ensure that if the RENAME query has 4 tokens, the second one is "MATRIX", and we then take the third token as the name of the matrix to be renamed, and the fourth as the new name.
2. Semantic - We first check if the matrix exists in the catalogue, and the new name of the matrix doesn't already exist in it.
### Implementation
We update the map that keeps track of the matrices, and then call a function in matrix to rename. It renames the matrix, and all the files on disk as well as in memory, using functions that we implemented in the Buffer Manager.
## CHECKSYMMETRY 
### Error Handling
1. Syntactic - We ensure that if the CHECKSYMMETRY query has 2 tokens, and we take the second token as the matrix to check for symmetry. 
2. Semantic - We first check if the matrix exists in the catalogue. 
### Implementation
The implementation of this is quite straightforward, as we just iterated through the blocks in the matrix in a pairwise-fashion, and check if they are symmetric.
It returns false as soon as it's found to be asymmetric, else returns true.

If checked earlier, we store the symmetry result in the matrix to avoid redundantly checking for symmetry again.
## COMPUTE 
### Error Handling
1. Syntactic - We ensure that if the COMPUTE query has 2 tokens, and we take the second token as the matrix to compute.
2. Semantic - We first check if the matrix exists in the catalogue, else print an error.
### Implementation
We first create a new Matrix, which has the same details as the matrix it is being copied from, but no pages are copied. Then, we call a function
compute in matrix, that retrieves the page of the original matrix, creates a copy, performs the operation, and then writes it as its own, leaving the original matrix pages unchanged.

## Statistics
Based on the input size, these were the block read + writes for each command

|               | 10    | 100     | 1000        |
|---------------|-------|---------|-------------|
| LOAD          | 0 + 1 | 0 + 49  | 0 + 4489    |
| RENAME        | 0 + 0 | 0 + 0   | 0 + 0       |
| PRINT         | 1 + 0 | 4 + 0   | 4 + 0       |
| TRANSPOSE     | 0 + 0 | 49 + 47 | 4489 + 4487 |
| COMPUTE       | 0 + 1 | 48 + 50 | 4488 + 4490 |
| CHECKSYMMETRY | 0 + 0 | 1 + 1   | 1 + 1       |
| EXPORT        | 1 + 0 | 49 + 0  | 4489 + 0    |

## Assumptions
1. There will be no overflows, all values fit in integers.
2. The matrices will be of a proper format
3. The block size can fit at least one integer.
 
## Learnings
1. Understanding the importance of OOPS, and how it can lead to a lot cleaner and easier to understand implementations (Specifically when implementing transpose functions for Pages).
2. Using concepts learnt from various fields can have nice applications (The concept of dirty bits was from Operating Systems).
3. Importance of predetermining implementations and designs for a more seamless development.

## Contributions
Vidit - Implemented the Matrix, LOAD and CHECKSYMMETRY functionality, and the necessary changes in other files.
Dheeraja - Implemented TRANSPOSE, COMPUTE and PRINT, and the necessary changes in other files to carry out this implementation.
Pramod - Implemented RENAME, EXPORT, COMPUTE.

The page design and overall structure of the project phase was done by everyone together.

We collaborated using LiveShare, as we were in different locations and made communications more seamless using this.

---

# Phase 2 - External Sort Phase

## External Sort Algorithm
## SORT command
## ORDER_BY command
## JOIN command
## GROUP_BY command
### Error Handling
1. Syntactic - We ensure that the GROUP BY query has 13 tokens, and that it is of the form `<new_table> <- GROUP BY <grouping_attribute> FROM <table_name> HAVING <aggregate(attribute)> <bin_op> <attribute_value> RETURN <aggregate_func(attribute)>`.
We check if the tokens in the given query match the format of the GROUP BY command and parse the necessary tokens. We also check here that the `<attribute>` value token is an integer value.
2. Semantic - We first check if the `<table_name>` token corresponds to an already loaded table, and that it contains the attributes specified for the grouping_attribute, the having clause and the return clause are valid attributes of the table specified by `<table_name>`. We further ensure that `<new_table>` does not already exist in the table catalogue.
### Implementation
We first make a copy of the original table into a temporary table named `"Temp_GROUP_BY_" + <table_name>`, and sort it by the grouping attribute.
Once it is sorted, all the rows that contain a common value for the grouping attribute now occur contiguously. So we iterate over the rows and for each set of contiguous rows having the same value of the grouping attribute, we accumulate the values of the attributes in the `HAVING` and `RETURN` clauses as specified by their respective aggregate functions.
When all the rows corresponding to a specific grouping attribute value have been accumulated in this way, we check if the aggregate value of the attribute matches that specified by the `HAVING` clause. Only if the constraint set by the `HAVING` clause is satisfied, the aggregate value of the attribute in the `RETURN` clause is written to the final resultant table.

We use function pointers to implement the binary operator in the `HAVING` clause. We have the enum type `BinaryOperator` to represent binary operators.
For each binary operator `<bin_op>`, we have defined a function that takes in two integers x and y and returns true if and only if `a <bin_op> b` is true. ie, for `<bin_op>` = `>=`, we have a function `geq` that returns true iff `a >= b`. We then make an array of pointers to these functions such that the array can be indexed by the enum value of each binary operator to get the appropriate function.
This same array is used to fetch the appropriate comparator for the `JOIN` command as well.

Similarly, we also have an enum type `AggregateFunction` to represent the aggregate functions min, max, sum and avg. As we iterate over the rows of a group, we accumulate the values of the required attributes using an appropriate function. For min and max, we accumulate using the min and max functions themselves. For sum and avg, we accumulate the sum of the attributes. In order to get the average, we simply divide this sum by the number of entries in that group.
Like the function pointers for the binary operators, we have a similar array of function pointers to these aggregate functions that can be indexed using the enum values.

## Assumptions
## Learnings
## Contributions

---