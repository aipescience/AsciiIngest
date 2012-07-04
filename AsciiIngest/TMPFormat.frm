numCols
numHeaderRows
#numCol	DType	PrefixLen	DataLen	Separator	SchemaColName	AssertAndConvList
D1	CHAR	0		10	','		haloName	ASRT_ISNOTNULL, CONV_CAPITALIZE,CONV_TRIM
D2	INT4	1		40	','		numParticles	ASRT_ISNOTNULL, ASRT_ISNOTNEGATIVE, CONV_MULTIPLY(D2)
D3	INT4	1		40	','		SKIP_THIS_COL	ASRT_ISNOTNULL, ASRT_ISNOTNEGATIVE, CONV_MULTIPLY(D2)
#D2	INT4	1		40	','		numParticles	ASRT_ISNOTNULL, ASRT_ISNOTNEGATIVE, CONV_MULTIPLY(D2; 12.0)
#These are examples of header items
#numCol	DType	LineNum:offset	DataLen Separator	SchemaColName	AssertList	
H4  INT4   0   0   'Redshift:' redshift    ASRT_ISNOTNULL,CONV_FLOOR
H5  REAL4   10:5  1   ''  expFact ASRT_ISNOTNULL,CONV_FLOOR,CONV_MULTIPLY(3.1415)
#These are examples of constant items
C6  REAL4   0   0   '3.14156'   pi  ASRT_ISNOTNULL
C7  INT4   0   0   '2' someMagicNumber
