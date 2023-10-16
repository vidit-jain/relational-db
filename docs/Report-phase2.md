# Project Report 
# Phase 2 - External Sort Phase

## External Sort Algorithm

The external sort algorithm used involves two phases: sorting and merging. Broadly, in the sorting phase, the blocks are brought into main memory and sorted before being written back. In the merging phase, in each stage, we merge sorted runs to obtain longer sorted runs. In the following discussion let `Nb` represent the size of the main-memory in blocks.


**Sorting phase:** 

In this phase, we iterate over the blocks of the table that we want to sort. We fetch the next `Nb - 1` blocks (or the remaining number of blocks to be fetched if it happens to be less than `Nb - 1`). This is because, we also write the sorted data back block-by-block meaning that we keep a block-sized buffer in memory to facilitate for this.

After fetching the blocks into our buffer, we sort them. To have a clean implementation for supporting sorting on multiple columns with different sorting strategies for each, we use a custom lambda comparator function that takes in the column indices and multipliers (1 indicating `ASC` order and `-1` otherwise) and returns the appropriate comparison result that is required by `std::sort` to sort the vector of rows.

As mentioned earlier, we write the sorted data back into the same blocks effectively performing an in-place sorting phase. We perform the write-backs sequentially one block at a time as opposed to writing individual rows back which would be less efficient due to the large number of file open, close and appends.

At the end of this stage, we obtain the initial file consisting of `ceil(B / (Nb - 1))` number of sorted runs (where `B` denotes the total block count).

**Merging phase:**

Represent the number of sorted runs to be processed in the current stage as `Nr` which is initially set to `Nr_0 = ceil(B / (Nb - 1))` as explained in the previous section. In a given merge phase stage, we merge the current `Nr` runs to form `ceil(Nr / (Nb - 1))` longer sorted runs, unless `Nr` is already 1 which implies that the file is already sorted.

We now describe the working of a particular merge phase stage. To begin, we fetch the first blocks each of the next `Nb - 1` runs (or less if there are lesser number of runs remaining). We add the entries to a priority queue with the appropriate comparator constructed using the input specification. We then continuously extract the top element from the priority queue and insert it into a write buffer which once filled performs a page write. When an element is extracted, if there are more rows following that element in its run, it is added to the priority queue.

Similar to the sorting phase, we perform batched writes. Once our write buffer (whose size corresponds to 1 block) fills up, we write it back to disk with the appropriate name. The stages go on till the number of runs reduces to 1 implying that the entire file is sorted.

For both the above phase, we list some implementation details in the following section.

## SORT command

### Error Handling
1. Syntactic - We ensure that the number of tokens is at least 6 (since the shorted command would have the syntax `SORT <table_name> BY <column_name> IN <ASC|DESC>`). We then make sure that the number of tokens other than `SORT, BY, IN, <table_name>` is divisible by 2, that is, there is equal number of column names and sorting strategies provided. We then ensure that the tokens at the appropriate positions are set to `BY` and `IN`. Then, for each of the tokens in the sorting strategy field, we ensure that its one of `ASC` or `DESC`.
2. Semantic - We simply ensure that the input `<table_name>` is loaded onto the database and that the specified column names to be sorted on are valid columns belonging to it.

### Implementation details

- Sorting phase:
  - The function for sort table takes an additional parameter - `originalTableName`. This is passed to the sorting phase of the algorithm, where we perform the reads from the supplied table name. This is implemented in this way in order to simplify the commands that follow. For the sort command, since we want the modification to affect the table, we set this parameter to equal the table name.
- Merging phase:
  - An immediate issue with the merging phase is that it cannot be performed in-place. We could do so in the sorting phase since we would only write-back after reading all the required blocks into memory. Here, however, when we have a block of data to be written back say, it need not correspond to the first block of the initial file and so, overwriting it would mean that we lose some data. To overcome this, we create a temporary table whose statistics (such as the rowCount, distinctValuesPerColumnCount etc.) are derived from the original table. Call this the `writeTable` and the original table as the `readTable`. In the first stage, we read from the `readTable` and write the resulting longer sorted runs to the `writeTable`. For the next stage, we simply swap the two table names. This avoids creating temporary table for each stage and instead, reuses the same blocks.
  - However, at the end of the merge phase, it might so happen that we perform the final complete sorted run write to the temporary file that we created depending on the parity of `ceil(log_{Nb - 1}(Nr_0))`. We want to copy over the content of this table onto our original table. Instead of performing block accesses, we simply rename the pages in buffer and disk to match that of the original table.
  - Finally, we erase the table from the catalogue and delete the corresponding files (if they were not renamed to the original table name)
  - Another implementation detail to be noted is obtaining the next row for each run in a particular merge phase stage after the top element from the priority queue is extracted. To facilitate this, we store the cursors to the runs along with the actual row in the priority queues (or rather, the corresponding index). We also keep a track of the number of rows that we have read from a particular run. This tells us whether a cursor should be queried for the next row.

## ORDER_BY command
### Error Handling
1. Syntactic - We ensure that the ORDER BY query has 8 tokens, and that it is of the form `<new_table> <- ORDER BY <attribute> ASC|DESC ON <table_name>`.
   We check if the tokens in the given query match the format of the GROUP BY command and parse the necessary tokens, we also make sure that the order is "ASC" or "DESC". 
2. Semantic - We check that `<new_table>` doesn't already exist, the table `<table_name>` is a table that exists, and `<attribute>` is a valid column in the table.

### Implementation
Once `SORT` was implemented, `ORDER_BY` was straightforward to implement. We added a parameter in the sort which allows a table feed off of pages of another table and sort them, which allows us to create a new table, and feed it the sorted pages of the original table, without affecting the original one.

## JOIN command
### Error Handling
1. Syntactic - We ensured the syntax was of the form `<new_relation_name> <- JOIN <tablename1>, <tablename2> ON <column1> <bin_op> <column2>`. This included checking if the number of tokens could be valid for some number of attributes. 
   We check if the tokens in the given query match the format of the GROUP BY command and parse the necessary tokens, and ensure the binary operator specified is valid.
2. Semantic - We check that `<new_relation_name>` doesn't already exist as a table, `<tablename1>`, `<tablename2>` are valid tables, and `<column1>`, `<column2>` are valid columns in their respective tables.
### Implementation

There were 3 different implementations of JOIN, based on the binary operator used
#### `==` operator
To process a join on the `==` binary operator, it required to first make a temporary copy of each of the tables we are joining, and sort each one of them on the concerned attribute using the external sort we implemented earlier

Once that was completed, we ran a two pointer approach on the tables using Cursor objects on each of them. 
1. Check which table is on a row with a smaller value for the attribute, and increment its cursor object. Repeat until they are equal. 
2. Create a copy of the Cursor object of table2, increment it and insert a new row in the join table until the column values don't match. 
3. Similarly, match all the rows in table2 equal to the current row in table1. 
4. Once complete, we increment both cursors.

This approach takes advantage of the sorted table as for a certain row, we don't need to go through the entire table to find all the matches, and our search space is a lot more limited.

#### `!=` operator
In this operator join too, we sort the tables in ascending order, and use a two pointer approach.
In this two pointer approach however, we keep track of the smallest index row in table2 which is greater than the current row in table1. This allows us to find out what all rows the current row in table1 can match with. There are a few parts in this algorithm
1. Increment Cursor for table1, and increment Cursor for table2 until `row1[col1] < row2[col2]` becomes true.
2. Make a new cursor for table2 which starts from the beginning, and increment and insert rows into join table until `row1[col1] == row2[col2]`
3. Create another copy, which is the copy of the current cursor of table2, and run it till the end, inserting the concatenated row in the join table for each row.
4. Go to step 1

This also takes advantage of the sorted table as we skip going over the rows where the values match, which provides a significant improvement in case there are many rows in each table which have equal values on the attribute.
#### `>`, `>=`, `<`, `<=` operator
First, we note that it's only necessary to sort one of the tables, there is no benefit of sorting both of them in the case of these binary operators.

Hence, we sort the second table in ascending order if `binary_operator = >/>=` and descending order if otherwise.

The algorithm is simplified to this

1. Increment the cursor for table1, and increment it.
2. Create a new cursor for table2 from the start, and keep matching until the binary operator specified doesn't hold true.
3. Go to step 1

To avoid implementing multiple versions for each of the conditions, I made use of function pointers to define the binary operator and decided the order to sort based on it as well, making the code have less redundancy

As the second table is sorted, we'll only increment until we get rows that we must insert into the table, hence we don't make unnecessary comparisons between rows.

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
1. We assume that there are no columns with the same names in the two tables to be joined. Hence, we safely assume that a table won't be joined with itself.
2. We assume there are no comma values in any of the integer values, as it would break the legacy codebase parsing methodology.
3. We take the average as the floor after division when implementing `GROUP_BY`
4. We assume there is enough disk space for another copy of the table while sorting to successfully carry the algorithm out.

## Learnings
1. We learnt more about C++ features such as `std::optional` which provided to be useful.
2. We saw the benefit of using function pointers to modularize and shorten/simplify our implementations
3. Understand how algorithmic complexities often do not suffice when understanding the implementation of an algorithm, we must understand how accesses are done to memory as it's one of the largest bottlenecks in memory-heavy algorithms like sort.
## Contributions

Everyone contributed equally to the report and in tweaking the existing files as required.

- Pramod - Implemented the external sort algorithm, sort command.
- Vidit - Implemented ORDER_BY, JOIN commands
- Dheeraja - Implemented GROUP_BY command

---

