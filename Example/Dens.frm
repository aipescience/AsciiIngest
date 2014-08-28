7
1
#numCol	DType	PrefixLen	DataLen	Separator	SchemaColName	AssertAndConvList
D1	INT4	0		0	' '		ix		
D2	INT4	0		0	' '		iy		
D3	INT4	0		0	' '		iz		
D4	REAL4	0		0	'\n'		dens		
C5	INT4   0   0   '0'	phkey		
S6	INT8   0   0   '-1'	SKIP_THIS_COL	CONV_ADD(1)
C7	INT8   0   0   '0'      webId		CONV_ADD(S6)
# additional computations:
# - generate webId, start with -1 +1 = 0 for first line

