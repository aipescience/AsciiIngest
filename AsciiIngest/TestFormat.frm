#A test format file
2
#numCol	DType	PrefixLen	DataLen	Separator	SchemaColName	AssertList	ConvList
1	CHAR	0		10	','		haloName	ISNOTNULL	CAPITALIZE,TRIM
2	INT4	1		10	'\n'		numParticles	ISNOTNULL, ISNOTNEGATIVE	
