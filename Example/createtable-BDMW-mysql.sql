use example;
create table BDMW (bdmId bigint not null, snapnum smallint not null, NInCat bigint not null, hostFlag bigint not null,
x float not null, y float not null, z float not null, vx float not null, vy float not null, vz float not null, np bigint not null,
Mvir float not null, Mtot float not null, Rvir float not null, Vrms float not null, Vcir float not null, conc float not null,
Xoff float not null, virialRatio float not null, spin float not null, Rrms float not null, axisratio_2_1 float not null,
axisratio_3_1 float not null, axis1_x float not null, axis1_y float not null, axis1_z float not null, ix int not null,
iy int not null, iz int not null, phkey int not null) Engine=MyISAM;

