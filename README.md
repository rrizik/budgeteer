# Budgeteer

A budgeting program.  

## Current Status
As of August 2024, this is only a Win32 program. There are plans to create Mac and Linux releases, but these are not scheduled anytime soon.

## Structure
The program consists of the following files and directories:

- **budgeteer.exe**
- **config.conf**
- **saves\budget.b**
- **shader\*.hlsl**

## Configuration (`config.conf`)

The `config.conf` file is used to specify which columns you are interested in parsing from CSV files. We support parsing three columns: `date`, `amount`, and `description`.

The format will look like this:
```plaintext
#date
date,Date,DATE,Post Date
#amount
amount amount,Debit,AMOUNT
#description
Description, description dddd, desc
```

This allows the user to specify multiple names in case they are loading data from different CSV files that define different headers. This also makes it easier for me, as the programmer, to avoid modifying or touching a CSV file before it's loaded. This structure might change in the future.

## Note
As this is still a new program, many aspects are volatile and subject to change. For example, maybe of the files will be deleted/removed including some files in the src that are not being used and the shader directory will be removed. Other things like how the config file is being used and potentially more might change in the near future.

