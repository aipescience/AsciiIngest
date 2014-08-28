38
17
D1 REAL4 0 0 ' ' x
D2 REAL4 0 0 ' ' y
D3 REAL4 0 0 ' ' z
D4 REAL4 0 0 ' ' vx
D5 REAL4 0 0 ' ' vy
D6 REAL4 0 0 ' ' vz
D7 REAL4 0 0 ' ' Mvir
D8 REAL4 0 0 ' ' Mtot
D9 REAL4 0 0 ' ' Rvir
D10 REAL4 0 0 ' ' Vrms
D11 REAL4 0 0 ' ' Vcir
D12 INT8 0 0 ' ' NInCat
D13 REAL4 0 0 ' ' conc
D14 REAL4 0 0 ' ' SKIP_THIS_COL
D15 REAL4 0 0 ' ' SKIP_THIS_COL
D16 REAL4 0 0 ' ' Xoff
D17 REAL4 0 0 ' ' virialRatio
D18 REAL4 0 0 ' ' spin
D19 REAL4 0 0 ' ' Rrms
D20 REAL4 0 0 ' ' axisratio_2_1
D21 REAL4 0 0 ' ' axisratio_3_1
D22 REAL4 0 0 ' ' axis1_x
D23 REAL4 0 0 ' ' axis1_y
D24 REAL4 0 0 '\n' axis1_z
C25 INT2  0 0 '0'  snapnum	 CONV_ADD(87)
C26 INT8  0 0 '0'  bdmId  	 CONV_ADD(C25), CONV_MULTIPLY(1.e11), CONV_ADD(D12)
C27 INT8  0 0 '0'  SKIP_THIS_COL CONV_ADD(C25), CONV_MULTIPLY(1.e11), CONV_ADD(D15)
C28 INT8  0 0 '0'  SKIP_THIS_COL CONV_ADD(D15), CONV_ADD(-1)
C29 INT8  0 0 '0'  SKIP_THIS_COL CONV_ISGE(D15; 1)
C30 INT8  0 0 '0'  hostFlag  	 CONV_IFTHENELSE(C29; C27; C28)
C31 INT4  0 0 '0'  phkey
C32 REAL4 0 0 '0.0'    SKIP_THIS_COL   CONV_ADD(D1), CONV_DIVIDE(1000.0), CONV_MULTIPLY(1024.0)
C33 INT4  0 0 '0.0'    ix   CONV_ADD(C32)
C34 REAL4 0 0 '0.0'    SKIP_THIS_COL   CONV_ADD(D2), CONV_DIVIDE(1000.0), CONV_MULTIPLY(1024.0)
C35 INT4  0 0 '0.0'    iy   CONV_ADD(C34)
C36 REAL4 0 0 '0.0'    SKIP_THIS_COL   CONV_ADD(D3), CONV_DIVIDE(1000.0), CONV_MULTIPLY(1024.0)
C37 INT4  0 0 '0.0'    iz   CONV_ADD(C36)
C38 INT8  0 0 '0.0'    np   CONV_ADD(D14)
#
# additional computations:
# - generate ix,iy,iz on the fly (do it for each in two steps; first float calculations, then round down to integer)
# - generate bdmId from snapnum and halo number in catalogue: snapnum*1e11 + NInCat;
# - generate proper host-bdmId for hostFlag, same recipe as for bdmID, EXCEPT: if haloFlag is 0 in file, we need to insert -1 into DB
# - 0-column for phkey
# - properly convert np from real to int

