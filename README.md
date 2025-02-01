# Budgeteer

A budgeting program.  

## Current Status
This is currently only a Win32 program. There are plans to create Mac and Linux releases, but these are not scheduled anytime soon.

## Structure
When you download the Zip file, you should be able to simply unzip the directory and run the program.

## Serialized Files:
There are currently three files that I write out to (although this will likely be reduced to either two or just one file).
- config.conf: This files contains any state information that I want to persist between instances. This also stores all the CSV profiles you create.
- saves/budget.b: This file contains the budget plan you create on the left side of the program.
- saves/<year>.b: These files contain all the transaction information per year.

## Loading a CSV file

To load a CSV file, click "Load CSV" and create a CSV profile by providing the date, amount, and description header names you want to look for. Select a file from disc and select a file format. If all boxes are green, then the file should load without issue and parse the information into the correct year and month.

## Note
As this is still a new program, everything is subject to be changed/modified/deleted as necessary.
- Date Formats: This is something I'm dealing with as I go. Right now I have a set number of date formats I support and that is it. I will be putting in more work later to support many other formats as well as for users to specify they own formats.


