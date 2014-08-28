
Examples for AsciiIngest
=====================

These examples show how to ingest ASCII-data into a database, using
`AsciiIngest` and the `DBIngestor`-library from http://github.com/adrpar.

**Prerequisites**:

* Linux-like system with MySQL-server installed
* A valid username and password for MySQL with the privilege to create 
  databases and tables and insert data. We will use `mypwd` as root-password
  in the command lines below. Please adjust this to your own password.
* DBIngestor and AsciiIngest installed

**MySQL preparation for examples**:

Create a database `` `example` `` for testing:

	$ mysql --user=root --password=mypwd
	mysql> create database example;


BDM data
-------
The example file `BDM-example.dat` contains an extract from a BDM halo 
catalogue, as produced by Anatoly Klypin. A typical file name is 
`CatshortW.0087.00.DAT`.
Such a file contains a header with some metadata, followed by one line for 
each halo.
The columns contain the values for certain properties of the halo and shall now 
be inserted into a database table. The mapping between data columns in the file
and database table fields is done via the structure or format file `BDM.frm`. 
It uses some additional features for converting data before ingestion. These 
will be explained below.

1. create a `BDMW` table in MySQL using `createtable-bdmw-mysql.sql`:

		$ mysql --user=root --password=mypwd < createtable-bdmw-mysql.sql

2. ingest the example data:

		$ ../AsciiIngest/build/AsciiIngest.x  -s mysql -D example -T BDMW -U root -P mypwd -O 3306 \
		-H 127.0.0.1 --greedyDelim=1 --isDryRun=0 -R 1 BDM.frm BDM-example.dat



Density data (grid)
-----------------
The example file `Dens-example.dat` contains an extract from a density file
with grid cell indizes ix, iy, iz and a density value. The original file
was created by Jaime Forero-Romero. The accompanying structure file `Dens.frm`
is explained in detail below. We apply here converters to generate a unique id
for each grid cell. (This could also be achieved with an auto-increment 
column on the database side).

1. create a `Dens` table in MySQL using `createtable-dens-mysql.sql`:

		$ mysql --user=root --password=mypwd < createtable-dens-mysql.sql

2. ingest the example data:

		../AsciiIngest/build/AsciiIngest.x  -s mysql -D example -T Dens -U root -P mypwd -O 3306 \
		-H 127.0.0.1 --greedyDelim=1 --isDryRun=0 -R 1 Dens.frm Dens-example.dat




Structure file details
-------------------
Please look first into the userguide for `AsciiIngest`, part IV, to get to 
know the basics of these files. Using converters and asserters looks quite
complicated at first, but they are prowerful tools to minimize the amount 
of adjustments on the database side after ingestion.


### BDM.frm ###
* 1\. row: number of rows of the structure file. Adjust this, 
if you add/delete lines from the structure file.

* 2\. row: number of headerlines to be skipped. Note 
that only the `CatshortCatshortW.*.00.DAT` files contain a header; subsequent files
with ending `.01.DAT` etc. for the same snapshot don't include a header. For them, 
you need to set the number of headerlines to 0, otherwise the first halos of each file
will be missing in the database.

* `D`-columns: contains the datatypes from the file and how they map to database fields. 
  Some of them are skipped at first using the keyword `SKIP_THIS_COLUMN` and 
  then used with the C-columns below. This allows to do some additional calculations 
  before ingesting the data. 
  
  The delimiter for the BDM-files is one or more spaces, so we put `' '` 
  for each data column. Only after the last column, we need `'\n'`.

* Constant columns (starting with `C`): Now it becomes tricky! AsciiIngest allows to 
  do some operations on the data before ingestion. We use this to achieve the following:
  
  - insert `snapnum`: The snapshot number is not included in the BDM-file, so it must 
     be inserted additionally, if we want to include it in the database table:
     
			C25 INT2  0 0 '0'  snapnum       CONV_ADD(87)

     This adds 87 as snapshot number and will insert it into field `snapnum` of the database.
     We can reuse the snapnum for other columns by referencing it as `C25`.
     
  - construct bdmId from snapshot number:
  
			C26 INT8  0 0 '0'  bdmId         CONV_ADD(C25), CONV_MULTIPLY(1.e11), CONV_ADD(D12)

     This line adds the snapshot number to 0, then multiplies with 10^11 and adds the number 
     of the halo in the catalogue (data column `D12`), i.e.:
     
			bdmId =  snapnum*1.e11 + NInCat 

     Adjust the power of ten, if you have less/more halos. The number of halos from all
     snapshots together must be smaller than this number. 

  - construct `hostFlag`: The `hostFlag` is 0 for distinct halos and corresponds to `NInCat` 
    otherwise. However, we want to refer to the `bdmId` of the corresponding host halo and 
    set the `hostFlag` to -1 for distinct, so that it cannot be misinterpreted. This is 
    achieved in 4 steps:
    
			C27 INT8  0 0 '0'  SKIP_THIS_COL CONV_ADD(C25), CONV_MULTIPLY(1.e11), CONV_ADD(D15)
		
 	 This creates a `bdmId` from `D15`, same recipe as above.
  	
			C28 INT8  0 0 '0'  SKIP_THIS_COL CONV_ADD(D15), CONV_ADD(-1)
		
	 This adds -1 to `D15`, just in case it is a distinct halo. 
		
			C29 INT8  0 0 '0'  SKIP_THIS_COL CONV_ISGE(D15; 1)
		
	 This evaluates, if `D15` is greater or equal than 1. So, if true, there is a 
	 host halo; if false, then the halo is distinct.
	
			C30 INT8  0 0 '0'  hostFlag      CONV_IFTHENELSE(C29; C27; C28)
		
	 This uses the result of the previous evaluation and inserts either a proper 
	 bdmId or -1 as hostFlag into the database.

  - insert a place-holder for `phkey`: The Peano-Hilbert keys will be inserted
    later using the database. But we still need to include this column. Here we 
    just insert a '0'-value everywhere.
    
			C31 INT4  0 0 '0'  phkey

  - get integer positions for grid cell indexes `ix` etc.:
  
			C32 REAL4 0 0 '0.0'    SKIP_THIS_COL   CONV_ADD(D1), CONV_DIVIDE(1000.0), CONV_MULTIPLY(1024.0)
			C33 INT4  0 0 '0.0'    ix   CONV_ADD(C32)

	 This takes e.g. the column of the `x` coordinate, `D1`, divides it by the 
	 box-side length (1000.0, same unit as `x`) and then multiplies it by the 
	 desired number of grid cells (should be same as will be used for phkey-creation 
	 later on the database). These calculations need to done with real-numbers, 
	 thus we map it first to a `REAL4` value and then convert to integer
	 in the next line.

  - convert `np` from real to bigint: The number of particles is given as a real-number 
     in the data file, but we want to store it as bigint in the database. We cannot just 
     map a real-data column to an int-field, because that would truncate numbers expressed
     as e.g. `1.35e8` in the file. Thus, we first read the number into a REAL4-datatype
     and later convert it to bigint:
    
			D14 REAL4 0 0 ' ' SKIP_THIS_COL
			[...]		
			C38 INT8  0 0 '0.0'    np   CONV_ADD(D14)

	
### Dens.frm ###
The structure file for density data looks much simpler, since we don't need
that many conversions.

* 1\. and 2\. rows: number of columns in structure file and number of headerlines to be 
  skipped. 
* Comments are marked with `#`.
* Data columns: Map the first three columns of the data file to the database fields `ix`, `iy`, `iz`. 
  The density is mapped to a 4-byte floating point number (REAL4) and will be written to the 
  database field `dens`.
  
  The delimiter is one or more spaces, so we put `' '` there and don't forget 
  to use `'\n''` for the final data column. 
  
* Constant/Stored columns: 
    - Add an empty `phkey` column: just fill with 0-values.

			C5      INT4   0   0   '0'      phkey

	- Add a stored value that can be increased by 1 for each new ingested line
	  and will be used as `webId`:
	
			S6      INT8   0   0   '-1'     SKIP_THIS_COL   CONV_ADD(1)
			C7      INT8   0   0   '0'      webId           CONV_ADD(S6)
			
  	  We need to start with -1 here to get `webid=0` for the first ingested row.







